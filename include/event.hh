#ifndef __EVENT_H__
#define __EVENT_H__

#include "TCanvas.h"
#include "TGraph.h"
#include "driver.hh"
#include <vector>
#include <stdlib.h>
#include <stdint.h>
#include "TMath.h"

// N_baseline_calc = nPreTrigger - N_BASELINE_NOT_USED
#define N_BASELINE_NOT_USED 5
#define FRACTION 0.9

using namespace std;

class event
{
public:
    event();
    event(Int_t iev, Int_t ich, ULong64_t ts, vector<Double_t>* tr, Bool_t isTestPulse, driver* dr);
    ~event();
    // initialize: calculate baseline, correct for baseline, find peak, calculate area
    void InitializeEvent();
    void Plot(TCanvas * canv);
    //~event();
    Int_t         getChannel(){return ichannel;};
    vector<Double_t>* getTrace() {return trace;};
    Double_t      getPeak()  {return peak;};
    Double_t      getArea()  {return area;};
    Bool_t        getIsTestPulse()  {return iLED;};
    ULong64_t        getTimeStamp() {return timestamp;};
    Double_t      getBaseline() {return baseline;};
    Double_t      calculateBaselineRMS();
    Double_t      calculateBaseline();
    Double_t      calculatePeak();
    Double_t      calculateIntegral();
    Double_t 	calculatePeakAndIntegral();
private:
    Int_t         ievent;
    Int_t         ichannel;
    ULong64_t        timestamp;
    vector<Double_t>* trace;
    Bool_t 	iLED;
    Double_t      baseline;
    Double_t      baselineRMS;
    Int_t         nBaselineCalc;
    Double_t      area;
    Double_t      peak;
    Float_t 	nDeltaT;
    Int_t 	nDataPoints;
};
/*
class slowevent
{
public:
      slowevent();
      slowevent(Int_t sid, Double_t sdata, ULong64_t sts);
      ~slowevent();
      Int_t	getSlowID(){return slowid;};
      Double_t	getSlowData(){return slowdata;};
      ULong64_t 	getSlowTimeStamp(){return slowtimestamp;};
private:
      Int_t	slowid;
      Double_t	slowdata;
      ULong64_t 	slowtimestamp;
};
*/
 #endif // __EVENT_H__

