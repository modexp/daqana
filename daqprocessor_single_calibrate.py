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
#  run dir: where do you want all the scipts to live?
run_dir = modulation_basedir + "/stoomboot"

import sys
sys.path.append('python')
from processorlib import *

############################################################################################


# global initialization of the run processing

print('MAIN:: Welcome to the modulation daq-processor...')
# parse the IO arguments below
filebase, dummy1, grafOn, longRoot, dummy2, dummy3 = parseArguments(sys.argv[1:])

# retrieve the run name
run = filebase.split('/')[-1]
if (run == ""):
    run = filebase.split('/')[len(filebase.split('/'))-2]

# compose the output directory
outdir = output_basedir + '/' + run

# make the output directory if it is not there yet
if not os.path.exists(outdir):
    cmd = 'mkdir ' + outdir
    os.system(cmd)

# calibration filename
calibration = output_basedir+'/calibration/CAL_'+run+'.root'


############################################################################################

#
# process slow control data
#
def process_slow_data():
    #  get the files from the data directory
    filenames, slownames = getFilenames(filebase)
    nb_files = len(filenames)
    nb_sfiles = len(slownames)
    slownames.sort()
    
    print('MAIN:: Beginning to parse slow data')
    for i in range(nb_sfiles):
        daqfile = generateDriverFile(outdir,slownames[i], 'NULL.root')
        slow_cmd_string = './slowdaq -i ' + daqfile
        os.system(slow_cmd_string)
        cmd_string = 'rm -f ' + daqfile
        os.system(cmd_string)
    print('MAIN:: Done parsing slow data, moving on to fast data')

############################################################################################

#
# process fast data without calibration and then make teh energy calibration
#
def make_calibration(calib):
    #
    # make the calibration
    #
    print('MAIN:: Make energy calibration')
    calscript = run_dir+'/do_calibrate_'+run+'.C'
    fout = open(calscript,'w')
    fin  = open(modulation_basedir+'/analysis/calibration/do_calibrate.C.TEMPLATE', 'r')
    for line in fin:
        line = line.replace('DATA_DIR',outdir+'/')
        line = line.replace('CAL_FILE',calib)
        fout.write(line)
    fin.close()
    fout.close()
    
    cmd_string = 'root -b -q ' + calscript
    os.system(cmd_string)

    return

############################################################################################

#
# run with calibration
#
def process_fast_data(calib):
#  get the files from the data directory
    filenames, slownames = getFilenames(filebase)
    nb_files = len(filenames)

    # no calibration? just run over 10 files...
    if (calib == 'NULL.root' and nb_files>10):
        nb_files = 10

    print('MAIN:: Run daqana with energy calibration')
    for file_id in range(0, nb_files):
        #for file_id in range(0, 10):
        print('FILE          file_id:',file_id)
        # generate driver file
        filename = filenames[file_id]
        daqfile = generateDriverFile(outdir,filename,calib)
        
        cmd_string = './daqana -i ' + daqfile
        cmd_string = cmd_string + ' -s -l'
        
        print('MAIN:: Processing ' + filename)
        os.system(cmd_string)
        print('MAIN:: Processing complete for ' + filename)
        print('MAIN:: Remove ' + daqfile)
        cmd_string = 'rm -f ' + daqfile
        os.system(cmd_string)

###############################################################################

#
# MAIN python code
#

#
# process the slow data
#
process_slow_data()

#
# run without calibration on a subset of the data and make energy calibration
#
process_fast_data('NULL.root')
make_calibration(calibration)

#
# run with calibration
#
process_fast_data(calibration)

#
# After run analysis
#
print('MAIN:: Make gain stability graphs')
gainscript = run_dir +'/do_gain_'+run+'.C'
gain_file = output_basedir+'/calibration/GAIN_'+run+'.root'
fout = open(gainscript,'w')
fin  = open(modulation_basedir+'/analysis/calibration/do_gain.C.TEMPLATE', 'r')
for line in fin:
    line = line.replace('DATA_DIR',outdir+'/')
    line = line.replace('GAIN_FILE',gain_file)
    print("GAIN "+line)
    fout.write(line)
fin.close()
fout.close()

cmd_string = 'root -b -q ' + gainscript
os.system(cmd_string)

print('MAIN:: Exit from the daq-processor. bye-bye.')
