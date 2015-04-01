#!/usr/bin/python
# -*- coding: utf-8 -*-
import sys
sys.path.append('python')

from processorlib import *

###############################################################################

#
# MAIN python code
#

print('MAIN:: Welcome to the modulation daq-processor...')
# parse the IO arguments below
filebase, outdir, grafOn, longRoot, slowOn, fastOn, calibration = parseArguments(sys.argv[1:])

#  get the files from the data directory
filenames, slownames = getFilenames(filebase)
nb_files = len(filenames)
nb_sfiles = len(slownames)
slownames.sort()

if slowOn:
  print('MAIN:: Beginning to parse slow data')
  for i in range(nb_sfiles):
    daqfile = generateDriverFile(outdir,slownames[i], calibration)
    slow_cmd_string = './slowdaq -i ' + daqfile
    os.system(slow_cmd_string)
    cmd_string = 'rm -f ' + daqfile
    os.system(cmd_string)
  print('MAIN:: Done parsing slow data, moving on to fast data')

if fastOn: 
  for file_id in range(0, nb_files):
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

    print('MAIN:: Processing ' + filename)
    os.system(cmd_string)
    print('MAIN:: Processing complete for ' + filename)
    print('MAIN:: Remove ' + daqfile)
    cmd_string = 'rm -f ' + daqfile
    os.system(cmd_string)


print('MAIN:: Exit from the daq-processor. bye-bye.')



