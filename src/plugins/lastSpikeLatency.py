### Last Spike Latency

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
		for p in t['passes']:
			if len(p) > 0:
				count.append(p[-1])
		means.append(SpikeDB.mean(count))
		err.append(SpikeDB.stddev(count))
	SpikeDB.plotXLabel(f['xvar'])
	SpikeDB.plotYLabel('Mean Last Spike Latency')
	SpikeDB.plotLine(x,means,err)

