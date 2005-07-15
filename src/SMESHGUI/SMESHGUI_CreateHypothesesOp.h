/**
*  SALOME SMESHGUI
*
*  Copyright (C) 2005  CEA/DEN, EDF R&D
*
*
*
*  File   : SMESHGUI_CreateHypothesesOp.h
*  Author : Sergey LITONIN
*  Module : SALOME
*/

#ifndef SMESHGUI_CreateHypothesesOp_H
#define SMESHGUI_CreateHypothesesOp_H

#include "SMESHGUI_Operation.h"

// IDL Headers
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(GEOM_Gen)
#include CORBA_SERVER_HEADER(SMESH_Mesh)

class SMESHGUI_CreateHypothesesDlg;

/*!
 * \brief Operation for creation of hypotheses and algorithms
*/
class SMESHGUI_CreateHypothesesOp : public SMESHGUI_Operation
{
  Q_OBJECT

public:

  SMESHGUI_CreateHypothesesOp( const bool theIsAlgo );
  virtual ~SMESHGUI_CreateHypothesesOp();

  virtual SalomeApp_Dialog*     dlg() const;

protected slots:

  virtual bool                  onApply();

protected:

  virtual void                  startOperation();

private:

  SMESHGUI_CreateHypothesesDlg* myDlg;
  bool myIsAlgo;
  
};


#endif
