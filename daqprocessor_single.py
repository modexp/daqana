#!/usr/bin/python3.4
# -*- coding: utf-8 -*-
from xml.dom.minidom import parse, parseString
import os, glob, math, getopt, sys

###############################################################################

INT_SIZE    = int(2) # an integere is 2-bytes
HEADER_SIZE = int(4) # NOTE: this is the header of an array. not the header of an event
MAX_NB_PROCESSES = int(1) # int(4) # test for now, have 4 processors running on each of the 4 cores
NUM_CHANNELS = int(8) # number of fast data channels (i.e. how many NaI detectors there are)

###############################################################################

#
# FUNCTIONS (main below....)
#


# interpret the command line arguments
def parseArguments(argv):
    inDir     = ''
    outDir    = ''
    grafOn    = 0
    longRoot  = 0
    slowOn    = 0
    fastOn    = 0
    calFile   = 'NULL.root'
    try:
        opts, args = getopt.getopt(argv,"hlgsfi:o:c:",["long","graf","slow","fast","idir=","odir=","cal="])
    except getopt.GetoptError:
        print('daqprocessor.py -i <inputfile> -o <outputfile> -g -s -f')
        sys.exit(2)
    for opt, arg in opts:
        if opt == '-h':
            print('daqprocessor.py -i <input dir> -o <output dir> -g')
            sys.exit()
        elif opt in ("-i", "--idir"):
            inDir = arg
        elif opt in ("-o", "--odir"):
            outDir = arg
        elif opt in ("-g","--graf"):
            grafOn = 1
        elif opt in ("-l","--long"):
            longRoot = 1
        elif opt in ("-s", "--slow"):
            slowOn = 1
        elif opt in ("-f", "--fast"):
            fastOn = 1
        elif opt in ("-c", "--cal"):
            calFile = arg

    print('CALCAL = ',calFile)
    return inDir, outDir, grafOn, longRoot, slowOn, fastOn, calFile

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

# retrieve the names of all binary files in a directory
def getFilenames(fbase):
    fnames = glob.glob(fbase+'/*.bin')
    print('fnames == []')
    print(fnames == [])
    if(fnames == []):
      fnames = glob.glob(fbase+'/*.slo')
      #fnames = [os.path.splitext(each)[0] for each in fnames
      
    return fnames

# retrieve the name of the xml file
def getXMLFilename(fname):

    # only remove the last bit with the 0000n.bin from the filename
    arr2 = fname.split('.')
  
    if (arr2[1] == 'bin'):
        arr = fname.split('_')
        # start with first element
        fb =arr[0]
        for i in range(1,len(arr)-1):
            fb = fb + '_' + arr[i]
    else: 
        fb = os.path.splitext(fname)[0]

    # compose the xml name
    XMLname = fb+'.XML'
    print('    XML file  = ' + XMLname)
    return XMLname
    
# retrieve the name of the slow file
def getSlowFilename(fname):
    arr2 = fname.split('.')
    if (arr2[1] == 'bin'):
        arr = fname.split('_')
        # start with first element
        fb =arr[0]
        for i in range(1,len(arr)-1):
            fb = fb + '_' + arr[i]
    else: 
        fb = os.path.splitext(fname)[0]

    # compose the slow name
    slowname = fb+'.slo'
    print('    Slow file  = ' + slowname)
    return slowname
    
# make the name of the temporary slow tree
def getOutSlowFilename(outdir,datafile):
    arr2 = datafile.split('.')
    datafile = arr2[0]
    
    arr = datafile.split('/')
    # get the last element
    fb = arr[len(arr)-1]
    
    arr = fb.split('_')
    fb = arr[0]
    for i in range(1,len(arr)-1):
        fb = fb + '_' + arr[i]
    
    # compose the root filename
    tempname = outdir + '/' + fb + '.sroot'
    #print '    Slow ROOT file = ' + tempname
    return tempname

# make the name of the daq file
def getDAQFilename(outdir,datafile):
  
    fb = os.path.splitext(datafile)[0]

    arr = datafile.split('/')
    # get the last element
    fb = arr[len(arr)-1]

    # compose the xml name
    DAQname = outdir+'/'+fb+'.tmp'
    print('    DAQ file  = ' + DAQname)
    return DAQname

# make the output root filename
def rootFilename(outdir,datafile):

    fb = os.path.splitext(datafile)[0]

    arr = datafile.split('/')
    # get the last element
    fb = arr[len(arr)-1]
      
    # compose the root filename
    froot = outdir + '/' + fb + '.root'
    print('    ROOT file = ' + froot)
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
    print('calculateNumberOfEvents:: nEventPerArray = ', str(nEventPerArray))
    print('calculateNumberOfEvents:: nArray         = ', str(nArray))
    print('calculateNumberOfEvents:: nEvent         = ', str(nEvent))
    return nEvent

# generate the driver file for daqana based on XML file and data filesize
def generateDriverFile(outdir,filename, calibration):
    
    #
    # NOTE: if you write an extra line to the daqfile driver for daqana ->
    #       change the driver.hh class of daqana accordingly. 
    #
    
    # parse the DAQ xml file

    print('XML::parsing ...')
    xmlfile = getXMLFilename(filename)
    dom     = parse(xmlfile)
    #  define the root filename: to the right location + right name
    if (fastOn == 1): rootfile = rootFilename(outdir,filename)
    # open driver file for daqana
    daqfile = getDAQFilename(outdir,filename)
#    if (slowOn == 1):
    slowfile = getSlowFilename(filename)
#   if (slowOn == 1):
    tempslowfile = getOutSlowFilename(outdir, filename)
    
    fdaq = open(daqfile,'w')
        
        #if (fastOn == 1):
        
        # get parameters from parser
    print('XML::read main run parameters ...')
        
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
    print('XML::calculate number of events ...')
    nEvent= calculateNumberOfEvents(filename, array_size, event_size)
    fdaq.write(filename + '\n')
    #if (slowOn == 1):
    fdaq.write(slowfile + '\n')
    fdaq.write(tempslowfile + '\n')

#if (fastOn == 1):
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
    
    print('XML::read channel attributes ...')
    channel_info = dom.getElementsByTagName('channel')
    for i in range(0,NUM_CHANNELS):
        active_channel = parseString(channel_info[i].toxml())
        index = getSingleElement(active_channel, 'index')
        print('XML:: reading channel: ', index, ' ...')
        ##fdaq.write(index + '\n') # write channel index (0-7)
        print('XML:: channel ', index, ': ', getSingleElement(active_channel, 'active'), ' ...')
        fdaq.write(getSingleElement(active_channel, 'active') + '\n') # write channel status (on/off)
        print('XML:: channel ', index, ' serial number: ', getSingleElement(active_channel, 'serial_numbers'), ' ..')
        fdaq.write(getSingleElement(active_channel, 'serial_numbers') + '\n') # write serial number
        print('XML:: channel ', index, ' detector type: ', getSingleElement(active_channel, 'det_type'), ' ...')
        fdaq.write(getSingleElement(active_channel, 'det_type') + '\n') # write detector type
        print('XML:: channel ', index, ' source: ', getSingleElement(active_channel, 'sources'), ' ...')
        fdaq.write(getSingleElement(active_channel, 'sources') + '\n') # write source
        print('XML:: channel ', index, ' trigger level: ', getSingleElement(active_channel, 'trigger_level'), ' ...')
        fdaq.write(getSingleElement(active_channel, 'trigger_level') + '\n') # write trigger level
        print('XML:: channel ', index, ' PMT voltage: ', getSingleElement(active_channel, 'voltage'), ' ...')
        fdaq.write(getSingleElement(active_channel, 'voltage') + '\n') # write voltage
          
          #if (slowOn == 1):
          #print 'XML:: read slow params ...'
    nSlowParams   = getSingleElement(dom,'num_params')
    nSlow = nSlowParams
    fdaq.write(nSlow + '\n')
    nSlow = int(float(nSlow));
    print ('XML:: read ', nSlowParams, ' total slow parameters')
    slowparam = dom.getElementsByTagName('slow')
    for i in range(0,nSlow):
        slow_chan = parseString(slowparam[i].toxml())
        branchname = getSingleElement(slow_chan, 'slowbranch')
        fdaq.write(branchname + '\n')
        print('XML:: including branch ', branchname, ' ...')


    fdaq.close

    return daqfile

###############################################################################

#
# MAIN python code
#

print('MAIN:: Welcome to the modulation daq-processor...')
# parse the IO arguments below
filebase, outdir, grafOn, longRoot, slowOn, fastOn, calibration = parseArguments(sys.argv[1:])
print('fastOn = ', fastOn)
#  get the files from the data directory
filenames = getFilenames(filebase)
nb_files = len(filenames)
print('number of files = ', nb_files)
cmds_to_ex = []
child_pids = []

if slowOn:
    print('MAIN:: Beginning to parse slow data')
    print(filenames)
    daqfile = generateDriverFile(outdir,filenames[0], calibration)
    slow_cmd_string = './slowdaq -i ' + daqfile
    os.system(slow_cmd_string)
    print('MAIN:: Done parsing slow data, moving on to fast data')

if fastOn:
    print(nb_files)
    # split files into nb_processes lists
    split_file_ids = dict([[process_nb, []] for process_nb in range(0, MAX_NB_PROCESSES)])
    print(range(0,nb_files))
    for file_id in range(0, nb_files):

        print(file_id)
        # generate driver file
        filename = filenames[file_id]
        daqfile = generateDriverFile(outdir,filename,calibration)
  
        cmd_string = './daqana -i ' + daqfile
        if(grafOn):
            cmd_string = cmd_string + ' -g'
        if(longRoot):
            cmd_string = cmd_string + ' -l'
        if(slowOn):
            cmd_string = cmd_string + ' -s'
        if(fastOn):
            cmd_string = cmd_string + ' -f'
        if((not fastOn) and (not slowOn)):
            print('MAIN:: User did not specify which data to parse, only filling fast data')
            #cmd_string = cmd_string + ' -s'

        cmds_to_ex.append(cmd_string)
        print('MAIN:: Processing ' + filename)
        print('MAIN:: CMD ' + cmd_string)
        os.system(cmd_string)
        print('MAIN:: Processing complete for ' + filename)
        print('MAIN:: Remove ' + daqfile)
        cmd_string = 'rm -f ' + daqfile
        os.system(cmd_string)

print('MAIN:: Exit from the daq-processor. bye-bye.')

