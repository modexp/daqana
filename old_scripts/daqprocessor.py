#!/usr/bin/python
# -*- coding: utf-8 -*-
import sys
sys.path.append('python')

from processorlib import *

MAX_NB_PROCESSES = int(4) # 4 processors running on each of the 4 cores

###############################################################################

#
# MAIN python code
#

print('MAIN:: Welcome to the modulation daq-processor...')
# parse the IO arguments below
filebase, outdir, grafOn, longRoot, slowOn, fastOn, calibration = parseArguments(sys.argv[1:])
print(slowOn, fastOn)
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
	      daqfile = generateDriverFile(outdir,filename,calibration)
   
	      cmd_string = './daqana -i ' + daqfile
	      if(grafOn):
              	cmd_string = cmd_string + ' -g'
	      if(longRoot):
              	cmd_string = cmd_string + ' -l'
	      if(slowOn):
              	cmd_string = cmd_string + ' -s'
	      #if(fastOn):
              #	cmd_string = cmd_string + ' -f'
	      if((not fastOn) and (not slowOn)):
              	print 'MAIN:: User did not specify which data to parse, only filling fast data'
		#cmd_string = cmd_string + ' -s'

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




