//  SMESH OBJECT : interactive object for SMESH visualization
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
//  File   : SMESH_Actor.h
//  Author : Nicolas REJNERI
//  Module : SMESH
//  $Header$

#ifndef SMESH_ACTOR_H
#define SMESH_ACTOR_H

#include "SALOME_Actor.h"
#include "SMESH_Object.h"

class vtkProperty;
class vtkShrinkFilter;
class vtkPolyDataMapper;
class vtkUnstructuredGrid;
class vtkMergeFilter;
class vtkPolyData;

class vtkMapper;
class vtkActor2D;
class vtkMaskPoints;
class vtkCellCenters;
class vtkLabeledDataMapper;
class vtkSelectVisiblePoints;

class vtkScalarBarActor;
class vtkLookupTable;

class vtkPlane;
class vtkImplicitBoolean;

class vtkTimeStamp;

class SMESH_DeviceActor;


class SMESH_Actor : public SALOME_Actor{
  friend class SMESH_VisualObj;

 public:
  vtkTypeMacro(SMESH_Actor,SALOME_Actor);
  static SMESH_Actor* New(TVisualObjPtr theVisualObj, 
			  const char* theEntry, 
			  const char* theName,
			  int theIsClear);
  
  virtual void ReleaseGraphicsResources(vtkWindow *renWin);
  virtual int RenderOpaqueGeometry(vtkViewport *viewport);
  virtual int RenderTranslucentGeometry(vtkViewport *viewport);
  virtual void Render(vtkRenderer *ren);

  virtual void AddToRender(vtkRenderer* theRenderer); 
  virtual void RemoveFromRender(vtkRenderer* theRenderer);

  virtual bool hasHighlight() { return true; }  
  virtual void highlight(bool theHighlight);  
  virtual void SetPreSelected(bool thePreselect = false);

  virtual bool IsInfinitive();  

  virtual void SetOpacity(float theValue);
  virtual float GetOpacity();

  void SetSufaceColor(float r,float g,float b);
  void GetSufaceColor(float& r,float& g,float& b);

  void SetBackSufaceColor(float r,float g,float b);
  void GetBackSufaceColor(float& r,float& g,float& b);

  void SetEdgeColor(float r,float g,float b);
  void GetEdgeColor(float& r,float& g,float& b);

  void SetNodeColor(float r,float g,float b);
  void GetNodeColor(float& r,float& g,float& b);

  void SetHighlightColor(float r,float g,float b);
  void GetHighlightColor(float& r,float& g,float& b);

  void SetPreHighlightColor(float r,float g,float b);
  void GetPreHighlightColor(float& r,float& g,float& b);
 
  float GetLineWidth();
  void SetLineWidth(float theVal);

  void SetNodeSize(float size) ;
  float GetNodeSize() ;

  virtual int GetNodeObjId(int theVtkID);
  virtual float* GetNodeCoord(int theObjID);

  virtual int GetElemObjId(int theVtkID);
  virtual vtkCell* GetElemCell(int theObjID);

  virtual int GetObjDimension( const int theObjId );

  virtual void SetVisibility(int theMode);
  void SetVisibility(int theMode, bool theIsUpdateRepersentation);

  enum EReperesent { ePoint, eEdge, eSurface};
  virtual void SetRepresentation(int theMode);

  enum EEntityMode { eEdges = 0x01, eFaces = 0x02, eVolumes = 0x04, eAllEntity = 0x07};
  unsigned int GetEntityMode() const { return myEntityMode;}
  void SetEntityMode(unsigned int theMode);

  void SetPointRepresentation(bool theIsPointsVisible);
  bool GetPointRepresentation();

  virtual float* GetBounds();
  virtual void SetTransform(SALOME_Transform* theTransform); 

  vtkUnstructuredGrid* GetUnstructuredGrid();
  virtual vtkDataSet* GetInput();
  virtual vtkMapper* GetMapper();

  virtual float GetShrinkFactor();
  virtual void  SetShrinkFactor(float value );

  virtual bool IsShrunkable() { return myIsShrinkable;}
  virtual bool IsShrunk() { return myIsShrunk;}
  virtual void SetShrink(); 
  virtual void UnShrink(); 

  void SetPointsLabeled(bool theIsPointsLabeled);
  bool GetPointsLabeled(){ return myIsPointsLabeled;}

  void SetCellsLabeled(bool theIsCellsLabeled);
  bool GetCellsLabeled(){ return myIsCellsLabeled;}

  enum eControl{eNone, eLengthEdges, eFreeBorders, eFreeEdges, eMultiConnection, 
		eArea, eTaper, eAspectRatio, eMinimumAngle, eWarping, eSkew,
		eAspectRatio3D};
  void SetControlMode(eControl theMode);
  eControl GetControlMode(){ return myColorMode;}

  enum e1DControl{e1DNone, e1DColored, e1DHighlited};
  e1DControl Get1DControlMode(){ return my1DColorMode;}

  vtkScalarBarActor* GetScalarBarActor(){ return myScalarBarActor;}

  void SetPlaneParam(float theDir[3], float theDist, vtkPlane* thePlane);
  void GetPlaneParam(float theDir[3], float& theDist, vtkPlane* thePlane);

  vtkImplicitBoolean* GetPlaneContainer(); 

  TVisualObjPtr GetObject() { return myVisualObj;}

  void SetControlsPrecision( const long p ) { myControlsPrecision = p; }
  long GetControlsPrecision() const { return myControlsPrecision; }

 protected:
  TVisualObjPtr myVisualObj;
  vtkTimeStamp* myTimeStamp;

  vtkScalarBarActor* myScalarBarActor;
  vtkLookupTable* myLookupTable;

  vtkProperty* mySurfaceProp;
  vtkProperty* myBackSurfaceProp;
  vtkProperty* myEdgeProp;
  vtkProperty* myNodeProp;

  SMESH_DeviceActor* myBaseActor;
  SMESH_DeviceActor* myNodeActor;
  SMESH_DeviceActor* myPickableActor;

  vtkProperty* myHighlightProp;
  vtkProperty* myPreselectProp;
  SMESH_DeviceActor* myHighlitableActor;
  SMESH_DeviceActor* myNodeHighlitableActor;

  eControl myColorMode;
  SMESH_DeviceActor* my2DActor;
  SMESH_DeviceActor* my3DActor;
  SMESH_DeviceActor* myControlActor;

  e1DControl my1DColorMode;
  vtkProperty* my1DProp;
  SMESH_DeviceActor* my1DActor;
  vtkProperty* my1DExtProp;
  SMESH_DeviceActor* my1DExtActor;

  unsigned int myEntityMode;
  bool myIsPointsVisible;

  bool myIsShrinkable;
  bool myIsShrunk;
  
  bool myIsPointsLabeled;
  vtkUnstructuredGrid* myPointsNumDataSet;
  vtkActor2D *myPointLabels;
  vtkMaskPoints* myPtsMaskPoints;
  vtkLabeledDataMapper* myPtsLabeledDataMapper;
  vtkSelectVisiblePoints* myPtsSelectVisiblePoints;

  bool myIsCellsLabeled;
  vtkUnstructuredGrid* myCellsNumDataSet;
  vtkActor2D *myCellsLabels;
  vtkMaskPoints* myClsMaskPoints;
  vtkCellCenters* myCellCenters;
  vtkLabeledDataMapper* myClsLabeledDataMapper;
  vtkSelectVisiblePoints* myClsSelectVisiblePoints;

  vtkImplicitBoolean* myImplicitBoolean;

  long myControlsPrecision;

  SMESH_Actor();
  ~SMESH_Actor();

  bool Init(TVisualObjPtr theVisualObj, 
	    const char* theEntry, 
	    const char* theName,
	    int theIsClear);

  void SetIsShrunkable(bool theShrunkable);
  void UpdateHighlight();
  void Update();

 private:
  // hide the two parameter Render() method from the user and the compiler.
  virtual void Render(vtkRenderer *, vtkMapper *) {};
  virtual void ShallowCopy(vtkProp *prop);
  virtual void SetMapper(vtkMapper *);
  static SMESH_Actor* New();

  // Not implemented.
  SMESH_Actor(const SMESH_Actor&);  
  void operator=(const SMESH_Actor&);
};


#endif //SMESH_ACTOR_H
