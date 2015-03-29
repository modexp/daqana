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

TChain *run = new TChain("T");
TCanvas *c1 = new TCanvas("c1","c1",700,400);

// ranges for plotting
const int   nbin = 300;
const float emin = 0.; // in keV
const float emax = 3000.; // in keV
const float adc_max_volt = 2.;
const float base_max_val = 2000;
char cmd[256];

/*----------------------------------------------------------------------------*/
Double_t get_delta_t(){
    //
    // get the time difference between start and end of the file
    //
    Double_t time,t_end,t_start;
    
    Int_t n_entries = run->GetEntries();
    run->SetBranchAddress("time",&time);
    run->GetEntry(0);
    t_start = time;
    run->GetEntry(n_entries-1);
    t_end   = time;
    
    return t_end - t_start;
}

/*----------------------------------------------------------------------------*/
void plot_spectrum(int ichannel){
    // plot the 1D energy spectrum for channel = ichannel
    TH1F *_e_all  = new TH1F("e_all","e_all",nbin,emin,emax);
    TH1F *_e_good  = new TH1F("e_good","e_good",nbin,emin,emax);
    TH1F *_e_err01 = new TH1F("e_err01","e_err01",nbin,emin,emax);
    TH1F *_e_err02 = new TH1F("e_err02","e_err02",nbin,emin,emax);
    // energy spectrum for good events
    sprintf(cmd,"channel==%i",ichannel);
    run->Draw("integral>>e_all",cmd);
    sprintf(cmd,"channel==%i&&error==0",ichannel);
    run->Draw("integral>>e_good",cmd);
    // energy spectrum for the bad ones
    sprintf(cmd,"channel==%i&&(error&0x01)!=0",ichannel);
    run->Draw("integral>>e_err01",cmd);
    sprintf(cmd,"channel==%i&&(error&0x02)!=0",ichannel);
    run->Draw("integral>>e_err02",cmd);
    // get time range
    Double_t dt = get_delta_t();
    // calculate total rate
    Double_t n_entries = _e_all->GetEntries();
    Double_t rate  = n_entries / dt;
    Double_t drate = sqrt(n_entries)/dt;
    
    dt = 1;
    
    _e_good->Scale(1./dt);
    _e_err01->Scale(1./dt);
    _e_err02->Scale(1./dt);
    
    // plot title
    sprintf(cmd,"Spectrum: channel = %i. Rate = %6.3f #\pm %6.3f kHz",ichannel,rate/1000,drate/1000);
    _e_good->SetTitle(cmd);
    // x-axis title
    _e_good->GetXaxis()->SetTitle("E (keV)");
    // y-axis title
    sprintf(cmd,"Rate (Hz / %i keV)",(emax - emin)/nbin);
    _e_good->GetYaxis()->SetTitle(cmd);
    
    // compose the legend for the plot
    TLegend *leg = new TLegend(0.6,0.7,0.89,0.89);
    leg->AddEntry(_e_good,"good events","f");
    leg->AddEntry(_e_err01,"overflow error","f");
    leg->AddEntry(_e_err02,"baseline rms error","f");
    leg->SetTextSize(0.03);
    leg->SetBorderSize(0);
    
    // draw it all....
    _e_good->Draw();
    _e_err01->SetLineColor(2);
    _e_err01->Draw("same");
    _e_err02->SetLineColor(6);
    _e_err02->Draw("same");
    leg->Draw();
}
/*----------------------------------------------------------------------------*/
void plot_2d(int ichannel){
    // plot the pulse height as a function of energy
    TH2F *_2d = new TH2F("h_vs_E","h_vs_E",nbin,emin,emax,nbin,0.,adc_max_volt);
    
    sprintf(cmd,"Pulse height -vs- Energy: channel = %i",ichannel);
    _2d->SetTitle(cmd);
    _2d->GetXaxis()->SetTitle("E (keV)");
    _2d->GetYaxis()->SetTitle("Pulse height (V)");
    _2d->Draw("");
    
    run->SetMarkerColor(1);
    sprintf(cmd,"channel==%i",ichannel);
    run->Draw("height:integral",cmd,"same");
    
    run->SetMarkerColor(2);
    sprintf(cmd,"channel==%i&&(error&0x01)!=0",ichannel);
    run->Draw("height:integral",cmd,"same");

    run->SetMarkerColor(6);
    sprintf(cmd,"channel==%i&&(error&0x02)!=0",ichannel);
    run->Draw("height:integral",cmd,"same");
    
}
/*----------------------------------------------------------------------------*/
void plot_baseline(int ichannel){
    // plot the 1D energy spectrum for channel = ichannel
    TH1F *_b_good = new TH1F("b_good","b_good",nbin,0,base_max_val);
    TH1F *_b_err01 = new TH1F("b_err01","b_err01",nbin,0,base_max_val);
    TH1F *_b_err02 = new TH1F("b_err02","b_err02",nbin,0,base_max_val);
    // energy spectrum for good events
    sprintf(cmd,"channel==%i&&error==0",ichannel);
    run->Draw("baseline>>b_good",cmd);
    // energy spectrum for the bad ones
    sprintf(cmd,"channel==%i&&(error&0x01)!=0",ichannel);
    run->Draw("baseline>>b_err01",cmd);
    sprintf(cmd,"channel==%i&&(error&0x02)!=0",ichannel);
    run->Draw("baseline>>b_err02",cmd);
    
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

    // add files to the chain .....
    sprintf(cmd,"%s*.root",runname.c_str());
    run->Add(cmd);
    
    // open the first file: will be used to retrieve settings
    sprintf(cmd,"%s_000000.root",runname.c_str());
    TFile *_file = new TFile(cmd,"READONLY");
    
    // no statistics box
    gStyle->SetOptStat(0);
  
    // what to plot?
    if        (plot_type == "spectrum") { // 1D energy spectrum
        plot_spectrum(ichannel);
    } else if (plot_type == "2d"){ // 2D puls height as a function of energy
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
        current_pad>Print(cmd);
        sprintf(cmd,"plots/%s_channel%i_%s.png",plot_type.c_str(),ichannel,tag.c_str());
        current_pad->Print(cmd);
    }
}
