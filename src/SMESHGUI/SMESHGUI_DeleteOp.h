//  SALOME SMESHGUI
//
//  Copyright (C) 2005  CEA/DEN, EDF R&D
//
//
//
//  File   : SMESHGUI_DeleteOp.h
//  Author : Sergey LITONIN
//  Module : SALOME


#ifndef SMESHGUI_DeleteOp_H
#define SMESHGUI_DeleteOp_H

#include "SMESHGUI_Operation.h"

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Group)

/*!
 * \brief Operation for deleting objects
*/

class SMESHGUI_DeleteOp : public SMESHGUI_Operation
{
  Q_OBJECT

public:
  SMESHGUI_DeleteOp();
  virtual ~SMESHGUI_DeleteOp();

protected:

  virtual bool isReadyToStart() const;
  virtual void startOperation();
};

#endif






