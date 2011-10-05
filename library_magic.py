import sys
import subprocess
import shutil

copied = []
ignore = ["libSystem.B.dylib","libstdc++.6.dylib"]
basefolder = sys.argv[1].rsplit("/",2)[0]

copy = True
recur = True

if len(sys.argv) == 3:
	copy = False

if len(sys.argv) == 4:
	copy = False
	recur = False

def update_libraries(executable):
	
	# Find all the dylib files and recursively add dependencies
	print "Checking dependencies of " + executable
	otool_cmd = ["otool", "-L",executable]
	execfolder = executable.rsplit("/",1)[0]
	otool_out = subprocess.check_output(otool_cmd).split("\n\t")
	execname = executable.rsplit("/",1)[1]

	for l in otool_out:
		s = l.split(".dylib")
		if len(s) > 1:
			lib = s[0]+".dylib"
			libname = lib.rsplit("/",1)[1]
			if libname not in ignore:
				if libname not in copied: 
					print "Requires: " + lib
					new_lib = execfolder+"/"+libname
					if (lib != new_lib):
						if copy:
							shutil.copyfile(lib, basefolder+"/lib/"+libname)
						copied.append(libname)
					new_library = execfolder+"/"+libname
					if recur:
						update_libraries(basefolder+"/lib/"+libname)

				# Always run the install tool
				install_name_tool = ["install_name_tool", "-change", lib, "@executable_path/../lib/"+libname, executable]
				print "Installing "+lib
				subprocess.call(install_name_tool)

			
# Update libraries on the default executable 
update_libraries(sys.argv[1])
