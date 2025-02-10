# MMMAL

The is a device adaptor for Micro-Manager. It allow for controlling of IX2 type microscopes from Olympus.

The main differences between this plugin and the stock adaptor included in the Micro-manager package:

 - MMMAL is _significantly_ faster.
 - MMMAL never disables the manual controls (buttons and focus wheel) on the instrument.
 - MMMAL reads configuration from the IX2-BSW program, which comes with each instrument -- so no two different set of definition files. 
 - MMMAL uses the [MAL](https://drive.google.com/file/d/1IXfZxyNJWCEjuss-72iNMeB8sSlYPZNO) library to control hardware, instead of using COM port directly. The library is also used by IX2-BSW.
 
