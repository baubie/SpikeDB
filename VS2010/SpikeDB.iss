[Setup]
SetupIconFile=C:\devel\SpikeDB\media\SpikeDB.ico
AppCopyright=2011-2012 Brandon Aubie, McMaster University
AppName=SpikeDB
AppVerName=SpikeDB 1.8
DefaultDirName={pf}\SpikeDB
Compression=lzma/Ultra64
ShowLanguageDialog=auto
UninstallDisplayName=SpikeDB
AppPublisher=Brandon Aubie
AppVersion=1.8
VersionInfoVersion=1.8
VersionInfoCompany=Brandon Aubie
VersionInfoTextVersion=1.8
VersionInfoCopyright=2011-2012 Brandon Aubie, McMaster University
VersionInfoProductName=SpikeDB
VersionInfoProductVersion=1.8
OutputDir=C:\devel\SpikeDB\VS2010
OutputBaseFilename=SpikeDB_Install
AppID={{6DBF498B-918B-4AA1-A676-4870C7F57905}
PrivilegesRequired=none
InternalCompressLevel=Ultra64
DefaultGroupName=SpikeDB

[Dirs]

[Files]
Source: C:\devel\SpikeDB\VS2010\SpikeDB\Release\SpikeDB.exe; DestDir: {app}\bin;
Source: C:\devel\SpikeDB\VS2010\Redist\bin\*.*; DestDir: {app}\bin; Flags: ignoreversion recursesubdirs
Source: C:\devel\SpikeDB\VS2010\Redist\etc\*.*; DestDir: {app}\etc; Flags: ignoreversion recursesubdirs
Source: C:\devel\SpikeDB\VS2010\Redist\lib\*.*; DestDir: {app}\lib; Flags: ignoreversion recursesubdirs
Source: C:\devel\SpikeDB\VS2010\Redist\share\*.*; DestDir: {app}\share; Flags: ignoreversion recursesubdirs
Source: C:\devel\SpikeDB\media\SpikeDB.ico; DestDir: {app}\share;
Source: C:\devel\SpikeDB\src\plugins\*.*; DestDir: {app}\plugins; Flags: ignoreversion recursesubdirs
Source: C:\devel\SpikeDB\VS2010\Redist\python-2.7.2.msi; DestDir: {tmp}; Flags: deleteafterinstall; 

[Icons]
Name: {group}\SpikeDB; Filename: {app}\bin\SpikeDB.exe; WorkingDir: {app}; IconFilename: {app}\share\SpikeDB.ico; 
Name: "{group}\Uninstall SpikeDB"; Filename: {uninstallexe}; 

[UninstallDelete]
Name: {app}\spikedb.settings; Type: files;

[Run]
Filename: msiexec.exe; Description: "Python 2.7.2 Installer"; Parameters: "/i {tmp}\python-2.7.2.msi"; 
