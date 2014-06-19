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
  Int_t i1_adc = i1 >> 2;
  return i1_adc;
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
}

ULong64_t daq::readTimestamp() {
  ULong64_t time_array[N_TIME_INT];
  
  for (Int_t i = 0; i < N_TIME_INT; i++) time_array[i] = readInt();

  ULong64_t timestamp =   ( ((int64_t)  time_array[3])        & 0xFFFF ) | 
                       ( (((int64_t) time_array[2]) << 16) & 0xFFFF0000 ) | 
                       ( (((int64_t) time_array[1]) << 32) & 0xFFFF00000000 ) | 
                       ( (((int64_t) time_array[0]) << 48) & 0xFFFF000000000000 );
		       //printf("%016llX\n", timestamp);

return timestamp;
}

event* daq::readEvent(driver* dr) {

    // read the channel number
    //int ichan = (readInt()>>2) - CHANNEL_OFFSET;
    Int_t i1, ichan, chanflag;
    i1 = readInt();
    ichan = readADCval(i1);
    ichan -= CHANNEL_OFFSET;
    chanflag = readFlag(i1);
    //cout << "Channel = " << ichan << " flag = " << chanflag << endl;
    if (chanflag != 1) cout << "Warning in daq::readEvent: Start bit not in correct place" << endl;
    if (chanflag != 1) cout << "Channel = " << ichan << " flag = " << chanflag << endl;
    

    ULong64_t timestamp = readTimestamp();
    //if (chanflag != 1) cout << timestamp << endl;
    //cout << "Before we define trace" << endl;
    // read the trace
    vector<Double_t> *trace = new vector<Double_t>();
    Bool_t isTestPulse = false;
    Int_t ival, flag;
    //cout <<"NEXT"<<endl;
    for(Int_t i=0; i<nSample; i++){
        i1 = readInt();
	ival = readADCval(i1);
        //if (chanflag != 1) cout <<i <<" "<<ival<<endl; 
        trace->push_back((Double_t)ival);
	flag = readFlag(i1);
	if (flag == 3) isTestPulse = true;
    }
    
    nEvent++;
    // create a single event from the channel number, time of the event, and the waveform information
    event* newEv = new event(nEvent,ichan,timestamp,trace,isTestPulse,dr);    
    //cout << "After we read event" << endl;
    
    return newEv;
}

