// Copyright (C) 2007-2013  CEA/DEN, EDF R&D, OPEN CASCADE
//
// Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
// CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
//
// See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//
// File      : DriverGMF_Write.hxx
// Created   : Mon Sep 17 15:36:47 2012
// Author    : Edward AGAPOV (eap)


#ifndef __DriverGMF_Write_HXX__
#define __DriverGMF_Write_HXX__

#include "SMESH_DriverGMF.hxx"

#include "Driver_SMESHDS_Mesh.h"
#include "SMDSAbs_ElementType.hxx"
#include "SMDS_ElemIterator.hxx"

#include <gp_Pnt.hxx>

struct TLocalSize
{
  TLocalSize( int theNbPoints, double theSize)
  {
    nbPoints = theNbPoints;
    size = theSize;
  }
  int nbPoints;
  double size;
};

// struct gp_Pnt
// {
//   gp_Pnt( double theX, double theY, double theZ )
//   {
//     x = theX;
//     y = theY;
//     z = theZ; 
//   };
//   double x;
//   double y;
//   double z;
// };

// class TSizeMap
// { 
// public:
//   TSizeMap():
//     points(), size(0.0)
//   {
//   };
// 
//   TSizeMap(const std::vector<gp_Pnt>& thePoints, double theSize )
//   {
//     points = thePoints;
//     size = theSize;
//   };
//   
//   double GetSize(){ return size; };
//   std::vector<gp_Pnt> GetPoints(){ return points; };
//   
// private:
//   std::vector<gp_Pnt> points;
//   double              size;
// };

/*!
 * \brief Driver Writing a mesh into a GMF file.
 */
class MESHDriverGMF_EXPORT DriverGMF_Write : public Driver_SMESHDS_Mesh
{
public:

  DriverGMF_Write();
  ~DriverGMF_Write();

  void SetExportRequiredGroups( bool toExport )
  {
    _exportRequiredGroups = toExport;
  }
   
  virtual Status Perform();
  Status PerformSizeMap();
  void AddSizeMapFromMesh( SMESHDS_Mesh* mesh, double size);
  void AddSizeMap( const std::vector<gp_Pnt>& points, double size );
  void SetSizeMapPrefix( std::string prefix )
  {
    mySizeMapPrefix = prefix;
  };
//   void WriteSizeMapFromMesh( double size );
//   void AddSizeMapSection( int meshID, int nbControlPoints );
//   void AppendSize( int meshID, double size );
//   int NbVerticesInFile();
//   int OpenFileToWrite();
//   void CloseFile( int );

 private:

  SMDS_ElemIteratorPtr elementIterator(SMDSAbs_ElementType  type); 
  SMDS_ElemIteratorPtr elementIterator(SMDSAbs_EntityType   type);  
  SMDS_ElemIteratorPtr elementIterator(SMDSAbs_GeometryType type);

  bool _exportRequiredGroups;
  int mySizeMapVerticesNumber;
  std::string mySizeMapPrefix;
  std::vector<SMESHDS_Mesh*> mySizeMapMeshes;
  std::vector<TLocalSize> myLocalSizes;
  std::vector<gp_Pnt> myPoints;
//   std::vector<TSizeMap> mySizeMaps;
};

#endif
