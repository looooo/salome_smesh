# -*- coding: utf-8 -*-
# Copyright (C) 2014-2020  EDF R&D
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
#
# See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
#

import logging

import sys
import salome

salome.salome_init()

import salome_notebook
notebook = salome_notebook.notebook

import os
from blocFissure import gmu

logging.info('start')

###
### GEOM component
###

import GEOM
from salome.geom import geomBuilder
import math
import SALOMEDS


geompy = geomBuilder.New()
O = geompy.MakeVertex(0, 0, 0)
OX = geompy.MakeVectorDXDYDZ(1, 0, 0)
OY = geompy.MakeVectorDXDYDZ(0, 1, 0)
OZ = geompy.MakeVectorDXDYDZ(0, 0, 1)
geomObj_1 = geompy.MakeMarker(0, 0, 0, 1, 0, 0, 0, 1, 0)
geomObj_2 = geompy.MakeMarker(0, 0, 0, 1, 0, 0, 0, 1, 0)
sk = geompy.Sketcher2D()
sk.addPoint(0.000000, 0.000000)
sk.addSegmentAbsolute(10.000000, 0.000000)
sk.addSegmentAbsolute(10.000000, 98.750000)
sk.addArcAngleRadiusLength(0, -1.250000, 90.000000)
sk.addSegmentAbsolute(15.000000, 100.000000)
sk.addSegmentAbsolute(15.000000, 120.000000)
sk.addSegmentAbsolute(9.945000, 120.000000)
sk.addSegmentAbsolute(9.945000, 108.000000)
sk.addSegmentAbsolute(0.000000, 105.000000)
sk.close()
Sketch_1 = sk.wire(geomObj_2)
Face_1 = geompy.MakeFaceWires([Sketch_1], 1)
Vertex_1 = geompy.MakeVertex(15, 108, 0)
Vertex_2 = geompy.MakeVertex(0, 94, 0)
Vertex_3 = geompy.MakeVertex(10, 94, 0)
Face_1_vertex_17 = geompy.GetSubShape(Face_1, [17])
Line_1 = geompy.MakeLineTwoPnt(Vertex_1, Face_1_vertex_17)
Face_1_vertex_9 = geompy.GetSubShape(Face_1, [9])
Line_2 = geompy.MakeLineTwoPnt(Face_1_vertex_17, Face_1_vertex_9)
Face_1_vertex_19 = geompy.GetSubShape(Face_1, [19])
Face_1_vertex_7 = geompy.GetSubShape(Face_1, [7])
Line_3 = geompy.MakeLineTwoPnt(Face_1_vertex_19, Face_1_vertex_7)
Line_4 = geompy.MakeLineTwoPnt(Vertex_2, Vertex_3)
coupe_vis = geompy.MakePartition([Face_1], [Line_1, Line_2, Line_3, Line_4], [], [], geompy.ShapeType["FACE"], 0, [], 0)
[tige, section, tige_haute, rond, tete, section_tete] = geompy.Propagate(coupe_vis)
conge = geompy.CreateGroup(coupe_vis, geompy.ShapeType["EDGE"])
geompy.UnionIDs(conge, [25])
appui = geompy.CreateGroup(coupe_vis, geompy.ShapeType["EDGE"])
geompy.UnionIDs(appui, [39])
p_imp = geompy.CreateGroup(coupe_vis, geompy.ShapeType["EDGE"])
geompy.UnionIDs(p_imp, [11])

Vertex_4 = geompy.MakeVertex(11.25, 98.75, 0)
Vertex_5 = geompy.MakeVertexWithRef(Vertex_4, -0.5, 0.5, 0)
Vertex_6 = geompy.MakeVertexWithRef(Vertex_4, -5, 5, 0)
Line_5 = geompy.MakeLineTwoPnt(Vertex_5, Vertex_6)
Partition_1 = geompy.MakePartition([Line_5], [conge], [], [], geompy.ShapeType["EDGE"], 0, [], 1)
[Vertex_7] = geompy.SubShapes(Partition_1, [4])
Vertex_8 = geompy.MakeVertexWithRef(Vertex_7, -1.1, 1.1, 0)
generatrice = geompy.MakeLineTwoPnt(Vertex_5, Vertex_8)
Revolution_1 = geompy.MakeRevolution2Ways(generatrice, OY, 60*math.pi/180.0)
Partition_2 = geompy.MakePartition([Revolution_1], [conge], [], [], geompy.ShapeType["FACE"], 0, [], 1)
Partition_2_vertex_11 = geompy.GetSubShape(Partition_2, [11])
Plane_1 = geompy.MakePlane(Partition_2_vertex_11, OY, 2000)
Partition_3 = geompy.MakePartition([Revolution_1], [Plane_1], [], [], geompy.ShapeType["FACE"], 0, [], 0)
Vertex_9 = geompy.MakeVertex(0, 99.633883, 1.977320000000001)
Vertex_10 = geompy.MakeVertex(0, 99.633883, -8.977320000000001)
Vertex_11 = geompy.MakeVertexWithRef(Vertex_9, 0, 0, -1)
Vertex11x = geompy.MakeVertexWithRef(Vertex_11, 1, 0, 0)
Line_11x = geompy.MakeLineTwoPnt(Vertex_11, Vertex11x)
Vertex_12 = geompy.MakeVertexWithRef(Vertex_10, 0, 0, 1)
Vertex12x = geompy.MakeVertexWithRef(Vertex_12, 1, 0, 0)
Line_12x = geompy.MakeLineTwoPnt(Vertex_12, Vertex12x)
Vertex_13 = geompy.MakeVertexWithRef(Vertex_11, 0, 1, 0)
Vertex_14 = geompy.MakeVertexWithRef(Vertex_12, 0, 1, 0)
Vertex_15 = geompy.MakeRotation(Vertex_9, Line_11x, 30*math.pi/180.0)
Vertex_16 = geompy.MakeRotation(Vertex_10, Line_12x, -30*math.pi/180.0)
Arc_1 = geompy.MakeArc(Vertex_15, Vertex_9, Vertex_13,False)
Arc_2 = geompy.MakeArc(Vertex_14, Vertex_10, Vertex_16,False)
Line_6 = geompy.MakeLineTwoPnt(Vertex_13, Vertex_14)
Line_8 = geompy.MakeLineTwoPnt(Vertex_16, Vertex_15)
Wire_1 = geompy.MakeWire([Arc_1, Arc_2, Line_6, Line_8], 1e-07)
Face_2 = geompy.MakeFaceWires([Wire_1], 1)
Extrusion_1 = geompy.MakePrismVecH(Face_2, OX, 15)
Revolution_2 = geompy.MakeRevolution2Ways(generatrice, OY, 65*math.pi/180.0)
Fissure = geompy.MakeCommonList([Extrusion_1, Revolution_2], True)
fondFiss = geompy.CreateGroup(Fissure, geompy.ShapeType["EDGE"])
geompy.UnionIDs(fondFiss, [9, 7, 4])
geompy.ExportBREP(Fissure, os.path.join(gmu.pathBloc, "materielCasTests", "visFiss.brep"))

geompy.addToStudy( O, 'O' )
geompy.addToStudy( OX, 'OX' )
geompy.addToStudy( OY, 'OY' )
geompy.addToStudy( OZ, 'OZ' )
geompy.addToStudy( Sketch_1, 'Sketch_1' )
geompy.addToStudy( Face_1, 'Face_1' )
geompy.addToStudy( Vertex_1, 'Vertex_1' )
geompy.addToStudy( Vertex_2, 'Vertex_2' )
geompy.addToStudy( Vertex_3, 'Vertex_3' )
geompy.addToStudyInFather( Face_1, Face_1_vertex_17, 'Face_1:vertex_17' )
geompy.addToStudy( Line_1, 'Line_1' )
geompy.addToStudyInFather( Face_1, Face_1_vertex_9, 'Face_1:vertex_9' )
geompy.addToStudy( Line_2, 'Line_2' )
geompy.addToStudyInFather( Face_1, Face_1_vertex_19, 'Face_1:vertex_19' )
geompy.addToStudyInFather( Face_1, Face_1_vertex_7, 'Face_1:vertex_7' )
geompy.addToStudy( Line_3, 'Line_3' )
geompy.addToStudy( Line_4, 'Line_4' )
geompy.addToStudy( coupe_vis, 'coupe_vis' )
geompy.addToStudyInFather( coupe_vis, tige, 'tige' )
geompy.addToStudyInFather( coupe_vis, section, 'section' )
geompy.addToStudyInFather( coupe_vis, tige_haute, 'tige_haute' )
geompy.addToStudyInFather( coupe_vis, rond, 'rond' )
geompy.addToStudyInFather( coupe_vis, tete, 'tete' )
geompy.addToStudyInFather( coupe_vis, section_tete, 'section_tete' )
geompy.addToStudyInFather( coupe_vis, conge, 'conge' )
geompy.addToStudyInFather( coupe_vis, appui, 'appui' )
geompy.addToStudyInFather( coupe_vis, p_imp, 'p_imp' )

geompy.addToStudy( Vertex_4, 'Vertex_4' )
geompy.addToStudy( Vertex_6, 'Vertex_6' )
geompy.addToStudy( Vertex_5, 'Vertex_5' )
geompy.addToStudy( Line_5, 'Line_5' )
geompy.addToStudy( Partition_1, 'Partition_1' )
geompy.addToStudyInFather( Partition_1, Vertex_7, 'Vertex_7' )
geompy.addToStudy( Vertex_8, 'Vertex_8' )
geompy.addToStudy( generatrice, 'generatrice' )
geompy.addToStudy( Revolution_1, 'Revolution_1' )
geompy.addToStudy( Partition_2, 'Partition_2' )
geompy.addToStudyInFather( Partition_2, Partition_2_vertex_11, 'Partition_2:vertex_11' )
geompy.addToStudy( Plane_1, 'Plane_1' )
geompy.addToStudy( Partition_3, 'Partition_3' )
geompy.addToStudy( Vertex_9, 'Vertex_9' )
geompy.addToStudy( Vertex_10, 'Vertex_10' )
geompy.addToStudy( Vertex_11, 'Vertex_11' )
geompy.addToStudy( Vertex_12, 'Vertex_12' )
geompy.addToStudy( Vertex_13, 'Vertex_13' )
geompy.addToStudy( Vertex_14, 'Vertex_14' )
geompy.addToStudy( Vertex_15, 'Vertex_15' )
geompy.addToStudy( Vertex_16, 'Vertex_16' )
geompy.addToStudy( Arc_1, 'Arc_1' )
geompy.addToStudy( Arc_2, 'Arc_2' )
geompy.addToStudy( Line_6, 'Line_6' )
geompy.addToStudy( Line_8, 'Line_8' )
geompy.addToStudy( Wire_1, 'Wire_1' )
geompy.addToStudy( Face_2, 'Face_2' )
geompy.addToStudy( Extrusion_1, 'Extrusion_1' )
geompy.addToStudy( Revolution_2, 'Revolution_2' )
geompy.addToStudy( Fissure, 'Fissure' )
geompy.addToStudyInFather( Fissure, fondFiss, 'fondFiss' )

###
### SMESH component
###

logging.info("Maillage de {}".format(coupe_vis.GetName()))

import  SMESH, SALOMEDS
from salome.smesh import smeshBuilder

smesh = smeshBuilder.New()

coupe_vis_1 = smesh.Mesh(coupe_vis)
Regular_1D = coupe_vis_1.Segment()
Nb_Segments_1 = Regular_1D.NumberOfSegments(10)
Quadrangle_2D = coupe_vis_1.Quadrangle(algo=smeshBuilder.QUADRANGLE)
Regular_1D_1 = coupe_vis_1.Segment(geom=tige)
Nb_Segments_2 = Regular_1D_1.NumberOfSegments(30)
Regular_1D_2 = coupe_vis_1.Segment(geom=section)
Nb_Segments_3 = Regular_1D_2.NumberOfSegments(10,1,[ 7, 11, 16, 23 ])

tige_1 = coupe_vis_1.GroupOnGeom(tige,'tige',SMESH.EDGE)
section_1 = coupe_vis_1.GroupOnGeom(section,'section',SMESH.EDGE)
tige_haute_1 = coupe_vis_1.GroupOnGeom(tige_haute,'tige_haute',SMESH.EDGE)
rond_1 = coupe_vis_1.GroupOnGeom(rond,'rond',SMESH.EDGE)
tete_1 = coupe_vis_1.GroupOnGeom(tete,'tete',SMESH.EDGE)
section_tete_1 = coupe_vis_1.GroupOnGeom(section_tete,'section_tete',SMESH.EDGE)
conge_1 = coupe_vis_1.GroupOnGeom(conge,'conge',SMESH.EDGE)
appui_1 = coupe_vis_1.GroupOnGeom(appui,'appui',SMESH.EDGE)
p_imp_1 = coupe_vis_1.GroupOnGeom(p_imp,'p_imp',SMESH.EDGE)

is_done = coupe_vis_1.Compute()
text = "coupe_vis_1.Compute"
if is_done:
  logging.debug(text+" : OK")
else:
  text = "Erreur au calcul du maillage.\n" + text
  logging.info(text)
  raise Exception(text)

# Découpage en 2 triangles du quadrangle dans l'angle
cg_x=0.972772
cg_y=104.835
cg_z=0.
l_ids = coupe_vis_1.FindElementsByPoint( cg_x,cg_y,cg_z, SMESH.FACE )
if ( len(l_ids) != 1 ):
  text = "Maillage {}.\nImpossible de trouver l'élément proche de ({},{},{}).".format(coupe_vis.GetName(),cg_x,cg_y,cg_z)
  raise Exception(text)
isDone = coupe_vis_1.SplitQuad( l_ids, 1 )
text = "SplitQuad de l'élément n° {} du maillage de {}".format(l_ids[0],coupe_vis.GetName())
if isDone:
  logging.debug(text+" : OK")
else:
  text = "Erreur.\n" + text
  logging.info(text)
  raise Exception(text)

visHex80 = smesh.CopyMesh( coupe_vis_1, 'visHex80', 1, 0)

_ = visHex80.RotationSweepObjects( [ visHex80 ], [ visHex80 ], [ visHex80 ], SMESH.AxisStruct( 0, 0, 0, 0, 10, 0 ), 0.0785398, 40, 1e-05, 1 )

[ tige_2, section_2, tige_haute_2, rond_2, tete_2, section_tete_2, conge_2, appui_2, p_imp_2, \
  tige_rotated, tige_top, section_rotated, section_top, tige_haute_rotated, tige_haute_top, \
  rond_rotated, rond_top, tete_rotated, tete_top, section_tete_rotated, section_tete_top, \
  conge_rotated, conge_top, appui_rotated, appui_top, p_imp_rotated, p_imp_top ] = visHex80.GetGroups()
Sub_mesh_1 = Regular_1D_1.GetSubMesh()
Sub_mesh_2 = Regular_1D_2.GetSubMesh()
visHex80.ExportMED(os.path.join(gmu.pathBloc, "materielCasTests", "visSain.med"))

## Set names of Mesh objects
smesh.SetName(tige_2, 'tige')
smesh.SetName(Regular_1D.GetAlgorithm(), 'Regular_1D')
smesh.SetName(Quadrangle_2D.GetAlgorithm(), 'Quadrangle_2D')
smesh.SetName(Nb_Segments_2, 'Nb. Segments_2')
smesh.SetName(Nb_Segments_1, 'Nb. Segments_1')
smesh.SetName(Nb_Segments_3, 'Nb. Segments_3')
smesh.SetName(coupe_vis_1.GetMesh(), 'coupe_vis')
smesh.SetName(visHex80.GetMesh(), 'visHex80')
smesh.SetName(p_imp_top, 'p_imp_top')
smesh.SetName(appui_top, 'appui_top')
smesh.SetName(conge_top, 'conge_top')
smesh.SetName(section_tete_top, 'section_tete_top')
smesh.SetName(tete_top, 'tete_top')
smesh.SetName(rond_top, 'rond_top')
smesh.SetName(tige_haute_top, 'tige_haute_top')
smesh.SetName(section_top, 'section_top')
smesh.SetName(tige_top, 'tige_top')
smesh.SetName(Sub_mesh_2, 'Sub-mesh_2')
smesh.SetName(Sub_mesh_1, 'Sub-mesh_1')
smesh.SetName(p_imp_1, 'p_imp')
smesh.SetName(p_imp_rotated, 'p_imp_rotated')
smesh.SetName(appui_1, 'appui')
smesh.SetName(appui_rotated, 'appui_rotated')
smesh.SetName(conge_rotated, 'conge_rotated')
smesh.SetName(section_tete_rotated, 'section_tete_rotated')
smesh.SetName(tete_rotated, 'tete_rotated')
smesh.SetName(rond_rotated, 'rond_rotated')
smesh.SetName(tige_haute_rotated, 'tige_haute_rotated')
smesh.SetName(section_rotated, 'section_rotated')
smesh.SetName(tige_1, 'tige')
smesh.SetName(tige_rotated, 'tige_rotated')
smesh.SetName(tige_haute_1, 'tige_haute')
smesh.SetName(section_1, 'section')
smesh.SetName(tete_1, 'tete')
smesh.SetName(rond_1, 'rond')
smesh.SetName(conge_1, 'conge')
smesh.SetName(appui_2, 'appui')
smesh.SetName(section_tete_1, 'section_tete')
smesh.SetName(p_imp_2, 'p_imp')
smesh.SetName(section_tete_2, 'section_tete')
smesh.SetName(conge_2, 'conge')
smesh.SetName(rond_2, 'rond')
smesh.SetName(tete_2, 'tete')
smesh.SetName(section_2, 'section')
smesh.SetName(tige_haute_2, 'tige_haute')


if salome.sg.hasDesktop():
  salome.sg.updateObjBrowser()
