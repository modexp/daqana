#ifndef __headers__
#define __headers__
#include <vector>
#endif

//
// Plot resolution as a function of energy
//
// Input: rootfile  - input ANA root filename(s). Wild cards work to analyze more than one run at once
//        save_plot - save plot to .pdf file (or other format)
//
// A.P. Colijn
//

#define NUMBER_OF_CHANNELS 8
#define MAX_PEAKS 5

//TChain *run = new TChain("ana");
TProfile *_hld;
TLegend *_leg;

// global variables
Double_t emin, emax;

Double_t v_tmp;
Int_t    ich_tmp;
Int_t    ipk_tmp;

// initial values of variable that os selected
float v0[NUMBER_OF_CHANNELS][MAX_PEAKS];

// source names
string source_name[] = {
    "background",
    "background",
    "^{44}Ti",
    "^{44}Ti",
    "^{60}Co",
    "^{60}Co",
    "^{137}Cs",
    "^{137}Cs"
};

float source_energy[NUMBER_OF_CHANNELS][MAX_PEAKS] =
//
// the energy peaks you wish to select should be in this list
// NOTE: the first peak should be the highest in the spectrum (sub-optimal, but handy for finding)
//
{
    {1460,-1,-1,-1,-1}, // channel0: no source
    {-1,-1,-1,-1,-1}, // channel1: no source
    {511.,1157.020,511.+1157.020,-1,-1}, // channel2: 44Ti
    {511.,1157.020,511.+1157.020,-1,-1}, // channel3: 44Ti
    {1173.2,1332.5,1173.2+1332.5,-1,-1}, // channel4: 60Co
    {1173.2,1332.5,1173.2+1332.5,-1,-1}, // channel5: 60Co
    {662.,-1,-1,-1,-1}, // channel6: 137Cs
    {662.,-1,-1,-1,-1}  // channel7: 137Cs
};

TCanvas *c1 = new TCanvas("c1","c1",800,400);
/*-------------------------------------------------------------------------------*/
string get_figname(string fname){
    // extract figure name from the filename
    string figname = "";
    figname = fname;
    size_t pos = figname.find_last_of("/")+1;
    figname = figname.substr(pos);
    pos = figname.find_last_of(".");
    figname = figname.substr(0,pos);
    pos = figname.find_first_of("_")+1;
    figname = figname.substr(pos);
    
    return figname;
}
/*-------------------------------------------------------------------------------*/
void initHistogram(string var, string type){
    //
    // init histogram on which to plot graphs
    //
    char hname[128], cmd[128];
    
    gStyle->SetOptStat(0);

    _hld = new TProfile("hld","hld",100,0,3000,0.,0.2);
    sprintf(hname,"%s",var.c_str());
    
    _hld->SetTitle(hname);
    _hld->GetXaxis()->SetTitle("Energy (keV)");
    _hld->GetYaxis()->SetTitle("FWHM/E");
    _hld->GetYaxis()->SetRangeUser(0.,0.1);
    
}
/*-------------------------------------------------------------------------------*/
TChain * initTree(string rootfile){
    //
    // (i) init tree
    // (ii) get time range
    //
    char cmd[128];
    // add files to the chain .....
    TChain *run = new TChain("ana");
    sprintf(cmd,"%s*.root",rootfile.c_str());
    run->Add(cmd);
    
    tmin = 9e99;
    tmax = 0;
    
    // loop over the files....
    TObjArray *fileElements=run->GetListOfFiles();
    TIter next(fileElements);
    TChainElement *chEl=0;
    while (( chEl=(TChainElement*)next() )) {
        TFile f(chEl->GetTitle());
        cout << f.GetName() <<endl;
        
        TParameter<Double_t> *t0      = (TParameter<Double_t>*)gDirectory->Get("t0");
        TParameter<Double_t> *runtime = (TParameter<Double_t>*)gDirectory->Get("runtime");
        if(t0->GetVal()<tmin) tmin = t0->GetVal();
        if(t0->GetVal()+runtime->GetVal()>tmax) tmax = t0->GetVal()+runtime->GetVal();
    cout <<"tmin = "<<tmin<<" tmax = "<<tmax<<endl;
        
        f.Close();
    }
    return run;
}
/*-------------------------------------------------------------------------------*/
void initVariable(string var, TChain *run){
    char cmd[128];
    
    sprintf(cmd,"%s",var.c_str());
    
    cout << "cmd "<<cmd<<endl;
    run->SetBranchAddress(cmd,&v_tmp);
    run->SetBranchAddress("channel",&ich_tmp);
    run->SetBranchAddress("peak",&ipk_tmp);
    for(int i=MAX_PEAKS*NUMBER_OF_CHANNELS; i>=0; i--){
        run->GetEntry(i);
        v0[ich_tmp][ipk_tmp] = v_tmp;
    }
}
/*-------------------------------------------------------------------------------*/

//
// MAIN ROUTINE
//
void resolution(string rootfile, bool save_plot){
    //
    // Plot resolution as a function of energy
    //
    // Input: rootfile  - input root filename
    //        save_plot - save plot to .pdf file (or other format)
    //
    // A.P. Colijn
    //
    char hname[128],cmd[128],cut[128];
    cout << "chain_ana:: plotting routine"<<endl;

    //
    // (i) initialize the chain of trees
    // (ii) retrieve the time range
    //
    TChain *run = initTree(rootfile);
    
    //
    // initialize the histogram on which all graphs will be drawn
    //
    initHistogram("xxx", "yyy");
    
    //
    // get the values at t=0 for the variable you want to plot (needed
    // if you want to plot the relative value of a variable
    //
  //  initVariable("xxx");

    sprintf(cmd,"res:e>>hld");
        
    TF1 *myfit = new TF1("resfit","sqrt(pow([0]/sqrt(x),2)+pow([1],2))", 400, 2600);
    myfit->SetParameter(0,2);
    myfit->SetParameter(1,0.01);
    //TF1 *myfit = new TF1("resfit","[0]/sqrt(x)", 400, 3000);

    run->Draw(cmd,"channel>1","prof");
    _hld->SetTitle("Resolution");
    _hld->SetMarkerStyle(24);
    _hld->SetMarkerColor(1);
    _hld->SetLineColor(1);
    _hld->Fit("resfit");
    _hld->Draw();
        

    // draw the legend
    
    c1->Update();
    
    if(save_plot){
        string figname = "plots/res_vs_e.pdf";
        c1->Print(figname.c_str());
        figname = "plots/res_vs_e.png";
        c1->Print(figname.c_str());
    }
    return;
}
