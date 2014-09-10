#include "driver.hh"
#include <iostream>
#include <fstream>

driver::driver(){}

driver::driver(string f){
    
    ifstream fin;
    fin.open(f.c_str());
    // get all info from the temporary info file.....
    fin >> DataFile;
    fin >> SlowFile;
    fin >> RootFile;
    fin >> location;
    fin >> initial_time;
    fin >> delta_t;
    fin >> nSample;
    fin >> nPreTrigger;
    fin >> nHeader;
    fin >> ArraySize;
    fin >> EventSize;
    fin >> nEvent;

    string aa;
    Float_t ff;

    for(Int_t i=0; i<8; i++) {
	//  active channels
        fin >>aa;
        active_channels.push_back(aa);
	//  serial numbers
        fin >>aa;
        det_serials.push_back(aa);
	//  detector types
        fin >>aa;
        det_types.push_back(aa);
	// sources
        fin >>aa;
        sources.push_back(aa);
	// trigger level
        fin >>ff;
        trigger_levels.push_back(ff);
	// PMT voltage
        fin >>ff;
        PMT_voltages.push_back(ff);
    }
    fin >> nSlow;
    for (Int_t i = 0; i< nSlow; i++) {
	fin >> aa;
	slowbranch_names.push_back(aa);
    }
    
    fin.close();
}

driver::~driver(){
  //nothing to do here
}