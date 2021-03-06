#
#  See README.md;
#

PROJECT(ETM)

# Install under /usr/local per default;
#IF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
#  SET(CMAKE_INSTALL_PREFIX "/usr/local" CACHE PATH "Installation directory" FORCE)
#ENDIF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

# Yes, help my buggy assert() statements; FIXME: do it better later;
IF( NOT CMAKE_BUILD_TYPE )
   SET( CMAKE_BUILD_TYPE Debug)
ENDIF()

# Should suffice?;
cmake_minimum_required(VERSION 3.0.0)

# Assume ROOTSYS was set up in the environment;
list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})

if(NOT DEFINED CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 11)
endif()

# Find ROOT;
FIND_PACKAGE(ROOT 6.14.0 REQUIRED COMPONENTS Core Graf Graf3d)
#---Define useful ROOT functions and macros (e.g. ROOT_GENERATE_DICTIONARY)
include(${ROOT_USE_FILE})
#include("${ROOT_DIR}/modules/RootNewMacros.cmake")

SET(ignoreMe "${GEANT}")
if(DEFINED GEANT)
  find_package(Geant4 REQUIRED ui_all vis_all)
  include(${Geant4_USE_FILE})
endif()
SET( CMAKE_CXX_FLAGS "-g -Wall -Wno-pedantic -Wno-format-security \
                      -Wno-reorder -Wno-sign-compare -Wno-narrowing" )
if(DEFINED GEANT)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_ETM2GEANT_")
endif()
if(DEFINED OPENCASCADE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_OPENCASCADE_")

  INCLUDE_DIRECTORIES ( 
    ${OPENCASCADE}/include/opencascade
  )
endif()
if(DEFINED BFIELD)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_BFIELD_")

  include_directories( ${BFIELD}/include )
endif()
if(DEFINED VGM)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_VGM_")

  include_directories( ${VGM}/include )
endif()

SET(LIBNAME etm)
SET(EXENAME exe)

SET( LIBSRC 
  source/eicroot/EicPOD.cc
  source/eicroot/EicDetName.cxx 
  source/eicroot/EicGeoMap.cxx 
  source/eicroot/EicNamePatternHub.cxx 
  source/eicroot/EicGeoParData.cxx 

  source/eicroot/EicGeoMedium.cxx
  source/eicroot/EicGeoMedia.cxx
  source/eicroot/GemGeoParData.cxx 
  source/eicroot/MuMegasGeoParData.cxx 
  source/eicroot/MapsGeoParData.cxx 
  source/eicroot/MapsMimosaAssembly.cxx 
  source/eicroot/FstGeoParData.cxx 
  source/eicroot/VstGeoParData.cxx 

  source/EtmOrphans.cc 
  source/EtmLine2D.cc 
  source/EtmEtaBoundary.cc 
  source/EtmPolygonGroup.cc 
  source/EtmPalette.cc 
  source/EtmVacuumChamber.cc 
  source/EtmAzimuthalScan.cc 
  source/EtmDetector.cc 
  source/EtmDetectorStack.cc 

  source/EicToyModel.cc 

  source/vc2020_03_20.cc 
)

INCLUDE_DIRECTORIES ( 
  ${ROOT_INCLUDE_DIR}

  ${CMAKE_SOURCE_DIR}/include
)

SET( HEADERS 
  # This will include all other ones; fine;
  ${CMAKE_SOURCE_DIR}/include/EicToyModel.h 
  ${CMAKE_SOURCE_DIR}/include/vc2020_03_20.h

  # FIXME: do it better later;
  ${CMAKE_SOURCE_DIR}/include/EicPOD.h

  ${CMAKE_SOURCE_DIR}/include/EicGeoMedium.h
  ${CMAKE_SOURCE_DIR}/include/EicGeoMedia.h
  ${CMAKE_SOURCE_DIR}/include/EicDetName.h
  ${CMAKE_SOURCE_DIR}/include/EicGeoMap.h
  ${CMAKE_SOURCE_DIR}/include/EicNamePatternHub.h
  ${CMAKE_SOURCE_DIR}/include/EicGeoParData.h
  ${CMAKE_SOURCE_DIR}/include/GemGeoParData.h
  ${CMAKE_SOURCE_DIR}/include/MuMegasGeoParData.h
  ${CMAKE_SOURCE_DIR}/include/MapsGeoParData.h
  ${CMAKE_SOURCE_DIR}/include/MapsMimosaAssembly.h
  ${CMAKE_SOURCE_DIR}/include/VstGeoParData.h
  ${CMAKE_SOURCE_DIR}/include/FstGeoParData.h
)

ROOT_GENERATE_DICTIONARY(G__${LIBNAME} ${HEADERS} LINKDEF include/etmLinkDef.h)
SET( LIBSRC ${LIBSRC} G__${LIBNAME}.cxx )

ADD_LIBRARY( ${LIBNAME} SHARED ${LIBSRC} ${HEADERS} )
TARGET_LINK_LIBRARIES( ${LIBNAME} ${ROOT_LIBRARIES} Geom)
if(DEFINED BFIELD)
  TARGET_LINK_LIBRARIES( ${LIBNAME} ${BFIELD}/lib/libbmf.so)
endif()
if(DEFINED OPENCASCADE)
  TARGET_LINK_LIBRARIES( ${LIBNAME} -L${OPENCASCADE}/lib -lTKSTEP -lTKIGES -lTKXDEIGES -lTKXDESTEP -lTKSTL -lTKPrim )
endif()
if(DEFINED GEANT)
  TARGET_LINK_LIBRARIES( ${LIBNAME} ${Geant4_LIBRARIES} )
endif()
if(DEFINED VGM)
  TARGET_LINK_LIBRARIES( ${LIBNAME} -L${VGM}/lib -L${VGM}/lib64 -lRootGM -lGeant4GM )
endif()

INSTALL(DIRECTORY ${CMAKE_SOURCE_DIR}/include/ DESTINATION include/etm PATTERN "e*.h" EXCLUDE)
INSTALL(TARGETS   ${LIBNAME}                   DESTINATION lib)
INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/lib${LIBNAME}.rootmap ${CMAKE_CURRENT_BINARY_DIR}/lib${LIBNAME}_rdict.pcm DESTINATION lib)
