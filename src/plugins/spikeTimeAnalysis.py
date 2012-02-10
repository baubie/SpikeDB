### Spike Time Analysis

def SpikeDBAdvanced():
	SpikeDB.addOptionRadio("spikeType", ["First Spike Latency", "Last Spike Latency", "Interspike Intervals"], "Spikes", 0)
	SpikeDB.addRuler()
	SpikeDB.addOptionRadio("type", ["Mean", "Median"], "Analysis Type", 0)

def SpikeDBRun():
	SpikeDB.plotClear()
	options = SpikeDB.getOptions()
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
					if options["spikeType"] == 0:
						count.append(p[0])
					if options["spikeType"] == 1:
						count.append(p[-1])
			if len(count) > 0:
				means.append(SpikeDB.mean(count))
				err.append(SpikeDB.stddev(count))
			else:
				means.append(SpikeDB.NOPOINT)
				err.append(SpikeDB.NOPOINT)

		SpikeDB.plotXLabel(f['xvar'])

		spikeType = ""
		typeName = ""
		if options["spikeType"] == 0:
			spikeType = "First"
		if options["spikeType"] == 1:
			spikeType = "Last"

		if options["type"] == 0:
			typeName = "Mean"
		if options["type"] == 1:
			typeName = "Median"

		SpikeDB.plotYLabel(spikeType + ' ' + typeName + ' (ms)')
		SpikeDB.plotLine(x,means,err)
