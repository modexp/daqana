//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Mon Jun  8 13:31:36 2015 by ROOT version 5.34/25
// from TTree ana/Analyzed spectra
// found on file: /data/atlas/users/acolijn/Modulation/calibration/ANA_mx_n_20150603_1325.root
//////////////////////////////////////////////////////////

#ifndef lifetime_h
#define lifetime_h

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>

// Header file for the classes stored in the TTree if any.

// Fixed size dimensions of array or collections stored in the TTree if any.

class lifetime {
public :
   TChain         *fChain;   //!pointer to the analyzed TTree or TChain
   Int_t           fCurrent; //!current Tree number in a TChain

   // Declaration of leaf types
   Double_t        t0;
   Double_t        time;
   Int_t           channel;
   Int_t           peak;
   Double_t        rate;
   Double_t        drate;
   Double_t        e;
   Double_t        res;
   Double_t        temp;

   // List of branches
   TBranch        *b_t0;   //!
   TBranch        *b_time;   //!
   TBranch        *b_channel;   //!
   TBranch        *b_peak;   //!
   TBranch        *b_rate;   //!
   TBranch        *b_drate;   //!
   TBranch        *b_e;   //!
   TBranch        *b_res;   //!
   TBranch        *b_temp;   //!

   lifetime(string fname);
   virtual ~lifetime();
   virtual Int_t    Cut(Long64_t entry);
   virtual Int_t    GetEntry(Long64_t entry);
   virtual Long64_t LoadTree(Long64_t entry);
   virtual void     Init(TChain *tree);
   virtual void     Life(int channel_sel, int peak_sel, string type, bool save);
   virtual Bool_t   Notify();
   virtual void     Show(Long64_t entry = -1);
};

#endif

#ifdef lifetime_cxx
lifetime::lifetime(string fname) : fChain(0) 
{
    char cmd[256];
    TChain *tree = new TChain("ana");
    sprintf(cmd,"%s/*.root",fname.c_str());
    tree->Add(cmd);
   Init(tree);
}

lifetime::~lifetime()
{
   if (!fChain) return;
   delete fChain->GetCurrentFile();
}

Int_t lifetime::GetEntry(Long64_t entry)
{
// Read contents of entry.
   if (!fChain) return 0;
   return fChain->GetEntry(entry);
}
Long64_t lifetime::LoadTree(Long64_t entry)
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

void lifetime::Init(TChain *tree)
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

   fChain->SetBranchAddress("t0", &t0, &b_t0);
   fChain->SetBranchAddress("time", &time, &b_time);
   fChain->SetBranchAddress("channel", &channel, &b_channel);
   fChain->SetBranchAddress("peak", &peak, &b_peak);
   fChain->SetBranchAddress("rate", &rate, &b_rate);
   fChain->SetBranchAddress("drate", &drate, &b_drate);
   fChain->SetBranchAddress("e", &e, &b_e);
   fChain->SetBranchAddress("res", &res, &b_res);
   fChain->SetBranchAddress("temp", &temp, &b_temp);
   Notify();
}

Bool_t lifetime::Notify()
{
   // The Notify() function is called when a new file is opened. This
   // can be either for a new TTree in a TChain or when when a new TTree
   // is started when using PROOF. It is normally not necessary to make changes
   // to the generated code, but the routine can be extended by the
   // user if needed. The return value is currently not used.

   return kTRUE;
}

void lifetime::Show(Long64_t entry)
{
// Print contents of entry.
// If entry is not specified, print current entry
   if (!fChain) return;
   fChain->Show(entry);
}
Int_t lifetime::Cut(Long64_t entry)
{
// This function may be called from Loop.
// returns  1 if entry is accepted.
// returns -1 otherwise.
   return 1;
}
#endif // #ifdef lifetime_cxx
