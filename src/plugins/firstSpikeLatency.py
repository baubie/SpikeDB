### First Spike Latency

SpikeDB.plotClear()
files = SpikeDB.getFiles(True)
maxDur = 50
for f in files:
	means = []
	err = []
	x = []
	raw = []
	for t in f['trials']:
		count = []
		x.append(t['xvalue'])	
		for p in t['spikes']:
			if len(p) > 0 and p[0] <= maxDur:
				count.append(p[0])
		means.append(SpikeDB.mean(count))
		err.append(SpikeDB.stddev(count))
	SpikeDB.plotLine(x,means,err)

