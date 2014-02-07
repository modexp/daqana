#include "daq.hh"

daq::daq(){}

daq::daq(driver* dr){
    // open binary data file
    daqfile.open(dr->getDataFile().c_str(), ios::binary | ios::in);
    // get some important variables from the driver....
    nBytePerArray  = dr->getArraySize();
    nSample        = dr->getNSample();
    // counter for the number of bytes that have been read
    nByteRead      = 0;
    // counter for the number of processed events
    nEvent = 0;
}


inline int daq::readInt(){
    int16_t* int_buff(0);
    
    //    char *char0 = new char[NBYTE_PER_INT];
    char buff[NBYTE_PER_INT];
    // if we are at the start of a new array then we read the array header first
    if(nByteRead%(nBytePerArray+ARRAY_HEADER_SIZE) == 0) readArrayHeader();

    daqfile.read(buff,NBYTE_PER_INT);
    int_buff = (int16_t*)buff;
    int  i1 = 0x00;
    i1 = int_buff[0];
    i1 = ( i1 >> 2);
    //    if(daqfile.read(buff,NBYTE_PER_INT)) memcpy(&i1,buff,NBYTE_PER_INT);
    //    printf("%x %x %i \n",buff[0],buff[1],i1);
    //    delete char0;

    //printf("%02x %02x\n",buff[0],buff[1]);
    nByteRead = nByteRead + 2;
    return i1;
}

void daq::readArrayHeader(){
    // each long array as two ints as a header.
    // first thing we need to do is read these bytes
    char buff[NBYTE_PER_INT];
    daqfile.read(buff,NBYTE_PER_INT);
    daqfile.read(buff,NBYTE_PER_INT);

    nByteRead += 4;
}

inline int daq::readLongInt(){
    
    int i1=0x00;
    char buff[NBYTE_PER_INT];
    
    // if we are at the start of a new array then we read the array header first
    if(nByteRead%(nBytePerArray+ARRAY_HEADER_SIZE) == 0) readArrayHeader();
    
    if(daqfile.read(buff,NBYTE_PER_INT)) memcpy(&i1,buff,NBYTE_PER_INT);
    //    printf("%x %x %i \n",char0[0],char0[1],i1);
    //    delete char0;
    
    nByteRead = nByteRead + 2;
    //printf("%02x %02x\n",buff[0],buff[1]);
    return i1;
}

long daq::readTimestamp(){
    int num16(32768);
    long time_array[N_TIME_INT];
    // read the timing information
    for(int i=0; i<N_TIME_INT; i++){
        time_array[i] = readLongInt();
        if(time_array[i] < 0) time_array[i] = num16 + (num16 + time_array[i]);
    }
    
    long timestamp = time_array[3] + (time_array[2]*(num16*2)) + (time_array[1]*(num16*2)*(num16*2)) + (time_array[0]*(num16*2)*(num16*2)*(num16*2));
    
    return timestamp;
}

event* daq::readEvent(driver* dr) {

    // read the channel number
    //int ichan = (readInt()>>2) - CHANNEL_OFFSET;
    int  ichan = readInt() - CHANNEL_OFFSET;
    long timestamp = readTimestamp();
    //cout << "Before we define trace" << endl;
    // read the trace
    vector<double> *trace = new vector<double>();
//    cout <<"NEXT"<<endl;
    for(int i=0; i<nSample; i++){
        int ival = readInt();
//        cout <<i <<" "<<ival<<endl; 
        trace->push_back((double)ival);
    }
    
    nEvent++;
    // create a single event from the channel number, time of the event, and the waveform information
    event* newEv = new event(nEvent,ichan,timestamp,trace,dr);    
    //cout << "After we read event" << endl;
    
    return newEv;
}

