//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Tue Mar  3 13:48:08 2015 by ROOT version 5.34/21
// from TTree T/Source data
// found on file: mx_n_20150226_1552_000000.bin.root
//////////////////////////////////////////////////////////

#ifndef correlator_h
#define correlator_h

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>

// Header file for the classes stored in the TTree if any.

// Fixed size dimensions of array or collections stored in the TTree if any.

class correlator {
public :
   string fileName;
   TTree          *fChain;   //!pointer to the analyzed TTree or TChain
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

   correlator(TTree *tree=0);
   correlator(string fname);

   virtual ~correlator();
   virtual Int_t    Cut(Long64_t entry);
   virtual Int_t    GetEntry(Long64_t entry);
   virtual Long64_t LoadTree(Long64_t entry);
   virtual void     Init(TTree *tree);
   virtual void     Loop(int isel0, int isel1);
   virtual Bool_t   Notify();
   virtual void     Show(Long64_t entry = -1);
};

#endif

#ifdef correlator_cxx
correlator::correlator(TTree *tree) : fChain(0) 
{
// if parameter tree is not specified (or zero), connect the file
// used to generate this class and read the Tree.
   if (tree == 0) {
      TFile *f = (TFile*)gROOT->GetListOfFiles()->FindObject("../../rootoutput/mx_n_20150310_1201_000000.bin.root");
      if (!f || !f->IsOpen()) {
         f = new TFile("../../rootoutput/mx_n_20150310_1201_000000.bin.root");
      }
      f->GetObject("T",tree);

   }
   Init(tree);
}

correlator::correlator(string fname) : fChain(0)
{
    fileName = fname;
    TTree *tree;
    TFile *f = new TFile(fileName.c_str());
    f->GetObject("T",tree);
    
    Init(tree);
}

correlator::~correlator()
{
   if (!fChain) return;
   delete fChain->GetCurrentFile();
}

Int_t correlator::GetEntry(Long64_t entry)
{
// Read contents of entry.
   if (!fChain) return 0;
   return fChain->GetEntry(entry);
}
Long64_t correlator::LoadTree(Long64_t entry)
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

void correlator::Init(TTree *tree)
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

Bool_t correlator::Notify()
{
   // The Notify() function is called when a new file is opened. This
   // can be either for a new TTree in a TChain or when when a new TTree
   // is started when using PROOF. It is normally not necessary to make changes
   // to the generated code, but the routine can be extended by the
   // user if needed. The return value is currently not used.

   return kTRUE;
}

void correlator::Show(Long64_t entry)
{
// Print contents of entry.
// If entry is not specified, print current entry
   if (!fChain) return;
   fChain->Show(entry);
}
Int_t correlator::Cut(Long64_t entry)
{
// This function may be called from Loop.
// returns  1 if entry is accepted.
// returns -1 otherwise.
   return 1;
}
#endif // #ifdef correlator_cxx
