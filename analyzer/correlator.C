#define correlator_cxx
#include "correlator.h"
#include <TH2.h>
#include <TStyle.h>
#include <TCanvas.h>
#include <iostream>

#define CH0 0
#define CH1 1
#define CH2 2
#define CH3 3
#define CH4 4
#define CH5 5
#define CH6 6
#define CH7 7

void correlator::Loop(int isel0, int isel1)
{
    //   In a ROOT session, you can do:
    //      Root > .L correlator.C
    //      Root > correlator t
    //      Root > t.GetEntry(12); // Fill t data members with entry number 12
    //      Root > t.Show();       // Show values of entry 12
    //      Root > t.Show(16);     // Read and show values of entry 16
    //      Root > t.Loop();       // Loop on all entries
    //
    
    //     This is the loop skeleton where:
    //    jentry is the global entry number in the chain
    //    ientry is the entry number in the current Tree
    //  Note that the argument to GetEntry must be:
    //    jentry for TChain::GetEntry
    //    ientry for TTree::GetEntry and TBranch::GetEntry
    //
    //       To read only selected branches, Insert statements like:
    // METHOD1:
    //    fChain->SetBranchStatus("*",0);  // disable all branches
    //    fChain->SetBranchStatus("branchname",1);  // activate branchname
    // METHOD2: replace line
    //    fChain->GetEntry(jentry);       //read all branches
    //by  b_branchname->GetEntry(ientry); //read only this branch
    if (fChain == 0) return;
    
    Long64_t nentries = fChain->GetEntriesFast();
    
    // private code below
    char fname[100];
    sprintf(fname,"cor_ch%02d_ch%02d.root",isel0,isel1);
    TFile *_f = new TFile(fname,"RECREATE");
    
    
    TTree *tr = new TTree("tr","tr");
    Float_t E0,E1,delta_t;
    Int_t   ch0, ch1;
    tr->Branch("E0",&E0,"E0/F");
    tr->Branch("E1",&E1,"E1/F");
    tr->Branch("ch0",&ch0,"ch0/I");
    tr->Branch("ch1",&ch1,"ch1/I");
    tr->Branch("dt",&delta_t,"dt/F");
    
    
    int ncorrelated = 0;
    int i0,i1;
    double t[2];
    double E[2];
    int    ch[2];
    
    ch[0] = -1;
    
    Long64_t nbytes = 0, nb = 0;
    for (Long64_t jentry=0; jentry<nentries;jentry++) {
        Long64_t ientry = LoadTree(jentry);
        if (ientry < 0) break;
        nb = fChain->GetEntry(jentry);   nbytes += nb;
        
        
        if ((channel == isel0 || isel0 == -1) || (channel == isel1 || isel1 == -1)) {
            // second part of a pair
            if(ch[0]>0){
                E[1]  = E[0];
                ch[1] = ch[0];
                t[1]  = t[0];
            }
            
            // first member of a pair
            E[0]  = integral;
            ch[0] = channel;
            t[0]  = time;
            
            // fill ntuple for each pair
            if(jentry>0){
                // order the channel numbers for easy plotting later on
                if(ch[0] < ch[1]){
                    i0 = 0;
                    i1 = 1;
                } else {
                    i0 = 1;
                    i1 = 0;
                }
                
                E0 = E[i0];
                E1 = E[i1];
                ch0 = ch[i0];
                ch1 = ch[i1];
                // this should give you a positive delta t (!)
                delta_t = t[0] - t[1];
                
                tr->Fill();
            }
        }
        // if (Cut(ientry) < 0) continue;
    }
    
    _f->Write();
    _f->Close();
}
