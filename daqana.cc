#include "TApplication.h"

#include <string>
#include <sstream>
#include <cstdio>
#include <cstdlib>

#include "driver.hh"
#include "daq.hh"
#include "event.hh"
#include "rootdriver.hh"

using namespace std;
//
// MAIN program
//
int main(int argc, char **argv)
{
    // switches
    int c = 0;
    string       DriverFilename;
    bool         graphics = false, longRoot = false;
    // parse switches
    while((c = getopt(argc,argv,"gli:")) != -1)
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
            default:
                exit(-1);
        }
    }
    
    // driver for the daq processing..... provided from the python script
    driver* myDriver = new driver(DriverFilename);
    // TApplication is needed to plot a canvas with an event
    TApplication *theApp;
    if(graphics) theApp =  new TApplication("tapp", &argc, argv);
    // create an instance of teh daq datatype: controls all the binary file handling
    daq myDaq(myDriver);
    // root management
    rootdriver myRoot(myDriver, longRoot);
    // loop over the events
    for(int iEvent=0; iEvent<myDriver->getNEvent(); iEvent++){
        //if(iEvent%10000==0) cout << "processed "<<iEvent<<" events"<<endl;
        // read the next event
        event *ev   = myDaq.readEvent(myDriver);
        // write the event to the root tree
        myRoot.Fill(ev);
        // if you want plot the event
        if(graphics) ev->Plot();
	// free event 
	delete ev;
    }
    // write run parameters to file
    myRoot.writeParameters(myDriver);
    myRoot.Close();
    delete myDriver;
    return 0;
}
