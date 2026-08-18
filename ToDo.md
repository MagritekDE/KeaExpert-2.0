To Do List for KeaExpert/SpinsolveExpert/Prospa



26/08/2025



* Check the Mouse lift is working - add script macros for this.
* Some experiments show incorrect pulse sequence visualisation.
* Check that all pulse experiment understand the different mode options.
* Check that external mode works with DSP spectrometers.
* Check that the firmware updater works for Spartan6/7 and FX3/DSP systems ~~and that the lastest binaries are included.~~



10/2/2026



* Add getxrange and getyrange as new 1D plot commands. These will display an expanding semitransparent rectangle which extends to the edge of the screen in the other dimension. It should return the limits as indices and values (x) or values (y). \[Prospa]



11/2/2026



* The getx/yrange commands should store their limits in a list so they can be redrawn. The list should include the direction (horiz/vert) and the colour (4\*1 matrix). \[Prospa]



12/2/2026



* Add an option to move a 1D trace to the front. This probably requires swapping the trace order. Is this in a linked list or an array. If the former it should be easy. \[Prospa]
* 
* Add 1D plot commands addxrange and addyrange. These would allow semitransparent regions to be set programmatically (e.g. for integrals). These should be stored in the same list described above. \[Prospa]



14/3/2026



* ~~Figure out how to switch end codes on the signal generator experiment so the RF stays on.~~
* Figure out a way to get delays the same on the FX3 and the DSP (see CPMGFast)
* In T1Sat replace repTime with ieTime.
* How to handle different probeheads? Do we want this to be loadable as it is at present?



10-18/8/2026



* ~~Merge all changes from Craig's 2.02.16 version to Bulat's 2.02.17~~
* Make sure the Prospa software is up to date wrt Spinsolve.
* Move the Prospa code to support 64 bit.
* ~~The Transceiver firmware folder does not include V3013.bin.~~
* ~~Reading the firmware versions on an old DSP system will latch up the software. How to resolve?~~
* It would be useful to have an internal temperature sensor. Does it have to be a PT100 probe input (check with Raschid).
* Parameter reader/writer:

  * A help file for the Kea Parameter reader/write would be useful.
  * The button 'Reset tab' and 'Reset all' parameters doesn't work in Kea parameter reader/writer.
  * The hardware capabilities button gives an error message.
  * ~~The firmware version numbers should be read only~~
  * Any changes to the interface should be noted and warning given when closing
* Ensure that FX3 firmware update for the TRex5 is working.
* Add simulators to as many experiments as possible.
* ~~Remove unnecessary DLLs from KeaExpert (such as Spinsolve and Terranova)~~
* ~~Have an option to leave the RF on at the end of the sequence. (Both DSP and FX3)~~
* Add an example experiment which uses linear prediction.
* It should be possible to not include an acquisition tab for some experiments. In this case just define some defaults.
* The end commands in an FX3 sequence should be documented.
* ttlon/off seem to be missing from the documentation.

