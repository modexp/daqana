#include "rootdriver.hh"

#include "TParameter.h"
#include "TDirectory.h"
#include "TNamed.h"

#include <iostream>

rootdriver::rootdriver(){}

rootdriver::rootdriver(driver *drv, Bool_t tmpbool, Bool_t slow, Bool_t fast){
    longRoot = tmpbool;
    slowOn = slow;
    fastOn = fast;
    
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
    
    // variables for slow data 
    Int_t nSlowParameters;
    const char* slowbranchname;
    char* temp;
    Int_t nSlowParams = drv->getNSlowParams(); // valid for XML version CM1.0
cout << nSlowParams << endl;
    //slowdata[nSlowParams];
    slowdata = (Double_t*) malloc(sizeof(Double_t) * nSlowParams);
    cout << "Number of slow parameters: " << nSlowParams << endl;
    for (Int_t i=0; i<nSlowParams; i++) slowdata[i]=0;
    
    // for fast and slow data
    slow_entry = 1;
    
    // open the root file
    f = new TFile(drv->getRootFile().c_str(),"RECREATE");
    
    // Define tree and branches
    tree = new TTree("T", "Source data");
    stree = new TTree("ST", "Slow data");
    
    if(fast){
	tree->Branch("channel", &chanNum, "channel/I");
	tree->Branch("integral", &integral, "integral/F");
	tree->Branch("height", &pkheight, "height/F");
	tree->Branch("time", &timestamp, "time/L");
	tree->Branch("istestpulse", &isTestPulse, "istestpulse/bool");
	tree->Branch("error", &errorCode, "error/I");
    }
    
    if(fast && longRoot){
        tree->Branch("baseline", &baseline, "baseline/f");
        tree->Branch("rms", &baselineRMS, "baselineRMS/f");
    }
    
    if(slow){
      const char* type = "/D";
      char buffer[256];
      for (Int_t i=0; i<nSlowParams; i++){
	slowbranchname = drv->getSlowBranchName(i).c_str();
	//cout << slowbranchname << endl;
	strncpy(buffer, slowbranchname, sizeof(buffer));
	strncat(buffer, type, sizeof(buffer));
	//cout << buffer << endl;
	stree->Branch(slowbranchname, &slowdata[i], buffer);
	if (fast) tree->Branch(slowbranchname, &slowdata[i], buffer);
      }
      
      stree->Branch("stime", &stimestamp, "slowtimestamp/l");

    }
    
    
}

ULong64_t rootdriver::SlowFill(slowevent *sev, ULong64_t old_stime){
    slowid 	= sev->getSlowID();
    sdata	= sev->getSlowData();
    new_stime  = sev->getSlowTimeStamp();
    
    //cout << "Slow ID: " << slowid << " Data: " << sdata << " Timestamp: " << new_stime << endl;


    if (new_stime <= old_stime) { 
      if (slowid > 7) cout << "WTF!!!!!! The slow ID was: " << slowid << " the data was: " << sdata << " and the timestamp was " << new_stime << endl;
	slowdata[slowid] = sdata;
	stimestamp = new_stime;
    }
    else { // TODO PUFFER!!!!!! THE MOST CONFUSED PUFFER IN THE SEA

      if (slowOn) stree->Fill();
      //cout << stimestamp << endl;
      //cout << "0: " << slowdata[0] << " 1: " << slowdata[1] << " 2: " << slowdata[2] << " 3: " << slowdata[3] << " 4: " << slowdata[4] << " 5: " << slowdata[5] << " 6: " << slowdata[6] << " 7: " << slowdata[7] << endl;
      //cout << "I have filled the tree!" << endl;
      slowdata[slowid] = sdata;
      stimestamp = new_stime;

    }
    old_stime = new_stime;
    
    return old_stime;
}

void rootdriver::FastFill(event *ev, driver *dr){
    // fill the variables
    chanNum    = ev->getChannel();
    integral   = ev->getArea();
    pkheight   = ev->getPeak();
    timestamp  = ev->getTimeStamp();
    isTestPulse = ev->getIsTestPulse();
    errorCode   = 0;
    
    //stree->SetBranchAddress("stime", &stimestamp);
    //cout << "I think the error is here..." << endl;
    //stree->SetBranchStatus("*",1);
    //for (Int_t i = 0; i < nSlowParams; i++) stree->SetBranchAddress(dr->getSlowBranchName(i).c_str(), &slowdata[i]);
    
    if(longRoot){
        baseline    = ev->getBaseline();
        baselineRMS = ev->calculateBaselineRMS();
    }
    
    if (slowOn && fastOn){
      //stree->Scan();
	stree->GetEntry(slow_entry);
	//cout << stimestamp << endl;
	//cout << "0: " << slowdata[0] << " 1: " << slowdata[1] << " 2: " << slowdata[2] << " 3: " << slowdata[3] << " 4: " << slowdata[4] << " 5: " << slowdata[5] << " 6: " << slowdata[6] << " 7: " << slowdata[7] << endl;
	//cout << "Fast timestamp: " << timestamp << " slow timestamp: " << stimestamp << endl;
	//cout << slowdata << endl;
	if (timestamp>stimestamp) { // if past this slowtimestamp, grab the next slow event 
	 //cout << "0: " << slowdata[0] << " 1: " << slowdata[1] << " 2: " << slowdata[2] << " 3: " << slowdata[3] << " 4: " << slowdata[4] << " 5: " << slowdata[5] << " 6: " << slowdata[6] << " 7: " << slowdata[7] << endl;
	//cout << "Fast timestamp: " << timestamp << " slow timestamp: " << stimestamp << endl;
	  //if (slow_entry == 215) cout << "I ran out of slow events here!!!!!!!!!!!" << endl;
	  cout << "I have filled in a new slow entry " << slow_entry << endl;
	  stree->GetEntry(slow_entry);
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
  f->Write();
}
