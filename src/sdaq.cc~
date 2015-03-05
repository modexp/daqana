#include "sdaq.hh"

sdaq::sdaq(){}

sdaq::sdaq(driver* dr){
    // open binary data file
    //daqfile.open(dr->getDataFile().c_str(), ios::binary | ios::in);
    // get some important variables from the driver....
    slowfile.open(dr->getSlowFile().c_str(),ios::binary | ios::in);
    // counter for the number of bytes that have been read
    slowByteRead   = 0;
    // counter for the number of processed events
}

Int_t sdaq::GetSlowFileSize(){
    slowfile.seekg(0, ios::end);
    Int_t file_length = slowfile.tellg();
    slowfile.seekg(0, ios::beg);
    //cout << "The file length is " << file_length << " and divided by doubles..." << file_length/NBYTE_PER_DBL << endl;
    file_length = file_length/(NBYTE_PER_DBL*3); // the three is for the file id, data and time stamp
    //cout << "I think the slow file size is: " << file_length << endl;
    return file_length;
}

inline Double_t sdaq::readDouble(){
    Double_t* dbl_buff(0);
    
    //    char *char0 = new char[NBYTE_PER_INT];
    char buff[NBYTE_PER_DBL];

    slowfile.read(buff,NBYTE_PER_DBL);
    dbl_buff = (Double_t*)buff;
    Double_t i1 = dbl_buff[0];
    //cout << i1 << endl;
    
    slowByteRead += NBYTE_PER_DBL;
    return i1;
}

inline ULong64_t sdaq::readU64(){
    ULong64_t* u64_buff(0);
    
    //    char *char0 = new char[NBYTE_PER_INT];
    char buff[NBYTE_PER_DBL];

    slowfile.read(buff,NBYTE_PER_DBL);
    u64_buff = (ULong64_t*)buff;
    ULong64_t i1 = u64_buff[0];

    slowByteRead += NBYTE_PER_DBL;
    
    //cout << i1 << endl;
    return i1;
}

slowevent* sdaq::readSlowEvent(){
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
  
  //if (slowid == 7) cout << "slowid: " << slowid << " data: " << sdata << " stime: " << stime << endl;
  
  slowevent* sev = new slowevent(slowid, sdata, stime);
  
  
  return sev;
  
}

