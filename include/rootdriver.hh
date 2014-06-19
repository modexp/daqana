#ifndef __ROOTDRIVER_H__
#define __ROOTDRIVER_H__

#include <TFile.h>
#include <TTree.h>


#include <vector>
#include <string>

#include "event.hh"
#include "driver.hh"

#include <stdint.h>

using namespace std;


class rootdriver
{
public:
    rootdriver();
    rootdriver(driver *drv, bool);
    void Fill(event *ev);
    void writeParameters(driver *drv);
    void Close();
    
private:
    TFile *f;
    TTree *tree;
    
    Int_t   chanNum;
    Float_t integral;
    Float_t pkheight;
    ULong64_t  timestamp;
    Bool_t   isTestPulse;
    Int_t   errorCode;
    
    Float_t baseline;
    Float_t baselineRMS;
    
    Bool_t longRoot;
};

#endif // __ROOTDRIVER_H__
