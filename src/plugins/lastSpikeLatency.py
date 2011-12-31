### Last Spike Latency

def SpikeDBRun():
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

			if len(count) > 0:
				means.append(SpikeDB.mean(count))
				err.append(SpikeDB.stddev(count))
			else:
				means.append(SpikeDB.NOPOINT)
				err.append(SpikeDB.NOPOINT)

		SpikeDB.plotXLabel(f['xvar'])
		SpikeDB.plotYLabel('Mean Last Spike Latency (ms)')
		SpikeDB.plotLine(x,means,err)

