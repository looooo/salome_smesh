# Aspect ratio 3D

# create mesh with volumes
from SMESH_mechanic import *
mesh.Tetrahedron()
mesh.Compute()
# get volumes with aspect ratio < 2.0
filter = smesh.GetFilter(smesh.VOLUME, smesh.FT_AspectRatio3D, smesh.FT_LessThan, 2.0)
ids = mesh.GetIdsFromFilter(filter)
print "Number of volumes with aspect ratio < 2.0:", len(ids)
