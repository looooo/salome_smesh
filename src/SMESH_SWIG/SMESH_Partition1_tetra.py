#  Copyright (C) 2005  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
#  CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
#
#  This library is free software; you can redistribute it and/or
#  modify it under the terms of the GNU Lesser General Public
#  License as published by the Free Software Foundation; either
#  version 2.1 of the License.
#
#  This library is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#  Lesser General Public License for more details.
#
#  You should have received a copy of the GNU Lesser General Public
#  License along with this library; if not, write to the Free Software
#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
#
#  See http://www.salome-platform.org/
#
#
# Tetrahedrization of the geometry generated by the Python script GEOM_Partition1.py
# Hypothesis and algorithms for the mesh generation are global
#
#%Make geometry (like CEA script (A1)) using Partition algorithm% from OCC
# -- Rayon de la bariere

import salome
import geompy
from math import sqrt

import StdMeshers
import NETGENPlugin

smesh = salome.lcc.FindOrLoadComponent("FactoryServer", "SMESH")
smesh.SetCurrentStudy(salome.myStudy)

smeshgui = salome.ImportComponentGUI("SMESH")
smeshgui.Init(salome.myStudyId)

#---------------------------------------------------------------

barier_height = 7.0
barier_radius = 5.6 / 2 # Rayon de la bariere
colis_radius = 1.0 / 2  # Rayon du colis
colis_step = 2.0        # Distance s�parant deux colis
cc_width = 0.11         # Epaisseur du complement de colisage

# --

cc_radius = colis_radius + cc_width
colis_center = sqrt(2.0)*colis_step/2

# --

boolean_common  = 1
boolean_cut     = 2
boolean_fuse    = 3
boolean_section = 4

# --

p0 = geompy.MakeVertex(0.,0.,0.)
vz = geompy.MakeVectorDXDYDZ(0.,0.,1.)

# --

barier = geompy.MakeCylinder(p0, vz, barier_radius, barier_height)

# --

colis = geompy.MakeCylinder(p0, vz, colis_radius, barier_height)
cc    = geompy.MakeCylinder(p0, vz,    cc_radius, barier_height)

colis_cc = geompy.MakeCompound([colis, cc])
colis_cc = geompy.MakeTranslation(colis_cc, colis_center, 0.0, 0.0)

colis_cc_multi = geompy.MultiRotate1D(colis_cc, vz, 4)

# --

alveole = geompy.MakePartition([colis_cc_multi, barier])

print "Analysis of the geometry to mesh (right after the Partition) :"

subShellList = geompy.SubShapeAll(alveole, geompy.ShapeType["SHELL"])
subFaceList  = geompy.SubShapeAll(alveole, geompy.ShapeType["FACE"])
subEdgeList  = geompy.SubShapeAll(alveole, geompy.ShapeType["EDGE"])

print "number of Shells in alveole : ", len(subShellList)
print "number of Faces  in alveole : ", len(subFaceList)
print "number of Edges  in alveole : ", len(subEdgeList)

subshapes = geompy.SubShapeAll(alveole, geompy.ShapeType["SHAPE"])

## there are 9 subshapes

comp1 = geompy.MakeCompound( [ subshapes[0], subshapes[1] ] )
comp2 = geompy.MakeCompound( [ subshapes[2], subshapes[3] ] )
comp3 = geompy.MakeCompound( [ subshapes[4], subshapes[5] ] )
comp4 = geompy.MakeCompound( [ subshapes[6], subshapes[7] ] )

compGOs = []
compGOs.append( comp1 )
compGOs.append( comp2 )
compGOs.append( comp3 )
compGOs.append( comp4 )
comp = geompy.MakeCompound( compGOs )

alveole = geompy.MakeCompound( [ comp, subshapes[8] ])

idalveole = geompy.addToStudy(alveole, "alveole")

print "Analysis of the geometry to mesh (right after the MakeCompound) :"

subShellList = geompy.SubShapeAll(alveole, geompy.ShapeType["SHELL"])
subFaceList  = geompy.SubShapeAll(alveole, geompy.ShapeType["FACE"])
subEdgeList  = geompy.SubShapeAll(alveole, geompy.ShapeType["EDGE"])

print "number of Shells in alveole : ", len(subShellList)
print "number of Faces  in alveole : ", len(subFaceList)
print "number of Edges  in alveole : ", len(subEdgeList)

status = geompy.CheckShape(alveole)
print " check status ", status

# ---- launch SMESH

print "-------------------------- create Hypothesis (In this case global hypothesis are used)"

print "-------------------------- NumberOfSegments"

numberOfSegments = 10

hypNbSeg = smesh.CreateHypothesis("NumberOfSegments", "libStdMeshersEngine.so")
hypNbSeg.SetNumberOfSegments(numberOfSegments)
print hypNbSeg.GetName()
print hypNbSeg.GetId()
print hypNbSeg.GetNumberOfSegments()

smeshgui.SetName(salome.ObjectToID(hypNbSeg), "NumberOfSegments_10")

print "-------------------------- MaxElementArea"

maxElementArea = 0.1

hypArea = smesh.CreateHypothesis("MaxElementArea", "libStdMeshersEngine.so")
hypArea.SetMaxElementArea(maxElementArea)
print hypArea.GetName()
print hypArea.GetId()
print hypArea.GetMaxElementArea()

smeshgui.SetName(salome.ObjectToID(hypArea), "MaxElementArea_0.1")

print "-------------------------- MaxElementVolume"

maxElementVolume = 0.5

hypVolume = smesh.CreateHypothesis("MaxElementVolume", "libStdMeshersEngine.so")
hypVolume.SetMaxElementVolume(maxElementVolume)
print hypVolume.GetName()
print hypVolume.GetId()
print hypVolume.GetMaxElementVolume()

smeshgui.SetName(salome.ObjectToID(hypVolume), "MaxElementVolume_0.5")

print "-------------------------- create Algorithms"

print "-------------------------- Regular_1D"

regular1D = smesh.CreateHypothesis("Regular_1D", "libStdMeshersEngine.so")
smeshgui.SetName(salome.ObjectToID(regular1D), "Wire Discretisation")

print "-------------------------- MEFISTO_2D"

mefisto2D = smesh.CreateHypothesis("MEFISTO_2D", "libStdMeshersEngine.so")
smeshgui.SetName(salome.ObjectToID(mefisto2D), "MEFISTO_2D")

print "-------------------------- NETGEN_3D"

netgen3D = smesh.CreateHypothesis("NETGEN_3D", "libNETGENEngine.so")
smeshgui.SetName(salome.ObjectToID(netgen3D), "NETGEN_3D")

# ---- init a Mesh with the alveole
shape_mesh = salome.IDToObject( idalveole )

mesh = smesh.CreateMesh(shape_mesh)
smeshgui.SetName(salome.ObjectToID(mesh), "MeshAlveole")

# ---- add hypothesis to alveole

print "-------------------------- add hypothesis to alveole"

mesh.AddHypothesis(shape_mesh,regular1D)
mesh.AddHypothesis(shape_mesh,hypNbSeg)

mesh.AddHypothesis(shape_mesh,mefisto2D)
mesh.AddHypothesis(shape_mesh,hypArea)

mesh.AddHypothesis(shape_mesh,netgen3D)
mesh.AddHypothesis(shape_mesh,hypVolume)

print "-------------------------- compute the mesh of alveole "
ret = smesh.Compute(mesh,shape_mesh)

if ret != 0:
    log=mesh.GetLog(0) # no erase trace
    for linelog in log:
        print linelog
    print "Information about the Mesh_mechanic:"
    print "Number of nodes       : ", mesh.NbNodes()
    print "Number of edges       : ", mesh.NbEdges()
    print "Number of faces       : ", mesh.NbFaces()
    print "Number of triangles   : ", mesh.NbTriangles()
    print "Number of volumes     : ", mesh.NbVolumes()
    print "Number of tetrahedrons: ", mesh.NbTetras()
else:
    print "problem when computing the mesh"

salome.sg.updateObjBrowser(1)
