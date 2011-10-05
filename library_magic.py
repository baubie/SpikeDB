import sys
import subprocess
import shutil

copied = []

def update_libraries(executable):
	
	# Find all the dylib files and recursively add dependencies
	print "\nChecking dependencies of " + executable
	otool_cmd = ["otool", "-L",executable]
	execfolder = executable.rsplit("/",1)[0]
	otool_out = subprocess.check_output(otool_cmd).split("\n\t")
	execname = executable.rsplit("/",1)[1]

	for l in otool_out:
		s = l.split(".dylib")
		if len(s) > 1:
			lib = s[0]+".dylib"
			libname = lib.rsplit("/",1)[1]
			if libname not in copied: 
				print "Requires: " + lib
				new_lib = execfolder+"/"+libname
				if (lib != new_lib):
					shutil.copyfile(lib, new_lib)
					copied.append(libname)
				install_name_tool = ["install_name_tool", "-change", lib, "./"+libname, executable]
				print "Installing "+lib
				subprocess.call(install_name_tool)
				new_library = execfolder+"/"+libname
				print "Calling on " + new_library
				update_libraries(new_library)

			
# Update libraries on the default executable 
update_libraries(sys.argv[1])
