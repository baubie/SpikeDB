
# Ca
SpikeDB.plotClear()
files = SpikeDB.getFiles(True)
for f in files:
	means = []
	x = []
	for t in f['trials']:
		x.append(t['xvalue'])	
		m = 0
		for p in t['spikes']:
			m += len(p)
		m /= len(t['spikes'])
		means.append(m)
	SpikeDB.plotLine(x,means,[])

