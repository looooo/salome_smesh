# Copyright (C) 2007-2020  CEA/DEN, EDF R&D, OPEN CASCADE
#
# Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
# CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
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

# For 'make test'

SALOME_GENERATE_TESTS_ENVIRONMENT(_test_env)
SET(TEST_HELPER ${KERNEL_ROOT_DIR}/bin/salome/test_helper.py)

FOREACH(test ${GOOD_TESTS})
  GET_FILENAME_COMPONENT(testname ${test} NAME_WE)
  ADD_TEST(NAME ${testname}
           COMMAND ${PYTHON_EXECUTABLE} -B ${TEST_HELPER} ${CMAKE_CURRENT_SOURCE_DIR}/${test})
  SET_TESTS_PROPERTIES(${testname} PROPERTIES ENVIRONMENT "${tests_env}")
ENDFOREACH()

# Application tests

SET(TEST_INSTALL_DIRECTORY ${SALOME_INSTALL_SCRIPT_SCRIPTS}/test/tests)

INSTALL(FILES CTestTestfileInstall.cmake
        DESTINATION ${TEST_INSTALL_DIRECTORY}
        RENAME CTestTestfile.cmake)

INSTALL(FILES tests.set DESTINATION ${TEST_INSTALL_DIRECTORY})

# WARNING! DO NOT INSTALL RESTRICTED_TESTS HERE!!!!
SET(ALL_TESTS ${BAD_TESTS} ${GOOD_TESTS})
SALOME_INSTALL_SCRIPTS("${ALL_TESTS}" ${TEST_INSTALL_DIRECTORY})
