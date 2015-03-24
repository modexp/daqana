#!/usr/bin/python3.4
# -*- coding: utf-8 -*-
from python.processorlib import *

###############################################################################
#
# MAIN python code
#

print('MAIN:: Welcome to the modulation daq-processor...')

# parse the IO arguments below
filebase, outdir, grafOn, longRoot, slowOn, fastOn, calibration = parseArguments(sys.argv[1:])
#  get the files from the data directory
filenames = getFilenames(filebase)
nb_files = len(filenames)
print('MAIN:: Number of files to process = ', nb_files)


if slowOn:
    print('MAIN:: Beginning to parse slow data')
    print(filenames)
    daqfile = generateDriverFile(outdir, filenames[0], calibration)
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
        #cmd_string = cmd_string + ' -s'
            cmd_string = cmd_string + ' '
        if(fastOn):
            cmd_string = cmd_string + ' -f'
        if((not fastOn) and (not slowOn)):
            print('MAIN:: User did not specify which data to parse, only filling fast data')
            #cmd_string = cmd_string + ' -s'

        print('MAIN:: Processing ' + filename)
        print('MAIN:: CMD ' + cmd_string)
        os.system(cmd_string)
        print('MAIN:: Processing complete for ' + filename)
        print('MAIN:: Remove ' + daqfile)
        cmd_string = 'rm -f ' + daqfile
        os.system(cmd_string)

print('MAIN:: Exit from the daq-processor. bye-bye.')


