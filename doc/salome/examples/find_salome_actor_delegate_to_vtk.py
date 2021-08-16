# Find if SALOME_ACTOR_DELEGATE_TO_VTK is activated

import salome
import os

result = os.environ.get('SALOME_ACTOR_DELEGATE_TO_VTK')
print(result)