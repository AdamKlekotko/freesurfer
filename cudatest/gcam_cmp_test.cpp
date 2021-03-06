#include <cstdlib>
#include <cstdio>

#include <iostream>
#include <iomanip>
#include <string>
using namespace std;

#include <boost/program_options.hpp>
namespace bpo = boost::program_options;

#include "gcamorph.h"
#include "gcamorphtestutils.hpp"

#include "chronometer.hpp"


#ifdef FS_CUDA
#include "devicemanagement.h"
#endif



// ==========================================================

const string inFileDefault = "gcamCMPinput";
#ifdef FS_CUDA
const string outFileDefault = "gcamCMPoutputGPU";
#else
const string outFileDefault = "gcamCMPoutputCPU";
#endif

string inFilename;
string outFilename;


const char* Progname = "gcam_cmp_test";




// ==========================================================

void ReadCommandLine( int ac, char* av[] ) {

  try {
    bpo::options_description desc("Allowed options");
    desc.add_options()
      ("help", "Produce help message" )
      ("input", bpo::value<string>(&inFilename)->default_value(inFileDefault), "Input filename (.nc will be appended)" )
      ("output", bpo::value<string>(&outFilename)->default_value(outFileDefault), "Output filename (.nc will be appended)" )
      ;

    
    bpo::variables_map vm;
    bpo::store( bpo::parse_command_line( ac, av, desc ), vm );
    bpo::notify( vm );
    
    if( vm.count( "help" ) ) {
      cout << desc << endl;
      exit( EXIT_SUCCESS );
    }
  }
  catch( exception& e ) {
    cerr << "Error: " << e.what() << endl;
    exit( EXIT_FAILURE );
  }
  catch( ... ) {
    cerr << "Unknown exception" << endl;
    exit( EXIT_FAILURE );
  }
}



// ==========================================================

int main( int argc, char *argv[] ) {

  SciGPU::Utilities::Chronometer tTotal;
  GCAMorphUtils myUtils;

  cout << "GCAM Metric Properties Tester" << endl;
  cout << "=============================" << endl << endl;

  

#ifdef FS_CUDA
#ifndef GCAMORPH_ON_GPU
  cerr << "GCAMORPH_ON_GPU is not defined." << endl;
  cerr << "Test meaningless" << endl;
  exit( EXIT_FAILURE );
#endif
#endif

#ifdef FS_CUDA
  AcquireCUDADevice();
#else
  cout << "CPU Version" << endl;
#endif
  cout << endl;

  ReadCommandLine( argc, argv );

  // ============================================

  // Read the input file
  GCAM* gcam = NULL;

  myUtils.Read( &gcam, inFilename );

  // Stop subsequent calls complaining
  gcam->ninputs = 1;

  // ============================================
  // Perform the calculation
  tTotal.Start();
  gcamComputeMetricProperties( gcam );
  tTotal.Stop();

  cout << "Computation took " << tTotal << endl;

  // =============================================
  // Write the output
  myUtils.Write( gcam, outFilename );

#ifdef FS_CUDA
  PrintGPUtimers();
#endif

  // ====================================
  // Release
  GCAMfree( &gcam );

#ifdef FS_CUDA
  PrintGPUtimers();
#endif

  
  exit( EXIT_SUCCESS );
}
