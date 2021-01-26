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
from .geomsmesh import smesh
from salome.smesh import smeshBuilder

# -----------------------------------------------------------------------------
# --- maillage face de fissure pour identification zone de defaut

def genereMeshCalculZoneDefaut(facefiss, minSize, maxSize):
  """
  -Permet de générer un maillage sur l'objet géométrique 'facefiss' via
   l'algorithme NETGEN_1D2D :
      -SetMaxSize     = dimension max d'un élément (maxSize)
      -SetSecondOrder = élément quadratique (Y=1, N=0)
      -SetOptimize    = élément régulier (Y=1, N=0)
      -SetFineness    = finesse du maillage
       [very_coarse, coarse, moderate, fine, very_fine, custom]
       [0,           1,      2,        3,    4,         5     ]
      -SetMinSize     = dimension min d'un élément (minSize)
      -SetQuadAllowed = permission quadrangle dans maillage triangle
  -On récupère les coordonnées de chaque noeud de la fissure qu'on stocke
   dans une liste sous la forme : [X0, Y0, Z0, ..., Xn, Yn, Zn]"""

  logging.info('start')
  text = "Maillage de '{}'".format(facefiss.GetName())
  logging.info(text)

  meshFissure = smesh.Mesh(facefiss)
  algo2d = meshFissure.Triangle(algo=smeshBuilder.NETGEN_1D2D)
  hypo2d = algo2d.Parameters()
  hypo2d.SetMaxSize( maxSize )
  hypo2d.SetSecondOrder( 0 )
  hypo2d.SetOptimize( 1 )
  hypo2d.SetFineness( 2 )
  hypo2d.SetMinSize( minSize )
  hypo2d.SetQuadAllowed( 0 )
  smesh.SetName(algo2d, "algo2d_zoneFiss")
  smesh.SetName(hypo2d, "hypo1d_zoneFiss")

  is_done = meshFissure.Compute()
  text = "meshFissure.Compute"
  if is_done:
    logging.info(text+" OK")
  else:
    text = "Erreur au calcul du maillage.\n" + text
    logging.info(text)
    raise Exception(text)

  coordsNoeudsFissure = list()
  nodeIds = meshFissure.GetNodesId()
  for id in nodeIds:
    coords = meshFissure.GetNodeXYZ(id)
    coordsNoeudsFissure.append(coords[0])
    coordsNoeudsFissure.append(coords[1])
    coordsNoeudsFissure.append(coords[2])

  logging.info('end')

  return coordsNoeudsFissure
