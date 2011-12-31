### Spike Probability

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

	SpikeDB.plotSetRGBA(0,0,0,0.15);
	SpikeDB.plotSetPointSize(0);
	SpikeDB.plotSetLineWidth(4);
	SpikeDB.plotLine([x[0],x[-1]], [0.5,0.5], [])
