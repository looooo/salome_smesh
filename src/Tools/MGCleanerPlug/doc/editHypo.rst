.. _hypothesis-label:

===========================
How to save MG-Cleaner Parameters 
===========================

As MG-Cleaner hypothesis are not meshing hypothesis for Salome (but hypothesis for MG-Cleaner), parameters
are stored in a special file. Default file is $HOME/.MGCleaner.dat. It is strongly recommended that you
change this name if you want to preserve the way you obtain a mesh : This file is never cleaned.
All sets of parameters are logged in it.


- To save the current setting, click on "Save Params" pushbutton. 
- A set of parameters is automatically written in the .MGCleaner.dat file when you run computation.  
- Restoring the default settings can be done by pushing "Default Params". 
- "Loading Params" will reload the last set of parameters



**example of .MGCleaner.dat**


.. code-block:: python

   # Save intermediate params
   # Params for mesh :
   Optimisation ='Quality improvement Only (0)'
   Units ='Relative'
   Chordal_Tolerance_Deviation=1.0
   Ridge_Detection=True
   Split_Edge=False
   Point_Smoothing=True
   Geometrical_Approximation=0.04
   Ridge_Angle=45.0
   Maximum_Size=-2.0
   Minimum_Size=-2.0
   Mesh_Gradation=1.3
   Verbosity=3
   Memory=0
   
   
   
   # Params for Hypothese : anHypo_MGCleaner_0
   # Params for mesh : Mesh_1
   Optimisation ='Quality improvement Only (0)'
   Units ='Relative'
   Chordal_Tolerance_Deviation=1.0
   Ridge_Detection=True
   Split_Edge=False
   Point_Smoothing=True
   Geometrical_Approximation=0.04
   Ridge_Angle=45.0
   Maximum_Size=-2.0
   Minimum_Size=-2.0
   Mesh_Gradation=1.3
   Verbosity=3
   Memory=0



