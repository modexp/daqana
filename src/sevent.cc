#include "sevent.hh"
#include <iostream>
#include <math.h>

slowevent::slowevent(){}

slowevent::slowevent(Int_t sid, Double_t sd, ULong64_t sts){
  // slow id (identifies parameter type, e.g. temperature, pressure, etc.)
    slowid = sid;
    // actual data
    slowdata = sd;
    // slow timestamp
    slowtimestamp = sts;
}

slowevent::~slowevent() {
  // nothing to do here
}