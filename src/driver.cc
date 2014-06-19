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
    for(Int_t i=0; i<8; i++) {
        fin >>aa;
        active_channels.push_back(aa);
    }
    // detector serial numbers
    for(Int_t i=0; i<8; i++) {
        fin >>aa;
        det_serials.push_back(aa);
    }
    // detector type
    for(Int_t i=0; i<8; i++) {
        fin >>aa;
        det_types.push_back(aa);
    }
    
    // sources
    for(Int_t i=0; i<8; i++) {
        fin >>aa;
        sources.push_back(aa);
    }
    Float_t ff;
    // trigger level
    for(Int_t i=0; i<8; i++) {
        fin >>ff;
        trigger_levels.push_back(ff);
    }
    // PMT voltage
    for(Int_t i=0; i<8; i++) {
        fin >>ff;
        PMT_voltages.push_back(ff);
    }
    
    fin.close();
}

driver::~driver(){
  //nothing to do here
}