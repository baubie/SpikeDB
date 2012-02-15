### Spike Time Analysis
try:
	from scipy import stats
	hasSciPy = True
except ImportError:
	hasSciPy = False

def SpikeDBAdvanced():
	SpikeDB.addOptionRadio("spikeType", ["First Spike Latency", "Last Spike Latency", "Interspike Intervals"], "Spikes", 0)
	SpikeDB.addRuler()

	typeOptions = ["Mean", "Median"]
	if hasSciPy:
		typeOptions.append("Shapiro-Wilk Test")
	SpikeDB.addOptionRadio("type", typeOptions, "Analysis Type", 0)

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
				if options["type"] == 0:
					means.append(SpikeDB.mean(count))
					err.append(SpikeDB.stddev(count))
				if options["type"] == 1:
					if len(count) % 2 == 1:
						median = count[int(len(count) * 0.5 - 0.5)]
					else:
						median = SpikeDB.mean([count[int(len(count) * 0.5)-1], count[int(len(count) * 0.5)]])
					means.append(median)
				if options["type"] == 2:

					if len(count) >= 3:
						W, p = stats.shapiro(count)
						means.append(p)
						print str(t['xvalue'])+'->(W:'+str(W)+', p:'+str(p)+')'
					else:
						means.append(SpikeDB.NOPOINT)
						print str(t['xvalue'])+'->Too Few Points (< 3)'
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
			typeName = "Mean (ms)"
		if options["type"] == 1:
			typeName = "Median (ms)"
		if options["type"] == 1:
			typeName = "Shapiro-Wilk p-value"

		SpikeDB.plotYLabel(spikeType + ' ' + typeName)
		SpikeDB.plotLine(x,means,err)
