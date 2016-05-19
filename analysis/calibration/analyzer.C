#define analyzer_cxx
/*---------------------------------------------------------------------------------------------------*/
//
// analyzer.C Routine to analyze spectra and calculate the rate of the sources
//
// Usage:
//  prompt> #include "analyzer.C"
//  prompt> analyzer ana(<directory_with_energy_calibrated_rootfiles>,<analysis_output_rootfile>)
//  prompt> ana.Loop()
//
// To inspect the fit results it is possible to plot the fit results. In analyzer.h  set:
// #define PLOT_ON_SCREEN 1
//
// To set the time interval in which a spectrum is calculated, in analyzer.h set:
// #define TIME_INTERVAL <interval_in_seconds>
//
// A.P.
/*---------------------------------------------------------------------------------------------------*/
#include "analyzer.h"
// RooFit include files
#include <RooRealVar.h>
#include <RooDataHist.h>
#include <RooHistPdf.h>
#include <RooGaussian.h>
#include <RooAddPdf.h>
#include <RooPlot.h>
#include <RooFitResult.h>
#include <RooExtendPdf.h>
//
#include <TH2.h>
#include <TStyle.h>
#include <TCanvas.h>
#include <TGraph.h>
#include <TVector.h>
#include <TParameter.h>
#include <TMatrixDSym.h>
#include <iostream>
//#include <vector>
//#include <numeric>
#include <stdio.h>
#include <TMath.h>
#include <TF1.h>

#define INDEX_TEMPERATURE 24
#define MAX_INDEX 400

using namespace RooFit;
#define MAX_PEAKS 5

/*---------------------------------------------------------------------------------------------------*/
float source_energy[NUMBER_OF_SOURCES][MAX_PEAKS] =
//
// the energy peaks you wish to select for the analysis should be in this list
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

//
// ranges for plotting
//
const int   nbin0 = 600;
// number of bins for the temporary fit histograms.... channels 0+1 have few entries so wider bins
float nbin[NUMBER_OF_CHANNELS]={nbin0/4,nbin0/4,nbin0,nbin0,nbin0,nbin0,nbin0,nbin0};
const float emin = 0.; // in keV
const float emax = 3000.; // in keV
const float adc_max_volt = 2.;
const float base_max_val = 2000;


/*----------------------------------------------------------------------------------------------------*/

//
// Gaussian function + 2nd order polynomial for simple rate fitting
//
Double_t fitf(Double_t *v, Double_t *par)
{
    Double_t arg = 0;
    if (par[2] != 0) arg = (v[0] - par[1])/par[2];
    
    Double_t fitval = par[0]*TMath::Exp(-0.5*arg*arg);
    fitval += par[3] + par[4]*v[0] + par[5]*v[0]*v[0];
    
    return fitval;
}
/*----------------------------------------------------------------------------------------------------*/
void analyzer::fit_spectrum(int ichannel, double *fit_range){
    //
    // RooFit based spectrum fitter
    //
    cout <<"analyzer::fit_spectrum  channel = "<<ichannel<<endl;
    
    // get the source ID
    int id = source_id[ichannel];
    
    //
    // identify the peaks in our specified energy range and store their peak ID.
    //
    int nselect = 0;
    int peak_id[MAX_PEAKS];
    for(int ipeak = 0; ipeak<MAX_PEAKS; ipeak++){
        double epeak = source_energy[id][ipeak];
        if ( (epeak > fit_range[0]) && (epeak < fit_range[1])){ // yes the peak is in range
            peak_id[nselect] = ipeak;
            nselect++;
        }
    }
    cout <<"analyzer::fit_spectrum found " <<nselect<<" peaks from "<<fit_range[0]
    <<" keV < E < "<<fit_range[1]<<" keV"<<endl;
    
    //
    // no peaks have been found..... leave fittng routine
    //
    if(nselect == 0) return;
    
    //
    // spectrum is a function of the energy
    //
    RooRealVar E("E","E (keV)",emin,emax);
    
    //
    // the background template for each of the sources obtained from a GEANT4 simulation
    //
    string mc_file="";
    if       (id == TI44){
        mc_file = "MC_ti44_modulation.root";
    } else if(id == CO60){
        mc_file = "MC_co60_modulation.root";
    } else if(id == CS137){
        mc_file = "MC_cs137_modulation.root";
    } else if(id == MN54){
        mc_file = "MC_mn54_modulation.root";
    } else if(id == K40){
        mc_file = "MC_k40_modulation.root";
    } else {
        cout <<"fit_spectrum:: BAD source identifier"<<endl;
    }
    
    cout <<"fit_spectrum:: channel = "<<ichannel<<" source_id = "<<id<<" MC template ="<<mc_file<<endl;
    
    TFile *f_mc = new TFile(mc_file.c_str(),"READONLY");
    TH1* h_bg  = (TH1*)f_mc->Get("h2");
    //
    // construct the background pdf
    //
    RooDataHist mc1("mc1","mc1",RooArgList(E),h_bg);
    f_mc->Close();
    RooHistPdf bg("bg","bg",E,mc1,0);
    
    //
    // define the Gaussians for the photo peaks
    //
    cout <<"analyzer::fit_spectrum Define photo peak Gaussians"<<endl;
    std::vector<RooRealVar*> pk_mean;
    std::vector<RooRealVar*> pk_sigma;
    std::vector<RooRealVar*> pk_frac;
    std::vector<RooGaussian*> pk_gaus;
    
    std::vector<RooRealVar*> pk_frac_tail;
    std::vector<RooRealVar*> pk_sigma_tail;
    std::vector<RooGaussian*> pk_gaus_tail;
    
    
    char vname[128];
    for (int isel=0; isel<nselect; isel++){
        int ipeak = peak_id[isel];
        double epeak = source_energy[id][ipeak];
        cout <<"fit_spectrum:: ichannel = "<<ichannel <<" ipeak = "<<ipeak<<" epeak = "<<epeak<<endl;
        sprintf(vname,"mean%i",isel);
        pk_mean.push_back(new RooRealVar(vname,vname,epeak,epeak-50,epeak+50));
        sprintf(vname,"sigma%i",isel);
        pk_sigma.push_back(new RooRealVar(vname,vname,25,5,100));
        sprintf(vname,"frac%i",isel);
        pk_frac.push_back(new RooRealVar(vname,vname,0.2,0.0,1.0));
        
        sprintf(vname,"gaus%i",isel);
        pk_gaus.push_back(new RooGaussian(vname,vname,E,*pk_mean[isel],*pk_sigma[isel]));
        //
        sprintf(vname,"frac_tail%i",isel);
        pk_frac_tail.push_back(new RooRealVar(vname,vname,0.01,0.0,0.1));
        sprintf(vname,"sigma_tail%i",isel);
        pk_sigma_tail.push_back(new RooRealVar(vname,vname,50,5,100));
        sprintf(vname,"gaus_tail%i",isel);
        pk_gaus_tail.push_back(new RooGaussian(vname,vname,E,*pk_mean[isel],*pk_sigma_tail[isel]));
        
    }
    cout <<"analyzer::fit_spectrum Define photo peak Gaussians ---- DONE"<<endl;
    //
    // normalization for the pdf will be a sperately fitted parameter
    //
    RooRealVar Norm("Norm","Normalization",1e6,0.,1e12);
    
    //
    // compose the joined pdf for background + signal
    //
    
    cout <<"analyzer::fit_spectrum Compose the combined pdf"<<endl;
    
    RooAddPdf *sum;
    if (nselect==1)  sum = new RooAddPdf("sum","g1+bg",RooArgList(*pk_gaus[0],bg),RooArgList(*pk_frac[0]));
    //      if(peak_id[0] == 2) {
    //         sum = new RooAddPdf("sum","g1+g2+bg",RooArgList(*pk_gaus[0],*pk_gaus_tail[0],bg),RooArgList(*pk_frac[0],*pk_frac_tail[0]));
    //      } else {
    //         sum = new RooAddPdf("sum","g1+bg",RooArgList(*pk_gaus[0],bg),RooArgList(*pk_frac[0]));
    //      }
    //    }
    if (nselect==2)  sum = new RooAddPdf("sum","g1+g2+bg",RooArgList(*pk_gaus[0],*pk_gaus[1],bg),RooArgList(*pk_frac[0],*pk_frac[1]));
    if (nselect==3)  sum = new RooAddPdf("sum","g1+g2+g3+bg",RooArgList(*pk_gaus[0],*pk_gaus[1],*pk_gaus[2],bg),RooArgList(*pk_frac[0],*pk_frac[1],*pk_frac[2]));
    
    cout <<"analyzer::fit_spectrum Compose the combined pdf ---- DONE "<<endl;
    
    E.setRange("signalRange",emin,emax);//fit_range[0],fit_range[1]);
    RooExtendPdf esum("esum","extended pdf with Norm",*sum,Norm,"signalRange");
    
    cout <<"analyzer::fit_spectrum Compose the combined pdf ---- extended DONE "<<endl;
    //
    // get the data from a TH1 root histogram
    //
    RooDataHist data("data","data",RooArgList(E),(TH1*)_pk_tmp[ichannel]);
    //
    // fit the pdf to the data
    //
    //RooFitResult *fr = esum.fitTo(data,Extended(kTRUE),Range(fit_range[0],fit_range[1]),Save());
    fr = esum.fitTo(data,Extended(kTRUE),Range(fit_range[0],fit_range[1]),Save());
    fr->Print();
    //
    // process the fitted variables and store in the output tree
    //
    for (int id=0; id<nselect; id++){
        //
        // covariance matrix elements for calculation of error on rate
        //
        processFitData(Norm,*pk_frac[id],*pk_mean[id],*pk_sigma[id], ichannel,peak_id[id]);
    }
    
    //
    // draw the fit results..... not for batch running. Set PLOT_ON_SCREEN variable to 0 in analyzer.h file
    //
    if(PLOT_ON_SCREEN){
        TCanvas *c1 = new TCanvas("c1","c1",600,400);
        //
        // plot the data with the fittted function
        //
        RooPlot *Eframe = E.frame();
        Eframe->SetTitle("");
        Eframe->GetXaxis()->SetRangeUser(fit_range[0],fit_range[1]);
        
        //
        // find maximum data value in plot range
        //
        _pk_tmp[ichannel]->GetXaxis()->SetRangeUser(fit_range[0],fit_range[1]);
        Int_t maxbin  = _pk_tmp[ichannel]->GetMaximumBin();
        Double_t maxval  = _pk_tmp[ichannel]->GetBinContent(maxbin);
        
        //
        // plot the data and pdfs. use the plot range as found before
        //
        data.plotOn(Eframe);
        esum.plotOn(Eframe);
        Eframe->SetMaximum(1.2*maxval);
        c1->SetLogy(1);
        
        for (int id=0; id<nselect; id++){
            esum.plotOn(Eframe,Components(*pk_gaus[id]),LineColor(2),LineWidth(2));
            pk_gaus[id]->paramOn(Eframe,Layout(0.55,0.88,0.85-id*0.15));
        }
        esum.plotOn(Eframe,Components(bg),LineColor(kGreen),LineWidth(2));
        
        cout << Eframe->chiSquare() <<endl;
        Eframe->Draw();
        
        c1->Update();
        
        int huh;
        cin>>huh;
    }
    //
    // cleanup
    //
    delete sum;
    delete fr;
}

/*----------------------------------------------------------------------------------------------------*/
void analyzer::fit_spectrum(int ichannel){
    //
    // RooFit based spectrum fitter
    //
    //
    cout <<"analyzer::fit_spectrum  channel = "<<ichannel<<endl;
    
    // get the souce id
    int id = source_id[ichannel];
    
    //
    // spectrum is a function of the energy
    //
    RooRealVar E("E","E (keV)",emin,emax);
    
    //
    // the background template for each of the sources obtained from a GEANT4 simulation
    //
    string mc_file="";
    if       (id == TI44){
        mc_file = "/user/z37/Modulation/analysis/calibration/MC_ti44_modulation.root";
    } else if(id == CO60){
        mc_file = "/user/z37/Modulation/analysis/calibration/MC_co60_modulation.root";
    } else if(id == CS137){
        mc_file = "/user/z37/Modulation/analysis/calibration/MC_cs137_modulation.root";
    } else if(id == MN54){
        mc_file = "/user/z37/Modulation/analysis/calibration/MC_mn54_modulation.root";
    } else if(id == K40){
        mc_file = "/user/z37/Modulation/analysis/calibration/MC_k40_modulation.root";
    } else {
        cout <<"fit_spectrum:: BAD source identifier"<<endl;
    }
    
    cout <<"fit_spectrum:: channel = "<<ichannel<<" source_id = "<<id<<" MC template ="<<mc_file<<endl;
    
    TFile *f_mc = new TFile(mc_file.c_str(),"READONLY");
    TH1* h_bg  = (TH1*)f_mc->Get("h2");
    //
    // construct the background pdf
    //
    RooDataHist mc1("mc1","mc1",RooArgList(E),h_bg);
    f_mc->Close();
    RooHistPdf bg("bg","bg",E,mc1,0);
    
    //
    // first Gauss for first photo-peak ....
    //
    Double_t Eval = source_energy[id][0];
    RooRealVar mean1("mean1","mean of gaussian 1",Eval,Eval-50,Eval+50);
    RooRealVar sigma1("sigma1","width of gaussians",25,5.,50.) ;
    RooRealVar g1frac("g1frac","fraction of gauss1",0.2,0.0,1.0) ;
    RooGaussian gauss1("gauss1","gaussian PDF",E,mean1,sigma1) ;
    // second Gauss....
    Eval = source_energy[id][1];
    RooRealVar mean2("mean2","mean of gaussian 2",Eval,Eval-50,Eval+50);
    RooRealVar sigma2("sigma2","width of gaussians",25,5.,100.) ;
    RooRealVar g2frac("g2frac","fraction of gauss2",0.2,0.,1.0) ;
    RooGaussian gauss2("gauss2","gaussian PDF",E,mean2,sigma2) ;
    // third Gauss
    Eval = source_energy[id][2];
    RooRealVar mean3("mean3","mean of gaussian 2",Eval,Eval-50,Eval+50);
    RooRealVar sigma3("sigma3","width of gaussians",25,5.,100.) ;
    RooRealVar g3frac("g3frac","fraction of gauss3",0.05,0.0,1.0) ;
    RooGaussian gauss3("gauss3","gaussian PDF",E,mean3,sigma3) ;
    
    //
    // normalization for the pdf will be a sperately fitted parameter
    //
    RooRealVar Norm("Norm","Normalization",1e6,0.,1e12);
    
    //
    // compose the joined pdf for background + signal
    //
    double fit_range[2];
    RooAddPdf *sum;
    if       (ichannel == 2 || ichannel == 3 ){
        // fit range
        fit_range[0] = 400;
        fit_range[1] = 1800;
        // pdf
        sum = new RooAddPdf("sum","g1+g2+g3+bg",RooArgList(gauss1,gauss2,gauss3,bg),RooArgList(g1frac,g2frac,g3frac));
    } else if(ichannel == 4 || ichannel == 5 ){
        // fit range
        fit_range[0] = 800;
        fit_range[1] = 2800;
        // pdf
        sum = new RooAddPdf("sum","g1+g2+g3+bg",RooArgList(gauss1,gauss2,gauss3,bg),RooArgList(g1frac,g2frac,g3frac));
    } else if(ichannel == 6 || ichannel == 7 ){
        // fit range
        fit_range[0] = 400;
        fit_range[1] = 1000;
        // pdf
        sum = new RooAddPdf("sum","g1+bg",RooArgList(gauss1,bg),RooArgList(g1frac));
    }
    E.setRange("signalRange",emin,emax);//fit_range[0],fit_range[1]);
    RooExtendPdf esum("esum","extended pdf with Norm",*sum,Norm,"signalRange");
    
    //
    // get the data from a TH1 root histogram
    //
    RooDataHist data("data","data",RooArgList(E),(TH1*)_pk_tmp[ichannel]);
    //
    // fit the pdf to the data
    //
    //RooFitResult *fr = esum.fitTo(data,Extended(kTRUE),Range(fit_range[0],fit_range[1]),Save());
    fr = esum.fitTo(data,Extended(kTRUE),Range(fit_range[0],fit_range[1]),Save());
    //
    // process the variables to rates
    //
    processFitData(Norm,g1frac,mean1,sigma1,ichannel,0);
    if(ichannel == 2 || ichannel ==3 || ichannel ==4 || ichannel == 5){
        processFitData(Norm,g2frac,mean2,sigma2,ichannel,1);
        processFitData(Norm,g3frac,mean3,sigma3,ichannel,2);
    }
    //
    // cleanup
    //
    delete sum;
    delete fr;
}

/*----------------------------------------------------------------------------------------------------*/
void analyzer::processFitData(RooRealVar N, RooRealVar f, RooRealVar E, RooRealVar sig, int ichannel, int ipeak){
    //
    // process the fit data in order to get the rate with errors etc into the ntuple
    //
    Double_t E1   = E.getValV();
    Double_t Norm = N.getValV();
    Double_t frac = f.getValV();
    //    Double_t R1   = Norm*frac/TIME_INTERVAL;
    Double_t R1   = Norm*frac/delta_t;
    //
    // Get covariance matrix elements. We calculate Rate = frac*Norm / delta_t, so
    //
    // dRate = sqrt(frac**2*cov(0,0) + 2*frac*Norm*cov(idx,0) + Norm**2*cov(idx,idx))/delta_t
    //
    // The idx should point to teh right element in the covariance matrix:
    //     frac0 -> idx=1
    //     frac1 -> idx=2
    //     frac2 -> idx=3
    //     etc etc
    //
    // Be carfull to change the order of the elements in teh covariance matrix: maybe there is a way to reference
    // by name as well.....
    //
    int idx = 0;
    string fName = f.GetName();
    //    cout <<">>"<<fName<<"<<"<<endl;
    if        (fName == "frac0") {
        idx = 1;
    } else if (fName == "frac1"){
        idx = 2;
    } else if (fName == "frac2"){
        idx = 3;
    }
    //   cout <<" cov00 = "<<covariance(0,0)<<" cov01 = "<<covariance(idx,0)<<" cov11 = "<<covariance(idx,idx)<<endl;
    Double_t dR1 = Norm*Norm*covariance(idx,idx);
    dR1 += 2*Norm*frac*covariance(idx,0);
    dR1 +=   frac*frac*covariance(0,0);
    dR1 = sqrt(dR1)/delta_t;
    //
    // calculate error on the rate
    //
    Double_t res  = 2.355*sig.getValV()/E1;
    cout <<ichannel<<" "<<ipeak<<" "<<E1<<" "<<res<<" R = "<<R1<<" +- "<<dR1<<endl;
    addTreeEntry(E1,R1,dR1,res,ichannel,ipeak);
}

/*----------------------------------------------------------------------------------------------------*/
void analyzer::addTreeEntry(Double_t E, Double_t R, Double_t dR, Double_t res, Int_t ich, Int_t ipk){
    //
    // fill the tree with the fit results.
    //
    // the slow data are processed elsewhere, but are also entering this tree:)
    //
    _t_energy = E;
    _t_rate   = R;
    _t_drate  = dR;
    _t_res    = res;
    _t_chanNum = ich;
    _t_peakNum = ipk;
    
    // this should be the only place where the fill command is called
    tree->Fill();
}
/*----------------------------------------------------------------------------------------------------*/
double analyzer::covariance(int i, int j){
    TMatrixDSym cov = fr->covarianceMatrix();
    double cc = cov[i][j];
    return cc;
}
/*----------------------------------------------------------------------------------------------------*/
void analyzer::book_histograms(){
    _f = new TFile(analyzer_file.c_str(),"RECREATE");
    
    char hname[128];
    // book histograms
    for (int ich = 0; ich<NUMBER_OF_CHANNELS; ich++){
        // spectra
        sprintf(hname,"_e_all_ch%1d",ich);
        _e_all.push_back(new TH1F(hname,hname,nbin0,emin,emax));
        sprintf(hname,"_e_good_ch%1d",ich);
        _e_good.push_back(new TH1F(hname,hname,nbin0,emin,emax));
        sprintf(hname,"_e_err1_ch%1d",ich);
        _e_err1.push_back(new TH1F(hname,hname,nbin0,emin,emax));
        sprintf(hname,"_e_err2_ch%1d",ich);
        _e_err2.push_back(new TH1F(hname,hname,nbin0,emin,emax));
        sprintf(hname,"_e_err4_ch%1d",ich);
        _e_err4.push_back(new TH1F(hname,hname,nbin0,emin,emax));
        // baseline
        sprintf(hname,"_b_good_ch%1d",ich);
        _b_good.push_back(new TH1F(hname,hname,nbin0,0,base_max_val));
        sprintf(hname,"_b_err1_ch%1d",ich);
        _b_err1.push_back(new TH1F(hname,hname,nbin0,0,base_max_val));
        sprintf(hname,"_b_err2_ch%1d",ich);
        _b_err2.push_back(new TH1F(hname,hname,nbin0,0,base_max_val));
        sprintf(hname,"_b_err4_ch%1d",ich);
        _b_err4.push_back(new TH1F(hname,hname,nbin0,0,base_max_val));
        // integral vs peak
        sprintf(hname,"_h_vs_E_good_ch%1d",ich);
        _2d_good.push_back(new TH2F(hname,hname,nbin0,emin,emax,nbin0,0.,adc_max_volt));
        sprintf(hname,"_h_vs_E_err1_ch%1d",ich);
        _2d_err1.push_back(new TH2F(hname,hname,nbin0,emin,emax,nbin0,0.,adc_max_volt));
        sprintf(hname,"_h_vs_E_err2_ch%1d",ich);
        _2d_err2.push_back(new TH2F(hname,hname,nbin0,emin,emax,nbin0,0.,adc_max_volt));
        sprintf(hname,"_h_vs_E_err4_ch%1d",ich);
        _2d_err4.push_back(new TH2F(hname,hname,nbin0,emin,emax,nbin0,0.,adc_max_volt));
        
        // temporary histograms for stability measurements
        sprintf(hname,"_pk_tmp%1d",ich);
        _pk_tmp.push_back(new TH1F(hname,hname,nbin[ich],emin,emax));
    }
    
    // temporary histogram for temperature measurements
    //_T = new TH1F("T","T",1000,10+273.15,40+273.15);
    
    // Define tree and branches
    tree = new TTree("ana", "Analyzed spectra");
    
    tree->Branch("t0", &_t_t0, "t0/D");
    tree->Branch("time", &_t_time, "time/D");
    tree->Branch("channel", &_t_chanNum, "channel/I");
    tree->Branch("peak", &_t_peakNum, "peak/I");
    tree->Branch("rate", &_t_rate, "rate/D");
    tree->Branch("drate", &_t_drate, "drate/D");
    tree->Branch("e", &_t_energy, "e/D");
    tree->Branch("res", &_t_res, "res/D");
    tree->Branch("temp", &_t_temp, "temp/D");
    tree->Branch("pres", &_t_pres, "pres/D");
    tree->Branch("bx", &_t_bx, "bx/D");
    tree->Branch("by", &_t_by, "by/D");
    tree->Branch("bz", &_t_bz, "bz/D");
    tree->Branch("btot", &_t_btot, "btot/D");
    tree->Branch("humid", &_t_humid, "humid/D");
}
/*----------------------------------------------------------------------------------------------------*/
void analyzer::fill_histograms(){
    // fill all the histograms
    
    channel = channel % 100 ;
    //_T->Fill(temp+273.15);
    
    _e_all[channel]->Fill(integral);
    if      (error == 0) {
        _pk_tmp[channel]->Fill(integral);
        _e_good[channel]->Fill(integral);
        _b_good[channel]->Fill(baseline);
        
        _2d_good[channel]->Fill(integral,height);
    }
    else if ((error&0x01)!=0) {
        _e_err1[channel]->Fill(integral);
        _b_err1[channel]->Fill(baseline);
        
        _2d_err1[channel]->Fill(integral,height);
    }
    else if ((error&0x02)!=0) {
        _e_err2[channel]->Fill(integral);
        _b_err2[channel]->Fill(baseline);
        
        _2d_err2[channel]->Fill(integral,height);
    }
    else if ((error&0x04)!=0) {
        // temporary error handling: err4 is not a real error yet
        _e_err4[channel]->Fill(integral);
        _b_err4[channel]->Fill(baseline);
        
        _2d_err4[channel]->Fill(integral,height);
    }
}
/*----------------------------------------------------------------------------------------------------*/
void analyzer::get_interval_data(){
    //
    // analyze the data from a time interval.
    // length of interval set by TIME_INTERVAL which can be set in header file
    //
    
    // time
    _t_t0    = tstart;
    _t_time  = (t0+time_since_start)/2.;
    
    cout<<"analyzer::get_interval_data:: time_since_start ="<<time_since_start<<endl;
    
    double range[2] = {0,3000};
    for(int ich=0; ich<NUMBER_OF_CHANNELS; ich++){
        
        // only analyze active channels
        if(channel_active[ich]){
            
            //
            // if we have a nice MC background model we use it to fit the spectrum
            // if we don't have a nice model we will do a simple fit instead
            //
            
            // which is the source?
            int id = source_id[ich];
            if        (id == TI44) {
                range[0] = 400; range[1] = 620;
                fit_spectrum(ich, range);
                range[0] = 1000; range[1] = 1300;
                fit_spectrum(ich, range);
                range[0] = 1500; range[1] = 2000;
                fit_spectrum(ich, range);
            } else if (id == CO60 ) {
                range[0] = 900; range[1] = 1600;
                fit_spectrum(ich, range);
                range[0] = 2200; range[1] = 2800;
                fit_spectrum(ich, range);
            } else if (id == CS137 ) {
                range[0] = 400; range[1] = 1000;
                fit_spectrum(ich, range);
            } else if (id == MN54) {
                range[0] = 0; range[1] = 2000;
                fit_spectrum(ich, range);
            } else if (id == K40) {
                range[0] = 1300; range[1] = 1600;
                fit_spectrum(ich, range);
            } else {
                //
                // if we deal with the background detectors we use a linear fit + gauss to fit the signal
                //
                fit_spectrum_simple(ich);
            }
        }
        _pk_tmp[ich]->Reset(); // reset the histogram
    } // loop over channels
}
/*----------------------------------------------------------------------------------------------------*/
void analyzer::fit_spectrum_simple(int ichannel){
    //    int huh;
    //    TCanvas *c1 = new TCanvas("c1","c1",600,400);
    //    int huh;
    
    // source id
    int id = source_id[ichannel];
    
    //
    // find all the selected energy peaks
    //
    int      maxbin;
    double   maxval;
    Double_t e_start, e0, demin, demax;
    for (int ipeak=0; ipeak<MAX_PEAKS; ipeak++){
        if(source_energy[id][ipeak] >0){
            //
            // find the fit starting values
            //
            
            //
            // first peak is special.... we use the GetMaximumBin() method in order
            // to find this peak even if there is a shift in gain!
            //
            
            
            if (ipeak != 0 ) {
                // get the position where the peak should be... according to the first fit
                e_start = e0*source_energy[id][ipeak] / source_energy[id][0];
                _pk_tmp[ichannel]->GetXaxis()->SetRangeUser(e_start-100,e_start+100);
            } else {
                // special care for channel 0 & channel 1: these tend to have a high background at low energy!
                if (ichannel == 0 || ichannel ==1){
                    e_start = source_energy[id][0];
                    _pk_tmp[ichannel]->GetXaxis()->SetRangeUser(e_start-100,e_start+100);
                }
            }
            
            maxbin  = _pk_tmp[ichannel]->GetMaximumBin();
            maxval  = _pk_tmp[ichannel]->GetBinContent(maxbin);
            e_start = _pk_tmp[ichannel]->GetBinCenter(maxbin);
            
            _pk_tmp[ichannel]->GetXaxis()->SetRangeUser(0.,3000.);
            //           _pk_tmp[ichannel]->Draw();
            
            //
            // fit a Gauss + background to a photopeak
            //
            TF1 *func = new TF1("fit",fitf,e_start-200,e_start+200,5);
            Double_t res_start = 0.06/2.35*sqrt(662.)*sqrt(e_start);
            func->SetParameters(maxval,e_start,res_start);
            func->SetParNames("C","mean","sigma");
            
            Double_t demin = 100;
            Double_t demax = 100;
            
            if(ichannel == 4 || ichannel == 5) { // 60Co has some peaks close to each other
                if(ipeak == 0) demax = 75;
                if(ipeak == 1) demin = 75;
            } else if (ichannel == 0 || ichannel == 1){
                demin = 200;
                demax = 200;
            }
            
            
            Double_t e_low  = e_start - demin;
            Double_t e_high = e_start + demax;
            
            _pk_tmp[ichannel]->Fit("fit","Q","",e_low,e_high);
            
            Double_t peak        = func->GetParameter(0);
            _t_energy            = func->GetParameter(1);
            if(ipeak ==0) e0 = _t_energy;
            Double_t sigma       = func->GetParameter(2);
            _t_res = 0;
            if(_t_energy>0) _t_res = 2.355*sigma/_t_energy ;
            
            Double_t bin_width = (emax-emin)/nbin[ichannel];
            _t_rate = TMath::Sqrt(2*TMath::Pi())*sigma*peak / delta_t / bin_width;
            cout <<"get_interval_data:: ich ="<<ichannel<<" ipeak = "<<ipeak
            <<" E = "<<_t_energy<<" keV  rate = "<<_t_rate<<" Hz  resolution  = "<<_t_res<<" % "<<endl;
            
            // fille the output tree.....
            addTreeEntry(_t_energy,_t_rate,1.0,_t_res,ichannel,ipeak);
            //            c1->Update();
            
            delete func;
        } // loop over peaks
    }
}
/*----------------------------------------------------------------------------------------------------*/
void analyzer::write_histograms(){
    //
    // write a few parameters to file
    //
    _f->cd();
    TNamed *Parameter = new TNamed("run",run.c_str());
    Parameter->Write();
    TParameter<Double_t> * tstartPar = new TParameter<Double_t>("t0",tstart);
    tstartPar->Write();
    TParameter<Double_t> * tendPar = new TParameter<Double_t>("runtime",time_since_start);
    tendPar->Write();
    //
    // write histograms to the output root file
    //
    for(int ich = 0; ich<NUMBER_OF_CHANNELS; ich++){
        _e_all[ich]->Write();
        _e_good[ich]->Write();
        _e_err1[ich]->Write();
        _e_err2[ich]->Write();
        _e_err4[ich]->Write();
        
        _b_good[ich]->Write();
        _b_err1[ich]->Write();
        _b_err2[ich]->Write();
        _b_err4[ich]->Write();
        
        _2d_good[ich]->Write();
        _2d_err1[ich]->Write();
        _2d_err2[ich]->Write();
        _2d_err4[ich]->Write();
    }
    
    char hname[128];
    
    // historam as placeholder for drawing stability graphs
    TH1F *_holder = new TH1F("hld","hld",1,0,time_since_start);
    string htitle = "run: "+run;
    _holder->SetTitle(htitle.c_str());
    //_holder->Write();
    
    tree->Write();
}
/*----------------------------------------------------------------------------------------------------*/
void analyzer::reset_interval_data(){
    
    n_interval = 0;
    
    _t_temp = 0;
    _t_pres = 0;
    _t_bx = 0;
    _t_by = 0;
    _t_bz = 0;
    _t_btot = 0;
    _t_humid = 0;
    
}
/*----------------------------------------------------------------------------------------------------*/
void analyzer::add_interval_data(){
    
    n_interval++;
    
    _t_temp += temp;
    _t_pres += pres;
    _t_bx += bx;
    _t_by += by;
    _t_bz += bz;
    _t_btot += btot;
    _t_humid += humid;
    
}
/*----------------------------------------------------------------------------------------------------*/
void analyzer::calculate_interval_data(){
    if(n_interval>0){
        _t_temp /= n_interval;
        _t_pres /= n_interval;
        _t_bx /= n_interval;
        _t_by /= n_interval;
        _t_bz /= n_interval;
        _t_btot /= n_interval;
        _t_humid /= n_interval;
    }
}
/*---------------------------------------------------------------------------------------------------*/
void analyzer::get_source_id()
{
    cout <<"analyzer::get_source_id"<<endl;
    
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
        } else {// if the channel is inactive, just set it to BACKGROUND (does not matter)
            source_id[ichannel] = BACKGROUND;
        }
    }
    cout <<"analyzer::get_source_id ... done"<<endl;
    
}
/*----------------------------------------------------------------------------------------------------*/
//
// MAIN:: Loop routine
//
void analyzer::Loop()
{
    if (fChain == 0) return;
    
    //
    // look in the first data file of the chain to see what sources are present
    //
    get_source_id();
    
    
    Long64_t nentries = fChain->GetEntriesFast();
    //
    // book histograms
    //
    book_histograms();
    
    //
    // start the event loop
    //
    cout<<"Start event loop.... nentries ="<<nentries<<endl;
    Long64_t nbytes = 0, nb = 0;
    Bool_t last_event = false;
    
    for (Long64_t jentry=0; jentry<nentries;jentry++) {
        //
        // get entry from the tree
        //
        Long64_t ientry = LoadTree(jentry);
        if (ientry < 0) last_event = true;
        
        if(!last_event){
            nb = fChain->GetEntry(jentry);   nbytes += nb;
            //
            // process the time information.
            //
            if (jentry == 0) {
                tstart = time;
                // reset all averages
                reset_interval_data();
            }
            time_since_start = time - tstart;
            if (jentry == 0) t0 = time_since_start;
            
            //
            // fill the monitoring histograms
            //
            fill_histograms();
            //
            // add the data for the slow control average calculations
            //
            add_interval_data();
            //
            // if we exceed the maximum time interval, get all the data recorded
            // during this time. then reset time for a new interval....
            //
        }
        delta_t = time_since_start - t0;
        if((delta_t > TIME_INTERVAL) || last_event) {
            // calculate the slow control avarages
            calculate_interval_data();
            // fitting of the peaks
            get_interval_data();
            // reset the time for the start of the next interval
            t0 = time_since_start;
            // reset the interval data for calculating averages
            reset_interval_data();
        }
        
        if(jentry%500000 == 0) cout<<"Processed "<<jentry<<" events"<<endl;
        
        if(last_event) break;
    }
    
    cout<<"Done event loop...."<<endl;
    
    //
    // write histograms to file and generate the stability graphs
    //
    write_histograms();
    cout<<"Done writing histograms"<<endl;
}
