#define ecal_cxx
#include "ecal.h"
#include <TH2.h>
#include <TStyle.h>
#include <TCanvas.h>
#include <TParameter.h>
#include <iostream>
#include <vector>
#include <stdio.h>

#define CH0 0
#define CH1 1
#define CH2 2
#define CH3 3
#define CH4 4
#define CH5 5
#define CH6 6
#define CH7 7

#define NUMBER_OF_CHANNELS 8

// (1)You need to identify where the peaks are in the histograms of the integrals.
// (2) Set range_low and range_high to find the range in which the peak with cal_energy should be
const double range_low[NUMBER_OF_CHANNELS] ={0.,0.,0.05e-6,0.05e-6,0.,0.,0.,0.};
const double range_high[NUMBER_OF_CHANNELS]={1e-6,1e-6,1e-6,1e-6,1e-6,1e-6,1e-6,1e-6};
const double cal_energy[NUMBER_OF_CHANNELS]={1000.,1000.,511.,511.,1173.,1173.,667.,667.};

void ecal::Loop()
{
    // energy calibration for modulation detectors
    if (fChain == 0) return;
    
    Long64_t nentries = fChain->GetEntriesFast();
    
    // private code below
    TFile *_f = new TFile("calibrate.root","RECREATE");

    std::vector<TH1F*> _integral;
    std::vector<TH1F*> _energy;
    std::vector<TH1F*> _energy_all;

    char tmp[100];
    for (int ich = 0; ich <NUMBER_OF_CHANNELS; ich++){
        sprintf(tmp,"integral_ch%02d",ich);
        _integral.push_back(new TH1F(tmp,tmp,1000,0.,1e-6));
        sprintf(tmp,"energy_ch%02d",ich);
        _energy.push_back(new TH1F(tmp,tmp,1000,0.,3000.));
        sprintf(tmp,"energy_all_ch%02d",ich);
        _energy_all.push_back(new TH1F(tmp,tmp,1000,0.,3000.));

    }
    
    cout<<"Start calibration loop.... nentries ="<<nentries<<endl;
    Long64_t nbytes = 0, nb = 0;
    for (Long64_t jentry=0; jentry<nentries;jentry++) {
        Long64_t ientry = LoadTree(jentry);
        if (ientry < 0) break;
        nb = fChain->GetEntry(jentry);   nbytes += nb;
        // fill the hisotgrams with 'good' events
        channel = channel % 100;
        if(error == 0) _integral[channel]->Fill(integral);
        if(jentry%100000 == 0) cout<<"Processed "<<jentry<<" events"<<endl;
        
    }
    cout<<"Done calibration loop...."<<endl;
    
    double ccal[NUMBER_OF_CHANNELS];
    
    std::vector<TParameter<double>*> _calibration;
    for(int ich=0; ich<NUMBER_OF_CHANNELS; ich++){
        // define the proper range for searching the calibration constant
        _integral[ich]->GetXaxis()->SetRangeUser(range_low[ich],range_high[ich]);
        // find the bin with maximum value in the range
        int ibin = _integral[ich]->GetMaximumBin();
        double val = _integral[ich]->GetBinCenter(ibin);
        ccal[ich] = cal_energy[ich] / val;
        sprintf(tmp,"cal_ch%02d",ich);
        _calibration.push_back(new TParameter<double>(tmp,ccal[ich]));
        _calibration[ich]->Write();
    }
    
    // rerun to fill the calibrated histograms
    for (Long64_t jentry=0; jentry<nentries;jentry++) {
        Long64_t ientry = LoadTree(jentry);
        if (ientry < 0) break;
        nb = fChain->GetEntry(jentry);   nbytes += nb;
        channel = channel % 100;
        // fill the hisotgrams with 'good' events
        if(error == 0) _energy[channel]->Fill(integral*ccal[channel]);
        _energy_all[channel]->Fill(integral*ccal[channel]);

    }
    
    
    _f->Write();
    _f->Close();
}
