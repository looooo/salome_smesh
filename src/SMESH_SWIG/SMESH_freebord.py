import salome
import geompy
import SMESH
import StdMeshers

smesh = salome.lcc.FindOrLoadComponent("FactoryServer", "SMESH")
smesh.SetCurrentStudy(salome.myStudy)

# Create box without one plane

box = geompy.MakeBox(0., 0., 0., 10., 20., 30.)
subShapeList = geompy.SubShapeAll(box, geompy.ShapeType["FACE"])

FaceList  = []
for i in range( 5 ):
  FaceList.append( subShapeList[ i ] )

aComp = geompy.MakeCompound( FaceList )
aBox = geompy.Sew( aComp, 1. )
idbox = geompy.addToStudy( aBox, "box" )
  
aBox  = salome.IDToObject( idbox )

# Create mesh

hyp1 = smesh.CreateHypothesis("NumberOfSegments", "StdMeshersEngine")
hyp1.SetNumberOfSegments(5)
hyp2 = smesh.CreateHypothesis("MaxElementArea", "StdMeshersEngine")
hyp2.SetMaxElementArea(20)
hyp3 = smesh.CreateHypothesis("MaxElementArea", "StdMeshersEngine")
hyp3.SetMaxElementArea(50)

algo1 = smesh.CreateHypothesis("Regular_1D", "StdMeshersEngine")
algo2 = smesh.CreateHypothesis("MEFISTO_2D", "StdMeshersEngine")

mesh = smesh.CreateMesh(aBox)
mesh.AddHypothesis(aBox,hyp1)
mesh.AddHypothesis(aBox,hyp2)
mesh.AddHypothesis(aBox,algo1)
mesh.AddHypothesis(aBox,algo2)

smesh.Compute(mesh,aBox)

smeshgui = salome.ImportComponentGUI("SMESH")
smeshgui.Init(salome.myStudyId);
smeshgui.SetName( salome.ObjectToID( mesh ), "Mesh_freebord" );

# Criterion : Free edges
aFilterMgr = smesh.CreateFilterManager()
aPredicate = aFilterMgr.CreateFreeBorders()
aFilter = aFilterMgr.CreateFilter()
aFilter.SetPredicate( aPredicate )

anIds = aFilter.GetElementsId( mesh )

# print result
print "Criterion: Free edges Nb = ", len( anIds )
for i in range( len( anIds ) ):
  print anIds[ i ]

# create group
aGroup = mesh.CreateGroup( SMESH.EDGE, "Free edges" )
aGroup.Add( anIds )


salome.sg.updateObjBrowser(1)
