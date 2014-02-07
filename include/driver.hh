#ifndef __DRIVER_H__
#define __DRIVER_H__

#include <string>
#include <vector>
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
    double getDeltaT(){return delta_t;};
    int    getNSample(){return nSample;};
    int    getNPreTrigger(){return nPreTrigger;};
    int    getNHeader(){return nHeader;};
    int    getArraySize(){return ArraySize;};
    int    getEventSize(){return EventSize;};
    int    getNEvent(){return nEvent;};
    string getActiveChannel(int ichan){return active_channels[ichan];}; 
    string getDetectorSerial(int ichan){return det_serials[ichan];};
    string getDetectorType(int ichan){return det_types[ichan];};
    string getSource(int ichan){return sources[ichan];};
    float  getTriggerLevel(int ichan){return trigger_levels[ichan];};
    float  getPMTvoltage(int ichan){return PMT_voltages[ichan];};
private:
    string DataFile;
    string RootFile;
    string location;
    double delta_t;
    int    nSample;
    int    nPreTrigger;
    int    nHeader;
    int    ArraySize;
    int    EventSize;
    int    nEvent;
    vector<string> active_channels;
    vector<string> det_serials;
    vector<string> det_types;
    vector<string> sources;
    vector<float>  trigger_levels;
    vector<float>  PMT_voltages;
};

#endif // __DRIVER_H__
