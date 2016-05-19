#include <iostream>
#include <string>
#include <vector>

#include <RooRealVar.h>
#include <RooGaussian.h>
#include <RooFitResult.h>
#include <RooDataHist.h>

using namespace RooFit ;
using namespace std;

const double emin = 0.;
const double emax = 3000.;

#define NUMBER_OF_CHANNELS 8
#define MAX_PEAKS 5
float source_energy[NUMBER_OF_CHANNELS][MAX_PEAKS] =
//
// the energy peaks you wish to select should be in this list
// NOTE: the first peak should be the highest in the spectrum (sub-optimal, but handy for finding)
//
{
    {1460.,-1,-1,-1,-1}, // channel0: no source
    {1460.,-1,-1,-1,-1}, // channel1: no source
    {511.,1157.020,511.+1157.020,-1,-1}, // channel2: 44Ti
    {511.,1157.020,511.+1157.020,-1,-1}, // channel3: 44Ti
    {1173.2,1332.5,1173.2+1332.5,-1,-1}, // channel4: 60Co
    {1173.2,1332.5,1173.2+1332.5,-1,-1}, // channel5: 60Co
    {662.,-1,-1,-1,-1}, // channel6: 137Cs
    {662.,-1,-1,-1,-1}  // channel7: 137Cs
};

/*---------------------------------------------------------------------------------------------*/
//
// fitspectrum.C - RooFit based spectrum fitter. This routine can fit the spectrum for a full
//                 modulation experiment run. The same algorithm is used in the analysis/calibration/analyzer.C
//                 routine to fit the spectra after the chosen time interval.
//
//                 In this routine it is easy to develop new fitting ideas.
//
// Arguments
//            data_file    : name of the data file to fit
//            mc_file      : Geant4 prediction of the background spectrum. The spectra are generated
//                           with the modexp/modusim package from GitHub
//            ichannel     : channel number [0..7]
//            e0           : lowest energy for fit (keV)
//            e1           : highest energy for fit (keV)
//
// A.P. 2015-07-02
//
/*---------------------------------------------------------------------------------------------*/
void fitspectrum(string data_file, string mc_file, int ichannel, double e0, double e1){
    double fit_range[2];
    fit_range[0] = e0;
    fit_range[1] = e1;
    
    //
    // RooFit based spectrum fitter
    //
    //
    cout <<"analyzer::fit_spectrum  channel = "<<ichannel<<endl;
    //
    // identify the peaks in our specified energy range and store their peak ID.
    //
    int nselect = 0;
    int peak_id[MAX_PEAKS];
    for(int ipeak = 0; ipeak<MAX_PEAKS; ipeak++){
        double epeak = source_energy[ichannel][ipeak];
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
    
    RooRealVar a0("a0","a0",0, -100,100);//,  0.9, 1.1);
    RooRealVar a1("a1","a1",1, 0.9,1.1);
    RooRealVar a2("a2","a2",0, -1e-5,1e-5);//, -0.1, 0.1);
    RooFormulaVar Es("Es","Es","a0+a1*E+a2*E*E",RooArgSet(a0,a1,a2,E));
    //
    // the background template for each of the sources obtained from a GEANT4 simulation
    //
    TFile *f_mc = new TFile(mc_file.c_str(),"READONLY");
    //TH1* h_bg  = (TH1*)f_mc->Get("h2");
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
    vector<RooRealVar*> pk_mean;
    vector<RooRealVar*> pk_sigma;
    vector<RooRealVar*> pk_frac;
    vector<RooGaussian*> pk_gaus;
    
    
    char vname[128];
    for (int id=0; id<nselect; id++){
        int ipeak = peak_id[id];
        double epeak = source_energy[ichannel][ipeak];
        cout <<"id = "<<id<<" peak = "<<peak_id[id]<<" E = "<<epeak<<" keV"<<endl;
        
        sprintf(vname,"mean%i",id);
        pk_mean.push_back(new RooRealVar(vname,vname,epeak,epeak-50,epeak+50));
        cout <<"tic"<<endl;
        sprintf(vname,"sigma%i",id);
        pk_sigma.push_back(new RooRealVar(vname,vname,25,5,100));
        cout <<"tac"<<endl;
        sprintf(vname,"frac%i",id);
        pk_frac.push_back(new RooRealVar(vname,vname,0.2,0.0,1.0));
        cout <<"toc"<<endl;
    }    
    for (int id=0; id<nselect; id++){
        sprintf(vname,"gaus%i",id);
        cout <<"vname >>"<<vname<<"<< pk  = "<< (pk_mean[id])->getValV()<<endl;
        pk_gaus.push_back(new RooGaussian(vname,vname,E,*pk_mean.at(id),*pk_sigma.at(id)));
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
    if (nselect==2)  sum = new RooAddPdf("sum","g1+g2+bg",RooArgList(*pk_gaus[0],*pk_gaus[1],bg),RooArgList(*pk_frac[0],*pk_frac[1]));
    if (nselect==3)  sum = new RooAddPdf("sum","g1+g2+g3+bg",RooArgList(*pk_gaus[0],*pk_gaus[1],*pk_gaus[2],bg),RooArgList(*pk_frac[0],*pk_frac[1],*pk_frac[2]));
    
    cout <<"analyzer::fit_spectrum Compose the combined pdf ---- DONE "<<endl;
    
    E.setRange("signalRange",emin,emax);//fit_range[0],fit_range[1]);
    RooExtendPdf esum("esum","extended pdf with Norm",*sum,Norm,"signalRange");
    
    cout <<"analyzer::fit_spectrum Compose the combined pdf ---- extended DONE "<<endl;
    //
    // get the data from a TH1 root histogram
    //
    TFile *f_data = new TFile(data_file.c_str(),"READONLY");
    char hname[128];
    sprintf(hname,"_e_good_ch%i",ichannel);
    TH1* h_data  = (TH1*)f_data->Get(hname);
    RooDataHist data("data","data",RooArgList(E),h_data);
    //
    // fit the pdf to the data
    //
    RooFitResult *fr = esum.fitTo(data,Extended(kTRUE),Range(fit_range[0],fit_range[1]),Save());
    fr->Print();
    
    //
    // draw the fit results..... not for batch running. Set PLOT_ON_SCREEN variable to 0 in analyzer.h file
    //
    TCanvas *c1 = new TCanvas("c1","c1",600,400);
    //
    // plot the data with the fittted function
    //
    RooPlot *Eframe = E.frame();
    Eframe->SetTitle("");
    Eframe->GetXaxis()->SetRangeUser(fit_range[0]-200,fit_range[1]+200);
    
    //
    // find maximum data value in plot range
    //
    h_data->GetXaxis()->SetRangeUser(fit_range[0],fit_range[1]);
    Int_t maxbin  = h_data->GetMaximumBin();
    Double_t maxval  = h_data->GetBinContent(maxbin);
    
    //
    // plot the data and pdfs. use the plot range as found before
    //
    data.plotOn(Eframe);
    esum.plotOn(Eframe);
    Eframe->SetMaximum(1.2*maxval);
//    c1->SetLogy(1);
    
    for (int id=0; id<nselect; id++){
        esum.plotOn(Eframe,Components(*pk_gaus[id]),LineColor(2),LineWidth(2));
        pk_gaus[id]->paramOn(Eframe,Layout(0.55,0.88,0.85-id*0.15));
    }
    esum.plotOn(Eframe,Components(bg),LineColor(kGreen),LineWidth(2));
    
    cout << Eframe->chiSquare() <<endl;
    Eframe->Draw();
    
    c1->Update();
    
    //
    // cleanup
    //
    delete sum;
    delete fr;
}
