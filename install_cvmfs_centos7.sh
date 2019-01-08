#!/bin/sh

echo "Installinf daqaan&slowdaq for Modulation data processing"

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

if [ ! -L daqana ]; then
  echo "    create: link to ../daqana-build/daqana"
  ln -s ../daqana-build/daqana daqana
fi
if [ ! -L slowdaq ]; then
  echo "    create: link to ../daqana-build/slowdaq"
  ln -s ../daqana-build/slowdaq slowdaq
fi

# end of config
