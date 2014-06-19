#include "rootdriver.hh"

#include "TParameter.h"
#include "TDirectory.h"
#include "TNamed.h"

#include <iostream>

rootdriver::rootdriver(){}

rootdriver::rootdriver(driver *drv, Bool_t tmpbool){
    longRoot = tmpbool;
    
    // initialize the Branch variables
    chanNum = -1;
    integral = 0.;
    pkheight = 0.;
    timestamp = 0;
    isTestPulse = false;
    errorCode = 0;
    // extra variables for extended root file
    baseline    = 0;
    baselineRMS = 0;
    
    // open the root file
    f = new TFile(drv->getRootFile().c_str(),"RECREATE");
    
    // Define tree and branches
    tree = new TTree("T", "Source data");
    tree->Branch("channel", &chanNum, "channel/I");
    tree->Branch("integral", &integral, "integral/F");
    tree->Branch("height", &pkheight, "height/F");
    tree->Branch("time", &timestamp, "time/L");
    tree->Branch("istestpulse", &isTestPulse, "istestpulse/Bool_t");
    tree->Branch("error", &errorCode, "error/I");
    
    if(longRoot){
        tree->Branch("baseline", &baseline, "baseline/f");
        tree->Branch("rms", &baselineRMS, "baselineRMS/f");
    }
}

void rootdriver::Fill(event *ev){
    // fill the variables
    chanNum    = ev->getChannel();
    integral   = ev->getArea();
    pkheight   = ev->getPeak();
    timestamp  = ev->getTimeStamp();
    isTestPulse = ev->getIsTestPulse();
    errorCode   = 0;
    //
    if(longRoot){
        baseline    = ev->getBaseline();
        baselineRMS = ev->calculateBaselineRMS();
    }
    
    // write to the tree
    tree->Fill();
}

void rootdriver::writeParameters(driver *drv){
    char pName[100];
    // make separate root folder for run info
    TDirectory *_info = f->mkdir("info");
    _info->cd();

    // the strings....
    TNamed *Parameter = new TNamed("datafile",drv->getDataFile().c_str());
    Parameter->Write();
    Parameter = new TNamed("location",drv->getLocation().c_str());
    Parameter->Write();
    
    // the integer parameters
    TParameter<Int_t> *intPar = new TParameter<Int_t>("nsamples",drv->getNSample());
    intPar->Write();
    intPar = new TParameter<Int_t>("npretrigger",drv->getNPreTrigger());
    intPar->Write();
    intPar = new TParameter<Int_t>("nevent",drv->getNEvent());
    intPar->Write();
    intPar = new TParameter<Int_t>("nheader",drv->getNHeader());
    intPar->Write();
    intPar = new TParameter<Int_t>("chunksize",drv->getArraySize());
    intPar->Write();
    intPar = new TParameter<Int_t>("eventsize",drv->getEventSize());
    intPar->Write();
    // the double parameters
    TParameter<Double_t> *dblPar = new TParameter<Double_t>("deltat",drv->getDeltaT());
    dblPar->Write();

    // sources in separate folder
    TDirectory *_sources = _info->mkdir("source");
    _sources->cd();
    for(Int_t i=0; i<8; i++) {
        sprintf(pName,"channel_%i",i);
        Parameter = new TNamed(pName,drv->getSource(i).c_str());
        Parameter->Write();
    }
    
    // serial numbers in separate folder
    TDirectory *_serial = _info->mkdir("serial");
    _serial->cd();
    for(Int_t i=0; i<8; i++) {
        sprintf(pName,"channel_%i",i);
        Parameter = new TNamed(pName,drv->getDetectorSerial(i).c_str());
        Parameter->Write();
    }
    
    // active channels in separate folder
    TDirectory *_active = _info->mkdir("active");
    _active->cd();
    for(Int_t i=0; i<8; i++) {
        sprintf(pName,"channel_%i",i);
        Parameter = new TNamed(pName,drv->getActiveChannel(i).c_str());
        Parameter->Write();
    }
    
    // trigger levels in separate folder
    TDirectory *_trigger = _info->mkdir("trigger");
    _trigger->cd();
    for(Int_t i=0; i<8; i++) {
        sprintf(pName,"channel_%i",i);
        dblPar = new TParameter<Double_t>(pName,drv->getTriggerLevel(i));
        dblPar->Write();
    }
    
    // PMT HV levels in separate folder
    TDirectory *_hv = _info->mkdir("hv");
    _hv->cd();
    for(Int_t i=0; i<8; i++) {
        sprintf(pName,"channel_%i",i);
        dblPar = new TParameter<Double_t>(pName,drv->getPMTvoltage(i));
        dblPar->Write();
    }
    
    f->cd();
}

void rootdriver::Close(){
  f->Write();
}
