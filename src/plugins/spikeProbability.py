### Mean Spike Count

# Calculate Mean Spike Count for Selected Files

SpikeDB.plotClear()
files = SpikeDB.getFiles(True)
for f in files:
	means = []
	err = []
	x = []
	raw = []
	for t in f['trials']:
		count = []
		x.append(t['xvalue'])	
		for p in t['spikes']:
			if len(p) > 0:
				count.append(1)
			else:
				count.append(0)
		means.append(SpikeDB.mean(count))
		err.append(SpikeDB.stddev(count))
	SpikeDB.plotXLabel(f['xvar'])
	SpikeDB.plotYLabel('Spike Probability')
	SpikeDB.plotYMin(0)
	SpikeDB.plotYMax(1.0000001)
	SpikeDB.plotLine(x,means,err)
