#ifndef __ROOTDRIVER_H__
#define __ROOTDRIVER_H__

#include <TFile.h>
#include <TTree.h>


#include <vector>
#include <string>

#include "event.hh"
#include "driver.hh"

#include <stdint.h>

using namespace std;

#define NUMBER_OF_CHANNELS 8

class rootdriver
{
public:
    rootdriver();
    rootdriver(driver *drv, Bool_t, Bool_t);
    void FastFill(event *ev, driver *dr);
    //ULong64_t SlowFill(slowevent *old_sev, ULong64_t old_stime);
    void writeParameters(driver *drv);
    void Close();
    
private:
    TFile *f;
    TFile *fs;
    
    TTree *tree;
    //TTree *stree;
    TTree *temp_slowtree;
    
    Int_t   	chanNum;
    Float_t 	integral;
    Float_t 	pkheight;
    Double_t  	timestamp;
    Int_t   	isTestPulse;
    Int_t   	errorCode;
    
    Bool_t 	longRoot;
    
    Float_t 	baseline;
    Float_t 	baselineRMS;
    
    Bool_t 	slowOn;
    
    Int_t 	slowid;
    Double_t	sdata;
    //Double_t	*sarr;
    //ULong64_t 	old_stime;
    //ULong64_t	new_stime;
    //ULong64_t	stimestamp;
    //ULong64_t	stime;
    Double_t 	old_stime;
    Double_t	new_stime;
    Double_t	stimestamp;
    Double_t	stime;
    Int_t 	nSlowParams;
    Double_t 	*slowdata;
    
    Int_t 	slow_entry;
    
    Double_t calibration_constant[NUMBER_OF_CHANNELS];
    
};

#endif // __ROOTDRIVER_H__
