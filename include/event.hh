#ifndef __EVENT_H__
#define __EVENT_H__

#include "TCanvas.h"
#include "TGraph.h"
#include "TH1F.h"
#include "driver.hh"
#include <vector>
#include <stdlib.h>
#include <stdint.h>
#include "TMath.h"
#include "TStyle.h"

// N_baseline_calc = nPreTrigger - N_BASELINE_NOT_USED
//#define N_BASELINE_NOT_USED 5
#define N_BASELINE_NOT_USED 10
#define FRACTION 0.9
#define RMS_MAX_VALUE 25. // maximum baseline RMS value to flag error

// ADC settings
#define ADC_MAX_VALUE   16384
#define ADC_MAX_VOLTAGE 2.0

// define the event error codes
#define ADC_OVERFLOW_ERROR 0x01
#define BASELINE_RMS_ERROR 0x02
#define LONG_PEAK_ERROR    0x04

// define the threshold value for hit counting
#define THRESHOLD_VALUE 1000


using namespace std;

class event
{
public:
    event();
    event(Int_t iev, Int_t ich, Double_t ts, vector<Double_t>* tr, Bool_t isTestPulse, driver* dr);
    ~event();
    // initialize: calculate baseline, correct for baseline, find peak, calculate area
    void InitializeEvent();
    void Plot(TCanvas * canv);
    void Print();
    //~event();
    Int_t         getChannel(){return ichannel;};
    vector<Double_t>* getTrace() {return trace;};
    Double_t      getPeak()  {return peak;};
    Double_t      getArea()  {return area;};
    Bool_t        getIsTestPulse()  {return iLED;};
    Double_t      getTimeStamp() {return timestamp;};
    Double_t      getBaseline() {return baseline;};
    Double_t      getBaselineRMS() {return baselineRMS;};
    
    Int_t         getNumberAboveThreshold(){return n_in_peak;};
    
    Double_t      calculateBaselineRMS();
    Double_t      calculateBaseline();
    Double_t      calculatePeak();
    Double_t      calculateIntegral();
    Double_t 	  calculatePeakAndIntegral();
    
    Int_t         getErrorCode(){return eventError;}
private:
    Int_t         eventError;
    Int_t         ievent;
    Int_t         ichannel;
    Double_t        timestamp;
    vector<Double_t>* trace;

    Bool_t 	iLED;
    Double_t      baseline;
    Double_t      baselineRMS;
    Int_t         nBaselineCalc;
    Double_t      area;
    Double_t      peak;
    Float_t 	nDeltaT;
    Int_t 	nDataPoints;
    
    Int_t         n_in_peak;
};

 #endif // __EVENT_H__

