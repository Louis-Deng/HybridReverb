# HybridReverb
A Hybrid Reverberator in C++ combining convolution and FDN-based algorithmic reverb. Part of the M.A. Music Technology Thesis - Louis Boyu Deng (2023)


includes Impulse Response (IR) from Aachen Impulse Response (AIR) Database

https://www.iks.rwth-aachen.de/en/research/tools-downloads/databases/aachen-impulse-response-database/


**DEPENDENCIES**

JUCE   https://juce.com

STK    https://ccrma.stanford.edu/software/stk/


**INSTALL**

open .jucer project file in Projucer (https://juce.com)

in project settings: 

  configure _header search path_ to include the stk/include/
  
in export settings: 

  _extra compiler flags_:          '-Istk/include/'
  
  _extra libraries to link_:       '-Lstk/src/'
  
  _extra libaraies to line_:       stk
  
