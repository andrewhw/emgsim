
# Simulation of EMG Data

This package provides a text-based interface to a muscle-EMG simualtion
library which generates output for use in quantitive EMG decoposition.

When run, a screen of options is presented to the user; these options
can be changed to reflect different types and sizes of muscle, as well
as different disease types.


## Compiling and Releases:


This project is under active development by several contributors.

Compiling under UNIX:
* Unix compilation is done using the command `make`

Compiling under Windows using Visual Studio:
* If the MSDEV (MS Developer Studio/Visual Studio) components have been included in your `PATH` variable, the project can be built by running the file `build.bat`
* If your PATH variable is not set up, you must manually build the project within MSDEV by performing the following:
	* browse to the directory this README file is in
	* open the workspace "simulator-nomfc.dsw"
	* ensure that the "Active Project" is set to be "simtext"
	* build

The above steps will produce an exectuable file called "simtext" in the current directory (same directory as the `README` file) which will generate EMG data when run from a command line.  Note that running "simtext -help" will print out a help screen describing the various command-line options

## Data Files Generated:

The files generated will be on the "E:" drive (or local directory on UNIX),
in a directory structure as follows:
* `e:\simulator\config\simulator.cfg` : config file in which last-run values were saved
* `e:\simulator\data\` : directory in which all output will be created
* `e:\simulator\data\sim<run-ID>` : a single generated run `<run-id>` is a unique number in the directory.  the first run is "000", and subsequent runs will be monotonically increasing
* `e:\simulator\data\sim*\<MuscleName>` : directory where muscular output data is put.  `<MuscleName>` comes from a user selection in the config screen.
* `e:\simulator\data\sim*\<MuscleName>\eemg` : directory where final output is placed, in files named: `micro1.dat` and `micro1.gst` where the `dat` file contains signal data, and the `gst` file contains a list of MUAPs and offset.  See below for a full description of these file formats


#DATA FILES:

The `.dat` files are written on IBM PC computers, so data widths and formats are those of the Intel x86 architecture.  .'. the following sizes are used:
* short   - 16 bit integer
* long    - 32 bit integer



## EMG .dat files:

The file format is as follows:

	<EmgFile>
	<EmgHeader>
		<Channel                : short>    : Unused
		<HP Cutoff              : short>    : unused, currently 5000
		<LP Cutoff              : short>    : unused, currently 500
		<scale                  : short>    : Used to calculate data values along
											  with compressionThreshold.
		<sampling Rate          : long>     : rate in samples/sec at which data is
											  collected.
		<number of Samples      : long>     : number of samples in file.
		<elapsed Time           : long>     : time elapsed, in sampling units.
											  This is the same as time.
		<compression Threshold  : short>    : used in conjunction with scale.
											  When read in all data values
											  should be multiplied by:
												scale/compressionThreshold
	</EmgHeader> : This header is .'. 22 bytes long
	<EmgData>
	<data point : N * short/>   : This is the EMG data.  There will be N shorts
								  in this section, where N is equal to "number
								  of samples" in the above header.
	</EmgData>
	</EmgFile>


## MUAP dco/gst files:

Both the .dco and .gst files have the same format.  The only difference
is the confidence the user has in the quality of the data in the file:
DCO files are generated from a decomp algorithm, while .gst files are
"Gold Standard" DCO files.

	<DcoFile>
	<DcoHeader>
		<Name       : 60 * char>    : name tag from generator of file.
		<Num Trains : short>        : Number of Muap trains in this file
		<Num Muaps  : short>        : Number of Muap structures in total in this file
	</DcoHeader>
									  There will be N muap structures in the
									  remainder of the file, where N is equal
	<Muap : N>                      : to Num Muaps in the header

		<firing Time    : float>    : time in samples when MUAP fired
		<buffer Offset  : long>     : offset in samples when muap fired (same as
									  time)
		<motor Unit     : short>    : the id of the motor unit (train) which this
									  MUAP belongs to
		<muap Number    : short>    : monotonically increasing id of the MUAP
									  in the file.  The first ID is 0.
		<certainty      : long>     : the certainty with which we beleive the
									  result.  Normalized 0-1
	</Muap>
	</DcoFile>



## MUAP files:


MUAP files will appear in the directory: `e:\simulator\data\sim*\<MuscleName>\tmp-mmuaps`
and they have the following form:

	<MuapFile>
	<MuapHeader>
		<Num Muaps      : long>     : Number of Muaps in this file.
									  There will be more than one
									  Muap in cases of needle movement.
									  Later there will be many, because
									  of Jitter.
		<Length of Muap : long>     : Number of data elements in a MUAP.
	</MuapHeader>
									  There will be N muaps in the
									  remainder of the file, where N is
	<Muap : N>                      : equal to Num Muaps in the header

		<data point : N * float/>   : This is the MUAP data.  There will
									  be M floats for each MUAP, where M
									  is the Length of Muap, above.
	</Muap>
	</MuapFile>


