### Mean Interspike Interval 

files = SpikeDB.getFiles(True)
for f in files:
	means = []
	err = []
	x = []
	for t in f['trials']:
		count = []
		x.append(t['xvalue'])	
		for spikes in t['passes']:
			if len(spikes) > 1:
				for s in range(1,len(spikes)):
					count.append(spikes[s]-spikes[s-1])

		if len(count) > 0:
			means.append(SpikeDB.mean(count))
			err.append(SpikeDB.stddev(count))
		else:
			means.append(SpikeDB.NOPOINT)
			err.append(SpikeDB.NOPOINT)

	SpikeDB.plotXLabel(f['xvar'])
	SpikeDB.plotYLabel('Mean Interspike Interval (ms)')
	SpikeDB.plotLine(x,means,err)

