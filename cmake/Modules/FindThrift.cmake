# Copyright (c) 2008-2025, Hazelcast, Inc. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

#[=======================================================================[.rst:
FindThrift
-------

Finds the Apache Thrift library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``thrift::thrift``
  The Apache Thrift library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``Thrift_FOUND``
  True if the system has the Thrift library.
``Thrift_VERSION``
  The version of the Thrift library which was found.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``Thrift_INCLUDE_DIR``
  The directory containing ``thrift/Thrift.h``.
``Thrift_LIBRARY``
  The path to the Thrift library.

#]=======================================================================]

include(FindPackageHandleStandardArgs)

find_package(Thrift QUIET NO_MODULE)

if (Thrift_FOUND)
    find_package_handle_standard_args(Thrift CONFIG_MODE)

    if(TARGET thrift::thrift)
        # The official ThriftConfig.cmake does not set INTERFACE_INCLUDE_DIRECTORIES.
        # THRIFT_INCLUDE_DIR points to the inner .../include/thrift directory;
        # we need its parent so consumers can write #include <thrift/Thrift.h>.
        get_target_property(_thrift_incs thrift::thrift INTERFACE_INCLUDE_DIRECTORIES)
        if(NOT _thrift_incs)
            get_filename_component(_thrift_include_parent "${THRIFT_INCLUDE_DIR}" DIRECTORY)
            set_target_properties(thrift::thrift PROPERTIES
                INTERFACE_INCLUDE_DIRECTORIES "${_thrift_include_parent}")
        endif()
        unset(_thrift_incs)
        unset(_thrift_include_parent)

        # thrift_export.h defaults to __declspec(dllimport) on MSVC unless
        # THRIFT_STATIC_DEFINE is set. Without it, the compiler emits __imp_*
        # references that a static library cannot satisfy (LNK2019).
        get_target_property(_thrift_type thrift::thrift TYPE)
        if(_thrift_type STREQUAL "STATIC_LIBRARY")
            set_property(TARGET thrift::thrift APPEND PROPERTY
                INTERFACE_COMPILE_DEFINITIONS "THRIFT_STATIC_DEFINE")
        endif()
        unset(_thrift_type)
    endif()

    return()
endif()

find_path(
    Thrift_INCLUDE_DIR
    NAMES thrift/Thrift.h
    PATH_SUFFIXES include
)

find_library(
    Thrift_LIBRARY
    NAMES thrift
)

if (Thrift_INCLUDE_DIR)
    # extract library version from thrift/config.h
    file(
        STRINGS ${Thrift_INCLUDE_DIR}/thrift/config.h
        _Thrift_config_version_line
        REGEX "#define PACKAGE_VERSION "
    )
    string(REGEX MATCH "[0-9\\.]+" Thrift_VERSION "${_Thrift_config_version_line}")
    unset(_Thrift_config_version_line)
endif()

find_package_handle_standard_args(
    Thrift
    FOUND_VAR Thrift_FOUND
    REQUIRED_VARS
        Thrift_LIBRARY
        Thrift_INCLUDE_DIR
    VERSION_VAR Thrift_VERSION
)

if(Thrift_FOUND AND NOT TARGET thrift::thrift)
    add_library(thrift::thrift UNKNOWN IMPORTED)
    set_target_properties(
        thrift::thrift PROPERTIES
        IMPORTED_LOCATION "${Thrift_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${Thrift_INCLUDE_DIR}"
        INTERFACE_COMPILE_DEFINITIONS "THRIFT_STATIC_DEFINE"
    )
endif()

mark_as_advanced(
    Thrift_INCLUDE_DIR
    Thrift_LIBRARY
)
