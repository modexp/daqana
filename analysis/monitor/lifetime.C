#define lifetime_cxx
#include "lifetime.h"
#include <TGraphErrors.h>
#include <TF1.h>
#include <TH2.h>
#include <TStyle.h>
#include <TCanvas.h>

#include <iostream>

#define LN2 0.69314718055994530942

const int nmax = 100000;

/*----------------------------------------------------------------------------------------------------*/
//
// Fit the half-life to the tree data produced by the analyzer.C code. 
//
// Usage from the ROOT6 command line
//
// prompt> #include<lifetime.C>
// prompt> lifetime l(<directory_with_ANA_files>)
// prompt> l.Life(<channel>, <peak>, <type>, <save>)
//
// <channel> : [0..7]
// <peak>    : [0..2] peak identifier in the spectrum. at the moment [0] for BG, [0..2] for 44Ti & 60Co, [0] for 137Cs
// <type>    : what to plot : "life" = rate vs time wit exponential fit
//                            "pull" = (data-fit)/fit_error
//                            "res"  = (data-fit)
// <save>    : to file or not?
//
// A.P.
//
/*----------------------------------------------------------------------------------------------------*/
void lifetime::Life(int channel_sel, int peak_sel, string type, bool save)
{
   TCanvas *c1 = new TCanvas("c1","c1",600,400);
   if (fChain == 0) return;

   Long64_t nentries = fChain->GetEntriesFast();

   Long64_t nbytes = 0, nb = 0;

   Double_t t[nmax],R[nmax],dR[nmax],tstart;
   Int_t n = 0,icounter=0;

   cout <<"lifetime::Life  reading data for CHANNEL = "<<channel_sel<<" PEAK = "<<peak_sel<<endl;
   for (Long64_t jentry=0; jentry<nentries;jentry++) {
      Long64_t ientry = LoadTree(jentry);
      if (ientry < 0) break;

      nb = fChain->GetEntry(jentry);   nbytes += nb;
      if(jentry == 0) tstart = t0;
      // if (Cut(ientry) < 0) continue;
      if(channel == channel_sel && peak == peak_sel ){

           //t[n] = t0+time-tstart;
           t[n] = t0+time;
           R[n] = rate;
           dR[n] = drate;//sqrt(rate*900)/900;
         
           n++; 
      }
   }

   cout <<"lifetime::Life  start fit"<<endl;
   //
   // make a TGraphErrors object and fit an exponential
   //
   TGraphErrors *g1 = new TGraphErrors(n,t,R,0,dR);
   
   char cmd[128];
   sprintf(cmd,"[0]*exp(-(x-%f)*0.6931471805599/[1]/3600/24/365)",t[0]);
   TF1 *f1 = new TF1("exp_life",cmd,0.0e6,10e6);
   f1->SetParameters(R[0],40);
   g1->Fit("exp_life");

   TH1F *_pull = new TH1F("pull","pull",50,-5,5);
   TH1F *_res  = new TH1F("res","res",50,-0.1,0.1);
   //
   // plot the results
   //
   gStyle->SetOptFit(111);
   if(type == "life"){
     //
     // plot rate vs time and fitted exponential
     //
     g1->SetMarkerStyle(24);
     g1->Draw("AP");
     g1->GetXaxis()->SetTimeFormat("%d/%m");
     g1->GetXaxis()->SetTimeDisplay(1);
     g1->GetXaxis()->SetTitle("time");

     sprintf(cmd,"Rate as a function of time. Channel = %i Photo-peak = %i",channel_sel,peak_sel);
     g1->SetTitle(cmd);
     g1->GetXaxis()->SetTitle("time");
     g1->GetYaxis()->SetTitle("rate (Hz)");

   } else if (type == "res"){
     //
     // plot residual distribution
     //
     double res;
     for(int i=0; i<n; i++){
       res = (R[i] - f1->Eval(t[i]) );
       _res->Fill(res);
     }
     sprintf(cmd,"Residual distribution. Channel = %i Photo-peak = %i",channel_sel,peak_sel);
     _res->SetTitle(cmd);
     _res->GetXaxis()->SetTitle("residual (Hz)");
     _res->SetLineColor(4);
     _res->SetMarkerColor(4);
     _res->SetMarkerStyle(20);
     _res->Fit("gaus");
     _res->Draw("pe1");
   } else if (type == "pull"){
     //
     // plot pull distribution
     //
     double res;
     for(int i=0; i<n; i++){
       res = (R[i] - f1->Eval(t[i]) )/dR[i];
       _pull->Fill(res);
     }
     sprintf(cmd,"Pull distribution. Channel = %i Photo-peak = %i",channel_sel,peak_sel);
     _pull->SetTitle(cmd);
     _pull->GetXaxis()->SetTitle("pull");
     _pull->SetLineColor(4);
     _pull->SetMarkerColor(4);
     _pull->SetMarkerStyle(20);
     _pull->Fit("gaus");
     _pull->Draw("pe1");
   }

   if(save){
     sprintf(cmd,"t12_ch%i_pk%i_100days.png",channel_sel,peak_sel);
     c1->Print(cmd);
   }
  
}
