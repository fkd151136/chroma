// $Id: inline_eig_aggregate.cc,v 1.4 2005-04-06 04:34:53 edwards Exp $
/*! \file
 *  \brief Inline eig measurement aggregator
 */

#include "meas/inline/eig/inline_eig_aggregate.h"
#include "meas/inline/eig/inline_eigbnds.h"

// Grab all fermacts to make sure they are registered
#include "actions/ferm/fermacts/fermacts_aggregate_w.h"

namespace Chroma
{

  //! Name and registration
  namespace InlineEigAggregateEnv
  {
    bool registerAll() 
    {
      bool success = true; 

      // Grab the fermacts
      success &= WilsonTypeFermActsEnv::registered;

      // Eig stuff
      success &= InlineEigBndsMdagMEnv::registered;
      return success;
    }

    const bool registered = registerAll();
  }

}
