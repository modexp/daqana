#ifndef ecal_h
#define ecal_h
#include <iostream>
#include <stdio.h>
#include <TROOT.h>
#include <TH1F.h>
#include <TChain.h>
#include <TFile.h>
#include <string>
#include <vector>

/*----------------------------------------------------------------------------*/

#define NUMBER_OF_SOURCES 6
#define BACKGROUND 0
#define TI44       1
#define CO60       2
#define CS137      3
#define MN54       4
#define K40        5

/*----------------------------------------------------------------------------*/

#define NUMBER_OF_CHANNELS 8
#define MAX_PEAKS 5
#define MAX_PARAMETERS 3

#define BEFORE_CALIBRATION 0
#define AFTER_CALIBRATION 1

//
// CALIBRATION_MODE 0 = one calibration for full run
// CALIBRATION_MODE 1 = one calibration every TIME_INTERVAL seconds
//
#define CALIBRATION_MODE 1
#define TIME_INTERVAL 1800 

/*----------------------------------------------------------------------------*/

// Header file for the classes stored in the TTree if any.

// Fixed size dimensions of array or collections stored in the TTree if any.

class ecal {
public :
   string          calFile;
   TChain          *fChain;   //!pointer to the analyzed TTree or TChain
   Int_t           fCurrent; //!current Tree number in a TChain

   // Declaration of leaf types
   Int_t           channel;
   Float_t         integral;
   Float_t         height;
   Double_t        time;
   UChar_t         istestpulse;
   Int_t           error;
   Float_t         baseline;
   Float_t         rms;

   // List of branches
   TBranch        *b_channel;   //!
   TBranch        *b_integral;   //!
   TBranch        *b_height;   //!
   TBranch        *b_time;   //!
   TBranch        *b_istestpulse;   //!
   TBranch        *b_error;   //!
   TBranch        *b_baseline;   //!
   TBranch        *b_baselineRMS;   //!

   ecal(string fname,string cname);
   virtual ~ecal();
   virtual Int_t    Cut(Long64_t entry);
   virtual Int_t    GetEntry(Long64_t entry);
   virtual Long64_t LoadTree(Long64_t entry);
   virtual void     Init(TChain *tree);
   virtual void     Loop();
   virtual Bool_t   Notify();
   virtual void     Show(Long64_t entry = -1);
    
   // histograms for calibration fitting
    void book_histograms();
    void fill_histograms(int ilevel);
    void ecal_single();
    void ecal_continuous();
    void do_calibration();
    void reset_histograms();
    void fill_tree(Double_t t0, Double_t t1);
    void get_source_id();
    
    
    TFile *_f;
    
    std::vector<TH1F*> _integral;
    std::vector<TH1F*> _energy;
    std::vector<TH1F*> _energy_all;
    //
    // time information: needed if you wish to have calibrations in continuous mode
    //
    Double_t t0,tstart,time_since_start;
    //
    // calibration output tree
    //
    TTree *_cal_tree;
    
    Int_t _cal_index;
    Double_t _cal_tmin;
    Double_t _cal_tmax;
    std::vector<Double_t> _cal_c0;
    std::vector<Double_t> _cal_c1;
    std::vector<Double_t> _cal_c2;
    std::vector<Double_t> _cal_chi2;
    
    double ccal[NUMBER_OF_CHANNELS][MAX_PARAMETERS];
    double cal_quality[NUMBER_OF_CHANNELS];
    
    Int_t source_id[NUMBER_OF_CHANNELS];
    Bool_t channel_active[NUMBER_OF_CHANNELS];
    
};

#endif

#ifdef ecal_cxx


ecal::ecal(string fname,string cname) : fChain(0) 
{
    char cmd[256];
    TChain *tree = new TChain("T");
    sprintf(cmd,"%s*.root",fname.c_str());
    tree->Add(cmd);
    
    calFile = cname;
//    size_t pos = fileName.find_last_of("/")+1;
//    fileName = fileName.substr(pos);
//    pos = fileName.find_last_of("_");
//    fileName = "CAL_"+fileName.substr(0,pos)+".root";

//    cout<< fileName<<endl;
    
    Init(tree);
}

ecal::~ecal()
{
   if (!fChain) return;
   delete fChain->GetCurrentFile();
}

Int_t ecal::GetEntry(Long64_t entry)
{
// Read contents of entry.
   if (!fChain) return 0;
   return fChain->GetEntry(entry);
}
Long64_t ecal::LoadTree(Long64_t entry)
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

void ecal::Init(TChain *tree)
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
    Notify();
}


Bool_t ecal::Notify()
{
   // The Notify() function is called when a new file is opened. This
   // can be either for a new TTree in a TChain or when when a new TTree
   // is started when using PROOF. It is normally not necessary to make changes
   // to the generated code, but the routine can be extended by the
   // user if needed. The return value is currently not used.

   return kTRUE;
}

void ecal::Show(Long64_t entry)
{
// Print contents of entry.
// If entry is not specified, print current entry
   if (!fChain) return;
   fChain->Show(entry);
}
Int_t ecal::Cut(Long64_t entry)
{
// This function may be called from Loop.
// returns  1 if entry is accepted.
// returns -1 otherwise.
   return 1;
}
#endif // #ifdef ecal_cxx
