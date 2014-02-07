#ifndef __ROOTDRIVER_H__
#define __ROOTDRIVER_H__

#include <TFile.h>
#include <TTree.h>


#include <vector>
#include <string>

#include "event.hh"
#include "driver.hh"

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
    
    int   chanNum;
    float integral;
    float pkheight;
    long  timestamp;
    int   isTestPulse;
    int   errorCode;
    
    float baseline;
    float baselineRMS;
    
    bool longRoot;
};

#endif // __ROOTDRIVER_H__
