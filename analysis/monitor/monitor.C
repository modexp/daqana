/*----------------------------------------------------------------------------*/

//
// monitor plots for Modulation data
//
// Usage: From root command line type ".x monitor.C(runname, plot_type, channel, log_scale, save_plot)"
//
// Input: runname    - name of the run you wish to analyze with the full path added to the rootfile
//        plot_type  - what to plot ['spectrum','2d','baseline']
//        channel    - channel number [0..7]
//        log_scale  - logarithmic y-axis [true, false]
//        save_plot  - save the plot to .pdf and .png file [true, false]
//
//
// AP 27-03-2015
//

/*----------------------------------------------------------------------------*/
#include <vector>
//#ifdef __MAKECINT__
//#pragma link C++ class vector<float>+;
//#eindif

TFile *_file;
//TChain *run = new TChain("T");
TCanvas *c1 = new TCanvas("c1","c1",700,400);

// ranges for plotting
const int   nbin = 600;
const float emin = 0.; // in keV
const float emax = 3000.; // in keV
const float adc_max_volt = 2.;
const float base_max_val = 2000;
char cmd[256];


/*----------------------------------------------------------------------------*/
void plot_spectrum(int ichannel){
    //
    // plot the 1D energy spectrum for channel = ichannel
    //
    char hname[128];

    Double_t de = (emax-emin)/nbin;

    sprintf(hname,"_e_all_ch%d",ichannel);
    TH1F *_e_all  = (TH1F*)gDirectory->Get(hname);
    sprintf(hname,"_e_good_ch%d",ichannel);
    TH1F *_e_good  = (TH1F*)gDirectory->Get(hname);
    sprintf(hname,"_e_err1_ch%d",ichannel);
    TH1F *_e_err01  = (TH1F*)gDirectory->Get(hname);
    sprintf(hname,"_e_err2_ch%d",ichannel);
    TH1F *_e_err02  = (TH1F*)gDirectory->Get(hname);
    sprintf(hname,"_e_err4_ch%d",ichannel);
    TH1F *_e_err04  = (TH1F*)gDirectory->Get(hname);
    

    // get time range
    TParameter<Double_t> *rt = (TParameter<Double_t>*)gDirectory->Get("runtime");
    Double_t dt = rt->GetVal();
//    Double_t dt = runtime->GetVal();
    // calculate total rate
    Double_t n_entries = _e_all->GetEntries();
    Double_t rate  = n_entries / dt;
    Double_t drate = sqrt(n_entries)/dt;
    
    _e_good->Scale(1./dt);
    _e_err01->Scale(1./dt);
    _e_err02->Scale(1./dt);
    _e_err04->Scale(1./dt);
    
    // plot title
    sprintf(cmd,"Spectrum: channel = %i. Rate = %6.3f #\\pm %6.3f kHz",ichannel,rate/1000,drate/1000);
    _e_good->SetTitle(cmd);
    // x-axis title
    _e_good->GetXaxis()->SetTitle("E (keV)");
    // y-axis title
    sprintf(cmd,"Rate (Hz / %i keV)",(int)((emax - emin)/nbin));
    _e_good->GetYaxis()->SetTitle(cmd);
    
    // compose the legend for the plot
    TLegend *leg = new TLegend(0.6,0.7,0.89,0.89);
    leg->AddEntry(_e_good,"good events","f");
    leg->AddEntry(_e_err01,"overflow error","f");
    leg->AddEntry(_e_err02,"baseline rms error","f");
    leg->AddEntry(_e_err04,"double peak error","f");
    leg->SetTextSize(0.03);
    leg->SetBorderSize(0);
    
    // draw it all....
    _e_good->Draw();
    _e_err01->SetLineColor(2);
    _e_err01->Draw("same");
    _e_err02->SetLineColor(6);
    _e_err02->Draw("same");
    _e_err04->SetLineColor(4);
    _e_err04->Draw("same");
    leg->Draw();
    
}
/*----------------------------------------------------------------------------*/
void plot_2d(int ichannel){
    char hname[128];
    gStyle->SetOptLogz();
    // plot the pulse height as a function of energy
    sprintf(hname,"_h_vs_E_good_ch%i",ichannel);
    TH2F *_2d_good = (TH2F*)gDirectory->Get(hname);
    sprintf(hname,"_h_vs_E_err1_ch%i",ichannel);
    TH2F *_2d_err1 = (TH2F*)gDirectory->Get(hname);
    sprintf(hname,"_h_vs_E_err2_ch%i",ichannel);
    TH2F *_2d_err2 = (TH2F*)gDirectory->Get(hname);
    sprintf(hname,"_h_vs_E_err4_ch%i",ichannel);
    TH2F *_2d_err4 = (TH2F*)gDirectory->Get(hname);
    
    sprintf(cmd,"Pulse height -vs- Energy: channel = %i",ichannel);
    _2d_good->SetTitle(cmd);
    _2d_good->SetMarkerColor(1);
    _2d_good->GetXaxis()->SetTitle("E (keV)");
    _2d_good->GetYaxis()->SetTitle("Pulse height (V)");
    _2d_good->Draw("colz");

    _2d_err1->SetMarkerColor(2);
    _2d_err1->Draw("same");
    _2d_err2->SetMarkerColor(2);
    _2d_err2->Draw("same");
    _2d_err4->SetMarkerColor(2);
    _2d_err4->Draw("same");
    _2d_good->Draw("samecolz");
    
}
/*----------------------------------------------------------------------------*/
void plot_baseline(int ichannel){
    char hname[128];

    sprintf(hname,"_b_good_ch%d",ichannel);
    TH1F *_b_good  = (TH1F*)gDirectory->Get(hname);
    sprintf(hname,"_b_err1_ch%d",ichannel);
    TH1F *_b_err01  = (TH1F*)gDirectory->Get(hname);
    sprintf(hname,"_b_err2_ch%d",ichannel);
    TH1F *_b_err02  = (TH1F*)gDirectory->Get(hname);
    sprintf(hname,"_b_err4_ch%d",ichannel);
    TH1F *_b_err04  = (TH1F*)gDirectory->Get(hname);
    
    sprintf(cmd,"Baseline: channel = %i",ichannel);
    _b_good->SetTitle(cmd);
    _b_good->GetXaxis()->SetTitle("Baseline (ADC counts)");
    _b_good->GetYaxis()->SetTitle("Number of entries");
    
    // compose the legend for the plot
    TLegend *leg = new TLegend(0.6,0.7,0.89,0.89);
    leg->AddEntry(_b_good,"good events","f");
    leg->AddEntry(_b_err01,"overflow error","f");
    leg->AddEntry(_b_err02,"baseline rms error","f");
    leg->SetTextSize(0.03);
    leg->SetBorderSize(0);
    
    // draw it all
    _b_good->Draw();
    _b_err01->SetLineColor(2);
    _b_err01->Draw("same");
    _b_err02->SetLineColor(6);
    _b_err02->Draw("same");
    leg->Draw();
}

/*----------------------------------------------------------------------------*/
void monitor(string runname, string plot_type, int ichannel, bool log_scale, bool save_plot)
{
    // documentation on top of this .C file

    // open the first file: will be used to retrieve settings
    sprintf(cmd,"%s",runname.c_str());
    _file = new TFile(cmd,"READONLY");
    
    // no statistics box
    gStyle->SetOptStat(0);
  
    // what to plot?
    if        (plot_type == "spectrum") { // 1D energy spectrum
        plot_spectrum(ichannel);
    } else if (plot_type == "2d"){ // 2D puls height as a function of energ
        plot_2d(ichannel);
    } else if (plot_type == "baseline"){
        plot_baseline(ichannel);
    } else {
        cout<< "Wrong plot type selected..... look at documentation in monitor.C"<<endl;
        return;
    }
    // set the logarithmic y-axis if required
    c1->SetLogy(log_scale);
    
    // save the plot to file
    if(save_plot) {
        string tag="lin";
        if(log_scale) tag = "log";

        TPad *current_pad = (TPad*)gROOT->GetSelectedPad();
        sprintf(cmd,"plots/%s_channel%i_%s.pdf",plot_type.c_str(),ichannel,tag.c_str());
        current_pad->Print(cmd);
        sprintf(cmd,"plots/%s_channel%i_%s.png",plot_type.c_str(),ichannel,tag.c_str());
        current_pad->Print(cmd);
    }
}
