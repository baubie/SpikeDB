def pretty(d, indent=0):
	for key, value in d.iteritems():
		print '\t' * indent + str(key)
		if isinstance(value, dict):
			pretty(value, indent+1)
		else:
			print '\t' * (indent+1)+str(value)


F = Files()
pretty(F[0])
