#include "TApplication.h"

#include <string>
#include <sstream>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
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
    bool         graphics = false, longRoot = false, slowOn = false;
    // parse switches
    while((c = getopt(argc,argv,"glsi:")) != -1)
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
            default:
                exit(-1);
        }
    }
    
    // driver for the daq processing..... provided from the python script
    cout << "daqana:: DriverFilename: " << DriverFilename << endl;
    driver* myDriver = new driver(DriverFilename, slowOn);
    // TApplication is needed to plot a canvas with an event
    TApplication *theApp;
    if(graphics) theApp =  new TApplication("tapp", &argc, argv);
    TCanvas *canv = new TCanvas("c1","c1",500,300);
    // create an instance of teh daq datatype: controls all the binary file handling
    daq myDaq(myDriver);
    // root management
    rootdriver myRoot(myDriver, longRoot, slowOn);
    
    // loop over the events
    int totalnumberofevents = myDriver->getNEvent();
    
    cout << "daqana:: number of events = " <<totalnumberofevents << endl;
    event *ev = NULL;
    for(int iEvent=0; iEvent<totalnumberofevents; iEvent++){
        //        for(int iEvent=0; iEvent<1000000; iEvent++){
        
        if(iEvent%100000==0) cout << "     processed "<<iEvent<<" events"<<endl;
        // read the next event
        ev   = myDaq.readEvent(myDriver);
        // write the event to the root tree
        myRoot.FastFill(ev, myDriver);
        
        // if you want plot the event
        Double_t pk  = ev->getPeak();
        Int_t    ich = ev->getChannel();
        Double_t rms = ev->calculateBaselineRMS();
        if(graphics && ( (ev->getErrorCode()&0x01) != 0) && (ich%100 == 3)) ev->Plot(canv);
        // free event
        myDaq.endEvent();
        
        ev = NULL;
    }
    
    cout <<"daqana:: Finished processing .... terminating daqana" <<endl;
    if(graphics) delete theApp;
    // write run parameters to file
    myRoot.writeParameters(myDriver);
    myRoot.Close();
    delete myDriver;
    cout <<"daqana:: Finished processing .... done" <<endl;
    return 0;
}
