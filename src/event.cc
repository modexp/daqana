#include "event.hh"
#include <iostream>
#include <math.h>

event::event(){}

event::event(Int_t iev, Int_t ich, Double_t ts, vector<Double_t>* tr, Bool_t isTestPulse, driver* dr){
    // initialize the event
    //drv = dr;
    // event number
    ievent = iev;
    // channel number
    ichannel = ich;
    // timestamp
    timestamp = ts;
    // trace info
    trace = tr;
    // test pulse info
    iLED = isTestPulse;
    // number of pre-trigger samples
    nBaselineCalc = dr->getNPreTrigger() - N_BASELINE_NOT_USED;
    // error code for teh event
    eventError = 0x00;
    // initialize the event.... calculate baseline, peak, integral
    nDeltaT = dr->getDeltaT();
    nDataPoints = dr->getNSample();
    
    InitializeEvent();
}

void event::InitializeEvent(){
    // calculate BASELINE
    baseline = calculateBaseline();
    // calculate BASELINE RMS
    baselineRMS = calculateBaselineRMS();
    // correct the pulse shape for the baseline
    //for(int i=0; i<(int)trace.size(); i++) {
    //   trace[i]-=baseline;
//       cout << i<<" BCOR "<<trace[i]<<endl;
    //}
    // calculate PEAK
    
    peak = calculatePeak();
    // calculate the AREA
    area = calculatePeakAndIntegral();
}

Double_t event::calculateBaseline(){
    Double_t b = 0;
    for(Int_t i=0; i<nBaselineCalc; i++) b+=trace->at(i);
    b /= nBaselineCalc;
    return b;
}

Double_t event::calculateBaselineRMS(){
    Double_t rms = 0;
    for(Int_t i=0; i<nBaselineCalc; i++) rms += pow(trace->at(i) - baseline ,2);
    rms /= nBaselineCalc;
    rms = sqrt(rms);
    
    if (rms>RMS_MAX_VALUE) eventError = (eventError | BASELINE_RMS_ERROR);
    
    return rms;
}

Double_t event::calculatePeak(){

    Double_t pk = -9999;
    for(Int_t i=0; i<trace->size(); i++){
        Double_t val = trace->at(i);
        if(val>pk) pk =val;
	if(val<0) eventError = (eventError | ADC_OVERFLOW_ERROR);
    }
    
    pk -= baseline;

    return pk;
}

Double_t event::calculateIntegral(){
    
    Double_t I = 0;
    for(Int_t i=0; i<trace->size(); i++){
        Double_t val = trace->at(i);
        I+=val;
    }
    
    I*=nDeltaT;
    I-= (baseline*nDeltaT*nDataPoints);
    //    cout<< "DELTAT = "<<drv.getDeltaT()<< " I = "<<I<<endl;
    
    
    
    return I;
    
}

//the following function assumes only 1 peak (the last one) at the moment and only calulates the integral.
//future function should also calculate the peak heigh, to save on time looping over wf.

Double_t event::calculatePeakAndIntegral() {
  Int_t maxPos = 0;
  Double_t max(0);
  Int_t i = 0;
  
  if (trace->size() != nDataPoints) {
    cout << "In calculate peak and integral -- trace is not the correct size" << endl;
    return 0.;
  }
  
  while (i < nDataPoints) {
      if ((trace->at(i)-baseline) > max) {
	  max = trace->at(i);
	  maxPos = i;
      }
      i++;
  }
  Double_t threshold = 0.5*(1-FRACTION);
  Double_t integral(0);
  
  // Find Start
  Int_t x = maxPos;
  //Double_t max = trace->at(maxPos);
  while (x>0 && (trace->at(x)-baseline) > threshold*max) x--;
  Int_t start = x;
  

  // Find Stop
  x = maxPos;
  while (x<nDataPoints && (trace->at(x)-baseline) > threshold*max) x++;
  Int_t stop = x;
 
  // For every point that falls between the start and stop positions, calculate the integral using the trapezoid rule
  for (x = start; x<stop-1; x++)
    {
      integral += ( 0.5*(trace->at(x+1) + trace->at(x)))-baseline; 
    }
  integral*= 2./(TMath::Power(2,14)-1.); // convert from labview units to volts
  integral*=nDeltaT;
  
  return integral;
}

event::~event() {
  delete trace;
  //
}

// Plot a TGraph of one pulse with the baseline in red, for debugging purposes
void event::Plot(TCanvas *canv)
{
  //if (iLED == true) {
    // Graphs for plotting individual pulses
    //TCanvas *canv = new TCanvas("c1","c1",0,0,450,450); // just for plotting individual pulses, can comment out if need
    TGraph *voltages = new TGraph();
    TGraph* g_base = new TGraph();
    
    for (Int_t m = 0; m<(nDataPoints-1); ++m)
    {
        voltages->SetPoint(m, m*nDeltaT, trace->at(m));
        g_base->SetPoint(m, m*nDeltaT, baseline);
    }
    canv->Clear();
    canv->Flush();
    voltages->Draw("AL");
    g_base->SetLineColor(2);
    g_base->Draw("same");
    cout << "So far we are on wave #: " << ievent <<endl;
    char tstr[100];
    sprintf(tstr,"Event = %i",ievent);
    //cout << "Integral: " << integral << endl;
    cout << "Baseline: " << baseline << endl;
    cout << "is test pulse " << iLED << endl;
    canv->SetTitle(tstr);
    canv->Modified();
    canv->Update();
    //char pdfname[256];
    //sprintf(pdfname,"pdf/shape_%i.pdf",ievent);
    //canv->Print(pdfname);
    Print();
    
    usleep(1000000);
    //canv->Close();
    //delete voltages;
    //delete g_base;
 // }
}

void event::Print()
{
    cout <<"event::Print() "<<endl;
    cout <<"event::Print() ievent      ="<<ievent<<endl;
    cout <<"event::Print() ts          ="<<timestamp<<endl;
    cout <<"event::Print() peak        ="<<peak<<endl;
    cout <<"event::Print() area        ="<<area<<endl;
    cout <<"event::Print() baseline    ="<<baseline<< " calculated from "<<nBaselineCalc<<" bins"<<endl;
    cout <<"event::Print() baselineRMS ="<<baselineRMS<<endl;

}
/*
slowevent::slowevent(){}

slowevent::slowevent(Int_t sid, Double_t sd, ULong64_t sts){
  // slow id (identifies parameter type, e.g. temperature, pressure, etc.)
    slowid = sid;
    // actual data
    slowdata = sd;
    // slow timestamp
    slowtimestamp = sts;
}

slowevent::~slowevent() {
  // nothing to do here
}
*/