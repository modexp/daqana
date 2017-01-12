#!/usr/bin/python
# -*- coding: utf-8 -*-

from xml.dom.minidom import parse, parseString
import os, glob, math, getopt, sys
from datetime import datetime, timedelta

###############################################################################

INT_SIZE    = int(2) # an integere is 2-bytes
HEADER_SIZE = int(4) # NOTE: this is the header of an array. not the header of an event
MAX_NB_PROCESSES = int(4) # 4 processors running on each of the 4 cores
NUM_CHANNELS = int(8) # number of fast data channels (i.e. how many NaI detectors there are)

###############################################################################

# parseArguments no longer used - should be removed completely in future
'''
# interpret the command line arguments
def parseArguments(argv):
    inDir     = ''
    outDir    = ''
    grafOn    = 0
    longRoot  = 0
    slowOn    = 0
    fastOn    = 0
    processLevel = 0
    calFile   = 'NULL.root'
    try:
        opts, args = getopt.getopt(argv,"hlgsfi:o:c:p:",["long","graf","slow","fast","idir=","odir=","cal","proc="])
    except getopt.GetoptError:
        print('daqprocessor.py -i <inputfile> -o <outputfile> -g -l -s -f -p <level>')
        sys.exit(2)

    for opt, arg in opts:
        if opt == '-h':
            print('daqprocessor.py -i <input dir> -o <output dir> -g -l -s -f -p <level>')
            sys.exit()
        elif opt in ("-i", "--idir"):
            inDir = arg
        elif opt in ("-o", "--odir"):
            outDir = arg
        elif opt in ("-g","--graf"):
            grafOn = 1
        elif opt in ("-p", "--proc"):
            processLevel = int(arg)
        elif opt in ("-l","--long"):
            longRoot = 1
        elif opt in ("-s", "--slow"):
            slowOn = 1
        elif opt in ("-f", "--fast"):
            fastOn = 1
        elif opt in ("-c", "--cal"):
            calFile = arg

    return inDir, outDir, grafOn, longRoot, slowOn, fastOn, calFile
'''

# function to retreive a single parameter from the xml file
def getSingleElement(dom,eName):
    val = "-1"
    theNodes=dom.getElementsByTagName(eName)
    for node in theNodes:
        if(node.firstChild):
            val = node.firstChild.data
            if(val == "--" or val == "xx"):
                val= "-1"

    return val

# Tests if a timestamp is with the useful range
def isRecent(timestamp, timeLimit):
    diff = datetime.now() - datetime.fromtimestamp(timestamp)
    return diff < timedelta(hours = timeLimit)

# See if directory exists and make it if not
def ensureDir(d):
    if not os.path.exists(d):
        os.makedirs(d)

# Find recent files with particular extension
def getFiles(parentDir, fileExtension, timeLimit=None):
    retFileList = []
    for (dir, _, fileList) in os.walk(parentDir):
        for file in fileList:
            _, extension = os.path.splitext(file)
            fullPath = dir + '/' + file
            if extension == '.' + fileExtension:
                if timeLimit == None or isRecent(os.path.getmtime(fullPath), timeLimit):
                    retFileList.append(fullPath)
    return retFileList


# retrieve the names of all binary files in a directory
def getFilenames(fbase):
    fnames = glob.glob(fbase+'/*.bin')
    #print 'fnames == []'
    #print fnames == []
    snames = glob.glob(fbase+'/*.slo')
    #fnames = [os.path.splitext(each)[0] for each in fnames
    
    return (fnames, snames)

# retrieve the name of the xml file
def getXMLFilename(fname):
    # only remove the last bit with the 0000n.bin from the filename
    arr2 = fname.split('.')
        
    arr = fname.split('_')
    # start with first element
    fb =arr[0]
    for i in range(1,len(arr)-1):
        fb = fb + '_' + arr[i]
                        
        # compose the xml name
        XMLname = fb+'.XML'
        #print '    XML file  = ' + XMLname
    return XMLname

# retrieve the name of the slow file
def getSlowFilename(fname):
    arr2 = fname.split('.')
    arr = fname.split('_')
    # start with first element
    fb =arr[0]
    for i in range(1,len(arr)-1):
        fb = fb + '_' + arr[i]
        fb = os.path.splitext(fname)[0]
        
        # compose the slow name
        slowname = fb+'.slo'
            #print '    Slow file  = ' + slowname
    return slowname

# make the name of the temporary slow tree
def getOutSlowFilename(outdir,datafile):
    fb=os.path.splitext(os.path.basename(datafile))[0][:-7] #remove file type and numbers
    tempname = outdir + '/' + fb + '.sroot'
    return tempname

# make the name of the daq file
def getDAQFilename(outdir,datafile):
    fb=os.path.splitext(os.path.basename(datafile))[0]
    DAQname = outdir+'/'+fb+'.tmp'
    return DAQname

# make the output root filename
def rootFilename(outdir,datafile):
    fb=os.path.splitext(os.path.basename(datafile))[0]
    froot = outdir + '/' + fb + '.root'
    return froot


# calculate the relevant numbers from the file
def calculateNumberOfEvents(filename,array_size,event_size):
    
    # get the file size
    fsize = os.path.getsize(filename)
    # calculate the number of arrays
    # use:
    # (i)  file_size = nEvent*event_size + nArray*HEADER_SIZE
    # (ii) nArray    = (file_size - nArray*HEADER_SIZE) / array_size :)
    #
    # to give you.....
    
    # the ceil is needed to ensure counting 'incompletely' filled arrays
    nArray = int(math.ceil(float(fsize) / (array_size + HEADER_SIZE)))
    # ... and next calculate the number of events
    nEvent = (fsize - nArray*HEADER_SIZE) / event_size
    # ... and the number of events / array
    nEventPerArray = (array_size) / event_size
    #print 'calculateNumberOfEvents:: nEventPerArray = ', str(nEventPerArray)
    #print 'calculateNumberOfEvents:: nArray         = ', str(nArray)
    #print 'calculateNumberOfEvents:: nEvent         = ', str(nEvent)
    return nEvent

# generate the driver file for daqana based on XML file and data filesize
def generateDriverFile(outdir,filename,calibration):
    
    #
    # NOTE: if you write an extra line to the daqfile driver for daqana ->
    #       change the driver.hh class of daqana accordingly.
    #
    
    # parse the DAQ xml file
    
    #print 'XML::parsing ...'
    xmlfile = getXMLFilename(filename)
    dom     = parse(xmlfile)
    #  define the root filename: to the right location + right name
    rootfile = rootFilename(outdir,filename)
    # open driver file for daqana
    daqfile = getDAQFilename(outdir,filename)
    slowfile = getSlowFilename(filename) # data file
    tempslowfile = getOutSlowFilename(outdir, filename) # root tree
    
    fdaq = open(daqfile,'w')
    
    # get parameters from parser
    #print 'XML::read main run parameters ...'
    
    chunk_size   = getSingleElement(dom,'waveforms_per_data_chunk')
    delta_t      = getSingleElement(dom,'time_per_sample')
    n_sample     = getSingleElement(dom,'samples_per_waveform')
    n_pretrigger = getSingleElement(dom,'pretrigger_samples')
    n_header     = int(getSingleElement(dom,'samples_per_event')) - int(n_sample)
    # array_size   = int(array_length)*INT_SIZE
    event_size   = (int(n_sample)+int(n_header))*INT_SIZE
    array_size   = int(chunk_size)*int(event_size)
    
    location     = getSingleElement(dom,'location')
    
    initial_time 	= getSingleElement(dom, 'initial_timestamp')
    
    #  get the nEvent and nEventPerArray from the file information
    #print 'XML::calculate number of events ...'
    nEvent= calculateNumberOfEvents(filename, array_size, event_size)
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
        index = getSingleElement(active_channel, 'index')
        #print 'XML:: reading channel: ', index, ' ...'
        ##fdaq.write(index + '\n') # write channel index (0-7)
        #print 'XML:: channel ', index, ': ', getSingleElement(active_channel, 'active'), ' ...'
        fdaq.write(getSingleElement(active_channel, 'active') + '\n') # write channel status (on/off)
        #print 'XML:: channel ', index, ' serial number: ', getSingleElement(active_channel, 'serial_numbers'), ' ...'
        fdaq.write(getSingleElement(active_channel, 'serial_numbers') + '\n') # write serial number
        #print 'XML:: channel ', index, ' detector type: ', getSingleElement(active_channel, 'det_type'), ' ...'
        fdaq.write(getSingleElement(active_channel, 'det_type') + '\n') # write detector type
        #print 'XML:: channel ', index, ' source: ', getSingleElement(active_channel, 'sources'), ' ...'
        fdaq.write(getSingleElement(active_channel, 'sources') + '\n') # write source
        #print 'XML:: channel ', index, ' trigger level: ', getSingleElement(active_channel, 'trigger_level'), ' ...'
        fdaq.write(getSingleElement(active_channel, 'trigger_level') + '\n') # write trigger level
        #print 'XML:: channel ', index, ' PMT voltage: ', getSingleElement(active_channel, 'voltage'), ' ...'
        fdaq.write(getSingleElement(active_channel, 'set_voltage') + '\n') # write voltage
    
    
    #print 'XML:: read slow params ...'
    nSlowParams   = getSingleElement(dom,'num_params')
    nSlow = nSlowParams
    fdaq.write(nSlow + '\n')
    nSlow = int(float(nSlow));
    #print 'XML:: read ', nSlowParams, ' total slow parameters'
    slowparam = dom.getElementsByTagName('slow')
    for i in range(0,nSlow):
        slow_chan = parseString(slowparam[i].toxml())
        branchname = getSingleElement(slow_chan, 'slowbranch')
        fdaq.write(branchname + '\n')
    #print 'XML:: including branch ', branchname, ' ...'
    
    
    fdaq.close
    
    return daqfile
