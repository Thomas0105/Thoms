# Thom's plugins for [VCVRack](https://vcvrack.com)

Pictogram is a module that yields cv values read from the pixels of an png-image.
Consider it as a sequencer and a sampler. It converts RGB (Red, Green, Blue) data
to voltage values in VCV-Rack.

<pre>Preview:                               Licence: GPL v3 or later</pre>
<p align="center">
   <img src="https://github.com/Thomas0105/Thoms/blob/master/images/Pictogram.png">
</p>
<p align="left">
   The preview shows Pictogram with an image loaded.
   To load an image just right click in the module area
   and choose "Load image (PNG)".<br> If you try to load another
   file type than png then Pictogram will explode...(Just kidding:-)<br>
   No, in this case Pictogram simply ignore the file.<br>Without an image
   loaded Pictogramm does nothing.<br>
   <br>
   Inputs are on the left side:<br>
   <b>Reset</b> will start the sequence from the begin immediatly<br>
   Note: The sequence loops automatically<br><br>
   <b>Clock</b> signal is required to loop the sequence. Any clock module<br>
   or VCO with a rectangle signal output will do.<br>
   <br>
   After loading a picture a select box appears in the middle. The box serves as a<br>
   tool to choose pixels from the image
</p>



