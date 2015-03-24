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
    
    if (fastOn) {
        // driver for the daq processing..... provided from the python script
        cout << "DriverFilename: " << DriverFilename << endl;
        driver* myDriver = new driver(DriverFilename, fastOn, slowOn);
        // TApplication is needed to plot a canvas with an event
        TApplication *theApp;
        if(graphics) theApp =  new TApplication("tapp", &argc, argv);
        TCanvas *canv = new TCanvas("c1","c1",0,0,450,450);
        // create an instance of teh daq datatype: controls all the binary file handling
        daq myDaq(myDriver);
        // root management
        rootdriver myRoot(myDriver, longRoot, slowOn, fastOn);
        
        //ULong64_t new_stime;
        
        // loop over the events
        int totalnumberofevents = myDriver->getNEvent();
        
        
        
        cout << "Fast on specified" << endl;
        cout << totalnumberofevents << endl;
        event *ev = NULL;
        for(int iEvent=0; iEvent<totalnumberofevents; iEvent++){
            //for(int iEvent=0; iEvent<10; iEvent++){
            
            if(iEvent%10000==0) cout << "processed "<<iEvent<<" events"<<endl;
            // read the next event
            ev   = myDaq.readEvent(myDriver);
            // write the event to the root tree
            myRoot.FastFill(ev, myDriver);
            
            // if you want plot the event
            Double_t pk  = ev->getPeak();
            Int_t    ich = ev->getChannel();
            Double_t rms = ev->calculateBaselineRMS();
            // SANDER HIERO!
            if(graphics && ich==4) ev->Plot(canv);
            // free event
            myDaq.endEvent();
        }
        
        cout <<"Finished processing .... terminating daqana" <<endl;
        if(graphics) delete theApp;
        // write run parameters to file
        myRoot.writeParameters(myDriver);
        myRoot.Close();
        delete myDriver;
        cout <<"Finished processing .... done" <<endl;
    }
    return 0;
}
