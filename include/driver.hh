#ifndef __DRIVER_H__
#define __DRIVER_H__

#include <string>
#include <vector>

#include <stdint.h>
#include <TROOT.h>

using namespace std;

class driver
{
public:
    driver();
    driver(string driver_file);
    ~driver();
    string getDataFile(){return DataFile;};
    string getRootFile(){return RootFile;};
    string getLocation(){return location;};
    Double_t getDeltaT(){return delta_t;};
    Int_t    getNSample(){return nSample;};
    Int_t    getNPreTrigger(){return nPreTrigger;};
    Int_t    getNHeader(){return nHeader;};
    Int_t    getArraySize(){return ArraySize;};
    Int_t    getEventSize(){return EventSize;};
    Int_t    getNEvent(){return nEvent;};
    string getActiveChannel(int ichan){return active_channels[ichan];}; 
    string getDetectorSerial(int ichan){return det_serials[ichan];};
    string getDetectorType(int ichan){return det_types[ichan];};
    string getSource(int ichan){return sources[ichan];};
    Float_t  getTriggerLevel(int ichan){return trigger_levels[ichan];};
    Float_t  getPMTvoltage(int ichan){return PMT_voltages[ichan];};
private:
    string DataFile;
    string RootFile;
    string location;
    Double_t delta_t;
    Int_t    nSample;
    Int_t    nPreTrigger;
    Int_t    nHeader;
    Int_t    ArraySize;
    Int_t    EventSize;
    Int_t    nEvent;
    vector<string> active_channels;
    vector<string> det_serials;
    vector<string> det_types;
    vector<string> sources;
    vector<Float_t>  trigger_levels;
    vector<Float_t>  PMT_voltages;
};

#endif // __DRIVER_H__
