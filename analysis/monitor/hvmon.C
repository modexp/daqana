/*----------------------------------------------------------------------------*/

//
// monitor HV supply for Modulation data
//
// Usage: From root command line type ".x hvmon.C(runname, plot_type, channel, log_scale, save_plot)"
//
// Input: runname    - name of the run you wish to analyze with the full path added
//        plot_type  - what to plot ['HV','I']
//        channel    - channel number [0..7, -1 = all]
//        save_plot  - save the plot to .pdf and .png file [true, false]
//
//
// AP 29-03-2015
//

/*----------------------------------------------------------------------------*/
TChain *run = new TChain("T");
TCanvas *c1 = new TCanvas("c1","c1",700,400);

// ranges for plotting
const int      n_channel = 8;
const Double_t v_range = 1.5;
const Double_t delta_t_plot = 10.;

char cmd[256],cuts[256];


/*----------------------------------------------------------------------------*/
Double_t get_t0(){
    //
    // get the start time of the Chain
    //
    Double_t time0;
    run->SetBranchAddress("time",&time0);
    run->GetEntry(0);
    return time0;
}
/*----------------------------------------------------------------------------*/
Double_t get_t1(){
    //
    // get the end time of the Chain
    //
    Double_t time1;
    Int_t n_entries = run->GetEntries();
    run->SetBranchAddress("time",&time1);
    run->GetEntry(n_entries-1);
    return time1;
}
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
void plot_HV(int ichannel){
    // plot the 1D energy spectrum for channel = ichannel
    Double_t t1 = get_t1();
    Double_t t0 = get_t0();
    
    TH1F *hv = new TH1F("hv","hv",1,t0,t1);
    
    hv->Fill(t0+0.01,1000);
    hv->GetYaxis()->SetRangeUser(-v_range,v_range);
    hv->SetTitle("High voltage stability");
    hv->GetXaxis()->SetTitle("time (sec)");
    hv->GetYaxis()->SetTitle("V - V_{set} (V)");
    hv->Draw();
    c1->Update();
    
    int icol = 2;
    TLegend *leg = new TLegend(0.6,0.5,0.89,0.89);
    
    // loop over all the HV channels....
    for(int i=0; i<n_channel; i++){
        // get the HV setpoint for this channel
        TParameter<Double_t> *par1;
        sprintf(cmd,"info/hv/channel_%i",i);
        gDirectory->GetObject(cmd,par1);
        Double_t hvset = par1->GetVal();
        //Double_t hvset = 0;
        cout << "channel = "<<i<< ". Set to "<<hvset <<" V"<<endl;
        delete par1;
        
        // plot if we are dealing with the selected channel
        if(i == ichannel || ichannel == -1){
            sprintf(cmd,"hv%i-%f:time>>hp%i",i,hvset,i);
            sprintf(cuts,"channel==%i",i);
            // set the color dynamically
            run->SetMarkerColor(icol);
            run->SetLineColor(icol);
            icol++;
            // plot as a profile
            run->Draw(cmd,cuts,"sameprof");
            c1->Update();
            // find the peak-to-peak variation
            sprintf(cmd,"hp%i",i);
            TProfile *ptemp;
            gDirectory->GetObject(cmd,ptemp);
            Double_t delta = ptemp->GetMaximum()-ptemp->GetMinimum();
            // add a legend entry
            sprintf(cuts,"channel%i #Delta = %4.3f V",i,delta);
            leg->AddEntry(cmd,cuts,"f");
        }
    }
    leg->SetBorderSize(0);
    leg->Draw();
    
}
/*----------------------------------------------------------------------------*/
void plot_I(int ichannel){
    
}
/*----------------------------------------------------------------------------*/
void plot_env(){
    //
    // plot temperature, humidity, pressure as a function of time
    //
    Double_t t1 = get_t1();
    Double_t t0 = get_t0();


    TH1F *env = new TH1F("env","env",1,t0,t1);
    
    env->Fill(t0+0.01,1000);
    
    env->GetYaxis()->SetRangeUser(0.95,1.05);
    env->SetTitle("Environment stability");
    env->GetXaxis()->SetTitle("time (sec)");
    env->GetYaxis()->SetTitle("value / value(t=0)");
    env->Draw();
    c1->Update();
    
    // get the initial values of the nevironment variables
    Double_t temp, pres, humid, btot;
    run->SetBranchAddress("temp",&temp);
    run->SetBranchAddress("pres",&pres);
    run->SetBranchAddress("humid",&humid);
    run->SetBranchAddress("btot",&btot);

    run->GetEntry(0);
    
    Double_t T0 = temp;
    Double_t p0 = pres;
    Double_t RH0 = humid;
    Double_t B0 = btot;
    
    // make a legend
    TLegend *leg = new TLegend(0.6,0.7,0.89,0.89);
    
    printf("t0=%26.20f \n",t0);
    printf("t1=%26.20f \n",t1);

    Int_t nbin = (int)((t1-t0)/delta_t_plot);
    TProfile *hp0 = new TProfile("hp0","hp0",nbin,t0,t1,-10,10);
    TProfile *hp1 = new TProfile("hp1","hp1",nbin,t0,t1,-10,10);
    TProfile *hp2 = new TProfile("hp2","hp2",nbin,t0,t1,-10,10);
    TProfile *hp3 = new TProfile("hp3","hp3",nbin,t0,t1,-10,10);
    
    // temperature
    sprintf(cmd,"temp/%f:time>>hp0",T0);
    run->Draw(cmd,"","sameprof");
    hp0->SetMarkerColor(2);
    hp0->SetLineColor(2);
    sprintf(cmd, "Temperature: T_{0} = %4.1f ^{#circ}C",T0);
    leg->AddEntry(hp0,cmd,"f");
    
    // pressure
    
    sprintf(cmd,"pres/%f:time>>hp1",p0);
    run->Draw(cmd,"","sameprof");
    hp1->SetMarkerColor(3);
    hp1->SetLineColor(3);
    sprintf(cmd, "Pressure: p_{0} = %4.1f kPa",p0/1000);
    leg->AddEntry(hp1,cmd,"f");
    
    // relative humidity
    sprintf(cmd,"humid/%f:time>>hp2",RH0);
    run->Draw(cmd,"","sameprof");
    hp2->SetMarkerColor(4);
    hp2->SetLineColor(4);
    sprintf(cmd, "Humidity: RH_{0} = %4.1f %%",RH0);
    leg->AddEntry(hp2,cmd,"f");
    
    // magnetic field
    sprintf(cmd,"btot/%f:time>>hp3",B0);
    run->Draw(cmd,"","sameprof");
    hp3->SetMarkerColor(6);
    hp3->SetLineColor(6);
    sprintf(cmd, "Magnetic field: B_{0} = %4.1f mG",B0);
    leg->AddEntry(hp3,cmd,"f");

    
    leg->SetBorderSize(0);
    leg->Draw();
    
}

/*----------------------------------------------------------------------------*/
void hvmon(string runname, string plot_type, int ichannel, bool save_plot)
{
    // for documentation see top of this .C macro

    // add files to the chain .....
    sprintf(cmd,"%s*.root",runname.c_str());
    run->Add(cmd);
    
    // open the first file: will be used to retrieve settings
//    sprintf(cmd,"%s_000000.root",runname.c_str());
//    TFile *_file = new TFile(cmd,"READONLY");
    
    // no statistics box in plots
    gStyle->SetOptStat(0);
  
    // what to plot?
    if        (plot_type == "HV") { // High voltage variations compared to the setpoint asa function of time
        plot_HV(ichannel);
    } else if (plot_type == "I"){ // Current as a function of tim
        plot_I(ichannel);
    } else if (plot_type == "env"){ // environment
        plot_env();
    } else {
        cout<< "Wrong plot type selected..... look at documentation in hvmon.C"<<endl;
        return;
    }
    
    // save the plot to file
    if(save_plot) {
        TPad *current_pad = (TPad*)gROOT->GetSelectedPad();
        sprintf(cmd,"plots/%s_channel%i_%s.pdf",plot_type.c_str(),ichannel);;
        current_pad->Print(cmd);
        sprintf(cmd,"plots/%s_channel%i_%s.png",plot_type.c_str(),ichannel);;
        current_pad->Print(cmd);
    }
}
