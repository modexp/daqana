#include "driver.hh"
#include <iostream>
#include <fstream>

driver::driver(){}

driver::driver(string f, bool fastOn, bool slowOn){
    
    ifstream fin;
    fin.open(f.c_str());
    string aa;
    // get all info from the temporary info file.....
    if (fastOn) fin >> DataFile;
    if (slowOn) {
        fin >> SlowFile;
        fin >> TempSlowFile;
        cout << " slowfile name: " <<SlowFile << endl;
        cout << "temp slowfile name: " <<TempSlowFile << endl;
    }
    
    if (fastOn) {
        fin >> RootFile;
        fin >> location;
        fin >> CalibrationFile;
        fin >> initial_time;
        fin >> delta_t;
        fin >> nSample;
        fin >> nPreTrigger;
        fin >> nHeader;
        fin >> ArraySize;
        fin >> EventSize;
        float ftemp;
        fin >> ftemp;
        nEvent = (int)ftemp;
        
        
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
            // print it
            cout <<i<<" active  = "<<active_channels[i]<<endl;
            cout <<i<<" serial  = "<<det_serials[i]<<endl;
            cout <<i<<" type    = "<<det_types[i]<<endl;
            cout <<i<<" source  = "<<sources[i]<<endl;
            cout <<i<<" trigger = "<<trigger_levels[i]<<endl;
            cout <<i<<" hv      = "<<PMT_voltages[i]<<endl;
        }
    }
    if (slowOn) {
        fin >> nSlow;
        for (Int_t i = 0; i< nSlow; i++) {
            fin >> aa;
            slowbranch_names.push_back(aa);
        }
    }
    
    fin.close();
}

driver::~driver(){
    //nothing to do here
}