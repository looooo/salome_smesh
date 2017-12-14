# Copyright (C) 2012-2016  CEA/DEN, EDF R&D, OPEN CASCADE
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
#
# See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
#

SALOME_INSTALL_SCRIPTS(collect_mesh_methods.py ${SALOME_INSTALL_BINS})
            
SET(DOC_SMESH_MeshersList StdMeshers)
SET(smesh_file "${CMAKE_CURRENT_SOURCE_DIR}/collect_mesh_methods.py")

SALOME_ACCUMULATE_ENVIRONMENT(SMESH_MeshersList NOCHECK ${DOC_SMESH_MeshersList})
SALOME_ACCUMULATE_ENVIRONMENT(PYTHONPATH NOCHECK ${CMAKE_CURRENT_BINARY_DIR}/tmp2)

# TODO: to think how to generate documentation with sphinx for StdMeshers
SET(_cmd_options ${smesh_file} -o tmp1/smeshBuilder.py StdMeshers)
SALOME_GENERATE_ENVIRONMENT_SCRIPT(_cmd env_script "${PYTHON_EXECUTABLE}" "${_cmd_options}")

# Make configuration file
SALOME_CONFIGURE_FILE(conf.py.in conf.py)

# Options
# 1. Common options
SET(PAPEROPT_a4 -D latex_paper_size=a4)

# 2. Options for EN documentation
SET(SPHINXOPTS)
SET(ALLSPHINXOPTS -c ${CMAKE_CURRENT_BINARY_DIR} -d doctrees -b html ${PAPEROPT_a4} ${SPHINXOPTS} ${CMAKE_CURRENT_SOURCE_DIR}/input SMESH)

# 3. Options for FR documentation
SET(FR_SPHINXOPTS -D language=fr)
SET(FR_ALLSPHINXOPTS -c ${CMAKE_CURRENT_BINARY_DIR} -d doctrees -b html ${PAPEROPT_a4} ${FR_SPHINXOPTS} ${CMAKE_CURRENT_SOURCE_DIR}/input SMESH_fr)

# 4. Options for generation POT files
SET(POT_SPHINXOPTS)
SET(POT_ALLSPHINXOPTS -c ${CMAKE_CURRENT_BINARY_DIR} -b gettext ${POT_SPHINXOPTS} ${CMAKE_CURRENT_SOURCE_DIR}/input potfiles)

# 5. Update PO files options
SET(PO_SPHINXOPTS)
SET(PO_ALLSPHINXOPTS ${PO_SPHINXOPTS} update -p potfiles)

SALOME_GENERATE_ENVIRONMENT_SCRIPT(_cmd_sphinx env_script_sphinx "${SPHINX_EXECUTABLE}" "${ALLSPHINXOPTS}")
SALOME_GENERATE_ENVIRONMENT_SCRIPT(_cmd_sphinx_fr env_script_sphinx "${SPHINX_EXECUTABLE}" "${FR_ALLSPHINXOPTS}")
SALOME_GENERATE_ENVIRONMENT_SCRIPT(_cmd_sphinx_pot env_script_sphinx "${SPHINX_EXECUTABLE}" "${POT_ALLSPHINXOPTS}")
SALOME_GENERATE_ENVIRONMENT_SCRIPT(_cmd_sphinx_intl_build env_script_sphinx "${SPHINX_INTL_EXECUTABLE}" "build")
SALOME_GENERATE_ENVIRONMENT_SCRIPT(_cmd_sphinx_intl_update env_script_sphinx "${SPHINX_INTL_EXECUTABLE}" "${PO_ALLSPHINXOPTS}")

ADD_CUSTOM_TARGET(usr_docs ${CMAKE_COMMAND} -E make_directory tmp1
  
  # Make temporary directory for python modules 
  COMMAND ${CMAKE_COMMAND} -E make_directory tmp2

  # Copy python modules to be documented into memporary directory
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/src/SMESH_SWIG/smeshBuilder.py tmp2/smeshBuilder.py
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/src/SMESH_SWIG/smesh_algorithm.py tmp2/smesh_algorithm.py
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/src/SMESH_SWIG/StdMeshersBuilder.py ${CMAKE_CURRENT_BINARY_DIR}/tmp2/StdMeshersBuilder.py
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/src/SMESH_PY/smeshstudytools.py ${CMAKE_CURRENT_BINARY_DIR}/tmp2

  # collect methods from StdMeshers
  COMMAND ${_cmd}

  # Internatiolization commands:
  # 1. Copy existing po files
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/locale ${CMAKE_CURRENT_BINARY_DIR}/locale
  # 2. sphinx-intl "build"
  COMMAND ${_cmd_sphinx_intl_build}

  # Generate EN documentation
  COMMAND ${_cmd_sphinx}

  # Generate FR documentation
  COMMAND ${_cmd_sphinx_fr}

  COMMAND ${CMAKE_COMMAND} -E remove_directory tmp1
  COMMAND ${CMAKE_COMMAND} -E remove_directory tmp2
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

FILE(GLOB POFILES ${CMAKE_CURRENT_BINARY_DIR}/locale/fr/LC_MESSAGES/*.po)

ADD_CUSTOM_TARGET(check_translations 
  # 1. Generate potfiles
  COMMAND ${_cmd_sphinx_pot}
  # 2. Copy existing po files
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/locale ${CMAKE_CURRENT_BINARY_DIR}/locale
  # 3. Update PO files
  COMMAND ${_cmd_sphinx_intl_update}    
)

ADD_DEPENDENCIES(check_translations usr_docs)

FOREACH(pofile ${POFILES})
  GET_FILENAME_COMPONENT(fn_wo_path ${pofile} NAME)	       
  ADD_CUSTOM_COMMAND(TARGET check_translations POST_BUILD
                     COMMAND ${CMAKE_COMMAND} -E
                     copy_if_different ${pofile} ${CMAKE_CURRENT_SOURCE_DIR}/locale/fr/LC_MESSAGES/${fn_wo_path})
ENDFOREACH()

INSTALL(CODE "EXECUTE_PROCESS(COMMAND \"${CMAKE_COMMAND}\" --build ${PROJECT_BINARY_DIR} --target usr_docs)")
INSTALL(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/SMESH DESTINATION ${SALOME_INSTALL_DOC}/gui)
INSTALL(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/SMESH_fr DESTINATION ${SALOME_INSTALL_DOC}/gui)

SET(make_clean_files SMESH ${tag_files} SMESH SMESH_fr doctrees potfiles locale)
SET_DIRECTORY_PROPERTIES(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "${make_clean_files}")
