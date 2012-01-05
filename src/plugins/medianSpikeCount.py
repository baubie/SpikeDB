### Median Spike Count


def SpikeDBAdvanced():
	SpikeDB.addOptionCheckbox("showLimits", "Draw Minimum and Maximum Lines (Single File Only)", True)
	SpikeDB.addOptionCheckbox("showAllPoints", "Draw spike counts for all trials (Single File Only)", True)

def SpikeDBRun():
	files = SpikeDB.getFiles(True)
	options = SpikeDB.getOptions()

	for f in files:
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
		SpikeDB.plotSetRGBA(0,0,0,1)
		SpikeDB.plotXLabel(f['xvar'])
		SpikeDB.plotYLabel('Median Spike Count')
		SpikeDB.plotLine(x,medians,err)
		SpikeDB.plotYMin(0)

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



