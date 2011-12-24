### Adjusted Mean Spike Count

files = SpikeDB.getFiles(True)
for f in files:
	means = []
	err = []
	x = []
	for t in f['trials']:
		count = []
		x.append(t['xvalue'])	
		for p in t['passes']:
			count.append(len(p))
		means.append(SpikeDB.mean(count))
		err.append(SpikeDB.stddev(count))
	# Adjust
	mean = 0
	if len(means) > 0:
		mean = sum(means)/len(means)
	for i in range(0, len(means)):
		means[i] = means[i] - mean

	SpikeDB.plotXLabel(f['xvar'])
	SpikeDB.plotYLabel('Adj Mean Spike Count')
	SpikeDB.plotLine(x,means,err)

