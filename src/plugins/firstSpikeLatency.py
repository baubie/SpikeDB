### First Spike Latency

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
				count.append(p[0])
		means.append(SpikeDB.mean(count))
		err.append(SpikeDB.stddev(count))
	SpikeDB.plotXLabel(f['xvar'])
	SpikeDB.plotYLabel('Mean First Spike Latency')
	SpikeDB.plotLine(x,means,err)

