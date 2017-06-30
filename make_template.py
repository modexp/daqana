#!/project/datagrid/anaconda/bin/python
# -*- coding: utf-8 -*-
# Template builder
# Builds a template based on the processed rootfiles in datadir.

# J. Angevaare 2016-11-18 jorana@nikhef.nl

import ROOT
# from tqdm import tqdm
from os import listdir
datadir = '/data/xenon/joranang/Modulation/templatemanydata/'
homedir = '/data/xenon/mod_admin/Modulation/daqana/'
use_x_files = 1

def main():
    flength = 4927292.0
    emin    = 0
    emax    = 3000
    numchan = 8
    nbin    = 600
    errorcut= 0
    testpcut= '\x00'


    # Opening the files
    all_files = sorted(listdir(datadir))
    root_files = []
    for file in all_files:
        if 'mx_n_20' in file and not '.png' in file and '.root' in file:
            root_files.append(file)
    if use_x_files != 0: root_files = root_files[-use_x_files:]

    print ('Starting::\tOpened the files:', root_files)
    
    print('\nStarting::\tFrom dates:')
    for file in root_files:
        print(file.split('_')[2])

    # Make a directory for the root files/trees/histograms
    rootdict = {}
    for ch in range(numchan):
        rootdict['rootf%i'%ch] = ROOT.TFile(datadir+"ch_%i.root"%ch, "recreate")
        # For the histograms
        file = rootdict['rootf%i'%ch]
        rootdict['tree%i'%ch] = file.Get('tree')
        name = 'ch%i'%ch
        rootdict['hist%i'%ch] = ROOT.TH1F(name,'',nbin, emin, emax) # Bins and energyrange
        
        # For the total time
        name = 'time'
        rootdict['time%i'%ch] = ROOT.TH1F(name,'', 1, 0, 1) # One bin
       
    # Print how many event we expect to proces.
    print('\nStarting::\tProcessing ROOT files')
    totlen = flength * len(root_files)
    print('Starting::\tExpected total number of events ~ %i' %totlen)

    # I want to know how many events I have processed and how many seconds the whole template will consist off
    eventcount, dt = 0, 0
    for roots in root_files:
        # Open a new file and write the histogram in Tree T;2
        name = datadir+roots
        f = ROOT.TFile.Open(name)
        try: tree = f.Get("T;3")        
        except AttributeError: 
            print('Loop::\tNo T;3 tree trying T;2 instead')
            try: tree = f.Get("T;2")
            except AttributeError: 
                print('Loop::\tNo T;2 or T;3 tree trying T;1 instead')    
                tree = f.Get("T;1")
        try: 
            tree.SetBranchStatus('*', 0)
            add_time = True
        except AttributeError: 
            print('Loop::\tTObject, no tree skipping this ROOT file')
            add_time = False
            continue

        # Activate only needed branches
        for branchname in ['channel', 'integral', 'istestpulse', 'time', 'error']:
            tree.SetBranchStatus(branchname, 1)

        for i , event in enumerate(tree):

            # Comparing the propperties of an event to the cuts as described in the top of the program.
            integral = event.integral  
            if integral < emax:
                if integral > emin:
                    if event.error == errorcut:
                        if event.istestpulse == testpcut:                        
                            rootdict['hist%i' %event.channel].Fill(integral)

            if i == 0 and add_time: 
                dt -= event.time 
        if add_time: dt += event.time
        eventcount += i
        f.Close()
	print('Loop::\tTotal time %e\tprocessed %.3e events \tat ~ %.2f procent\tprocessed %s\n' %(dt, eventcount, 100*float(eventcount)/totlen, roots))
        #print('Loop::\tprocessed %.2e events \tat ~ %.2f procent'%(eventcount, 100*float(eventcount)/totlen))
        #print('Loop::\tprocessed %s\n' %roots)

    print ('After loop::\tDone processing writing root files')

    # Write and close the rootfiles
    for ch in range(numchan):

        rootdict['time%i'%ch].Fill(0.5)
        rootdict['time%i'%ch].SetBinContent(1, dt)
        rootdict['rootf%i' %ch].Write()
        rootdict['rootf%i' %ch].Close()

    # I Want to have a report on some properties of the template.
    fout = open(homedir+'report.txt','w')
    fout.write('Report on the ch-template builder\n')
    fout.write('Total events = %i\n' %eventcount)
    fout.write('Total files  = %i\n' %len(root_files))
    fout.write('Average      = %.1f\n' %(eventcount/len(root_files)))
    fout.write('Total time   = %f seconds\n' %dt)
    fout.write('Total time   = %.1f days' %(dt/(3600*24)))
    fout.close()

    print 'After loop::\tAll done'


if __name__ == '__main__':
    main()
