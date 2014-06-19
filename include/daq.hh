#ifndef __DAQ_H__
#define __DAQ_H__

#include "event.hh"
#include "driver.hh"

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <stdint.h>

#define NBYTE_PER_INT     2
#define N_TIME_INT        4
#define N_HEADER_INT      5
#define CHANNEL_OFFSET    100
#define ARRAY_HEADER_SIZE 4

using namespace std;

class daq
{
public:
    daq();
    daq(driver* dr);
    //  ~daq();
    event* readEvent(driver* dr);
    
private:
    ULong64_t  readTimestamp();
    Int_t  readInt();
    void readArrayHeader();
    Int_t readADCval(Int_t read_int);
    Int_t readFlag(Int_t read_int);
    
    Int_t  nEventPerArray;
    Int_t  nSample;
    Int_t  nEvent;
    Int_t  nByteRead;
    Int_t  nBytePerArray;
    ifstream daqfile;
    event    *ev;
    
};

#endif // __EVENT_H__
