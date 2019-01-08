#!/bin/sh

echo "Setting up CentOS7 environment for Modulation data processing"

#gcc
source /cvmfs/sft.cern.ch/lcg/releases/LCG_94/gcc/8.2.0/x86_64-centos7/setup.sh
#cmake
source /cvmfs/sft.cern.ch/lcg/releases/LCG_94/CMake/3.11.1/x86_64-centos7-gcc8-opt/CMake-env.sh
#python
source /cvmfs/sft.cern.ch/lcg/releases/LCG_94/Python/2.7.15/x86_64-slc6-gcc8-opt/Python-env.sh
#geant4
source /cvmfs/sft.cern.ch/lcg/releases/LCG_94/Geant4/10.04.p02/x86_64-centos7-gcc8-opt/bin/geant4.sh
#ROOT
source /cvmfs/sft.cern.ch/lcg/releases/LCG_94/ROOT/6.14.04/x86_64-centos7-gcc8-opt/ROOT-env.sh

# end of config
