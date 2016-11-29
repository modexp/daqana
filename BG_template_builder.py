# Template builder
# Builds a template based on the processed rootfiles in datadir.

# J. Angevaare 2016-11-18 jorana@nikhef.nl

import ROOT
from os import listdir

datadir = '/data/xenon/joranang/Modulation/templatemanydata/'
homedir = '/data/xenon/joranang/Modulation/templatemanydata/'
flength = 3559972
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
print ('Opened the files:', root_files)

print('\nFrom dates:')
for file in root_files:
    print(file.split('_')[2])

# Make a directory for the root files/trees/histograms
rootdict = {}
for ch in range(numchan):
    rootdict['rootf%i'%ch] = ROOT.TFile(homedir+"ch_%i.root"%ch, "recreate")
    file = rootdict['rootf%i'%ch]
    rootdict['tree%i'%ch] = file.Get('tree')
    name = 'ch%i'%ch
    rootdict['hist%i'%ch] = ROOT.TH1F(name,'',nbin,emin,emax) # Bins and energyrange
   
# Print how many event we expect to proces.
print('\nProcessing ROOT files')
totlen = flength * len(root_files)
print('Expected total number of events ~ %i' %totlen)

# I want to know how many events I have processed and how many seconds the whole template will consist off
eventcount, dt = 0, 0
for roots in root_files:
    # Open a new file and write the histogram in Tree T;2
    name = homedir+roots
    f = ROOT.TFile.Open(name)
    tree = f.Get("T;2")
    time = []
    for i , event in enumerate(tree) :
        eventcount += 1
        if eventcount % 100000 == 0:
            print 'processed %.2e events' %eventcount, '\tat ~ %.2f procent'%(100*float(eventcount)/totlen)
        # Comparing the propperties of an event to the cuts as described in the top of the program.
        channel = event.channel
        integral = event.integral
        istestpulse = event.istestpulse
        error = event.error
        time.append(event.time)
        for ch in range(numchan):
            if istestpulse == testpcut and integral < emax and integral > emin and error == errorcut and channel == ch:
                rootdict['hist%i' %ch].Fill(integral)
    dt += time[-1] - time[0]
    print 'processed %.2e events' %eventcount, '\tat ~ %.2f procent'%(100*float(eventcount)/totlen)
    print 'processed %s\n' %roots

print 'Done processing writing root files'

# Write and close the rootfiles
for ch in range(numchan):
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

print 'All done'




