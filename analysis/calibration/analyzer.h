#ifndef analyzer_h
#define analyzer_h

#include <RooRealVar.h>
#include <RooFitResult.h>
#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>
#include <TH1F.h>
#include <TH2F.h>
#include <string>
#include <vector>
#include <iostream>

/*----------------------------------------------------------------------------*/

#define TIME_INTERVAL 3600
#define PLOT_ON_SCREEN 0

/*----------------------------------------------------------------------------*/
#define NUMBER_OF_CHANNELS 8

/*----------------------------------------------------------------------------*/
#define NUMBER_OF_SOURCES  6
#define BACKGROUND 0
#define TI44       1
#define CO60       2
#define CS137      3
#define MN54       4
#define K40        5

/*----------------------------------------------------------------------------*/

// Header file for the classes stored in the TTree if any.

// Fixed size dimensions of array or collections stored in the TTree if any.

class analyzer {

    public :
    RooFitResult *fr;
    string fileName, analyzer_file, run;
    
    
    TChain          *fChain;   //!pointer to the analyzed TTree or TChain
    Int_t           fCurrent; //!current Tree number in a TChain
    
    // Declaration of leaf types
    Int_t           channel;
    Float_t         integral;
    Float_t         height;
    Double_t        time;
    UChar_t         istestpulse;
    Int_t           error;
    Float_t         ratio;
    Float_t         baseline;
    Float_t         rms;
    Double_t        temp;
    Double_t        pres;
    Double_t        bx;
    Double_t        by;
    Double_t        bz;
    Double_t        btot;
    Double_t        humid;
    
    
    
    // List of branches
    TBranch        *b_channel;   //!
    TBranch        *b_integral;   //!
    TBranch        *b_height;   //!
    TBranch        *b_time;   //!
    TBranch        *b_istestpulse;   //!
    TBranch        *b_error;   //!
    TBranch        *b_ratio;   //!
    TBranch        *b_baseline;   //!
    TBranch        *b_baselineRMS;   //!
    TBranch        *b_temp;   //!
    TBranch        *b_pres;   //!
    TBranch        *b_bx;   //!
    TBranch        *b_by;   //!
    TBranch        *b_bz;   //!
    TBranch        *b_btot;   //!
    TBranch        *b_humid;   //!
    
    analyzer(string fname, string cname);
    virtual ~analyzer();
    virtual Int_t    Cut(Long64_t entry);
    virtual Int_t    GetEntry(Long64_t entry);
    virtual Long64_t LoadTree(Long64_t entry);
    virtual void     Init(TChain *tree);
    virtual void     Loop();
    virtual Bool_t   Notify();
    virtual void     Show(Long64_t entry = -1);
    
    
    void book_histograms();
    void fill_histograms();
    void reset_interval_data();
    void add_interval_data();
    void calculate_interval_data();
    void get_interval_data();
    void write_histograms();
    void fit_spectrum(int ichannel);
    void fit_spectrum_simple(int ichannel);
    void fit_spectrum(int ichannel, double *fit_range);
    void addTreeEntry(Double_t E, Double_t R, Double_t dR, Double_t res, Int_t ich, Int_t ipk);
    void processFitData(RooRealVar N, RooRealVar f, RooRealVar E, RooRealVar sig, int ichannel, int ipeak);
    void get_source_id();
    
    double covariance(int i, int j);
    // histograms
    TFile *_f;
    vector<TH1F*> _e_all,_e_good,_e_err1,_e_err2,_e_err4,_pk_tmp;
    vector<TH1F*> _b_good,_b_err1,_b_err2,_b_err4;
    vector<TH2F*> _2d_good,_2d_err1,_2d_err2,_2d_err4;
    //TH1F *_T;
    
    TTree *tree;
    
    Double_t    _t_t0;
    Double_t    _t_time;
    Int_t       _t_chanNum;
    Int_t       _t_peakNum;
    Double_t    _t_rate;
    Double_t    _t_drate;
    Double_t    _t_energy;
    Double_t    _t_res;
    Double_t    _t_temp;
    
    Double_t    _t_pres;
    Double_t    _t_bx;
    Double_t    _t_by;
    Double_t    _t_bz;
    Double_t    _t_btot;
    Double_t    _t_humid;
    
    // time information
    Double_t t0,tstart,time_since_start,delta_t;
    
    // event counter in interval
    Int_t n_interval;

    // which source is a which channel?
    Int_t source_id[NUMBER_OF_CHANNELS];
    Bool_t channel_active[NUMBER_OF_CHANNELS];


};

#endif

#ifdef analyzer_cxx


analyzer::analyzer(string fname, string cname) : fChain(0)
{
    char cmd[256];
    TChain *tree = new TChain("T");
    sprintf(cmd,"%s*.root",fname.c_str());
    tree->Add(cmd);
    
    analyzer_file = cname;
    
    // extract the run number from the input directory
    
    // replace double // by / ..... just in case
    //    fname.replace(fname.find("//"),2,"/");
    size_t pos = fname.find_last_of("/");
    bool ok = true;
    size_t index = pos-1;
    // find the / before the last one: the run number is between these characters
    while(ok){
        if(fname.substr(index,1) == "/") {
            ok =false;
        } else {
            index--;
        }
    }
    run = fname.substr(index+1, pos - index - 1);
    cout << "analyzer:: run = "<<run<<endl;
    
    
    Init(tree);
}

analyzer::~analyzer()
{
    if (!fChain) return;
    delete fChain->GetCurrentFile();
}

Int_t analyzer::GetEntry(Long64_t entry)
{
    // Read contents of entry.
    if (!fChain) return 0;
    return fChain->GetEntry(entry);
}
Long64_t analyzer::LoadTree(Long64_t entry)
{
    // Set the environment to read one entry
    if (!fChain) return -5;
    Long64_t centry = fChain->LoadTree(entry);
    if (centry < 0) return centry;
    if (fChain->GetTreeNumber() != fCurrent) {
        fCurrent = fChain->GetTreeNumber();
        Notify();
    }
    return centry;
}

void analyzer::Init(TChain *tree)
{
    // The Init() function is called when the selector needs to initialize
    // a new tree or chain. Typically here the branch addresses and branch
    // pointers of the tree will be set.
    // It is normally not necessary to make changes to the generated
    // code, but the routine can be extended by the user if needed.
    // Init() will be called many times when running on PROOF
    // (once per file to be processed).
    
    // Set branch addresses and branch pointers
    if (!tree) return;
    fChain = tree;
    fCurrent = -1;
    fChain->SetMakeClass(1);
    
    fChain->SetBranchAddress("channel", &channel, &b_channel);
    fChain->SetBranchAddress("integral", &integral, &b_integral);
    fChain->SetBranchAddress("height", &height, &b_height);
    fChain->SetBranchAddress("time", &time, &b_time);
    fChain->SetBranchAddress("istestpulse", &istestpulse, &b_istestpulse);
    fChain->SetBranchAddress("error", &error, &b_error);
    fChain->SetBranchAddress("baseline", &baseline, &b_baseline);
    fChain->SetBranchAddress("rms", &rms, &b_baselineRMS);
    fChain->SetBranchAddress("ratio", &ratio, &b_ratio);
    fChain->SetBranchAddress("temp", &temp, &b_temp);
    
    fChain->SetBranchAddress("pres", &pres, &b_pres);
    fChain->SetBranchAddress("bx", &bx, &b_bx);
    fChain->SetBranchAddress("by", &by, &b_by);
    fChain->SetBranchAddress("bz", &bz, &b_bz);
    fChain->SetBranchAddress("btot", &btot, &b_btot);
    fChain->SetBranchAddress("humid", &humid, &b_humid);
    
    Notify();
}


Bool_t analyzer::Notify()
{
    // The Notify() function is called when a new file is opened. This
    // can be either for a new TTree in a TChain or when when a new TTree
    // is started when using PROOF. It is normally not necessary to make changes
    // to the generated code, but the routine can be extended by the
    // user if needed. The return value is currently not used.
    
    return kTRUE;
}

void analyzer::Show(Long64_t entry)
{
    // Print contents of entry.
    // If entry is not specified, print current entry
    if (!fChain) return;
    fChain->Show(entry);
}
Int_t analyzer::Cut(Long64_t entry)
{
    // This function may be called from Loop.
    // returns  1 if entry is accepted.
    // returns -1 otherwise.
    return 1;
}
#endif // #ifdef analyzer_cxx
