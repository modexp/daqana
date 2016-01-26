#!/usr/bin/python
# -*- coding: utf-8 -*-
import sys,argparse,os,multiprocessing
sys.path.append('python')

from processorlib import processor as proc
MAX_NB_PROCESSES = 4 # 4 processors running on each of the 4 cores

###############################################################################

def systhread(command,daq):
    print 'MAIN:: BEGINNING TO PROCESS THE FILE'
    os.system(command)
    print 'MAIN:: Processing complete'
    command = 'rm -f ' + daq
    print 'MAIN:: Remove ' + daq
    os.system(command)


#
# MAIN python code
#

print('MAIN:: Welcome to the modulation daq-processor...')
# parse the IO arguments below
parser=argparse.ArgumentParser(description="Welcome to the Modulation experiemnt data processor")

parser.add_argument("inDir", help="Input directory")
parser.add_argument("outDir", help="Output directory")
parser.add_argument("-g","--graf", help="Graph individual WFs",action="store_true")
parser.add_argument("-l","--long", help="longRoot, whatever that means",action="store_true")
parser.add_argument("-s","--slow", help="Process slow control data",action="store_true")
parser.add_argument("-f","--fast", help="Process fast data",action="store_true")
parser.add_argument("-c","--cal", help="Use the calibration data", type=str, default='NULL.root')

args=parser.parse_args()
filebase = args.inDir
outdir = args.outDir
grafOn = args.graf
longRoot = args.long
slowOn = args.slow
fastOn = args.fast
calibration = args.cal

#  get the files from the data directory
processor=proc(filebase,outdir)

filenames, slownames = processor.getFilenames()
nb_files = len(filenames)
nb_sfiles = len(slownames)
slownames.sort()

if slowOn:
  print('MAIN:: Beginning to parse slow data')
  for slowfile in slownames:
    daqfile = processor.generateDriverFile(slowfile, calibration)
    slow_cmd_string = './slowdaq -i ' + daqfile
    print "processing slow"
    os.system(slow_cmd_string)
    cmd_string = 'rm -f ' + daqfile
    os.system(cmd_string)
  print('MAIN:: Done parsing slow data, moving on to fast data')



if fastOn:
    parallelProcesses = multiprocessing.cpu_count()
    p = multiprocessing.Pool(parallelProcesses)
    for file in filenames:
        daqfile = processor.generateDriverFile(file,calibration)
        cmd_string = './daqana -i ' + daqfile
        if(grafOn):
            cmd_string = cmd_string + ' -g'
        if(longRoot):
            cmd_string = cmd_string + ' -l'
        if(slowOn):
            cmd_string = cmd_string + ' -s'

        print 'MAIN:: Processing ' + file
        p.apply_async(systhread,args = (cmd_string,daqfile))

    p.close()
    p.join()

print 'MAIN:: Exit from the daq-processor. bye-bye.'







