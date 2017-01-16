#!/usr/bin/python
# -*- coding: utf-8 -*-

#
# daqprocessor_single_calibrate.py -i <input_dir> -l -s -p <process_level>
#
# Arguments
#                -i <input_dir>     : directory with Modulation data (should have same name as run)
#                -l                 : produce long root files 
#                -s                 : process slow control data
#                -f                 : process fast data
#                -p <process_level> :  0 = full reprocess of run
#                                      1 = calibrate + reprocess + analyze
#                                      2 = analyze only
#
# Process a run in 3 steps: 
#  (1) slow data
#  (2) process subset of raw data (#0-#9) to get energy calibration
#  (3) run with the energy calibration found in (2) on the full dataset 
#
# IMPORTANT VARIABLES:
# Environment variables must be set
# see http://www.physics.purdue.edu/darkmatters/doku.php?id=modulation:daq:enviromentvariables
#
# A.P. Colijn - colijn@nikhef.nl
#
############################################################################################
import sys,os,argparse
sys.path.append('python')
from processorlib import *
############################################################################################

# global initialization of the run processing
print('MAIN:: Welcome to the modulation daq-processor...')

#
# get the environment variable passed as argument and if it is not set exit with an error msg
#
def get_env_var_ensure(env_var_name):
    val = os.environ.get(env_var_name)
    if val == None:
        print("MAIN:: The environment variable %s is not set. Exiting" % env_var_name)
        sys.exit(1)
    return val

# get required directories from the environment variables

# the root raw data directory (used to determine the run name)
# Code will not fail without but will get messy if there are nested directories in the input directory
raw_data_basedir = os.environ.get('MODEXP_RAW_DATA_DIR')

# where the analysis scripts are stored (should contain folders calibration and monitor)
analysis_scripts_dir = get_env_var_ensure('MODEXP_ANALYSIS_DIR')

# output_basedir to be set to directory where the output structure should be
output_basedir = get_env_var_ensure('MODEXP_PROCESSED_DATA_DIR')

#  run dir: where do you want all the scipts to live?
run_dir = get_env_var_ensure('MODEXP_TEMP_SCRIPTS_DIR')

# where to put calibration files (including unprocessed data root files)
cal_output = get_env_var_ensure('MODEXP_CALIBRATION_DATA_DIR')

# analysis output directory -- output from analyze.C goes here
ana_output = get_env_var_ensure('MODEXP_ANALYSIS_DATA_DIR')
############################################################################################

# parse the IO arguments below
parser=argparse.ArgumentParser(description="Welcome to the Modulation experiemnt data processor")

parser.add_argument("inDir", help="Input directory")
#parser.add_argument("outDir", help="Output directory")

parser.add_argument("-g","--graf", help="Graph individual WFs",action="store_true")
parser.add_argument("-l","--long", help="longRoot, whatever that means",action="store_true")
parser.add_argument("-p","--process", type=int, help="the process level. 0 = full reprocess of run, 1 = calibrate + reprocess + analyze, 2 = analyze only")
parser.add_argument("-s","--slow", help="Process slow control data",action="store_true")
parser.add_argument("-f","--fast", help="Process fast data",action="store_true")
parser.add_argument("-c","--cal", help="Use the calibration data", type=str, default='NULL.root')
parser.add_argument("-t","--time", type=float, help="Only process data files modified in the last this number of hours")

args=parser.parse_args()
filebase = args.inDir
grafOn = args.graf
longRoot = args.long
processLevel = args.process
bothOn = (args.slow == args.fast)
slowOn = bothOn or args.slow
fastOn = bothOn or args.fast
calibration = args.cal
timeLimit = args.time # None if no time limit (default from argparse)

# Make necessary directories if they don't exist yet
ensureDir(cal_output)
ensureDir(ana_output)

############################################################################################

# Get  the run directory from the bin file location
def get_run_directory(binfilename):
    splitfilename = binfilename.split('/')
    rundir = '/'.join(splitfilename[:-2])
    return rundir

# Construct subdirectory for processed data to go in from the run directory
# If data is in suddir of standard raw data location use full subpath
# otherwise just use the name of the folder the run is stored in
def data_subdir(run_directory):
    if raw_data_basedir and run_directory.startswith(raw_data_basedir):
        subdir = run_directory[len(raw_data_basedir) + 1:]
        subdir = '/'.join(subdir.split('/'))
        if (subdir == ""):
            subdir = run_directory.split('/')[-1]
    else:
        # Data wasn't in the default place. Just use topmost directory passed as the run name
        subdir = run_directory.split('/')[-1]
        if (subdir == ""):
            subdir = run_directory.split('/')[-2]
    return subdir

# Make run name form the sub directory
def make_run_name(subdir):
    return '_'.join(subdir.split('/'))


############################################################################################

#
# process slow control data
#
def process_slow_data(basedir):
    # Get all .bin and .slo files in data directory
    slownames = getFiles(basedir, "slo", timeLimit)
    slownames.sort()
    
    print('MAIN:: Beginning to parse slow data')
    for filename in slownames:
        # Generate the output directory and make sure it exists
        rundir = get_run_directory(filename)
        subdir = data_subdir(rundir)
        outdir_tot = os.path.join(output_basedir, subdir)
        ensureDir(outdir_tot)

        # generate driver file
        daqfile = generateDriverFile(outdir_tot, filename, 'NULL.root')

        # generate and run command
        slow_cmd_string = './slowdaq -i ' + daqfile
        os.system(slow_cmd_string)
        cmd_string = 'rm -f ' + daqfile
        os.system(cmd_string)
    print('MAIN:: Done parsing slow data, moving on to fast data')

############################################################################################

#
# process fast data without calibration and then make the energy calibration
#
def make_calibration(indir, calib, run):
    #
    # make the calibration
    #
    print('MAIN:: Make energy calibration')
    calscript = run_dir+'/do_calibrate_'+run+'.C'
    fout = open(calscript,'w')
    
    #
    # compose the calibration execution script
    #
    fout.write('#include "%s/calibration/ecal.C" \n' % analysis_scripts_dir);
    fout.write('void do_calibrate_'+run+'(){ \n')
    fout.write('  ecal e("'+indir+'/","'+calib+'"); \n')
    fout.write('  e.Loop(); \n')
    fout.write('}\n')
    
    fout.close()
    
    #
    # execute the calibration
    #
    cmd_string = 'root -b -q ' + calscript
    os.system(cmd_string)

    return

############################################################################################

#
# Process all fast data in a subdirectory of basedir, with or without calibration
# calib is location of ecal.C output or 'NULL.root' to process without calibration
#
def process_fast_data(basedir, calib):
    # Get all .bin files in data directory
    filenames = getFiles(basedir, "bin", timeLimit)

    print('MAIN:: Run daqana with/without energy calibration')
    for filename in filenames:
        # Generate the output directory and make sure it exists
        rundir = get_run_directory(filename)
        subdir = data_subdir(rundir)
        outdir_tot = os.path.join(output_basedir, subdir)
        if calib == 'NULL.root':
            outdir_tot = os.path.join(outdir_tot, 'calibration')
        ensureDir(outdir_tot)
        
        print('FILE:', filename)
        # generate driver file
        daqfile = generateDriverFile(outdir_tot,filename,calib)
        
        cmd_string = './daqana -i ' + daqfile
        if (calib != 'NULL.root'):
          # include the slow data
          cmd_string = cmd_string + ' -s -l'
        else:
          # no slow data when we do the calibration
          cmd_string = cmd_string + ' -l'

        if(grafOn):
          cmd_string = cmd_string + ' -g'
        
        print('MAIN:: Processing ' + filename)
        os.system(cmd_string)
        print('MAIN:: Processing complete for ' + filename)
        print('MAIN:: Remove ' + daqfile)
        cmd_string = 'rm -f ' + daqfile
        os.system(cmd_string)

############################################################################################
def makelink(mc_path, mc_link):
   # make a symbolic link to the MC template root file
   if not os.path.exists(mc_link):
      cmd_string = 'ln -s '+mc_path+'/'+mc_link+' .'
      os.system(cmd_string)
      print('makelink:: '+cmd_string)
############################################################################################

#
# After run analysis
#
def do_analysis(indir, analyzer_file):
    print('do_analyzer:: Check if symlinks to MC root files exist')
    # simulation templates from MC
    makelink(analysis_scripts_dir+'/calibration','MC_ti44_modulation.root')
    makelink(analysis_scripts_dir+'/calibration','MC_co60_modulation.root')
    makelink(analysis_scripts_dir+'/calibration','MC_cs137_modulation.root')
    
    print('do_analyzer:: Make analyzer script and run it ....')
    
    analyzerscript = run_dir +'/do_analyzer_'+runname+'.C'
    fout = open(analyzerscript,'w')
    
    #
    # compose the analyzer execution script
    #
    fout.write('#include "'+analysis_scripts_dir+'/calibration/analyzer.C" \n');
    fout.write('void do_analyzer_'+runname+'(){ \n')
    fout.write('  analyzer ana("'+indir+'/","'+analyzer_file+'"); \n')
    fout.write('  ana.Loop(); \n')
    fout.write('} \n')
    fout.close()

    #
    # execute the analysis
    #
    cmd_string = 'root -b -q ' + analyzerscript
    os.system(cmd_string)


###############################################################################

#
# MAIN python code (see instructions on top)
#

if fastOn:
    print('daqprocessor::MAIN process level = ',processLevel);
    
    #
    # Store all unique run directories (i.e. one level up from folders containing .bin)
    #
    filenames = getFiles(filebase, "bin", timeLimit)
    rundirectories = set()
    for binfilename in filenames:
        rundir = get_run_directory(binfilename)
        rundirectories.add(rundir)
        
    #
    # run without calibration on a subset of the data to make the calibration input
    #
    
    if (processLevel <= 0):
        process_fast_data(filebase, 'NULL.root')
        
    #
    #  run with calibration
    #
    if (processLevel <= 1):
        for rundir in rundirectories:
            subdir = data_subdir(rundir)
            indir = os.path.join(output_basedir, subdir, 'calibration')
            runname = make_run_name(subdir)
            calibration = cal_output+'/CAL_'+runname+'.root'
            
            # Do the calibration
            make_calibration(indir, calibration, runname)
            
            # Process slow and then fast data
            if slowOn: process_slow_data(filebase)
            process_fast_data(filebase, calibration)
            

    #
    # do the post-processor analysis
    #
    if (processLevel <= 2):
        for rundir in rundirectories:
            subdir = data_subdir(rundir)
            indir = os.path.join(output_basedir, subdir)
            runname = make_run_name(subdir)
            analyzer_file = ana_output + '/ANA_' + runname + '.root'
            do_analysis(indir, analyzer_file)

#
# Only if fast was not on do we need to now process the slow data
# if both were on it was done above
#
elif slowOn:
    print('daqprocessor::MAIN process slow only')
    process_slow_data(filebase)

print('MAIN:: Exit from the daq-processor. bye-bye.')

###############################################################################
