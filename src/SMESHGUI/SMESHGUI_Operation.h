//  SALOME SMESHGUI
//
//  Copyright (C) 2005  CEA/DEN, EDF R&D
//
//
//
//  File   : SMESHGUI_Operation.h
//  Author : Sergey LITONIN
//  Module : SALOME


#ifndef SMESHGUI_Operation_H
#define SMESHGUI_Operation_H

#include <SalomeApp_Operation.h>
#include <SALOME_InteractiveObject.hxx>
#include <SVTK_Selection.h>

class SMESHGUI;
class SVTK_ViewWindow;
class SVTK_Selector;
class TColStd_MapOfInteger;


/*
  Class       : SMESHGUI_Operation
  Description : Base class for all SMESH operations
*/

class SMESHGUI_Operation : public SalomeApp_Operation
{
  Q_OBJECT

public:
  SMESHGUI_Operation( SalomeApp_Application* );
  virtual ~SMESHGUI_Operation();

protected:
  void      setSelectionMode( const Selection_Mode );
  void      highlight( const Handle( SALOME_InteractiveObject )&, const bool, const bool = true );
  void      addOrRemoveIndex( const Handle( SALOME_InteractiveObject )&, const TColStd_MapOfInteger&, const bool );

  virtual void startOperation();
  
  SMESHGUI*        getSMESHGUI() const;
  SVTK_ViewWindow* getViewWindow() const;
  SVTK_Selector*   getSelector() const;
  
private:
  SVTK_ViewWindow* myViewWindow;
  SVTK_Selector*   mySelector;
};

#endif






