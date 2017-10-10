PROJECT(Burza)

cmake_minimum_required(VERSION 2.8)

set(PROJECT_ROOT  ${PROJECT_SOURCE_DIR}/..)

set(CMAKE_MODULE_PATH  ${PROJECT_SOURCE_DIR})
MESSAGE("CMAKE_MODULE_PATH: ${CMAKE_MODULE_PATH}")

set(Burza_VERSION_MAJOR 1)
set(Burza_VERSION_MINOR 03)
ADD_DEFINITIONS(-D_UNIX)
ADD_DEFINITIONS(-std=c++11)

#get source files in the subdirectories instead of writing them all by hand.
AUX_SOURCE_DIRECTORY(${PROJECT_ROOT}/kdev_src KDEV_SRC_FILES)
AUX_SOURCE_DIRECTORY(${PROJECT_ROOT}/src SRC_FILES)
AUX_SOURCE_DIRECTORY(${PROJECT_ROOT}/src/irrxml SRC_IRRXML_FILES)
AUX_SOURCE_DIRECTORY(${PROJECT_ROOT}/src/BasicExcelFormat SRC_BasicExcel_FILES)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_ROOT}/run_linux)

ADD_EXECUTABLE(burza ${PROJECT_ROOT}/src/burza.cpp 
${KDEV_SRC_FILES}
${SRC_FILES}
${SRC_IRRXML_FILES}
${SRC_BasicExcel_FILES}
)

include_directories(
  ${PROJECT_ROOT}/src; 
  ${PROJECT_ROOT}/src/irrxml
  ${PROJECT_ROOT}/src/BasicExcelFormat
  ${PROJECT_ROOT}/kdev_src
 )

find_package(PkgConfig)
PKG_CHECK_MODULES(GTKMM REQUIRED gtkmm-2.4)
MESSAGE("gtkmm found? ${GTKMM_FOUND}")
PKG_CHECK_MODULES(GLADEMM REQUIRED libglademm-2.4)
MESSAGE("Glade found? ${GLADEMM_FOUND}")
PKG_CHECK_MODULES(ICU REQUIRED icu-io icu-i18n icu-uc)
MESSAGE("ICU found? ${ICU_FOUND}")
PKG_CHECK_MODULES(PQXX REQUIRED libpqxx)
MESSAGE("PQXX found? ${PQXX_FOUND}")

set(PRJ_LIB_DIR "${PROJECT_SOURCE_DIR}/../lib")

include_directories(	
  ${GLADEMM_INCLUDE_DIRS}
  ${GTKMM_INCLUDE_DIRS}
  ${ICU_INCLUDE_DIRS}
  ${PQXX_INCLUDE_DIRS}
  "${PRJ_LIB_DIR}/libharu/include"
  )

link_directories( 
  ${GTKMM_LIBRARY_DIRS} 
  ${GLADEMM_LIBRARY_DIRS}
  ${ICU_LIBRARY_DIRS}
  ${PQXX_LIBRARY_DIRS}
  
  )
  
target_link_libraries(burza

  ${GTKMM_LIBRARIES}
  ${GLADEMM_LIBRARIES}
  ${ICU_LIBRARIES}
  ${PQXX_LIBRARIES}

# this is here just because automated thing does not work. Remove after it starts to behave correctly.
  -licuio -licui18n -licuuc -licudata
  
# deflate of zlib is missing for hpdf
  -lz

  # std threads c++11
  -lpthread
)

if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
  target_link_libraries(burza "${PRJ_LIB_DIR}/libharu/lib/libhpdfs.a")
else ( CMAKE_SIZEOF_VOID_P EQUAL 8 )
  target_link_libraries(burza "${PRJ_LIB_DIR}/libharu/lib/libhpdfs64.a")
endif( CMAKE_SIZEOF_VOID_P EQUAL 8 )

# How to dynamically link libraries from specified path:
# You can use a special keyword $ORIGIN to say ‘relative to the actual location of the executable’. 
# Program will then dynamically load libraries from specified subfolder
SET( GCC_LINK_FLAGS_liborigin "-Wl,-rpath=$ORIGIN/lib -Wl,-Bdynamic")
SET( CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${GCC_LINK_FLAGS_liborigin}" )
