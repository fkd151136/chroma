// $Id: eoprec_clover_orbifold_linop_w.cc,v 1.1 2009-02-09 22:16:59 edwards Exp $
/*! \file
 *  \brief Even-odd preconditioned Clover fermion linear operator with orbifold
 *
 * 3D-Orbifold construction follows arXiv:0811.2127
 */

#include "actions/ferm/linop/eoprec_clover_orbifold_linop_w.h"

using namespace QDP::Hints;

namespace Chroma 
{ 

  //! Creation routine with Anisotropy
  /*!
   * \param u_ 	    gauge field     	       (Read)
   * \param param_  fermion kappa   	       (Read)
   */
  void EvenOddPrecCloverOrbifoldLinOp::create(Handle< FermState<T,P,Q> > fs, 
					      const CloverFermActParams& param_)
  {
    START_CODE();

    // QDPIO::cout << __PRETTY_FUNCTION__ << ": enter" << endl;

    param = param_;

    clov.create(fs, param);
 
    invclov.create(fs,param,clov);  // make a copy
    invclov.choles(0);  // invert the cb=0 part

    D.create(fs, param.anisoParam);

    clov_deriv_time = 0;
    clov_apply_time = 0;

    if ( param.twisted_m_usedP )
    {
      QDPIO::cerr << "EvenOddPrecCloverOrbifoldLinOp:: no twisted-mass allowed";
      QDP_abort(1);
    }

    // Orbifold Sanity checks
    if (QDP::Layout::subgridLattSize()[0] != QDP::Layout::lattSize()[0])
    {
      QDPIO::cerr << "Requires x-dir on-node";
      QDP_abort(1);
    }

    if (QDP::Layout::subgridLattSize()[1] != QDP::Layout::lattSize()[1])
    {
      QDPIO::cerr << "Requires y-dir on-node";
      QDP_abort(1);
    }

    if (QDP::Layout::subgridLattSize()[2] != QDP::Layout::lattSize()[2])
    {
      QDPIO::cerr << "Requires z-dir on-node";
      QDP_abort(1);
    }

    // QDPIO::cout << __PRETTY_FUNCTION__ << ": exit" << endl;
    END_CODE();
  }

  //! Apply the the odd-odd block onto a source vector
  void 
  EvenOddPrecCloverOrbifoldLinOp::oddOddLinOp(LatticeFermion& chi, const LatticeFermion& psi, 
					      enum PlusMinus isign) const
  {
    START_CODE();

    swatch.reset(); swatch.start();
    clov.apply(chi, psi, isign, 1);
    swatch.stop();
    clov_apply_time += swatch.getTimeInSeconds();

    END_CODE();
  }


  //! Apply the the even-even block onto a source vector
  void 
  EvenOddPrecCloverOrbifoldLinOp::evenEvenLinOp(LatticeFermion& chi, const LatticeFermion& psi, 
						enum PlusMinus isign) const
  {
    START_CODE();

    // Nuke for testing
    swatch.reset(); swatch.start();
    clov.apply(chi, psi, isign, 0);
    swatch.stop();
    clov_apply_time += swatch.getTimeInSeconds();
    
    END_CODE();
  }

  //! Apply the inverse of the even-even block onto a source vector
  void 
  EvenOddPrecCloverOrbifoldLinOp::evenEvenInvLinOp(LatticeFermion& chi, const LatticeFermion& psi, 
						   enum PlusMinus isign) const
  {
    START_CODE();

    swatch.reset(); swatch.start();
    invclov.apply(chi, psi, isign, 0);
    swatch.stop();
    clov_apply_time += swatch.getTimeInSeconds();
    
    END_CODE();
  }
  

  //! Apply even-odd linop component
  /*!
   * The operator acts on the entire even sublattice
   *
   * \param chi 	  Pseudofermion field     	       (Write)
   * \param psi 	  Pseudofermion field     	       (Read)
   * \param isign   Flag ( PLUS | MINUS )   	       (Read)
   */
  void 
  EvenOddPrecCloverOrbifoldLinOp::evenOddLinOp(LatticeFermion& chi, 
					       const LatticeFermion& psi, 
					       enum PlusMinus isign) const
  {
    START_CODE();

    Real mhalf = -0.5;

    D.apply(chi, psi, isign, 0);
    chi[rb[0]] *= mhalf;
  
    END_CODE();
  }

  //! Apply odd-even linop component
  /*!
   * The operator acts on the entire odd sublattice
   *
   * \param chi 	  Pseudofermion field     	       (Write)
   * \param psi 	  Pseudofermion field     	       (Read)
   * \param isign   Flag ( PLUS | MINUS )   	       (Read)
   */
  void 
  EvenOddPrecCloverOrbifoldLinOp::oddEvenLinOp(LatticeFermion& chi, 
					       const LatticeFermion& psi, 
					       enum PlusMinus isign) const
  {
    START_CODE();

    Real mhalf = -0.5;

    D.apply(chi, psi, isign, 1);
    chi[rb[1]] *= mhalf;
  
    END_CODE();
  }

  // Orbifold term
  void EvenOddPrecCloverOrbifoldLinOp::orbifold(LatticeFermion& chi, 
						const LatticeFermion& psi, 
						int z, int cb) const
  {
    LatticeFermion tmp;
    GammaConst<4,8> g4; // gamma_4
    GammaConst<4,4> g3; // gamma_3

    for(int tt=0; tt < QDP::Layout::subgridLattSize()[3]; ++tt)
    {
      int t = tt + QDP::Layout::nodeCoord()[3]*QDP::Layout::subgridLattSize()[3];

      for(int xx=0; xx < QDP::Layout::lattSize()[0] >> 1; ++xx)
      {
	for(int y=0; y < QDP::Layout::lattSize()[1]; ++y)
	{
	  multi1d<int> coord(4);
	  int x = 2*xx + ((cb+y+z+t) & 1);
	  
	  // output sites on z
	  coord[0] = x;
	  coord[1] = y;
	  coord[2] = z;
	  coord[3] = t;
	  int site = QDP::Layout::linearSiteIndex(coord);

	  coord[0] = QDP::Layout::lattSize()[0] - 1 - x;
	  coord[1] = QDP::Layout::lattSize()[1] - 1 - y;
	  int site_n = QDP::Layout::linearSiteIndex(coord);

	  tmp.elem(site) = g4 * psi.elem(site_n);
	  chi.elem(site) += g3 * tmp.elem(site) + tmp.elem(site);
	}
      }
    }
  }


  // Override inherited one with a few more funkies
  void EvenOddPrecCloverOrbifoldLinOp::operator()(LatticeFermion & chi, 
						  const LatticeFermion& psi, 
						  enum PlusMinus isign) const
  {
    START_CODE();

    LatticeFermion tmp1; moveToFastMemoryHint(tmp1);
    LatticeFermion tmp2; moveToFastMemoryHint(tmp2);
    Real mquarter = -0.25;

    //  chi_o  =  A_oo * psi_o  +   D_oe   A^(-1)_ee  D_eo  psi_o

    //  tmp1_e = D_eo  psi_o
    D.apply(tmp1, psi, isign, 0);

    // tmp1_e += (Orbifold term)*psi_o
    orbifold(tmp1, psi, 0, 0); // z=0, cb=0
    orbifold(tmp1, psi, QDP::Layout::lattSize()[2]-1, 0); // z=L-1, cb=0

    // tmp2_e = A_ee^{-1} * tmp1_e
    swatch.reset(); swatch.start();
    invclov.apply(tmp2, tmp1, isign, 0);
    swatch.stop();
    clov_apply_time += swatch.getTimeInSeconds();

    // tmp1_o = D_oe * tmp2_e
    D.apply(tmp1, tmp2, isign, 1);

    // tmp1_o += (Orbifold term)*tmp2_e
    orbifold(tmp1, tmp2, 0, 1); // z=0, cb=1
    orbifold(tmp1, tmp2, QDP::Layout::lattSize()[2]-1, 1); // z=L-1, cb=1

    //  chi_o  =  A_oo  psi_o  -  tmp1_o
    swatch.reset(); swatch.start();
    clov.apply(chi, psi, isign, 1);
    swatch.stop();
    clov_apply_time += swatch.getTimeInSeconds();

    chi[rb[1]] += mquarter*tmp1;


    END_CODE();
  }


  //! Apply the even-even block onto a source vector
  void 
  EvenOddPrecCloverOrbifoldLinOp::derivEvenEvenLinOp(multi1d<LatticeColorMatrix>& ds_u, 
						     const LatticeFermion& chi, const LatticeFermion& psi, 
						     enum PlusMinus isign) const
  {
    START_CODE();
    
    swatch.reset(); swatch.start();
    clov.deriv(ds_u, chi, psi, isign, 0);
    swatch.stop();
    clov_deriv_time  += swatch.getTimeInSeconds();

    END_CODE();
  }

  //! Apply the even-even block onto a source vector
  void 
  EvenOddPrecCloverOrbifoldLinOp::derivLogDetEvenEvenLinOp(multi1d<LatticeColorMatrix>& ds_u,
							   enum PlusMinus isign) const
  {
    START_CODE();

    // Testing Odd Odd Term - get nothing from even even term
    invclov.derivTrLn(ds_u, isign, 0);
    
    END_CODE();
  }

  //! Apply the the even-odd block onto a source vector
  void 
  EvenOddPrecCloverOrbifoldLinOp::derivEvenOddLinOp(multi1d<LatticeColorMatrix>& ds_u, 
						    const LatticeFermion& chi, const LatticeFermion& psi, 
						    enum PlusMinus isign) const
  {
    START_CODE();
    ds_u.resize(Nd);
    D.deriv(ds_u, chi, psi, isign, 0);
    for(int mu=0; mu < Nd; mu++) { 
      ds_u[mu]  *= Real(-0.5);
    }
    END_CODE();
  }
 
  //! Apply the the odd-even block onto a source vector
  void 
  EvenOddPrecCloverOrbifoldLinOp::derivOddEvenLinOp(multi1d<LatticeColorMatrix>& ds_u, 
						    const LatticeFermion& chi, const LatticeFermion& psi, 
						    enum PlusMinus isign) const
  {
    START_CODE();
    ds_u.resize(Nd);

    D.deriv(ds_u, chi, psi, isign, 1);
    for(int mu=0; mu < Nd; mu++) { 
      ds_u[mu]  *= Real(-0.5);
    }
    END_CODE();
  }

  // Inherit this
  //! Apply the the odd-odd block onto a source vector
  void 
  EvenOddPrecCloverOrbifoldLinOp::derivOddOddLinOp(multi1d<LatticeColorMatrix>& ds_u, 
						   const LatticeFermion& chi, const LatticeFermion& psi, 
						   enum PlusMinus isign) const
  {   
    START_CODE();

    swatch.reset(); swatch.start();
    clov.deriv(ds_u, chi, psi, isign, 1);
    swatch.stop();
    clov_deriv_time += swatch.getTimeInSeconds();
    
    END_CODE();
  }


  //! Return flops performed by the operator()
  unsigned long EvenOddPrecCloverOrbifoldLinOp::nFlops() const
  {
    unsigned long cbsite_flops = 2*D.nFlops()+2*clov.nFlops()+4*Nc*Ns;
    if(  param.twisted_m_usedP ) { 
      cbsite_flops += 4*Nc*Ns; // a + mu*b : a = chi, b = g_5 I psi
    }
    return cbsite_flops*(Layout::sitesOnNode()/2);
  }

  //! Get the log det of the even even part
  // BUt for now, return zero for testing.
  Double EvenOddPrecCloverOrbifoldLinOp::logDetEvenEvenLinOp(void) const  {
    return invclov.cholesDet(0);
  }
} // End Namespace Chroma
