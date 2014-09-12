#include "TApplication.h"

#include <string>
#include <sstream>
#include <cstdio>
#include <cstdlib>

#include "driver.hh"
#include "daq.hh"
#include "event.hh"
#include "rootdriver.hh"
#include <TCanvas.h>

using namespace std;
//
// MAIN program
//
int main(int argc, char **argv)
{
    // switches
    int c = 0;
    string       DriverFilename;
    bool         graphics = false, longRoot = false, slowOn = false, fastOn = false;
    // parse switches
    while((c = getopt(argc,argv,"glsfi:")) != -1)
    {
        switch(c)   {
            case 'i': // name of binary file from Labview
                DriverFilename = optarg;
                break;
            case 'g':
                graphics = true;
                break;
            case 'l':
                longRoot = true;
                break;
	    case 's':
		slowOn = true;
		break;
	    case 'f':
		fastOn = true;
		break;
            default:
                exit(-1);
        }
    }
      // driver for the daq processing..... provided from the python script
      driver* myDriver = new driver(DriverFilename);
      // TApplication is needed to plot a canvas with an event
      TApplication *theApp;
      if(graphics) theApp =  new TApplication("tapp", &argc, argv);
      TCanvas *canv = new TCanvas("c1","c1",0,0,450,450);
      // create an instance of teh daq datatype: controls all the binary file handling
      daq myDaq(myDriver);
      // root management
      rootdriver myRoot(myDriver, longRoot, slowOn, fastOn);
      ULong64_t new_stime;
      
      // loop over the events
      int totalnumberofevents = myDriver->getNEvent();
      if (slowOn) {
	  cout << "Slow on specified" << endl;
	  slowevent *sev;
	  int totalslowevents = myDaq.GetSlowFileSize();
	  cout << " I think there are " << totalslowevents << " slow events" << endl;
	  //totalslowevents /= myDriver->getNSlowParams();
	  ULong64_t old_stime = 0;
	
	  for (int iSlowEv = 0; iSlowEv < totalslowevents; iSlowEv++){
	      sev = myDaq.readSlowEvent();
	      new_stime = myRoot.SlowFill(sev, old_stime);
	      //cout << "i = " << iSlowEv << endl;
	      old_stime = new_stime;
	      delete sev;
	  }
	cout << "I am done processing slow events!!!! " << endl;
      }

      if (fastOn) {

	  cout << "Fast on specified" << endl;
	  cout << totalnumberofevents << endl;
	  event *ev;
	  for(int iEvent=0; iEvent<totalnumberofevents; iEvent++){
	      if(iEvent%10000==0) cout << "processed "<<iEvent<<" events"<<endl;
	      // read the next event
	      ev   = myDaq.readEvent(myDriver);
	      // write the event to the root tree
	      myRoot.FastFill(ev, myDriver);
	      // if you want plot the event
	      if(graphics) ev->Plot(canv);
	      // free event 
	      delete ev;
	    }
      }
      //delete theApp;
      // write run parameters to file
      myRoot.writeParameters(myDriver);
      myRoot.Close();
      delete myDriver;
      return 0;
}
