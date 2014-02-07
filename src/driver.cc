#include "driver.hh"
#include <iostream>
#include <fstream>

driver::driver(){}

driver::driver(string f){
    
    ifstream fin;
    fin.open(f.c_str());
    // get all info from the temporary info file.....
    fin >> DataFile;
    fin >> RootFile;
    fin >> location;
    fin >> delta_t;
    fin >> nSample;
    fin >> nPreTrigger;
    fin >> nHeader;
    fin >> ArraySize;
    fin >> EventSize;
    fin >> nEvent;

    string aa;
    // active channels
    for(int i=0; i<8; i++) {
        fin >>aa;
        active_channels.push_back(aa);
    }
    // detector serial numbers
    for(int i=0; i<8; i++) {
        fin >>aa;
        det_serials.push_back(aa);
    }
    // detector type
    for(int i=0; i<8; i++) {
        fin >>aa;
        det_types.push_back(aa);
    }
    
    // sources
    for(int i=0; i<8; i++) {
        fin >>aa;
        sources.push_back(aa);
    }
    float ff;
    // trigger level
    for(int i=0; i<8; i++) {
        fin >>ff;
        trigger_levels.push_back(ff);
    }
    // PMT voltage
    for(int i=0; i<8; i++) {
        fin >>ff;
        PMT_voltages.push_back(ff);
    }
    
    fin.close();
}

driver::~driver(){
  //nothing to do here
}