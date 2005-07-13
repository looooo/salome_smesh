//  SMESH SMESHGUI : GUI for SMESH component
//
//  Copyright (C) 2003  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
//  CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS 
// 
//  This library is free software; you can redistribute it and/or 
//  modify it under the terms of the GNU Lesser General Public 
//  License as published by the Free Software Foundation; either 
//  version 2.1 of the License. 
// 
//  This library is distributed in the hope that it will be useful, 
//  but WITHOUT ANY WARRANTY; without even the implied warranty of 
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
//  Lesser General Public License for more details. 
// 
//  You should have received a copy of the GNU Lesser General Public 
//  License along with this library; if not, write to the Free Software 
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA 
// 
//  See http://www.opencascade.org/SALOME/ or email : webmaster.salome@opencascade.org 
//
//
//
//  File   : SMESHGUI_EditHypothesesDlg.h
//  Author : Nicolas REJNERI
//  Module : SMESH
//  $Header$

#ifndef DIALOGBOX_EDIT_HYPOTHESES_H
#define DIALOGBOX_EDIT_HYPOTHESES_H

#include <qstring.h>
#include <qmap.h>
#include <qlistbox.h>

#include <SMESHGUI_Dialog.h>

typedef QMap< QString, int > MapIOR;
typedef QMap< QString, QString > MapIORText;

class ListBoxIOR : public QListBoxText
{
public:
  enum { RTTI_IOR = 1000 };

public:
  ListBoxIOR (QListBox* listbox,
              const QString& ior,
              const QString& text = QString::null )
  : QListBoxText(listbox, text), myIOR(ior) {}
  virtual ~ListBoxIOR() {};
  virtual int rtti() const { return RTTI_IOR; }
  QString GetIOR() const { return myIOR; }

private:
  QString myIOR;
};


//=================================================================================
// class    : SMESHGUI_EditHypothesesDlg
// purpose  :
//=================================================================================
class SMESHGUI_EditHypothesesDlg : public SMESHGUI_Dialog
{
    Q_OBJECT

public:
    enum { MeshOrSubMesh, GeomShape };
    typedef enum { HypoDef, HypoAssign, AlgoDef, AlgoAssign } ListType;
    
public:
    SMESHGUI_EditHypothesesDlg();
    ~SMESHGUI_EditHypothesesDlg();

    void setListsEnabled( const bool );
    int  hypoCount() const;
    int  algoCount() const;
    bool isModified() const;
    int  findItem( QListBox*, const QString& );

signals:
    void needToUpdate();
    
private:
    bool      isOld(QListBoxItem* hypItem);

private:
    MapIOR  myMapOldHypos, myMapOldAlgos;
    int     myNbModification;

    QGroupBox*    GroupHypotheses;
    QLabel*       TextHypDefinition;
    QListBox*     ListHypDefinition;
    QLabel*       TextHypAssignation;
    QListBox*     ListHypAssignation;

    QGroupBox*    GroupAlgorithms;
    QLabel*       TextAlgoDefinition;
    QListBox*     ListAlgoDefinition;
    QLabel*       TextAlgoAssignation;
    QListBox*     ListAlgoAssignation;

private slots:
    void removeItem(QListBoxItem*);
    void addItem(QListBoxItem*);

private:
  friend class SMESHGUI_EditHypothesesOp;
};

#endif // DIALOGBOX_EDIT_HYPOTHESES_H
