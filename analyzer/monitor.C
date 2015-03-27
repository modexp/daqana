/*----------------------------------------------------------------------------*/

// ranges for plotting
const int   nbin = 250;
const float emin = 0.; // in keV
const float emax = 3000.; // in keV
const float adc_max_volt = 2.;
const float base_max_val = 2000;
char cmd[256];

/*----------------------------------------------------------------------------*/
void plot_spectrum(int ichannel){
    // plot the 1D energy spectrum for channel = ichannel
    TH1F *_e_good = new TH1F("e_good","e_good",nbin,emin,emax);
    TH1F *_e_err01 = new TH1F("e_err01","e_err01",nbin,emin,emax);
    TH1F *_e_err02 = new TH1F("e_err02","e_err02",nbin,emin,emax);
    // energy spectrum for good events
    sprintf(cmd,"channel==%i&&error==0",ichannel);
    T->Draw("integral>>e_good",cmd);
    // energy spectrum for the bad ones
    sprintf(cmd,"channel==%i&&(error&0x01)!=0",ichannel);
    T->Draw("integral>>e_err01",cmd);
    sprintf(cmd,"channel==%i&&(error&0x02)!=0",ichannel);
    T->Draw("integral>>e_err02",cmd);
    
    sprintf(cmd,"Spectrum: channel = %i",ichannel);
    _e_good->SetTitle(cmd);
    _e_good->GetXaxis()->SetTitle("E (keV)");
    _e_good->GetYaxis()->SetTitle("Number of entries");
    
    // compose the legend for the plot
    TLegend *leg = new TLegend(0.6,0.6,0.89,0.89);
    leg->AddEntry(_e_good,"good events","f");
    leg->AddEntry(_e_err01,"overflow error","f");
    leg->AddEntry(_e_err02,"baseline rms error","f");
    leg->SetTextSize(0.03);
    leg->SetBorderSize(0);
    
    // draw it all
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
    
    T->SetMarkerColor(1);
    sprintf(cmd,"channel==%i",ichannel);
    T->Draw("height:integral",cmd,"same");
    
    T->SetMarkerColor(2);
    sprintf(cmd,"channel==%i&&(error&0x01)!=0",ichannel);
    T->Draw("height:integral",cmd,"same");

    T->SetMarkerColor(6);
    sprintf(cmd,"channel==%i&&(error&0x02)!=0",ichannel);
    T->Draw("height:integral",cmd,"same");
    
}
/*----------------------------------------------------------------------------*/
void plot_baseline(int ichannel){
    // plot the 1D energy spectrum for channel = ichannel
    TH1F *_b_good = new TH1F("b_good","b_good",nbin,0,base_max_val);
    TH1F *_b_err01 = new TH1F("b_err01","b_err01",nbin,0,base_max_val);
    TH1F *_b_err02 = new TH1F("b_err02","b_err02",nbin,0,base_max_val);
    // energy spectrum for good events
    sprintf(cmd,"channel==%i&&error==0",ichannel);
    T->Draw("baseline>>b_good",cmd);
    // energy spectrum for the bad ones
    sprintf(cmd,"channel==%i&&(error&0x01)!=0",ichannel);
    T->Draw("baseline>>b_err01",cmd);
    sprintf(cmd,"channel==%i&&(error&0x02)!=0",ichannel);
    T->Draw("baseline>>b_err02",cmd);
    
    sprintf(cmd,"Baseline: channel = %i",ichannel);
    _b_good->SetTitle(cmd);
    _b_good->GetXaxis()->SetTitle("Baseline (ADC counts)");
    _b_good->GetYaxis()->SetTitle("Number of entries");
    
    // compose the legend for the plot
    TLegend *leg = new TLegend(0.6,0.6,0.89,0.89);
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
void monitor(int ichannel, string plot_type, bool save_plot)
{
    //
    // monitor plots for Modulation data
    //
    // Input: channel    - channel number [0..7]
    //        plot_type  - what to plot ['spectrum','2d','baseline']
    //        save_plot  - save the plot to .pdf and .png file [true, false]
    //
    // AP 27-03-2015
    //
    
    gStyle->SetOptStat(0);
    // what to plot?
    if        (plot_type == "spectrum") { // 1D energy spectrum
        plot_spectrum(ichannel);
    } else if (plot_type == "2d"){ // 2D puls height as a function of energy
        plot_2d(ichannel);
    } else if (plot_type == "baseline"){
        plot_baseline(ichannel);
    } else {
        cout<< "Wrong plot type selected"<<endl;
    }
    
    if(save_plot) {
        sprintf(cmd,"plots/%s_channel%i.pdf",plot_type.c_str(),ichannel);
        c1->Print(cmd);
        sprintf(cmd,"plots/%s_channel%i.png",plot_type.c_str(),ichannel);
        c1->Print(cmd);
    }
}
