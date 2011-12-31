### Mean Spike Count

def SpikeDBRun():
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
		SpikeDB.plotXLabel(f['xvar'])
		SpikeDB.plotYLabel('Mean Spike Count')
		SpikeDB.plotLine(x,means,err)
		SpikeDB.plotYMin(0)

