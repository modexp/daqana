#ifndef __DRIVER_H__
#define __DRIVER_H__

#include <string>
#include <vector>

#include "TROOT.h"
#include <stdint.h>

using namespace std;

class driver
{
public:
    driver();
    driver(string driver_file, bool fastOn, bool slowOn);
    ~driver();
    string getDataFile(){return DataFile;};
    string getRootFile(){return RootFile;};
    string getCalibrationFile(){return CalibrationFile;};
    string getSlowFile(){return SlowFile;};
    string getTempSlowFile(){return TempSlowFile;}
    string getLocation(){return location;};
    Double_t getDeltaT(){return delta_t;};
    Int_t    getNSample(){return nSample;};
    Int_t    getNPreTrigger(){return nPreTrigger;};
    Int_t    getNHeader(){return nHeader;};
    Int_t    getArraySize(){return ArraySize;};
    Int_t    getEventSize(){return EventSize;};
    Int_t    getNEvent(){return nEvent;};
    Int_t    getNSlowParams(){return nSlow;}
    ULong64_t 	getInitialTime(){return initial_time;}
    string getActiveChannel(Int_t ichan){return active_channels[ichan];}; 
    string getDetectorSerial(Int_t ichan){return det_serials[ichan];};
    string getDetectorType(Int_t ichan){return det_types[ichan];};
    string getSource(Int_t ichan){return sources[ichan];};
    Float_t  getTriggerLevel(Int_t ichan){return trigger_levels[ichan];};
    Float_t  getPMTvoltage(Int_t ichan){return PMT_voltages[ichan];};
    string getSlowBranchName(Int_t slowchan){return slowbranch_names[slowchan];};
private:
    string DataFile;
    string RootFile;
    string SlowFile;
    string CalibrationFile;
    string TempSlowFile;
    string location;
    Double_t delta_t;
    Int_t    nSample;
    Int_t    nPreTrigger;
    Int_t    nHeader;
    Int_t    ArraySize;
    Int_t    EventSize;
    Int_t    nEvent;
    Int_t    nSlow;
    ULong64_t 	initial_time;
    vector<string> active_channels;
    vector<string> det_serials;
    vector<string> det_types;
    vector<string> sources;
    vector<Float_t>  trigger_levels;
    vector<Float_t>  PMT_voltages;
    vector<string> slowbranch_names;
};

#endif // __DRIVER_H__
