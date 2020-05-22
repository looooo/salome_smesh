# Copyright (C) 2012-2020  CEA/DEN, EDF R&D, OPEN CASCADE
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

INCLUDE(tests.set)

SALOME_CONFIGURE_FILE(tests.py.in tests.py)

# Install examples

SET(EXAMPLES_TESTS ${BAD_TESTS} ${GOOD_TESTS})
SALOME_INSTALL_SCRIPTS("${EXAMPLES_TESTS}" ${SALOME_INSTALL_DOC}/examples/SMESH)

# Tests

IF(SALOME_BUILD_TESTS)

  SET(TEST_REINIT_SALOME "False")
  SET(TEST_HELPER ${KERNEL_ROOT_DIR}/bin/salome/test_helper.py)

  SALOME_GENERATE_TESTS_ENVIRONMENT(tests_env)

  IF(SMESH_JOIN_TESTS)
    ADD_TEST(NAME all_examples COMMAND ${PYTHON_EXECUTABLE} -B ${TEST_HELPER} tests.py)
  ELSE(SMESH_JOIN_TESTS)
    FOREACH(test ${GOOD_TESTS})
      GET_FILENAME_COMPONENT(testname ${test} NAME_WE)
      ADD_TEST(NAME ${testname}
               COMMAND ${PYTHON_EXECUTABLE} -B ${TEST_HELPER} ${CMAKE_CURRENT_SOURCE_DIR}/${test})
      SET_TESTS_PROPERTIES(${testname} PROPERTIES ENVIRONMENT "${tests_env}")
    ENDFOREACH()
  ENDIF(SMESH_JOIN_TESTS)

  # Application tests
  # Note: we don't install test scripts as they are already installed to the documentation folder

  SET(TEST_INSTALL_DIRECTORY ${SALOME_INSTALL_SCRIPT_SCRIPTS}/test/examples)

  CONFIGURE_FILE(CTestTestfileInstall.cmake ${CMAKE_CURRENT_BINARY_DIR}/CTestTestfileInstall.cmake @ONLY)
  INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/CTestTestfileInstall.cmake
          DESTINATION ${TEST_INSTALL_DIRECTORY}
          RENAME CTestTestfile.cmake)
  INSTALL(FILES tests.set DESTINATION ${TEST_INSTALL_DIRECTORY})

ENDIF(SALOME_BUILD_TESTS)
