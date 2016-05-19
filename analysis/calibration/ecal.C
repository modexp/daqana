#define ecal_cxx
#include "ecal.h"
#include <TF1.h>
#include <TMath.h>
#include <TH2.h>
#include <TGraphErrors.h>
#include <TStyle.h>
#include <TCanvas.h>
#include <TParameter.h>
#include <iostream>
#include <vector>
#include <stdio.h>

/*---------------------------------------------------------------------------------------------------*/
//
// ecal.C Routine to do the energy calibration of a modulation run
//
// Usage:
//  prompt> #include "ecal.C"
//  prompt> ecal e(<directory_with_uncalibrated_rootfiles>,<calibration_output_rootfile>)
//  prompt> e.Loop()
//
// Important parameter in ecal.h: CALIBRATION_MODE 0 : one calibration for full run (not adviced)
//                                CALIBRATION_MODE 1 : re-calibrate every TIME_INTERVAL seconds (see ecal.h)
//
// A.P.
/*---------------------------------------------------------------------------------------------------*/
// (1) You need to identify where the first peak is in the histograms of the integrals.
// (2) Set range_low and range_high to find the range in which the peak with cal_energy should be

//                                           CH0       CH1         CH2         CH3       CH4    CH5    CH6    CH7
const double range_low[NUMBER_OF_CHANNELS] ={0.16e-6 , 0.14e-6   , 0.05e-6   , 0.05e-6 , 0.   , 0.   , 0.   , 0.  };
const double range_high[NUMBER_OF_CHANNELS]={1e-6    , 1e-6      , 1e-6      , 1e-6    , 1e-6 , 1e-6 , 1e-6 , 1e6 };

/*---------------------------------------------------------------------------------------------------*/
float source_energy[NUMBER_OF_SOURCES][MAX_PEAKS] =
//
// the energy peaks you wish to select for the calibration should be in this list
//
{
    {1460.,-1,-1,-1,-1},                 // ID0: Background
    {511.,1157.020,511.+1157.020,-1,-1}, // ID1: Ti44
    {1173.2,1332.5,1173.2+1332.5,-1,-1}, // ID2: Co60
    {661.7,-1,-1,-1,-1},                 // ID3: CS137
    {-1,-1,-1,-1,-1},                    // ID4: MN54
    {1460.,-1,-1,-1,-1}                  // ID5: K40
};

/*---------------------------------------------------------------------------------------------------*/
Double_t fitf_gauss(Double_t *v, Double_t *par)
{
    Double_t arg = 0;
    if (par[2] != 0) arg = (v[0] - par[1])/par[2];
    
    Double_t fitval = par[0]*TMath::Exp(-0.5*arg*arg);
    
    fitval += par[3]+par[4]*v[0];
    
    return fitval;
}
/*---------------------------------------------------------------------------------------------------*/
void ecal::book_histograms(){
    //
    // book histograms for energy calibration
    //
    cout <<"ecal::book_histograms"<<endl;
    _f = new TFile(calFile.c_str(),"RECREATE");
    
    char tmp[100];
    for (int ich = 0; ich <NUMBER_OF_CHANNELS; ich++){
        sprintf(tmp,"integral_ch%02d",ich);
        _integral.push_back(new TH1F(tmp,tmp,1000,0.,1e-6));
        //
        // plots are only meaningfull if CALIBRATION_MODE==0
        //
        if(CALIBRATION_MODE == 0){
            sprintf(tmp,"energy_ch%02d",ich);
            _energy.push_back(new TH1F(tmp,tmp,1000,0.,3000.));
            sprintf(tmp,"energy_all_ch%02d",ich);
            _energy_all.push_back(new TH1F(tmp,tmp,1000,0.,3000.));
        }
    }
    
    //
    // book output root tree for calibration constants
    //
    _cal_tree = new TTree("cal","energy calibration data");
    _cal_tree->Branch("id", &_cal_index, "id/I");
    _cal_tree->Branch("cal_tmin", &_cal_tmin, "cal_tmin/D");
    _cal_tree->Branch("cal_tmax", &_cal_tmax, "cal_tmax/D");
    _cal_tree->Branch("c0",&_cal_c0);
    _cal_tree->Branch("c1",&_cal_c1);
    _cal_tree->Branch("c2",&_cal_c2);
    _cal_tree->Branch("chi2",&_cal_chi2);
    //
    // initialize the vectors to length NUMBER_OF_CHANNELS
    //
    _cal_index = 0;
    _cal_tmin = 0;
    _cal_tmax = 9e99;
    for(int ich=0;ich<NUMBER_OF_CHANNELS;ich++) {
        _cal_c0.push_back(0.0);
        _cal_c1.push_back(0.0);
        _cal_c2.push_back(0.0);
        _cal_chi2.push_back(0.0);
    }
    
    cout <<"ecal::book_histograms ... done"<<endl;
}
/*---------------------------------------------------------------------------------------------------*/
void ecal::fill_histograms(int ilevel){
    
    Long64_t nentries = fChain->GetEntriesFast();
    cout<<"Start calibration loop.... nentries ="<<nentries<<" LEVEL = "<<ilevel<<endl;
    Long64_t nbytes = 0, nb = 0;
    for (Long64_t jentry=0; jentry<nentries;jentry++) {
        Long64_t ientry = LoadTree(jentry);
        if (ientry < 0) break;
        nb = fChain->GetEntry(jentry);   nbytes += nb;
        // fill the hisotgrams with 'good' events
        channel = channel % 100;
        
        if        (ilevel == BEFORE_CALIBRATION){
            if(error == 0) _integral[channel]->Fill(integral);
        } else if (ilevel == AFTER_CALIBRATION){
            if(error == 0) _energy[channel]->Fill(ccal[channel][0]+integral*ccal[channel][1]);
            _energy_all[channel]->Fill(ccal[channel][0]+integral*ccal[channel][1]);
        }
        if(jentry%100000 == 0) cout<<"Processed "<<jentry<<" events"<<endl;
        
    }
    cout<<"Done calibration loop...."<<endl;
    
}
/*---------------------------------------------------------------------------------------------------*/
void ecal::get_source_id()
{
    
    cout <<"ecal::get_source_id"<<endl;
    char channel_name[100];
    
    // get the name of the first file in the data chain
    TFile * _f_tmp = fChain->GetFile();
    _f_tmp->cd("info/active");
    
    TNamed *isActive;
    for(int ichannel=0; ichannel<NUMBER_OF_CHANNELS; ichannel++){
        
        sprintf(channel_name,"channel_%i",ichannel);
        gDirectory->GetObject(channel_name,isActive);
        string active = isActive->GetTitle();
        if(active == "On"){
            channel_active[ichannel] = kTRUE;
        } else {
            channel_active[ichannel] = kFALSE;
        }
        
    }
    // retrieve the source information
    _f_tmp->cd("info/source");
    
    TNamed *sourceName;
    for(int ichannel=0; ichannel<NUMBER_OF_CHANNELS; ichannel++){
        
        sprintf(channel_name,"channel_%i",ichannel);
        gDirectory->GetObject(channel_name,sourceName);
        string source = sourceName->GetTitle();
        
        if(channel_active[ichannel]){
            cout <<"ecal::get_source_id  channel = "<<ichannel<<" source = "<<source<<endl;
            if(source == "Background"){
                source_id[ichannel] = BACKGROUND;
            } else if ( source == "Ti-44"){
                source_id[ichannel] = TI44;
            } else if ( source == "Co-60"){
                source_id[ichannel] = CO60;
            } else if ( source == "Cs-137"){
                source_id[ichannel] = CS137;
            } else if ( source == "Mn-54"){
                source_id[ichannel] = MN54;
            } else if ( source == "K-40"){
                source_id[ichannel] = K40;
            } else {
                cout <<"ecal::get_source_id() Unidentified source ..... TERMINATE"<<endl;
                exit(-1);
            }
        } else { // if the channel is inactive, just set it to BACKGROUND (does not matter)
            source_id[ichannel] = BACKGROUND;
        }
    }
    cout <<"ecal::get_source_id ... done"<<endl;
    
}
/*---------------------------------------------------------------------------------------------------*/
void ecal::Loop()
{
    //
    // look in the first data file of the chain to see what sources are present
    //
    get_source_id();
    
    //
    // howto do the calibration....
    //
    if (CALIBRATION_MODE == 0) {
        //
        // one single calibration from all the specified input files
        //
        ecal_single();
    } else {
        //
        // multiple calibrations made in interval lengths specified by the TIME_INTERVAL variable
        //
        ecal_continuous();
    }
    //
    // done
    //
}

/*---------------------------------------------------------------------------------------------------*/
void ecal::ecal_continuous(){
    //
    // energy calibration for modulation detectors
    //
    if (fChain == 0) return;
    //
    // book the histograms needed for the energy calibration
    //
    book_histograms();
    
    //
    // prepare the event loop
    //
    Long64_t nentries = fChain->GetEntriesFast();
    cout<<"Start calibration loop.... nentries ="<<nentries<<endl;
    Long64_t nbytes = 0, nb = 0;
    
    //
    // loop over all events in the tree
    //
    for (Long64_t jentry=0; jentry<nentries;jentry++) {
        Long64_t ientry = LoadTree(jentry);
        if (ientry < 0) break;
        nb = fChain->GetEntry(jentry);   nbytes += nb;
        
        //
        // process the time information.
        //
        if (jentry == 0) {
            tstart = time;
            // SET THE MINIMUM TIME OF THE FIRST INTERVAL
            _cal_tmin = time;
        }
        time_since_start = time - tstart;
        if (jentry == 0) t0 = time_since_start;
        
        // fill the hisotgrams with 'good' events
        channel = channel % 100;
        
        //
        // fill the histogram
        //
        if(error == 0 && !istestpulse) _integral[channel]->Fill(integral);
        
        if(time_since_start - t0 > TIME_INTERVAL) {
            // maximum validity time of this calibration....
            _cal_tmax = time;
            //
            // do the calibration
            //
            do_calibration();
            //
            // file the output tree
            //
            fill_tree(_cal_tmin,_cal_tmax);
            //
            // reset the histograms
            //
            reset_histograms();
            //
            // reset the time for the start of the next interval
            //
            t0 = time_since_start;
            // SET THE MINIMUM TIME OF THE NEXT INTERVAL
            _cal_tmin = time;
        }
        if(jentry%100000 == 0) cout<<"Processed "<<jentry<<" events"<<endl;
    }
    //
    // if a run is very short we need to make a calibration anyhow on the available data
    //
    if(time_since_start < TIME_INTERVAL) do_calibration();
    
    //
    // if the last bit of data dont have their own calibration, use the previous
    //
    fill_tree(_cal_tmin,9e99);
    
    _f->Write();
    _f->Close();
}
/*---------------------------------------------------------------------------------------------------*/
void ecal::reset_histograms(){
    //
    // the histograms need to be reset for the next calibration period
    //
    for(int ich=0; ich<NUMBER_OF_CHANNELS; ich++)
        _integral[ich]->Reset();
    
}
/*---------------------------------------------------------------------------------------------------*/
void ecal::ecal_single(){
    //
    // energy calibration for modulation detectors
    //
    if (fChain == 0) return;
    //
    // book the histograms needed for the energy calibration
    //
    book_histograms();
    //
    // loop over the events and fill histograms: before calibration
    //
    fill_histograms(BEFORE_CALIBRATION);
    
    //
    // do the actual calibration
    //
    do_calibration();
    
    //
    // write the output parameters to TParameters (legacy)
    //
    char parname[100];
    for(int ich=0; ich<NUMBER_OF_CHANNELS; ich++){
        for(int ipar=0; ipar<MAX_PARAMETERS; ipar++){
            sprintf(parname,"cal_ch%02d_c%i",ich,ipar);
            TParameter <double> *p1 = new TParameter<double>(parname,ccal[ich][ipar]);
            p1->Write();
        }
    }
    //
    // write the ouptput parameters to the TTree
    //
    fill_tree(0,9e99); // valid for teh ful run
    
    //
    // loop over the events and fill histograms: post-calibration
    //
    fill_histograms(AFTER_CALIBRATION);
    
    
    _f->Write();
    _f->Close();
    
}

/*---------------------------------------------------------------------------------------------------*/
void ecal::fill_tree(Double_t t0, Double_t t1){
    //
    // validity range of this calibration
    //
    _cal_tmin = t0;
    _cal_tmax = t1;
    
    //
    // fill the tree variables (the time window variables are set earlier....
    //
    for(int ich = 0; ich<NUMBER_OF_CHANNELS; ich++){
        _cal_c0[ich] = ccal[ich][0];
        _cal_c1[ich] = ccal[ich][1];
        _cal_c2[ich] = ccal[ich][2];
        _cal_chi2[ich] = cal_quality[ich];
    }
    
    _cal_index++;
    //
    // write to tree
    //
    _cal_tree->Fill();
    
}

/*---------------------------------------------------------------------------------------------------*/
void ecal::do_calibration(){
    //
    // Fit the calibration parameters from the spectrum
    //
    // Strategy: (i) if there is only one photo peak the calibration is a simple proportionality constant
    //           (ii) fit a 1st order polynomial if there are multiple photo peaks identified
    //
    // AP
    //
    //    TCanvas *cv1 = new TCanvas("cv1","cv1",600,400);
    //    c1->Draw();
    //    int huh;
    char parname[100];
    for(int ich=0; ich<NUMBER_OF_CHANNELS; ich++){
        ccal[ich][0] = 0;
        ccal[ich][1] = 1;
        ccal[ich][2] = 0;
        //
        // only calibrate Active channels!
        //
        if(channel_active[ich]){
            cout <<"do_calibration:: CHANNEL = "<<ich<<endl;
            // find the number of peaks to calibrate on.....
            int npeak = 0;
            // the source identifier for this channel
            int id = source_id[ich];
            
            for (int i=0; i<MAX_PEAKS; i++){
                if(source_energy[id][i]>0) npeak++;
            }
            
            /// if(npeak == 0){ // no calibration possible ..... E = 1.0*integral.
            // if the calibration fails I want a simple linear realtion between integral and energy
            /// }
            
            // STEP1: find the highest peak
            // define the proper range for searching the calibration constant
            _integral[ich]->GetXaxis()->SetRangeUser(range_low[ich],range_high[ich]);
            // find the bin with maximum value in the range
            int ibin      = _integral[ich]->GetMaximumBin();
            double val    = _integral[ich]->GetBinCenter(ibin);
            double maxval = _integral[ich]->GetBinContent(ibin);
            
            
            Double_t ee[MAX_PEAKS];
            Double_t dee[MAX_PEAKS];
            Double_t Vs_peak[MAX_PEAKS];
            Double_t dVs_peak[MAX_PEAKS];
            Double_t chi2_fit_av=0;
            
            Double_t vlow,vhigh;
            for(int ipeak = 0; ipeak<npeak; ipeak++){
                //
                // Rough estimate of the expected resolution
                //
                Double_t sig_expected = 1.5/sqrt(source_energy[id][ipeak])/2.35;
                //
                // fit ranges: if you are the first peak, then we use as start value for
                // the mean the maximum bin as found above. otherwise we scale the
                // starting point from the energy peak we found before in the first fit
                //
                if (ipeak != 0) val = Vs_peak[0]*source_energy[id][ipeak]/source_energy[id][0];
                sig_expected = sig_expected*val; // scale resolution to uncalibrated energy
                
                vlow  = val - 5*sig_expected;//0.025e-6;
                vhigh = val + 5*sig_expected;//0.025e-6;
                
                if(ich == 4 || ich ==5){
                    if(ipeak == 0 ) {
                        vhigh = val+3*sig_expected;//0.015e-6;
                    }
                    if(ipeak == 1 ) vlow  = val-3*sig_expected;//0.015e-6;
                }
                
                //
                // fit a Gauss around the desired location in the spectrum
                //
                
                // starting value for the height
                _integral[ich]->GetXaxis()->SetRangeUser(vlow,vhigh);
                ibin   = _integral[ich]->GetMaximumBin();
                maxval = _integral[ich]->GetBinContent(ibin);
                _integral[ich]->GetXaxis()->SetRangeUser(0.,1.);
                
                
                // do the fit
                TF1 *func = new TF1("fit",fitf_gauss,vlow,vhigh,5);
                func->SetParameters(maxval,val,sig_expected);
                func->SetParNames("C","mean","sigma");
                _integral[ich]->Fit("fit","Q","",vlow,vhigh);
                //            if(ich==2 || ich==4){
                //              cout << ich<<" " <<ipeak <<" chi2 = "<<func->GetChisquare() / func->GetNDF()<<endl;
                //              _integral[ich]->Draw();
                //              cv1->Update();
                //              cin>>huh;
                //            }
                
                //
                // get the parameters
                //
                ee[ipeak]       = source_energy[id][ipeak];
                dee[ipeak]      = 0.;
                Vs_peak[ipeak]  = func->GetParameter(1);
                dVs_peak[ipeak] = func->GetParError(1);
                chi2_fit_av    += func->GetChisquare() / func->GetNDF();
            }
            
            //
            // * calculate the calibration parameters.
            // * distinct case when fit is possible (npeak > 1)and when it is not (npeak == 1).
            //
            if (npeak == 1){ // just one point... so the energy calibration is a simple proportionality
                ccal[ich][0] = 0;
                ccal[ich][1] = source_energy[id][0] / Vs_peak[0];
                ccal[ich][2] = 0;
            } else if(npeak > 1){ // more than one point... now we fit a straight line
                //
                // Fit a 1st order polynomial to the integral vs energy (inverse calibration). Done
                // like this because I have the errors on the area, and the energy points are
                // infinitely precise - just the photo peak energy values.
                //
                TGraphErrors *gcal = new TGraphErrors(npeak,ee,Vs_peak,0,dVs_peak);
                gcal->Fit("pol1");
                TF1 *ff = gcal->GetFunction("pol1");
                // get the parameters.... but remember I have fitted the inverse calibration here
                // so now I need to invert as well....
                //   integral = v0+v1*E  -> E = -v0/v1 + 1/v1 * integral
                Double_t v0 = ff->GetParameter(0);
                Double_t v1 = ff->GetParameter(1);
                // construct the calibration constants
                ccal[ich][0] = -v0/v1;
                ccal[ich][1] = 1/v1;
                ccal[ich][2] = 0;
                
                
                // // // TEST TEST 2ND order polynomial - errors are bad...... but representative
                ////TGraphErrors *gcal2 = new TGraphErrors(npeak,Vs_peak,ee,0,dVs_peak);
                ////gcal2->Fit("pol2");
                ////TF1 *gg = gcal2->GetFunction("pol2");
                ////ccal[ich][0] = gg->GetParameter(0);
                ////ccal[ich][1] = gg->GetParameter(1);
                ////ccal[ich][2] = gg->GetParameter(2);
                
            }
            cal_quality[ich] = chi2_fit_av/npeak;
        }
    }
}
/*---------------------------------------------------------------------------------------------------*/
