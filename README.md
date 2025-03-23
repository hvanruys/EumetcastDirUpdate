EumetcastDirUpdate (deprecated see CleanUp)
==================

is a small C++ program that moves the received files from Eumetcast to a subdirectory YYYY/MM/DD. 

The EumetcastWatcherOut.ini is a Qt ini file. It contains two main sections : "filetemplates" and "segmentdirectories".
In the [filetemplates] section we specify the files that we want to move to the YYYY/MM/DD directory. It contains the template itself and the start number of the date in the filename. 
If the template does not match with the file just received, the file is deleted. 
The [segmentdirectories] contains the directories that must be inspected.
For Linux users it is best to install the program as a daemon.


