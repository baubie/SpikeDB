import sys
import subprocess

executable = sys.argv[1]
execfolder = sys.argv[1].rsplit("/",1)[0]
libdir = execfolder+"/lib"
otool_cmd = ["otool", "-L",executable]

# Run otool
otool_out = subprocess.check_output(otool_cmd).split("\n\t")

# Find all the dylib files
for l in otool_out:
	s = l.split(".dylib")
	if len(s) > 1:
		lib = s[0]+".dylib"
		print "Library Found: " + lib
