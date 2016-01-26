#!/usr/bin/python
# -*- coding: utf-8 -*-

from xml.dom.minidom import parse, parseString
import os, glob, math, getopt, sys, argparse

###############################################################################

INT_SIZE    = 2 # an integere is 2-bytes
HEADER_SIZE = 4 # NOTE: this is the header of an array. not the header of an event
MAX_NB_PROCESSES = 4 # 4 processors running on each of the 4 cores
NUM_CHANNELS = 8 # number of fast data channels (i.e. how many NaI detectors there are)

###############################################################################

#
# FUNCTIONS (main below....)
#


class processor:

    def __init__(self,indir,outdir):
        self.fbase=indir
        self.outdir=outdir
        self.fnames,self.snames = self.getFilenames()
        
    # function to retreive a single parameter from the xml file
    def getSingleElement(self,dom,eName):
        val = "-1"
        theNodes=dom.getElementsByTagName(eName)
        for node in theNodes:
            if(node.firstChild):
                val = node.firstChild.data
                if(val == "--" or val == "xx"):
                    val= "-1"

        return val

    # retrieve the names of all binary files in a directory
    def getFilenames(self):
        fnames = glob.glob(self.fbase+'/*.bin')
        snames = glob.glob(self.fbase+'/*.slo')
        #fnames = [os.path.splitext(each)[0] for each in fnames
        
        return fnames, snames
    
    def getSlowFilename(self,fname):
        arr2 = fname.split('.')
        arr = fname.split('_')
        # start with first element
        fb =arr[0]
        for i in range(1,len(arr)-1):
            fb = fb + '_' + arr[i]
            fb = os.path.splitext(fname)[0]
        print fb
        # compose the slow name
        slowname = fb+'.slo'
        #print '    Slow file  = ' + slowname
        return slowname
    
    # retrieve the name of the xml file
    def getXMLFilename(self):
        XMLname=glob.glob(self.fbase+'*.XML')
        if len(XMLname)>1:
            sys.exit( "MORE THAN 1 XML FILE IN DIRECTORY, QUITTING PROGRAM")
        return XMLname[0] #should only be 1 XML file, choose first one

    # make the name of the temporary slow tree
    def getOutSlowFilename(self,datafile):
        fb=os.path.splitext(os.path.basename(datafile))[0][:-7] #remove file type and numbers
        tempname = self.outdir + '/' + fb + '.sroot'
        return tempname

    # make the name of the daq file
    def getDAQFilename(self,datafile):
        fb=os.path.splitext(os.path.basename(datafile))[0]
        DAQname = self.outdir+'/'+fb+'.tmp'
        return DAQname

    # make the output root filename
    def rootFilename(self,datafile):
        fb=os.path.splitext(os.path.basename(datafile))[0]
        froot = self.outdir + '/' + fb + '.root'
        return froot


    # calculate the relevant numbers from the file
    def calculateNumberOfEvents(self,filename,array_size,event_size):
        # get the file size
        fsize = os.path.getsize(filename)
        nArray = int(math.ceil(float(fsize) / (array_size + HEADER_SIZE)))
        # ... and next calculate the number of events
        nEvent = (fsize - nArray*HEADER_SIZE) / event_size
        # ... and the number of events / array
        nEventPerArray = (array_size) / event_size
        return nEvent

    # generate the driver file for daqana based on XML file and data filesize
    def generateDriverFile(self,filename,calibration):
        
        #
        # NOTE: if you write an extra line to the daqfile driver for daqana ->
        #       change the driver.hh class of daqana accordingly.
        #
        
        # parse the DAQ xml file
        
        #print 'XML::parsing ...'
        
        xmlfile = self.getXMLFilename()
        dom = parse(xmlfile)
        #  define the root filename: to the right location + right name
        rootfile = self.rootFilename(filename)
        # open driver file for daqana
        daqfile = self.getDAQFilename(filename)
        slowfile = self.getSlowFilename(filename) # data file
        tempslowfile = self.getOutSlowFilename(filename) # root tree
        
        fdaq = open(daqfile,'w')
        
        # get parameters from parser
        #print 'XML::read main run parameters ...'
        
        chunk_size   = self.getSingleElement(dom,'waveforms_per_data_chunk')
        delta_t      = self.getSingleElement(dom,'time_per_sample')
        n_sample     = self.getSingleElement(dom,'samples_per_waveform')
        n_pretrigger = self.getSingleElement(dom,'pretrigger_samples')
        n_header     = int(self.getSingleElement(dom,'samples_per_event')) - int(n_sample)
        # array_size   = int(array_length)*INT_SIZE
        event_size   = (int(n_sample)+int(n_header))*INT_SIZE
        array_size   = int(chunk_size)*int(event_size)
        
        location     = self.getSingleElement(dom,'location')
        
        initial_time 	= self.getSingleElement(dom, 'initial_timestamp')
        
        #  get the nEvent and nEventPerArray from the file information
        #print 'XML::calculate number of events ...'
        nEvent= self.calculateNumberOfEvents(filename, array_size, event_size)
        fdaq.write(filename + '\n')
        fdaq.write(slowfile + '\n')
        fdaq.write(tempslowfile + '\n')
        fdaq.write(rootfile + '\n')
        fdaq.write(location +'\n')
        fdaq.write(calibration + '\n')
        fdaq.write(initial_time + '\n')
        fdaq.write(delta_t + '\n')
        fdaq.write(n_sample + '\n')
        fdaq.write(n_pretrigger + '\n')
        fdaq.write(str(n_header) + '\n')
        fdaq.write(str(array_size) + '\n')
        fdaq.write(str(event_size) + '\n')
        fdaq.write(str(nEvent) + '\n')
        
        #print 'XML::read channel attributes ...'
        channel_info = dom.getElementsByTagName('channel')
        for i in range(0,NUM_CHANNELS):
            active_channel = parseString(channel_info[i].toxml())
            index = self.getSingleElement(active_channel, 'index')
            #print 'XML:: reading channel: ', index, ' ...'
            ##fdaq.write(index + '\n') # write channel index (0-7)
            #print 'XML:: channel ', index, ': ', getSingleElement(active_channel, 'active'), ' ...'
            fdaq.write(self.getSingleElement(active_channel, 'active') + '\n') # write channel status (on/off)
            #print 'XML:: channel ', index, ' serial number: ', getSingleElement(active_channel, 'serial_numbers'), ' ...'
            fdaq.write(self.getSingleElement(active_channel, 'serial_numbers') + '\n') # write serial number
            #print 'XML:: channel ', index, ' detector type: ', getSingleElement(active_channel, 'det_type'), ' ...'
            fdaq.write(self.getSingleElement(active_channel, 'det_type') + '\n') # write detector type
            #print 'XML:: channel ', index, ' source: ', getSingleElement(active_channel, 'sources'), ' ...'
            fdaq.write(self.getSingleElement(active_channel, 'sources') + '\n') # write source
            #print 'XML:: channel ', index, ' trigger level: ', getSingleElement(active_channel, 'trigger_level'), ' ...'
            fdaq.write(self.getSingleElement(active_channel, 'trigger_level') + '\n') # write trigger level
            #print 'XML:: channel ', index, ' PMT voltage: ', getSingleElement(active_channel, 'voltage'), ' ...'
            fdaq.write(self.getSingleElement(active_channel, 'set_voltage') + '\n') # write voltage
        
        
        #print 'XML:: read slow params ...'
        nSlowParams   = self.getSingleElement(dom,'num_params')
        nSlow = nSlowParams
        fdaq.write(nSlow + '\n')
        nSlow = int(float(nSlow));
        #print 'XML:: read ', nSlowParams, ' total slow parameters'
        slowparam = dom.getElementsByTagName('slow')
        for i in range(0,nSlow):
            slow_chan = parseString(slowparam[i].toxml())
            branchname = self.getSingleElement(slow_chan, 'slowbranch')
            fdaq.write(branchname + '\n')
        #print 'XML:: including branch ', branchname, ' ...'
        
        
        fdaq.close
        
        return daqfile
