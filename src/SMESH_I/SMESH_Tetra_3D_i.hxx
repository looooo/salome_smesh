//=============================================================================
// File      : SMESH_Tetra_3D_i.hxx
// Created   : Jeudi 31 Janvier 2003
// Author    : Nadir Bouhamou CEA
// Project   : SALOME
// Copyright : CEA 2003
// $Header$
//=============================================================================
#ifndef _SMESH_TETRA_3D_I_HXX_
#define _SMESH_TETRA_3D_I_HXX_

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_BasicHypothesis)

#include "SMESH_3D_Algo_i.hxx"

#include "SMESH_Tetra_3D.hxx"

class SMESH_Tetra_3D_i:
  public POA_SMESH::SMESH_Tetra_3D,
  public SMESH_3D_Algo_i
{
public:
  SMESH_Tetra_3D_i(const char* anHyp,
		   int studyId,
		   ::SMESH_Gen* genImpl);

  virtual ~SMESH_Tetra_3D_i();

protected:
  virtual void SetImpl(::SMESH_Tetra_3D* impl);

  ::SMESH_Tetra_3D* _impl;
};

#endif
