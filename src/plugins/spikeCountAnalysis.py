### Spike Count Analysis
try:
	from scipy import stats
	hasSciPy = True
except ImportError:
	hasSciPy = False


def SpikeDBAdvanced():
	typeOptions = ["Mean SpikeCount", "Median Spike Count", "Spike Probability"]
	if hasSciPy:
		typeOptions.append("Shapiro-Wilk Test")

	SpikeDB.addOptionRadio("type", typeOptions, "Analysis Type", 0)
	SpikeDB.addRuler()
	SpikeDB.addOptionCheckbox("showLimits", "Draw Minimum and Maximum Lines (Single File Only)", True)
	SpikeDB.addOptionCheckbox("showAllPoints", "Draw spike counts for all trials (Single File Only)", True)

def ShapiroWilk(f):
	values = []
	x = []

	for t in f['trials']:
		count = []
		x.append(t['xvalue'])
		for p in t['passes']:
			count.append(len(p))
		if (len(count) >= 3):
			W, p = stats.shapiro(count)
			values.append(p)
			print str(t['xvalue'])+'->(W:'+str(W)+', p:'+str(p)+')'
		else:
			values.append(SpikeDB.NOPOINT)
			print str(t['xvalue'])+'->Too Few Points (< 3)'
	return x, values

def meanSpikeCount(f):
	means = []
	err = []
	x = []
	minCount = []
	maxCount = []
	allPointsX = []
	allPointsY = []
	for t in f['trials']:
		tmpMin = 10000000000;
		tmpMax = 0;
		count = []
		x.append(t['xvalue'])
		for p in t['passes']:
			count.append(len(p))
			if len(p) > tmpMax: tmpMax = len(p)
			if len(p) < tmpMin: tmpMin = len(p)
			allPointsY.append(len(p))
			allPointsX.append(t['xvalue'])
		minCount.append(tmpMin)
		maxCount.append(tmpMax)
		means.append(SpikeDB.mean(count))
		err.append(SpikeDB.stddev(count))
	return x, means, err, minCount, maxCount, allPointsX, allPointsY

def spikeProbability(f):
	prob = []
	err = []
	x = []
	for t in f['trials']:
		count = []
		x.append(t['xvalue'])
		for p in t['passes']:
			if len(p) > 0:
				count.append(1)
			else:
				count.append(0)
		prob.append(SpikeDB.mean(count))
		err.append(SpikeDB.stddev(count))
	return x, prob, err

def medianSpikeCount(f):
	means = []
	medians = []
	err = []
	x = []
	minCount = []
	maxCount = []
	allPointsX = []
	allPointsY = []
	for t in f['trials']:
		tmpMin = 10000000000;
		tmpMax = 0;
		count = []
		x.append(t['xvalue'])
		for p in t['passes']:
			count.append(len(p))
			if len(p) > tmpMax: tmpMax = len(p)
			if len(p) < tmpMin: tmpMin = len(p)
			allPointsY.append(len(p))
			allPointsX.append(t['xvalue'])
		minCount.append(tmpMin)
		maxCount.append(tmpMax)
		median = 0
		if len(count) > 0:
			if len(count) % 2 == 1:
				median = count[int(len(count) * 0.5 - 0.5)]
			else:
				median = SpikeDB.mean([count[int(len(count) * 0.5)-1], count[int(len(count) * 0.5)]])
		medians.append(median)
		err.append(SpikeDB.stddev(count))
	return x, medians, err, minCount, maxCount, allPointsX, allPointsY


def SpikeDBRun():
	files = SpikeDB.getFiles(True)
	options = SpikeDB.getOptions()

	for f in files:
		x = []
		means = []
		err = []
		minCount = []
		maxCount = []
		allPointsX = []
		allPointsY = []

		if options["type"] == 0:
			x, means, err, minCount, maxCount, allPointsX, allPointsY = meanSpikeCount(f)
			SpikeDB.plotSetRGBA(0,0,0,1)
			SpikeDB.plotXLabel(f['xvar'])
			SpikeDB.plotYLabel('Mean Spike Count')
			SpikeDB.plotLine(x,means,err)
			SpikeDB.plotYMin(0)

		if options["type"] == 1:
			x, medians, err, minCount, maxCount, allPointsX, allPointsY = medianSpikeCount(f)
			SpikeDB.plotSetRGBA(0,0,0,1)
			SpikeDB.plotXLabel(f['xvar'])
			SpikeDB.plotYLabel('Median Spike Count')
			SpikeDB.plotLine(x,medians,err)
			SpikeDB.plotYMin(0)

		if options["type"] == 2:
			x, values, err = spikeProbability(f)
			SpikeDB.plotSetRGBA(0,0,0,1)
			SpikeDB.plotXLabel(f['xvar'])
			SpikeDB.plotYLabel('Spike Probability')
			SpikeDB.plotLine(x,values,err)
			SpikeDB.plotYMin(0)
			SpikeDB.plotYMax(1.0000001)

		if options["type"] == 3:
			x, values = ShapiroWilk(f)
			SpikeDB.plotSetRGBA(0,0,0,1)
			SpikeDB.plotXLabel(f['xvar'])
			SpikeDB.plotYLabel('Shapiro-Wilk p-value')
			SpikeDB.plotLine(x,values,[])
			SpikeDB.plotYMin(0)
			for i in range(len(x)):
				if values[i] < 0.0001:
					print "p("+str(x[i])+") < 0.0001"
				elif values[i] < 0.001:
					print "p("+str(x[i])+") < 0.001"
				elif values[i] < 0.01:
					print "p("+str(x[i])+") < 0.01"
				elif values[i] < 0.05:
					print "p("+str(x[i])+") < 0.05"
				else:
					print "p("+str(x[i])+") >= 0.05"

		if options["type"] == 0 or options["type"] == 1:
			if options["showLimits"] and len(files) == 1:
				SpikeDB.plotSetRGBA(1,0,0,0.15)
				SpikeDB.plotLine(x,minCount,[])
				SpikeDB.plotSetRGBA(0,0,1,0.15)
				SpikeDB.plotLine(x,maxCount,[])

			if options["showAllPoints"] and len(files) == 1:
				SpikeDB.plotSetRGBA(0,0,0,0.2)
				SpikeDB.plotSetLineWidth(0)
				SpikeDB.plotSetPointSize(4)
				SpikeDB.plotLine(allPointsX, allPointsY, [])



