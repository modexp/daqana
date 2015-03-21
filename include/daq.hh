#ifndef __DAQ_H__
#define __DAQ_H__

#include "event.hh"
#include "driver.hh"

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <stdint.h>
#include <TROOT.h>

#define NBYTE_PER_INT     2
#define N_TIME_INT        4
#define N_HEADER_INT      5
#define CHANNEL_OFFSET    100
#define ARRAY_HEADER_SIZE 4
#define NBYTE_PER_DBL     8

using namespace std;

class daq
{
public:
    daq();
    daq(driver* dr);
    //~daq();
    //void daqClose();
    event* readEvent(driver* dr);
    //slowevent* readSlowEvent();
    //Int_t   GetSlowFileSize();
    void endEvent();
    
private:
    event   *newEv;
    Double_t  readTimestamp();
    Int_t   readInt();

    Int_t   readLongInt();
    Int_t   readADCval(Int_t i1);
    Int_t   readFlag(Int_t i1);
    void  readArrayHeader();
    //Double_t readDouble();
    //ULong64_t readU64();
    
    Int_t  nEventPerArray;
    Int_t  nSample;
    Int_t  nEvent;
    Int_t  nByteRead;
    //Int_t  slowByteRead;
    Int_t  nBytePerArray;
    ULong64_t	initial_timestamp;
    Double_t	deltat;
    ifstream daqfile;
    //ifstream slowfile;
    //event    *ev;
    //slowevent *sev;
    
};

#endif // __EVENT_H__
