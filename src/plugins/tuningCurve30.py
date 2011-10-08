### Tuning Curve (30% Threshold)

# Only look at selected files
files = SpikeDB.getFiles(True)

# First, figure out the axes
# Ensure the file we use actually loops over frequency
freqs = []
amps = []
channel = 1
threshold = 0.3

for f in files:
	if f['frequency'][1] == SpikeDB.VARYING or f['frequency'][2] == SpikeDB.VARYING:
		for t in f['trials']:
			channel = 1
			if f['frequency'][2] == SpikeDB.VARYING:
				channel = 2
			if t['frequency'][channel] not in freqs:
				freqs.append(t['frequency'][channel])
	if f['attenuation'][channel] not in amps:
		amps.append(f['attenuation'][channel])
freqs.sort()
amps.sort()

X = []
Y = []
X2 = []
Y2 = []

# Loop over every file
for f in files:
	# Only look at files that vary over frequency (either channel is fine)
	if f['frequency'][1] == SpikeDB.VARYING or f['frequency'][2] == SpikeDB.VARYING:
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
		
		# Figure out the lower threshold
		added = False
		for m in range(0,len(prob)):
			if prob[m] >= threshold:
				X.append(freqs[m])
				channel = 1
				if f['frequency'][2] == SpikeDB.VARYING:
					channel = 2
				Y.append(-1*f['attenuation'][channel])
				added = True
				break
		if added == False:
			X.append(freqs[-1])
			Y.append(-1*f['attenuation'][channel])

		# Figure out the upper threshold
		added = False
		for m in range(len(prob)-1,0,-1):
			if prob[m] >= threshold:
				X.insert(0,freqs[m])
				channel = 1
				if f['frequency'][2] == SpikeDB.VARYING:
					channel = 2
				Y.insert(0,-1*f['attenuation'][channel])
				added = True
				break
		if added == False:
			X.insert(0,freqs[-1])
			Y.insert(0,-1*f['attenuation'][channel])

if (len(X+X2) >= 1):
	SpikeDB.plotXLabel('Frequency (kHz)')
	SpikeDB.plotYLabel('Attenuation')
	SpikeDB.plotLine(X+X2,Y+Y2,[])
	SpikeDB.plotXMin(min(X+X2)-0.5*min(X+X2))
	SpikeDB.plotXMax(max(X+X2)+0.5*max(X+X2))
	SpikeDB.plotYMin(min(Y+Y2)-10)
	SpikeDB.plotYMax(max(Y+Y2)+10)

