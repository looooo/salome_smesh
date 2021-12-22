#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""Raffinement uniforme d'un maillage en tétraèdres

Copyright 2021 EDF
Gérald NICOLAS
+33.1.78.19.43.52
"""
__revision__ = "V02.03"

import os
import tempfile

import salome

#=============== Options ====================
# REPDATA = répertoire du cas-test
REPDATA = tempfile.mkdtemp()
LONG = 36.
LARG = 24.
NOM_MAILLAGE = "BOX"
#============================================

from salome.shaper import model

model.begin()
partSet = model.moduleDocument()

### Create Part
Part_1 = model.addPart(partSet)
Part_1_doc = Part_1.document()
model.addParameter(Part_1_doc, "Long", "{}".format(LONG))
model.addParameter(Part_1_doc, "Larg", "{}".format(LARG))
model.addParameter(Part_1_doc, "HAUT", "(LARG+LONG)")

### Create Box
Box_1 = model.addBox(Part_1_doc, "LONG", "LARG", "HAUT")

### Create Group
Group_1 = model.addGroup(Part_1_doc, "Vertices", [model.selection("VERTEX", "[Box_1_1/Front][Box_1_1/Left][Box_1_1/Top]")])

### Create Group
Group_2_objects = [model.selection("EDGE", "[Box_1_1/Left][Box_1_1/Top]"),
                   model.selection("EDGE", "[Box_1_1/Front][Box_1_1/Left]"),
                   model.selection("EDGE", "[Box_1_1/Left][Box_1_1/Bottom]")]
Group_2 = model.addGroup(Part_1_doc, "Edges", Group_2_objects)

### Create Group
Group_3_objects = [model.selection("EDGE", "[Box_1_1/Front][Box_1_1/Top]"),
                   model.selection("EDGE", "[Box_1_1/Front][Box_1_1/Left]"),
                   model.selection("EDGE", "[Box_1_1/Front][Box_1_1/Bottom]")]
Group_3 = model.addGroup(Part_1_doc, "Edges", Group_3_objects)

### Create Group
Group_4 = model.addGroup(Part_1_doc, "Faces", [model.selection("FACE", "Box_1_1/Right")])

model.end()

###
### SHAPERSTUDY component
###

model.publishToShaperStudy()
import SHAPERSTUDY
l_aux = SHAPERSTUDY.shape(model.featureStringId(Box_1))

###
### SMESH component
###

import  SMESH
from salome.smesh import smeshBuilder

smesh = smeshBuilder.New()

Maillage_1 = smesh.Mesh(l_aux[0])
Maillage_1.SetName(NOM_MAILLAGE)

L_MAILLE = min(LARG,LONG)/5.
NETGEN_1D_2D_3D = Maillage_1.Tetrahedron(algo=smeshBuilder.NETGEN_1D2D3D)
NETGEN_3D_Parameters_1 = NETGEN_1D_2D_3D.Parameters()
NETGEN_3D_Parameters_1.SetMaxSize( L_MAILLE )
NETGEN_3D_Parameters_1.SetMinSize( L_MAILLE/5. )
NETGEN_3D_Parameters_1.SetSecondOrder( 1 )
_ = Maillage_1.GroupOnGeom(l_aux[1],l_aux[1].GetName(),SMESH.NODE)
_ = Maillage_1.GroupOnGeom(l_aux[2],l_aux[2].GetName(),SMESH.EDGE)
_ = Maillage_1.GroupOnGeom(l_aux[3],l_aux[3].GetName(),SMESH.EDGE)
_ = Maillage_1.GroupOnGeom(l_aux[4],l_aux[4].GetName(),SMESH.FACE)

_ = Maillage_1.Compute()

#FIC_MAIL = os.path.join(REPDATA, "Uniform_02.med")
#Maillage_1.ExportMED(FIC_MAIL)

# Uniform refinement
import SMESHHOMARD
smeshhomard = smesh.Adaptation("Uniform")
#Case_1 = homard.CreateCase(NOM_MAILLAGE, FIC_MAIL, REPDATA)
Case_1 = homard.CreateCase(NOM_MAILLAGE, Maillage_1.GetMesh(), REPDATA)
Case_1.SetConfType(0)
smeshhomard.SetKeepMedOUT(True)
smeshhomard.SetPublishMeshOUT(True)
smeshhomard.SetMeshNameOUT("{}_R".format(NOM_MAILLAGE))
FIC_MAIL = os.path.join(REPDATA, "Uniform_02_R.med")
smeshhomard.SetMeshFileOUT(FIC_MAIL)
smeshhomard.SetKeepWorkingFiles(False)
codret = smeshhomard.Compute()

# Check
if os.path.isfile(FIC_MAIL):
  os.remove(FIC_MAIL)
else:
  print("Test Uniform refinement: Error: no output med file")
  assert(False)

# Ménage
os.rmdir(REPDATA)

if salome.sg.hasDesktop():
  salome.sg.updateObjBrowser()
