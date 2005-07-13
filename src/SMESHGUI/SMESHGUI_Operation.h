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
#include <SMESHGUI_Dialog.h>
#include <SALOME_InteractiveObject.hxx>
#include <SVTK_Selection.h>

#include <SALOMEDSClient.hxx>

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
  SMESHGUI_Operation();
  virtual ~SMESHGUI_Operation();

  static  int       prefix( const QString& );
  // Return hard-coded prefix using to differ intersecting types
  
protected:
  typedef enum
  {
    Object,
    MeshNode,
    MeshElement

  } SelectedObjectType;

protected:
  Selection_Mode    selectionMode() const;
  void              setSelectionMode( const Selection_Mode );
  void              highlight( const Handle( SALOME_InteractiveObject )&,
                               const bool, const bool = true );
  void              addOrRemoveIndex( const Handle( SALOME_InteractiveObject )&,
                                      const TColStd_MapOfInteger&, const bool );

  //! sets the dialog widgets to state just after operation start
  virtual void      initDialog();

  virtual void      startOperation();
  virtual bool      isReadyToStart();

  SMESHGUI*         getSMESHGUI() const;
  SVTK_ViewWindow*  viewWindow() const;
  SVTK_Selector*    selector() const;

  _PTR(Study)       studyDS() const;


  //! Get names, types and ids of selected objects
  virtual void      selected( QStringList&, SMESHGUI_Dialog::TypesList&, QStringList& ) const;
  
  //! Find type by id
  virtual int       typeById( const QString&, const SelectedObjectType ) const;
  
  //! Char using to divide <entry> and <id> in string id representation. By default, '#'
  virtual QChar     idChar() const;
  
  //! Set accroding dialog active or inactive
  virtual void      setDialogActive( const bool );

protected slots:
  virtual void onOk();
  virtual bool onApply();
  virtual void onCancel();
};

#endif






