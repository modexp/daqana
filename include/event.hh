#ifndef __EVENT_H__
#define __EVENT_H__

#include "TCanvas.h"
#include "TGraph.h"
#include "driver.hh"
#include <vector>
#include <stdlib.h>

// N_baseline_calc = nPreTrigger - N_BASELINE_NOT_USED
#define N_BASELINE_NOT_USED 5

using namespace std;

class event
{
public:
    event();
    event(int iev, int ich, long ts, vector<double>* tr, bool isTestPulse, driver* dr);
    ~event();
    // initialize: calculate baseline, correct for baseline, find peak, calculate area
    void InitializeEvent();
    void Plot();
    //~event();
    int         getChannel(){return ichannel;};
    vector<double>* getTrace() {return trace;};
    double      getPeak()  {return peak;};
    double      getArea()  {return area;};
    long        getTimeStamp() {return timestamp;};
    double      getBaseline() {return baseline;};
    bool 	getIsTestPulse() {return iLED;};
    double      calculateBaselineRMS();
    double      calculateBaseline();
    double      calculatePeak();
    double      calculateIntegral();
    double      calculatePeakAndIntegral();
private:
    int         ievent;
    int         ichannel;
    long        timestamp;
    vector<double>* trace;
    int 	iLED;
    double      baseline;
    double      baselineRMS;
    int         nBaselineCalc;
    double      area;
    double      peak;
    float nDeltaT;
    int nDataPoints;
};

#endif // __EVENT_H__

