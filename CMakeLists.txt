# Copyright 2013-present Barefoot Networks, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Makefile for a fake backend that is used for merging two P4_16 files

set (P4MERGE_SRCS
  p4merge.cpp
  mergeManager.cpp
  )
set (P4MERGE_HDRS
  midend.h
  mergeManager.h
  )
#set (P4MERGE_FRONTEND_SRCS
#  ${CMAKE_CURRENT_SOURCE_DIR}/frontends/mfrontend.cpp
#  ${CMAKE_CURRENT_SOURCE_DIR}/frontends/mfrontend.h
#  )


add_cpplint_files (${CMAKE_CURRENT_SOURCE_DIR} "${P4MERGE_SRCS};${P4MERGE_HDRS}")
#set(EXTENSION_FRONTEND_SOURCES ${EXTENSION_FRONTEND_SOURCES} ${P4MERGE_FRONTEND_SRCS} PARENT_SCOPE)

build_unified(P4MERGE_SRCS ALL)
add_executable(p4merge ${P4MERGE_SRCS} ${EXTENSION_P4_14_CONV_SOURCES})
target_link_libraries (p4merge ${P4C_LIBRARIES} ${P4C_LIB_DEPS})
add_dependencies(p4merge genIR)

install (TARGETS p4merge
  RUNTIME DESTINATION ${P4C_RUNTIME_OUTPUT_DIRECTORY})
#install (DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/p4include
#  DESTINATION ${P4C_ARTIFACTS_OUTPUT_DIRECTORY})
#install (FILES ${CMAKE_CURRENT_SOURCE_DIR}/driver/p4c.p4merge.cfg
#  DESTINATION ${P4C_ARTIFACTS_OUTPUT_DIRECTORY}/p4c_src)

# hack to get around the fact that the test scripts expect the backend
# binary to be in the top level directory. This should go away when we
# remove automake and fix the scripts.
add_custom_target(linkp4merge
  COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_CURRENT_BINARY_DIR}/p4merge ${P4C_BINARY_DIR}/p4merge
  COMMAND ${CMAKE_COMMAND} -E create_symlink ${P4C_BINARY_DIR}/p4include ${CMAKE_CURRENT_BINARY_DIR}/p4include
  COMMAND ${CMAKE_COMMAND} -E create_symlink ${P4C_BINARY_DIR}/p4_14include ${CMAKE_CURRENT_BINARY_DIR}/p4_14include
  )
add_dependencies(p4c_driver linkp4merge)

# Tests

#set(P4TEST_DRIVER ${P4C_SOURCE_DIR}/backends/p4test/run-p4-sample.py)


