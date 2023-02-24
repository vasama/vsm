# Install script for directory: D:/Code/vsm/core

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "D:/Code/vsm/core")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "D:/Code/vsm/core/build/Debug/vsm_core.lib")
  elseif(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "D:/Code/vsm/core/build/Release/vsm_core.lib")
  elseif(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "D:/Code/vsm/core/build/MinSizeRel/vsm_core.lib")
  elseif(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "D:/Code/vsm/core/build/RelWithDebInfo/vsm_core.lib")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/vsm" TYPE FILE FILES
      "D:/Code/vsm/core/include/vsm/assert.h"
      "D:/Code/vsm/core/include/vsm/concepts.hpp"
      "D:/Code/vsm/core/include/vsm/lift.hpp"
      "D:/Code/vsm/core/include/vsm/platform.h"
      "D:/Code/vsm/core/include/vsm/preprocessor.h"
      "D:/Code/vsm/core/include/vsm/standard.hpp"
      "D:/Code/vsm/core/include/vsm/tag_invoke.hpp"
      "D:/Code/vsm/core/include/vsm/type_traits.hpp"
      "D:/Code/vsm/core/include/vsm/utility.hpp"
      )
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/vsm/detail/platform" TYPE FILE FILES
      "D:/Code/vsm/core/include/vsm/detail/platform/arch.h"
      "D:/Code/vsm/core/include/vsm/detail/platform/clang.h"
      "D:/Code/vsm/core/include/vsm/detail/platform/compiler.h"
      "D:/Code/vsm/core/include/vsm/detail/platform/gcc.h"
      "D:/Code/vsm/core/include/vsm/detail/platform/gnu.h"
      "D:/Code/vsm/core/include/vsm/detail/platform/linux.h"
      "D:/Code/vsm/core/include/vsm/detail/platform/msvc.h"
      "D:/Code/vsm/core/include/vsm/detail/platform/os.h"
      "D:/Code/vsm/core/include/vsm/detail/platform/win32.h"
      "D:/Code/vsm/core/include/vsm/detail/platform/x86.h"
      "D:/Code/vsm/core/include/vsm/detail/platform/x86_32.h"
      "D:/Code/vsm/core/include/vsm/detail/platform/x86_64.h"
      )
  elseif(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/vsm" TYPE FILE FILES
      "D:/Code/vsm/core/include/vsm/assert.h"
      "D:/Code/vsm/core/include/vsm/concepts.hpp"
      "D:/Code/vsm/core/include/vsm/lift.hpp"
      "D:/Code/vsm/core/include/vsm/platform.h"
      "D:/Code/vsm/core/include/vsm/preprocessor.h"
      "D:/Code/vsm/core/include/vsm/standard.hpp"
      "D:/Code/vsm/core/include/vsm/tag_invoke.hpp"
      "D:/Code/vsm/core/include/vsm/type_traits.hpp"
      "D:/Code/vsm/core/include/vsm/utility.hpp"
      )
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/vsm/detail/platform" TYPE FILE FILES
      "D:/Code/vsm/core/include/vsm/detail/platform/arch.h"
      "D:/Code/vsm/core/include/vsm/detail/platform/clang.h"
      "D:/Code/vsm/core/include/vsm/detail/platform/compiler.h"
      "D:/Code/vsm/core/include/vsm/detail/platform/gcc.h"
      "D:/Code/vsm/core/include/vsm/detail/platform/gnu.h"
      "D:/Code/vsm/core/include/vsm/detail/platform/linux.h"
      "D:/Code/vsm/core/include/vsm/detail/platform/msvc.h"
      "D:/Code/vsm/core/include/vsm/detail/platform/os.h"
      "D:/Code/vsm/core/include/vsm/detail/platform/win32.h"
      "D:/Code/vsm/core/include/vsm/detail/platform/x86.h"
      "D:/Code/vsm/core/include/vsm/detail/platform/x86_32.h"
      "D:/Code/vsm/core/include/vsm/detail/platform/x86_64.h"
      )
  elseif(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/vsm" TYPE FILE FILES
      "D:/Code/vsm/core/include/vsm/assert.h"
      "D:/Code/vsm/core/include/vsm/concepts.hpp"
      "D:/Code/vsm/core/include/vsm/lift.hpp"
      "D:/Code/vsm/core/include/vsm/platform.h"
      "D:/Code/vsm/core/include/vsm/preprocessor.h"
      "D:/Code/vsm/core/include/vsm/standard.hpp"
      "D:/Code/vsm/core/include/vsm/tag_invoke.hpp"
      "D:/Code/vsm/core/include/vsm/type_traits.hpp"
      "D:/Code/vsm/core/include/vsm/utility.hpp"
      )
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/vsm/detail/platform" TYPE FILE FILES
      "D:/Code/vsm/core/include/vsm/detail/platform/arch.h"
      "D:/Code/vsm/core/include/vsm/detail/platform/clang.h"
      "D:/Code/vsm/core/include/vsm/detail/platform/compiler.h"
      "D:/Code/vsm/core/include/vsm/detail/platform/gcc.h"
      "D:/Code/vsm/core/include/vsm/detail/platform/gnu.h"
      "D:/Code/vsm/core/include/vsm/detail/platform/linux.h"
      "D:/Code/vsm/core/include/vsm/detail/platform/msvc.h"
      "D:/Code/vsm/core/include/vsm/detail/platform/os.h"
      "D:/Code/vsm/core/include/vsm/detail/platform/win32.h"
      "D:/Code/vsm/core/include/vsm/detail/platform/x86.h"
      "D:/Code/vsm/core/include/vsm/detail/platform/x86_32.h"
      "D:/Code/vsm/core/include/vsm/detail/platform/x86_64.h"
      )
  elseif(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/vsm" TYPE FILE FILES
      "D:/Code/vsm/core/include/vsm/assert.h"
      "D:/Code/vsm/core/include/vsm/concepts.hpp"
      "D:/Code/vsm/core/include/vsm/lift.hpp"
      "D:/Code/vsm/core/include/vsm/platform.h"
      "D:/Code/vsm/core/include/vsm/preprocessor.h"
      "D:/Code/vsm/core/include/vsm/standard.hpp"
      "D:/Code/vsm/core/include/vsm/tag_invoke.hpp"
      "D:/Code/vsm/core/include/vsm/type_traits.hpp"
      "D:/Code/vsm/core/include/vsm/utility.hpp"
      )
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/vsm/detail/platform" TYPE FILE FILES
      "D:/Code/vsm/core/include/vsm/detail/platform/arch.h"
      "D:/Code/vsm/core/include/vsm/detail/platform/clang.h"
      "D:/Code/vsm/core/include/vsm/detail/platform/compiler.h"
      "D:/Code/vsm/core/include/vsm/detail/platform/gcc.h"
      "D:/Code/vsm/core/include/vsm/detail/platform/gnu.h"
      "D:/Code/vsm/core/include/vsm/detail/platform/linux.h"
      "D:/Code/vsm/core/include/vsm/detail/platform/msvc.h"
      "D:/Code/vsm/core/include/vsm/detail/platform/os.h"
      "D:/Code/vsm/core/include/vsm/detail/platform/win32.h"
      "D:/Code/vsm/core/include/vsm/detail/platform/x86.h"
      "D:/Code/vsm/core/include/vsm/detail/platform/x86_32.h"
      "D:/Code/vsm/core/include/vsm/detail/platform/x86_64.h"
      )
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
file(WRITE "D:/Code/vsm/core/build/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
