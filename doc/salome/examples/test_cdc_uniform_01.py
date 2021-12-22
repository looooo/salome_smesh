#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""Raffinement uniforme d'un maillage en triangles

Copyright 2021 EDF
Gérald NICOLAS
+33.1.78.19.43.52
"""
__revision__ = "V02.04"

import os
import tempfile

import salome

#=============== Options ====================
# REPDATA = répertoire du cas-test
REPDATA = tempfile.mkdtemp()
LONG = 36.
LARG = 24.
NOM_MAILLAGE = "Maillage"
#============================================

from salome.shaper import model

model.begin()
partSet = model.moduleDocument()

### Create Part
Part_1 = model.addPart(partSet)
Part_1_doc = Part_1.document()
model.addParameter(Part_1_doc, "Long", "{}".format(LONG))
model.addParameter(Part_1_doc, "Larg", "{}".format(LARG))

### Create Sketch
Sketch_1 = model.addSketch(Part_1_doc, model.defaultPlane("XOY"))

### Create SketchLine
SketchLine_1 = Sketch_1.addLine(36, 0, 0, 0)

### Create SketchProjection
SketchProjection_1 = Sketch_1.addProjection(model.selection("VERTEX", "PartSet/Origin"), False)
SketchPoint_1 = SketchProjection_1.createdFeature()
Sketch_1.setCoincident(SketchLine_1.endPoint(), SketchPoint_1.result())

### Create SketchLine
SketchLine_2 = Sketch_1.addLine(0, 0, 0, 24)

### Create SketchLine
SketchLine_3 = Sketch_1.addLine(0, 24, 36, 24)

### Create SketchLine
SketchLine_4 = Sketch_1.addLine(36, 24, 36, 0)
Sketch_1.setCoincident(SketchLine_4.endPoint(), SketchLine_1.startPoint())
Sketch_1.setCoincident(SketchLine_1.endPoint(), SketchLine_2.startPoint())
Sketch_1.setCoincident(SketchLine_2.endPoint(), SketchLine_3.startPoint())
Sketch_1.setCoincident(SketchLine_3.endPoint(), SketchLine_4.startPoint())
Sketch_1.setHorizontal(SketchLine_1.result())
Sketch_1.setVertical(SketchLine_2.result())
Sketch_1.setHorizontal(SketchLine_3.result())
Sketch_1.setVertical(SketchLine_4.result())
Sketch_1.setLength(SketchLine_3.result(), "Long")
Sketch_1.setLength(SketchLine_2.result(), "Larg")
model.do()

### Create Face
Face_1 = model.addFace(Part_1_doc, [model.selection("COMPOUND", "all-in-Sketch_1")])

### Create Symmetry
Symmetry_1 = model.addSymmetry(Part_1_doc, [model.selection("COMPOUND", "all-in-Face_1")], model.selection("EDGE", "PartSet/OY"), keepOriginal = True, keepSubResults = True)

### Create Symmetry
Symmetry_2 = model.addSymmetry(Part_1_doc, [model.selection("COMPOUND", "all-in-Symmetry_1")], model.selection("EDGE", "PartSet/OX"), keepOriginal = True, keepSubResults = True)

### Create Rotation
Rotation_1 = model.addRotation(Part_1_doc, [model.selection("COMPOUND", "all-in-Symmetry_2")], axis = model.selection("EDGE", "PartSet/OX"), angle = 90, keepSubResults = True)

### Create Recover
Recover_1 = model.addRecover(Part_1_doc, Rotation_1, [Symmetry_2.result()])

### Create Partition
Partition_1 = model.addPartition(Part_1_doc, [model.selection("COMPOUND", "all-in-Rotation_1"), model.selection("COMPOUND", "all-in-Recover_1")], keepSubResults = True)

### Create Group
for IAUX in range(1,9):
  _ = model.addGroup(Part_1_doc, "Faces", [model.selection("FACE", "Partition_1_1_{}".format(IAUX))])

model.end()

###
### SHAPERSTUDY component
###

model.publishToShaperStudy()
import SHAPERSTUDY
l_aux = SHAPERSTUDY.shape(model.featureStringId(Partition_1))

###
### SMESH component
###

import  SMESH
from salome.smesh import smeshBuilder

smesh = smeshBuilder.New()

Maillage_1 = smesh.Mesh(l_aux[0])
Maillage_1.SetName(NOM_MAILLAGE)

L_MAILLE = min(LARG,LONG)/5.
NETGEN_1D_2D = Maillage_1.Triangle(algo=smeshBuilder.NETGEN_1D2D)
NETGEN_2D_Parameters_1 = NETGEN_1D_2D.Parameters()
NETGEN_2D_Parameters_1.SetMaxSize( L_MAILLE )
NETGEN_2D_Parameters_1.SetMinSize( L_MAILLE/10. )
for groupe in l_aux[1:]:
  groupe_nom = groupe.GetName()
  _ = Maillage_1.GroupOnGeom(groupe,groupe_nom,SMESH.FACE)

_ = Maillage_1.Compute()

FIC_MAIL = os.path.join(REPDATA, "Uniform_01.med")
Maillage_1.ExportMED(FIC_MAIL)

# Uniform refinement
import SMESHHOMARD
smeshhomard = smesh.Adaptation("Uniform")
Case_1 = smeshhomard.CreateCase(NOM_MAILLAGE, FIC_MAIL, REPDATA)
Case_1.SetConfType(0)
smeshhomard.SetKeepMedOUT(True)
smeshhomard.SetPublishMeshOUT(True)
smeshhomard.SetMeshNameOUT("{}_R".format(NOM_MAILLAGE))
FIC_MAIL = os.path.join(REPDATA, "Uniform_01_R.med")
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
