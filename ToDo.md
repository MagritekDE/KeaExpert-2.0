To Do List for KeaExpert/SpinsolveExpert/Prospa



26/08/2025



* Check the Mouse lift is working - add script macros for this.
* Some experiments show incorrect pulse sequence visualisation.
* Check that all pulse experiment understand the different mode options.
* Check that external mode works with DSP spectrometers.
* Check that the firmware updater works for Spartan6/7 and FX3/DSP systems and that the lastest binaries are included.



10/2/2026



* Add getxrange and getyrange as new 1D plot commands. These will display an expanding semitransparent rectangle which extends to the edge of the screen in the other dimension. It should return the limits as indices and values (x) or values (y). \[Prospa] 



11/2/2026



* The getx/yrange commands should store their limits in a list so they can be redrawn. The list should include the direction (horiz/vert) and the colour (4\*1 matrix). \[Prospa]



12/2/2026



* Add an option to move a 1D trace to the front. This probably requires swapping the trace order. Is this in a linked list or an array. If the former it should be easy. \[Prospa]
* 
* Add 1D plot commands addxrange and addyrange. These would allow semitransparent regions to be set programmatically (e.g. for integrals). These should be stored in the same list described above. \[Prospa]



