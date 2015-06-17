#ifndef __ROOTDRIVER_H__
#define __ROOTDRIVER_H__

#include <TFile.h>
#include <TTree.h>
#include <TH1F.h>


#include <vector>
#include <string>

#include "event.hh"
#include "driver.hh"

#include <stdint.h>

using namespace std;

#define NUMBER_OF_CHANNELS 8
#define MAX_PARAMETERS 3

class rootdriver
{
public:
    rootdriver();
    rootdriver(driver *drv, Bool_t, Bool_t);
    void FastFill(event *ev, driver *dr);
    void readSlowEvent(Int_t islow);
    //ULong64_t SlowFill(slowevent *old_sev, ULong64_t old_stime);
    void writeParameters(driver *drv);
    void Close();
    
private:
    TFile *f;
    TFile *fs;
    
    TTree *tree;
    TTree *temp_slowtree;
    
    Int_t   	chanNum;
    Float_t 	integral;
    Float_t 	pkheight;
    Double_t  	timestamp;
    Int_t   	isTestPulse;
    Int_t   	errorCode;
    Float_t     energyRatio;
    
    Bool_t 	longRoot;
    
    Float_t 	baseline;
    Float_t 	baselineRMS;
    
    Bool_t 	slowOn;
    
    Int_t 	slowid;
    Double_t	sdata;
    //Double_t	*sarr;
    ULong64_t 	old_stime;
    ULong64_t	new_stime;
    ULong64_t	stimestamp;
    ULong64_t	stime;
    ULong64_t   time_end_of_range;
    //Double_t 	old_stime;
    //Double_t	new_stime;
    //Double_t	stimestamp;
    //Double_t	stime;
    Int_t 	    nSlowParams;
    //APC Double_t 	*slowdata;
    vector<Double_t> slowdata;

    
    Int_t 	slow_entry;
    Int_t   number_of_slow_events;
    
    Double_t calibration_constant[NUMBER_OF_CHANNELS][MAX_PARAMETERS];
    
};

#endif // __ROOTDRIVER_H__
