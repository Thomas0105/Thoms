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
   To load an image just right click <br> in the module area
   and choose "Load image (PNG)".<br> If you try to load another
   file type than png then Pictogram will explode...(Just kidding:-)<br>
   No, in this case Pictogram simply ignore the file.<br>Without an image
   loaded Pictogramm does nothing.<br>
   <br>
   <b>Inputs</b> are on the left side:<br>
   <b>Reset</b> will start the sequence from the begin immediatly<br>
   Note: The sequence loops automatically<br><br>
   <b>Clock</b> signal is required to loop the sequence. Any clock module<br>
   or VCO with a rectangle signal output will do.<br>
   <br>
   After loading a picture a <b>select box</b> appears in the middle. The box serves as a<br>
   tool to choose pixels from the image. <b>Shift-drag</b> moves the box around and <b>Space-drag</b><br>
   resizes the box.<br>
   <br>
   <b>Outputs</b> are on the right side:<br>
   <b>Scale knob </b>adjusts the voltage scale (1V...10V) for all the color outputs<br>
   <b>Offset knob </b>adjusts an offset value for the scale (-5V...5V)<br>
   <pre>
         Bipolar examples:
   -1V to 1V; Scale = 2V, Offset = 0V
   -3V to 3V; Scale = 6V, Offset = 0V
   -2V to 4V; Scale = 6V, Offset = 1V; Scale(6V) / 2 = 3V; Offset(1V) - 3V = -2V; 1V + 3V = 4V
         Unipolar examples:
   0V to 1V; Scale = 1V, Offset = 0.5V; Scale(1V) / 2 = Offset(0.5V); 0.5V - 0.5V = 0V; 0.5V + 0.5V = 1V
   0V to 3V; Scale = 3V, Offset = 1.5V;
   0V to 8V; Scale = 8V, Offset = 4.0V
   </pre>
   <b>Red </b>part of a pixel converted to Control Voltage<br>
   <b>Green </b>part of a pixel converted to CV<br>
   <b>Blue </b>part of a pixel converted to CV<br>
   <b>Hue </b>or tone of a pixel converted to CV<br>
   <b>Saturation </b>or intensity of a pixel converted to CV<br>
   <b>Luminance </b>or lightness of a pixel converted to CV<br>
   
   
   
   
   
   
   
   
   
   
   
   
</p>



