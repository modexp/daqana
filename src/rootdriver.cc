#include "rootdriver.hh"

#include "TParameter.h"
#include "TDirectory.h"
#include "TNamed.h"

#include <iostream>

rootdriver::rootdriver(){}

rootdriver::rootdriver(driver *drv, Bool_t tmpbool, Bool_t slow){
    longRoot = tmpbool;
    slowOn = slow;
    
    // initialize the Branch variables
    chanNum = -1;
    integral = 0.;
    pkheight = 0.;
    timestamp = 0;
    isTestPulse = false;
    errorCode = 0;
    
    // calibration constants
    for(int ich = 0; ich < NUMBER_OF_CHANNELS; ich++) {
        calibration_constant[ich] = 1.0;
    }
    
    // read the calibration constants if you wish
    string calFile = drv->getCalibrationFile();
    if(calFile != "NULL.root"){
        TFile *g = new TFile(calFile.c_str(),"READONLY");
        TParameter<double>* cal;
        char tmp[100];
        for(int ich = 0; ich<NUMBER_OF_CHANNELS; ich++){
            sprintf(tmp,"cal_ch%02d",ich);
            cal = (TParameter<double>*)g->Get(tmp);
            calibration_constant[ich] = cal->GetVal();
        }
        g->Close();
        
    }
    
    // extra variables for extended root file
    baseline    = 0;
    baselineRMS = 0;
    
    // open the output root file
    f = new TFile(drv->getRootFile().c_str(),"RECREATE");
    
    // Define tree and branches
    tree = new TTree("T", "Source data");
    
    tree->Branch("channel", &chanNum, "channel/I");
    tree->Branch("integral", &integral, "integral/F");
    tree->Branch("height", &pkheight, "height/F");
    tree->Branch("time", &timestamp, "time/D");
    tree->Branch("istestpulse", &isTestPulse, "istestpulse/bool");
    tree->Branch("error", &errorCode, "error/I");
    
    if(longRoot){
        tree->Branch("baseline", &baseline, "baseline/f");
        tree->Branch("rms", &baselineRMS, "baselineRMS/f");
    }
    
    //
    // Open the slow control datafile + initialize the data
    //
    
    // variables for slow data
    Int_t nSlowParameters;
    const char* slowbranchname;
    char* temp;
    Int_t nSlowParams = drv->getNSlowParams(); // valid for XML version CM1.0
    slowdata = (Double_t*) malloc(sizeof(Double_t) * nSlowParams);
    cout << "rootdriver:: Number of slow parameters: " << nSlowParams << endl;
    for (Int_t i=0; i<nSlowParams; i++) slowdata[i]=0;
    
    if(slowOn){
        cout << "rootdriver:: slow file name is = " <<  drv->getTempSlowFile().c_str() << endl;
        fs = new TFile(drv->getTempSlowFile().c_str(), "ROOT");
        temp_slowtree = (TTree*)fs->Get("ST");
        slow_entry = 1;
        
        const char* type = "/D";
        char buffer[256];
        for (Int_t i=0; i<nSlowParams; i++){
            slowbranchname = drv->getSlowBranchName(i).c_str();
            //cout << slowbranchname << endl;
            strncpy(buffer, slowbranchname, sizeof(buffer));
            strncat(buffer, type, sizeof(buffer));
            //cout << buffer << endl;
            temp_slowtree->SetBranchAddress(slowbranchname, &slowdata[i]);
            //stree->Branch(slowbranchname, &slowdata[i], buffer);
            tree->Branch(slowbranchname, &slowdata[i], buffer);
        }
        //stree->Branch("stime", &stime, "slowtimestamp/l");
        temp_slowtree->SetBranchAddress("stime", &stimestamp);
        //stree->SetDirectory(0);
        tree->SetDirectory(0);
        cout << "N slow = "<<temp_slowtree->GetEntriesFast()<<endl;
        temp_slowtree->GetEntry(0);
        cout << "slow time = "<<stimestamp<<endl;
        for (Int_t i=0; i<nSlowParams; i++) cout<<drv->getSlowBranchName(i)<< " = "<<slowdata[i]<<endl;
    }
}

void rootdriver::FastFill(event *ev, driver *dr){
    // fill the variables
    chanNum    = ev->getChannel() % 100;
    integral   = ev->getArea()*calibration_constant[chanNum];
    pkheight   = ev->getPeak();
    timestamp  = ev->getTimeStamp();
    isTestPulse = ev->getIsTestPulse();
    errorCode   = ev->getErrorCode();
    
    if(longRoot){
        baseline    = ev->getBaseline();
        baselineRMS = ev->getBaselineRMS();
    }
    
    if (slowOn){
        //temp_slowtree->GetEntry(slow_entry);
        
        if (timestamp>=stimestamp && slow_entry<temp_slowtree->GetEntriesFast()) { // if past this slowtimestamp, grab the next slow event
            //cout << "I have filled in a new slow entry " << slow_entry << endl;
            temp_slowtree->GetEntry(slow_entry);
            slow_entry++;
        }
        
        
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
    tree->Write();
    
    f->Close();
    if(slowOn) fs->Close();
}
