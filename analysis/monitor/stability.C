#ifndef __headers__
#define __headers__
#include <vector>
#endif

//
// Plot rate, energy, resolution as a function of time.
//
// Input: rootfile  - input ANA rootfile(s). Wild cards work if you wish to analyze a long chain of runs!
//        var       - "e", "rate","res"
//        type      - "abs", "rel"
//        save_plot - save plot to .pdf file (or other format)
//
// A.P. Colijn
//

#define NUMBER_OF_CHANNELS 8
#define MAX_PEAKS 5

TChain *run = new TChain("ana");
TH1F *_hld;
TLegend *_leg;

// global variables
Double_t tmin, tmax;

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

    _hld = new TH1F("hld","hld",1,tmin,tmax);
    sprintf(hname,"%s",var.c_str());
    
    _hld->SetTitle(hname);
    _hld->GetXaxis()->SetTimeFormat("%d/%m");
    _hld->GetXaxis()->SetTimeDisplay(1);
    _hld->GetXaxis()->SetTitle("time");
    _hld->GetYaxis()->SetRangeUser(0.,300);
    _hld->Draw();
    if(type == "rel") {
        _hld->GetYaxis()->SetRangeUser(-0.05,0.05);
        _hld->GetYaxis()->SetTitle("(v - v(t=0)) / v(t=0)");
    } else {
        string yname;
        if ( var == "rate" ){
            yname = "Rate (Hz)";
        } else if ( var == "e" ){
            yname = "Energy (keV)";
        } else if ( var == "res") {
            yname = "FWHM/E";
        }
        _hld->GetYaxis()->SetTitle(yname.c_str());
    }
    
    if(type == "abs"){
        sprintf(cmd,"%s",var.c_str());
        Double_t ymax = run->GetMaximum(cmd);
        _hld->GetYaxis()->SetRangeUser(0.,1.6*ymax);
    }
    
    // make a legend
    _leg = new TLegend(0.65,0.63,0.95,0.89);
    _leg->SetFillStyle(0);
    
}
/*-------------------------------------------------------------------------------*/
void initTree(string rootfile){
    //
    // (i) init tree
    // (ii) get time range
    //
    char cmd[128];
    // add files to the chain .....
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
        
        f.Close();
    }
}
/*-------------------------------------------------------------------------------*/
void initVariable(string var){
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
void stability(string rootfile, string var, string type, bool save_plot){
    // documentation on top....
    char hname[128],cmd[128],cut[128];
    cout << "chain_ana:: plotting routine"<<endl;

    //
    // (i) initialize the chain of trees
    // (ii) retrieve the time range
    //
    initTree(rootfile);
    
    //
    // initialize the histogram on which all graphs will be drawn
    //
    initHistogram(var, type);
    
    //
    // get the values at t=0 for the variable you want to plot (needed
    // if you want to plot the relative value of a variable
    //
    initVariable(var);

    //
    // loop over all the channels
    //
    for(int ich = 0; ich<NUMBER_OF_CHANNELS; ich++){
        
        if(type == "abs"){
            sprintf(cmd,"%s:t0+time",var.c_str());
            cout << " cmd ="<<cmd<<endl;
        } else {
            sprintf(cmd,"(%s-%f)/%f:t0+time",var.c_str(),v0[ich][0],v0[ich][0]);
        }
        //sprintf(cut,"(channel==%d)",ich);
        
        sprintf(cut,"(channel==%d)",ich);
        run->SetMarkerStyle(1);
        run->SetMarkerColor(ich+1);
        run->SetLineColor(ich+1);
        run->Draw(cmd,cut,"same");
        
        // add legend entry
        sprintf(cmd, "ch%d - %s ", ich, source_name[ich].c_str());
        TLine *l1 = new TLine();
        l1->SetLineColor(ich+1);
        _leg->AddEntry(l1,cmd,"l");
    }

    // draw the legend
    _leg->SetBorderSize(0);
    _leg->Draw();
    
    c1->Update();
    
    if(save_plot){
        string figname = "plots/"+var+"_"+type+".pdf";
        c1->Print(figname.c_str());
        figname = "plots/"+var+"_"+type+".png";
        c1->Print(figname.c_str());
    }
    return;
}
