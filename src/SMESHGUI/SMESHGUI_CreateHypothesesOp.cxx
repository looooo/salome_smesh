/**
*  SALOME SMESHGUI
*
*  Copyright (C) 2005  CEA/DEN, EDF R&D
*
*
*
*  File   : SMESHGUI_CreateHypothesesOp.cxx
*  Author : Sergey LITONIN
*  Module : SALOME
*/

#include "SMESHGUI_CreateHypothesesOp.h"

#include "SMESHGUI_CreateHypothesesDlg.h"
#include "SMESHGUI_HypothesesUtils.h"
#include "SMESHGUI_Hypotheses.h"
#include "SMESHGUI.h"

#include "SALOME_ListIteratorOfListIO.hxx"

#include <qstring.h>
#include <qstringlist.h>

/*!
 * \brief Constructor
  * \param theIsAlgo - If TRUE when operation is used for creation of gypotheses
*
* Constructor does nothing
*/
SMESHGUI_CreateHypothesesOp::SMESHGUI_CreateHypothesesOp( const bool theIsAlgo )
: SMESHGUI_Operation(),
  myIsAlgo( theIsAlgo ),
  myDlg( 0 )
{
}

/*!
 * \brief Destructor
*/
SMESHGUI_CreateHypothesesOp::~SMESHGUI_CreateHypothesesOp()
{
}

/*!
 * \brief Start operation
*
* Virtual method redefined from the base class initializes and shows dialog
*/
void SMESHGUI_CreateHypothesesOp::startOperation()
{
  if ( myDlg == 0 )
    myDlg = new SMESHGUI_CreateHypothesesDlg( myIsAlgo );

  SMESHGUI_Operation::startOperation();

  QStringList aHypList, aPluginNames, aLabels, anIconIds;
    
  aHypList = SMESH::GetAvailableHypotheses( myIsAlgo );
  for ( int i = 0, n = aHypList.count(); i < n; ++i )
  {
    HypothesisData* aHypData = SMESH::GetHypothesisData( aHypList[i] );
    if ( aHypData )
    {
      aPluginNames.append( aHypData->PluginName );
      aLabels.append( aHypData->Label );
      anIconIds.append( aHypData->IconId );
    }
    else
    {
      aPluginNames.append( "" );
      aLabels.append( "" );
      anIconIds.append( "" );
    }
  }

  myDlg->init( aHypList, aPluginNames, aLabels, anIconIds );
  myDlg->show();
}

/*!
 * \brief Gets dialog
  * \return Pointer to the dialog used by this operation
*
* This pointer is used by base operation for its different purposes
*/
SalomeApp_Dialog* SMESHGUI_CreateHypothesesOp::dlg() const
{
  return myDlg;
}

/*!
 * \brief onApply
  * \return TRUE if hypothesis or algorithm is created successfully, FALSE otherwise
*
* Virtual slot redefined from base class is called when "Apply" button clicked and
* creates hypothesis or algorithm
*/
bool SMESHGUI_CreateHypothesesOp::onApply()
{
  if ( isStudyLocked() )
    return false;
    
  QString aHypType = myDlg->hypName();
  if ( aHypType == "" )
    return false;

  char* sHypType = ( char* )aHypType.latin1();

  HypothesisData* aHypData = SMESH::GetHypothesisData( sHypType );
  if ( !aHypData )
    return false;
    
  QString aClientLibName = aHypData->ClientLibName;

  if ( aClientLibName == "" )
  {
    // Call hypothesis creation server method ( without GUI )
    QString aHypName = aHypData->Label;
    SMESH::CreateHypothesis( sHypType, aHypName, myIsAlgo );
  }
  else
  {
    // Get hypotheses creator client ( GUI )
    SMESHGUI_GenericHypothesisCreator* aCreator =
      SMESH::GetHypothesisCreator( sHypType );

    // Create hypothesis/algorithm
    aCreator->CreateHypothesis( myIsAlgo, dlg() );
  }

  return true;
}


















