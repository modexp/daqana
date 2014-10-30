#ifndef __SEVENT_H__
#define __SEVENT_H__

#include "TCanvas.h"
#include "TGraph.h"
#include "driver.hh"
#include <vector>
#include <stdlib.h>
#include <stdint.h>
#include "TMath.h"


using namespace std;

class slowevent
{
public:
      slowevent();
      slowevent(Int_t sid, Double_t sdata, ULong64_t sts);
      ~slowevent();
      Int_t	getSlowID(){return slowid;};
      Double_t	getSlowData(){return slowdata;};
      ULong64_t 	getSlowTimeStamp(){return slowtimestamp;};
private:
      Int_t	slowid;
      Double_t	slowdata;
      ULong64_t 	slowtimestamp;
};
 #endif // __SEVENT_H__
