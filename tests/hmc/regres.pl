#
#  $Id: regres.pl,v 1.1 2006-02-13 04:50:05 edwards Exp $
#
#  This is the portion of a script this is included recursively
#

#
# Each test has a name, input file name, output file name,
# and the good output that is tested against.
#
@regres_list = 
    (
     {
	 exec_path   => "$top_builddir/mainprogs/main" , 
	 execute     => "hmc" , 
	 input       => "$test_dir/hmc/hmc.prec_wilson.ini.xml" , 
	 output      => "hmc.prec_wilson.candidate.xml",
	 metric      => "$test_dir/hmc/hmc.prec_wilson.metric.xml" ,
	 controlfile => "$test_dir/hmc/hmc.prec_wilson.out.xml" ,
     }
     );
