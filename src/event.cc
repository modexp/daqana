#include "event.hh"
#include <iostream>
#include <math.h>

event::event(){}

event::event(Int_t iev, Int_t ich, ULong64_t ts, vector<Double_t>* tr, Bool_t isTestPulse, driver* dr){
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
    isTestPulse = iLED;
    // number of pre-trigger samples
    nBaselineCalc = dr->getNPreTrigger() - N_BASELINE_NOT_USED;
    // initialize the event.... calculate baseline, peak, integral
    nDeltaT = dr->getDeltaT();
    nDataPoints = dr->getNSample();
    
    InitializeEvent();
}

void event::InitializeEvent(){
    // calculate BASELINE
    baseline = calculateBaseline();
    // correct the pulse shape for the baseline
    //for(int i=0; i<(int)trace.size(); i++) {
    //   trace[i]-=baseline;
//       cout << i<<" BCOR "<<trace[i]<<endl;
    //}
    // calculate PEAK
    
    peak = 0.;
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
    return rms;
}

Double_t event::calculatePeak(){

    Double_t pk = -9999;
    for(Int_t i=0; i<trace->size(); i++){
        Double_t val = trace->at(i);
        if(val>pk) pk =val;
    }
    
    pk -= baseline;

    return pk;
}

Double_t event::calculateIntegral(){
    
    Double_t I = 0;
    for(int i=0; i<trace->size(); i++){
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

Double_t event::calculatePeakAndIntegral(){

    Double_t I = 0;
    Float_t preamp=0.;
    Float_t amp = 0.;
    
    Bool_t htflag = false;
    
    Int_t ht = 1000.; //some random value
    Int_t lt = 100.; //some other random value
    
    for(Int_t i=0; i<trace->size(); i++){ //go over all ze bins
        amp = trace->at(i) - baseline;
        
        if (amp>lt){ //made it over lt, start counting stuff
            preamp+=amp; //it might not make it over ht, so use a different variable
            
            if (amp>ht) { //it can now count as a peak
                htflag = true; //save the fact its officially a peak
                }}
        
        else{ //no longer above lt
            if (htflag) { // but did reach above ht
                I=preamp; // assign the value to the integral
                preamp=0.;
            }}}
    
    return I;
    
}

event::~event() {
  delete trace;
  //
}

// Plot a TGraph of one pulse with the baseline in red, for debugging purposes
void event::Plot()
{
    // Graphs for plotting individual pulses
    TCanvas *canv = new TCanvas("c1","c1",0,0,450,450); // just for plotting individual pulses, can comment out if need
    TGraph *voltages = new TGraph();
    TGraph* g_base = new TGraph();
    
    for (Int_t m = 0; m<(nDataPoints-1); ++m)
    {
        voltages->SetPoint(m, m*nDeltaT, trace->at(m));
        g_base->SetPoint(m, m*nDeltaT, baseline);
    }
    
    voltages->Draw("AL");
    g_base->SetLineColor(2);
    g_base->Draw("same");
    cout << "So far we are on wave #: " << ievent <<endl;
    char tstr[100];
    sprintf(tstr,"Event = %i",ievent);
    if (iLED == true) cout << "This event was a test pulse from the LED" << endl;
    canv->SetTitle(tstr);
    canv->Modified();
    canv->Update();
    char pdfname[256];
    sprintf(pdfname,"pdf/shape_%i.pdf",ievent);
    canv->Print(pdfname);
    usleep(100000);

}
