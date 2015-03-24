#include "daq.hh"

daq::daq(){
    newEv = NULL;
}

daq::daq(driver* dr){
    newEv = NULL;
    
    // open binary data file
    daqfile.open(dr->getDataFile().c_str(), ios::binary | ios::in);
    // get some important variables from the driver....
    //slowfile.open(dr->getSlowFile().c_str(),ios::binary | ios::in);
    nBytePerArray  = dr->getArraySize();
    nSample        = dr->getNSample();
    initial_timestamp = dr->getInitialTime();
    //initial_timestamp = 3618500646;
    cout << "The initial time is: " << initial_timestamp << endl;
    deltat = dr->getDeltaT();
    // counter for the number of bytes that have been read
    nByteRead      = 0;
    //slowByteRead   = 0;
    // counter for the number of processed events
    nEvent = 0;
}


inline Int_t daq::readInt(){
    int16_t* int_buff(0);
    
    //    char *char0 = new char[NBYTE_PER_INT];
    char buff[NBYTE_PER_INT];
    // if we are at the start of a new array then we read the array header first
    if(nByteRead%(nBytePerArray+ARRAY_HEADER_SIZE) == 0) readArrayHeader();
    
    daqfile.read(buff,NBYTE_PER_INT);
    int_buff = (int16_t*)buff;
    Int_t i1 = int_buff[0];
    //    if(daqfile.read(buff,NBYTE_PER_INT)) memcpy(&i1,buff,NBYTE_PER_INT);
    //    printf("%x %x %i \n",buff[0],buff[1],i1);
    //    delete char0;
    
    //printf("%02x %02x\n",buff[0],buff[1]);
    nByteRead = nByteRead + 2;
    return i1;
}

inline Int_t daq::readADCval(Int_t i1){
    return (i1>>2) & 0x3fff;
}

inline Int_t daq::readFlag(Int_t i1){
    Int_t flag = i1 & 0x0003;
    return flag;
}

void daq::readArrayHeader(){
    // each long array as two ints as a header.
    // first thing we need to do is read these bytes
    char buff[NBYTE_PER_INT];
    daqfile.read(buff,NBYTE_PER_INT);
    daqfile.read(buff,NBYTE_PER_INT);
    
    nByteRead += 4;
    
    return;
}

Double_t daq::readTimestamp() {
    ULong64_t time_array[N_TIME_INT];
    
    for (Int_t i = 0; i < N_TIME_INT; i++) time_array[i] = readInt();
    
    ULong64_t fpgatime =   ( ((int64_t)  time_array[3])        & 0xFFFF ) |
    ( (((int64_t) time_array[2]) << 16) & 0xFFFF0000 ) |
    ( (((int64_t) time_array[1]) << 32) & 0xFFFF00000000 ) |
    ( (((int64_t) time_array[0]) << 48) & 0xFFFF000000000000 );
    //printf("%016llX\n", timestamp);
    
    Double_t timestamp = (Double_t)initial_timestamp + ((Double_t)fpgatime * deltat);
    
    //cout <<"DEBUG:: timestamp = "<<timestamp<<endl;
    return timestamp;
}

event* daq::readEvent(driver* dr) {
    
    Int_t i1, ichan, chanflag;
    i1 = readInt();
    ichan = readADCval(i1);
    ichan -= CHANNEL_OFFSET;
    chanflag = readFlag(i1);
    if (chanflag != 1) cout << "Warning in daq::readEvent: Start bit not in correct place" << endl;
    
    //ULong64_t timestamp = readTimestamp();
    Double_t timestamp = readTimestamp();
    //cout << "Before we define trace" << endl;
    // read the trace
    vector<Double_t> *trace = new vector<Double_t>();
    Bool_t isTestPulse = false;
    Int_t ival, flag;
    //    cout <<"NEXT"<<endl;
    for(Int_t i=0; i<nSample; i++){
        i1 = readInt();
        ival = readADCval(i1);
        //        cout <<i <<" "<<ival<<endl;
        trace->push_back((Double_t)ival);
        flag = readFlag(i1);
        if (flag == 3) {
            isTestPulse = true;
        }
    }
    
    
    nEvent++;
    // create a single event from the channel number, time of the event, and the waveform information
    
    newEv = new event(nEvent,ichan,timestamp,trace,isTestPulse,dr);
    ///event newEv(nEvent,ichan,timestamp,trace,isTestPulse,dr);
    //cout << "After we read event" << endl;
    
    ///return &newEv;
    return newEv;
}


void daq::endEvent(){
    delete newEv;
}
/*
 Int_t daq::GetSlowFileSize(){
 slowfile.seekg(0, ios::end);
 Int_t file_length = slowfile.tellg();
 slowfile.seekg(0, ios::beg);
 cout << "The file length is " << file_length << " and divided by doubles..." << file_length/NBYTE_PER_DBL << endl;
 file_length = file_length/(NBYTE_PER_DBL*3); // the three is for the file id, data and time stamp
 cout << "I think the slow file size is: " << file_length << endl;
 return file_length;
 }
 */
/*
 inline Double_t daq::readDouble(){
 Double_t* dbl_buff(0);
 
 //    char *char0 = new char[NBYTE_PER_INT];
 char buff[NBYTE_PER_DBL];
 
 slowfile.read(buff,NBYTE_PER_DBL);
 dbl_buff = (Double_t*)buff;
 Double_t i1 = dbl_buff[0];
 
 slowByteRead += NBYTE_PER_DBL;
 return i1;
 }
 
 inline ULong64_t daq::readU64(){
 ULong64_t* u64_buff(0);
 
 //    char *char0 = new char[NBYTE_PER_INT];
 char buff[NBYTE_PER_DBL];
 
 slowfile.read(buff,NBYTE_PER_DBL);
 u64_buff = (ULong64_t*)buff;
 ULong64_t i1 = u64_buff[0];
 
 slowByteRead += NBYTE_PER_DBL;
 return i1;
 }
 
 slowevent* daq::readSlowEvent(){
 Int_t slowid;
 Double_t sdata;
 ULong64_t stime;
 
 Double_t numread;
 // read slow id
 numread = readDouble();
 slowid = (Int_t)numread;
 
 //read data
 numread = readDouble();
 sdata = numread;
 
 // read time stamp
 stime = readU64();
 
 cout << "slowid: " << slowid << " data: " << sdata << " stime: " << stime << endl;
 
 slowevent* sev = new slowevent(slowid, sdata, stime);
 
 
 return sev;
 
 }
 */
/*
 void daq::daqClose(){
 
 daqfile.close();
 slowfile.close();
 
 }
 */
