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
    long  readTimestamp();
    int  readInt();
    int  readLongInt();
    void readArrayHeader();
    
    int  nEventPerArray;
    int  nSample;
    int  nEvent;
    int  nByteRead;
    int  nBytePerArray;
    ifstream daqfile;
    event    *ev;
    
};

#endif // __EVENT_H__
