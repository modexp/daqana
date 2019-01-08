#!/bin/sh

echo "Installinf daqaan&slowdaq for Modulation data processing"


# install using cmake in daqana-build directory
if [ ! -d ../daqana-build ]; then
  echo "    create: ../daqana-build"
  mkdir ../daqana-build
  cd ../daqana-build
  cmake -DCMAKE_INSTALL_PREFIX="" ../daqana/
  make
  cd ../daqana
else 
  cd ../daqana-build
  make
  cd ../daqana
fi

#make sure the processor can be executed
chmod +x daqprocessor_single_calibrate.py

# link to the binaries that are inside the ../daqana-build directory
if [ ! -L daqana ]; then
  echo "    create: link to ../daqana-build/daqana"
  ln -s ../daqana-build/daqana daqana
fi
if [ ! -L slowdaq ]; then
  echo "    create: link to ../daqana-build/slowdaq"
  ln -s ../daqana-build/slowdaq slowdaq
fi

# end of config
