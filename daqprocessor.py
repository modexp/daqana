#!/usr/bin/python
from xml.dom.minidom import parse, parseString
import os, glob, math, getopt, sys

import argparse #neat argument parser
parser=argparse.ArgumentParser()


###############################################################################

INT_SIZE    = int(2) # an integere is 2-bytes
HEADER_SIZE = int(4) # NOTE: this is the header of an array. not the header of an event
MAX_NB_PROCESSES = int(4) # test for now, have 4 processors running on each of the 4 cores
###############################################################################

#
# FUNCTIONS (main below....)
#

# function to retreive a single parameter from the xml file
def getSingleElement(dom,eName):
    val = "-1"
    theNodes=dom.getElementsByTagName(eName)
    for node in theNodes:
        if(node.firstChild):
            inval=node.firstChild.data
            if inval !="--" or inval!="xx":
                val = node.firstChild.data
    return val

# retrieve the names of all binary files in a directory
def getFilenames(fbase):
    fnames = glob.glob(fbase+'/*.bin')
    return fnames

# retrieve the name of the xml file
def getXMLFilename(fname):
    fb=fname[:-11]+'.XML'
    print '    XML file  = ' + fb
    return fb

# make the name of the daq file
def getDAQFilename(outdir,fname):
    arr = fname.split('/')
    # get the last element
    fb = arr[-1]
    # compose the xml name
    DAQname = outdir+'/'+fb+'.tmp'
    print '    DAQ file  = ' + DAQname
    return DAQname

# make the output root filename
def rootFilename(outdir,datafile):
    fb = datafile.split('/')[-1]
    froot = outdir + '/' + fb + '.root'
    print '    ROOT file = ' + froot
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
    print 'calculateNumberOfEvents:: nEventPerArray = ', str(nEventPerArray)
    print 'calculateNumberOfEvents:: nArray         = ', str(nArray)
    print 'calculateNumberOfEvents:: nEvent         = ', str(nEvent)
    return nEvent

# generate the driver file for daqana based on XML file and data filesize
def generateDriverFile(outdir,filename):
    
    #
    # NOTE: if you write an extra line to the daqfile driver for daqana ->
    #       change the driver.hh class of daqana accordingly. 
    #
    
    # parse the DAQ xml file
    print 'XML::parsing ...'
    xmlfile = getXMLFilename(filename)
    dom     = parse(xmlfile)
    #  define the root filename: to the right location + right name
    rootfile = rootFilename(outdir,filename)
    # open driver file for daqana
    daqfile = getDAQFilename(outdir,filename)
    
    # get parameters from parser
    print 'XML::read main run parameters ...'

    chunk_size   = getSingleElement(dom,'waveforms_per_data_chunk')
    delta_t      = getSingleElement(dom,'time_per_sample')
    n_sample     = getSingleElement(dom,'samples_per_waveform')
    n_pretrigger = getSingleElement(dom,'pretrigger_samples')
    n_header     = int(getSingleElement(dom,'samples_per_event')) - int(n_sample)
    # array_size   = int(array_length)*INT_SIZE
    event_size   = (int(n_sample)+int(n_header))*INT_SIZE
    array_size   = int(chunk_size)*int(event_size)

    location     = getSingleElement(dom,'location')

    #  get the nEvent and nEventPerArray from the file information
    print 'XML::calculate number of events ...'
    nEvent= calculateNumberOfEvents(filename, array_size, event_size)
    fdaq = open(daqfile,'w')
    fdaq.write(filename + '\n')
    fdaq.write(rootfile + '\n')
    fdaq.write(location +'\n')
    fdaq.write(delta_t + '\n')
    fdaq.write(n_sample + '\n')
    fdaq.write(n_pretrigger + '\n')
    fdaq.write(str(n_header) + '\n')
    fdaq.write(str(array_size) + '\n')
    fdaq.write(str(event_size) + '\n')
    fdaq.write(str(nEvent) + '\n')
    
    print 'XML::read active_channels ...'
    active_channels = parseString(dom.getElementsByTagName('active_channels')[0].toxml())
    for i in range(0,8):
        fdaq.write( getSingleElement(active_channels,'channel_'+str(i)) + '\n')
    
    print 'XML::read det_serials ...'
    det_serials = parseString(dom.getElementsByTagName('serial_numbers')[0].toxml())
    for i in range(0,8):
        fdaq.write(getSingleElement(det_serials,'channel_'+str(i))+ '\n')
    
    print 'XML::read det_types ...'
    det_types = parseString(dom.getElementsByTagName('det_type')[0].toxml())
    for i in range(0,8):
        fdaq.write( getSingleElement(det_types,'channel_'+str(i))+ '\n')
    
    print 'XML::read sources ...'
    sources = parseString(dom.getElementsByTagName('sources')[0].toxml())
    for i in range(0,8):
        fdaq.write( getSingleElement(sources,'channel_'+str(i))+ '\n')
    
    print 'XML::read trigger_levels ...'
    trigger_levels = parseString(dom.getElementsByTagName('trigger_level')[0].toxml())
    for i in range(0,8):
        fdaq.write( getSingleElement(trigger_levels,'channel_'+str(i))+ '\n')
    
    print 'XML::read PMT_voltages ...'
    PMT_voltages = parseString(dom.getElementsByTagName('voltage')[0].toxml())
    for i in range(0,8):
        fdaq.write( getSingleElement(PMT_voltages,'channel_'+str(i))+ '\n')
    fdaq.close

    return daqfile

###############################################################################

#
# MAIN python code
#

print 'MAIN:: Welcome to the modulation daq-processor...'
# parse the IO arguments below
parser.add_argument("idir", help="use specific input directory")
parser.add_argument("odir", help="use specific ouput directory")
parser.add_argument("-g","--graf", help="grafOn",action="store_true")
parser.add_argument("-l","--long", help="longRoot",action="store_true")
args=parser.parse_args()
filebase, outdir = args.idir, args.odir
grafOn, longRoot = args.graf, args.long

#  get the files from the data directory
filenames = getFilenames(filebase)
nb_files = len(filenames)

cmds_to_ex = []
child_pids = []

# split files into nb_processes lists
split_file_ids = dict([[process_nb, []] for process_nb in range(0, MAX_NB_PROCESSES)])
for file_id in range(0, nb_files):
    split_file_ids[file_id % MAX_NB_PROCESSES].append(file_id)

# run on all the binary files in the input directory
for process_nb in range(MAX_NB_PROCESSES):
    # fork into desired number of processes
    print 'I am at process # ', process_nb
    try:
        # forking will produce both a child and a parent process starting here.  if the process is a child, it will return 0, if the process is a parent, it will return the PID of its child
        pid = os.fork()
        
        if pid: # this is the parent so push the children on a stack
            child_pids.append(pid)
        #print 'This is a parent, so the child\'s pid is: ', pid
        
        else: # this is a child
            #print 'I am a child, my pid is:  ', os.getpid()
            # for the number of files we want to split across the number of cores, process each file
            for file_id in split_file_ids[process_nb]:
                
                # generate driver file
                filename = filenames[file_id]
                daqfile = generateDriverFile(outdir,filename)
                
                cmd_string = './daqana -i ' + daqfile
                if(grafOn):
                    cmd_string += ' -g'
                if(longRoot):
                    cmd_string += ' -l'
        
                cmds_to_ex.append(cmd_string)
                print 'MAIN:: Processing ' + filename
                os.system(cmd_string)
                print 'MAIN:: Processing complete for ' + filename
                print 'MAIN:: Remove ' + daqfile
                cmd_string = 'rm -f ' + daqfile
                os.system(cmd_string)
            sys.exit(0)
    
	
    except OSError:
        print 'error: couldn\'t fork!'
        sys.exit(0)

for child in child_pids:
    try: 
        os.waitpid(child, 0)
    except KeyboardInterrupt:
        for child in child_pids:
            os.kill(child, signal.SIGTERM)
            sys.exit(0)

# execute the daqana command with the right arguments

print 'MAIN:: Exit from the daq-processor. bye-bye.'


