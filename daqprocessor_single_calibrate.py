#!/usr/bin/python
# -*- coding: utf-8 -*-

#
# daqprocessor_single_calibrate.py -i <input_dir> -o <output_dir>
#
# Process a run in 3 steps: 
#  (1) slow data
#  (2) process subset of raw data (#0-#9) to get energy calibration
#  (3) run with the energy calibration found in (2) on the full dataset 
#
# IMPORTANT VARIABLES:
# modulation_basedir should contain daqana and analysis packages from github
modulation_basedir = "/user/z37/Modulation"
# output_basedir to be set to directory where teh output structure should be
output_basedir = "/data/atlas/users/acolijn/Modulation"

import sys
sys.path.append('python')
from processorlib import *


###############################################################################

#
# MAIN python code
#

print('MAIN:: Welcome to the modulation daq-processor...')
# parse the IO arguments below
filebase, dummy1, grafOn, longRoot, slowOn, dummy2 = parseArguments(sys.argv[1:])

# retriever the run name
run = filebase.split('/')[-1]
if (run == ""):
  run = filebase.split('/')[len(filebase.split('/'))-2]

# compose the output directory
outdir = output_basedir + '/' + run

# make the output directory if it is not there yet
if not os.path.exists(outdir):
  cmd = 'mkdir ' + outdir
  os.system(cmd)

# make the calibration directory for the 1st round of processing if it is not there yet
caldir = outdir + '/calibration'
if not os.path.exists(caldir):
  cmd = 'mkdir ' + caldir
  os.system(cmd)

#  get the files from the data directory
filenames, slownames = getFilenames(filebase)
nb_files = len(filenames)
nb_sfiles = len(slownames)
slownames.sort()

#
# process the slow data
#
if slowOn:
  print('MAIN:: Beginning to parse slow data')
  for i in range(nb_sfiles):
    daqfile = generateDriverFile(outdir,slownames[i], 'NULL.root')
    slow_cmd_string = './slowdaq -i ' + daqfile
    os.system(slow_cmd_string)
    cmd_string = 'rm -f ' + daqfile
    os.system(cmd_string)
  print('MAIN:: Done parsing slow data, moving on to fast data')

#
# run without calibration on a subset of the data
#
if (nb_files < 10):
   nb_files_cal = nb_files
else:
   nb_files_cal = 10

for file_id in range(0, nb_files_cal):
    # generate driver file
    filename = filenames[file_id]
    daqfile = generateDriverFile(caldir,filename,'NULL.root')

    cmd_string = './daqana -i ' + daqfile
    if(longRoot):
        cmd_string = cmd_string + ' -l'
    if((not fastOn) and (not slowOn)):
        print('MAIN:: User did not specify which data to parse, only filling fast data')

    print('MAIN:: Processing ' + filename)
    os.system(cmd_string)
    print('MAIN:: Processing complete for ' + filename)
    print('MAIN:: Remove ' + daqfile)
    cmd_string = 'rm -f ' + daqfile
    os.system(cmd_string)
#
# make the calibration
#
calscript = modulation_basedir+'/analysis/calibration/do_calibrate.C'
calibration = output_basedir+'/calibration/CAL_'+run+'.root'
fout = open(calscript,'w')
fin  = open(calscript+'.TEMPLATE', 'r')
for line in fin:
    line = line.replace('DATA_DIR',caldir+'/')
    line = line.replace('CAL_FILE',calibration)
    fout.write(line)
fin.close()
fout.close()

cmd_string = 'root -b -q ' + calscript
os.system(cmd_string)

#
# run with calibration
#
for file_id in range(0, nb_files):
    print('FILE          file_id:',file_id)
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
    if((not fastOn) and (not slowOn)):
        print('MAIN:: User did not specify which data to parse, only filling fast data')

    print('MAIN:: Processing ' + filename)
    os.system(cmd_string)
    print('MAIN:: Processing complete for ' + filename)
    print('MAIN:: Remove ' + daqfile)
    cmd_string = 'rm -f ' + daqfile
    os.system(cmd_string)


print('MAIN:: Exit from the daq-processor. bye-bye.')
