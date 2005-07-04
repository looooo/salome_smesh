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
  typedef enum
  {
    Object,
    MeshNode,
    MeshElement

  } SelectedObjectType;

protected:
  void              setSelectionMode( const Selection_Mode );
  void              highlight( const Handle( SALOME_InteractiveObject )&,
                               const bool, const bool = true );
  void              addOrRemoveIndex( const Handle( SALOME_InteractiveObject )&,
                                      const TColStd_MapOfInteger&, const bool );

  virtual void      startOperation();
  virtual bool      isReadyToStart();

  SMESHGUI*         getSMESHGUI() const;
  SVTK_ViewWindow*  viewWindow() const;
  SVTK_Selector*    selector() const;



  virtual int       prefix( const QString& ) const;
  // Return hard-coded prefix using to differ intersecting types

  virtual void      selected( QStringList&, SalomeApp_Dialog::TypesList&, QStringList& ) const;
  // Get names, types and ids of selected objects

  virtual int       typeById( const QString&, const SelectedObjectType ) const;
  // Find type by id

  virtual QChar     idChar() const;
  // Char using to divide <entry> and <id> in string id representation
  // By default, '#'
  
};

#endif






