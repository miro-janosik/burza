PROJECT(Burza)

cmake_minimum_required(VERSION 2.8)

set(PROJECT_ROOT  ${PROJECT_SOURCE_DIR}/..)

set(CMAKE_MODULE_PATH  ${PROJECT_SOURCE_DIR})
MESSAGE("CMAKE_MODULE_PATH: ${CMAKE_MODULE_PATH}")

set(Burza_VERSION_MAJOR 1)
set(Burza_VERSION_MINOR 02)
ADD_DEFINITIONS(-D_UNIX)

OPTION( DOES_NOTHING "This option does nothing, it is as hint." ON )

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
pkg_check_modules(GTKMM REQUIRED gtkmm-2.4)
pkg_check_modules(GLADEMM REQUIRED libglademm-2.4)
# pkg_check_modules(PQXX REQUIRED pqxx)

MESSAGE("gtkmm: ${GTKMM_FOUND}, ${GTKMM_INCLUDE_DIRS} , ${GTKMM_LIBRARY_DIRS}.")
MESSAGE("Glade: ${GLADEMM_FOUND}, ${GLADEMM_LIBRARY_DIRS} , ${GLADEMM_INCLUDE_DIRS} , ${GLADEMM_LIBRARIES} .")
set(PQXX_LIBRARY_DIRS "/usr/lib")
set(PQXX_INCLUDE_DIRS "/usr/include")
set(PQXX_LIBRARIES "libpqxx.so")

set(PRJ_LIB_DIR "${PROJECT_SOURCE_DIR}/../lib")

# -Wl,-Bstatic -l/usr/lib/libpqxx.so 
SET( GCC_LINK_FLAGS_liborigin "-Wl,-rpath=$ORIGIN/lib -Wl,-Bdynamic")
SET( CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${GCC_LINK_FLAGS_liborigin}" )

include_directories(	
  ${GLADEMM_INCLUDE_DIRS}
  ${GTKMM_INCLUDE_DIRS}
  ${PQXX_INCLUDE_DIRS}
  "${PRJ_LIB_DIR}/libharu/include"
#  "${PRJ_LIB_DIR}/libXL/include"
  )

link_directories( 
  ${GTKMM_LIBRARY_DIRS} 
  ${GLADEMM_LIBRARY_DIRS}
  ${PQXX_LIBRARY_DIRS}
#  ${PRJ_LIB_DIR}"/libXL"
  )

target_link_libraries(burza
  ${GTKMM_LIBRARIES}
  ${GLADEMM_LIBRARIES}
  ${PQXX_LIBRARIES}
  "${PRJ_LIB_DIR}/libharu/lib/libhpdf.a"
#  "${PRJ_LIB_DIR}/libXL/libxl.so"
  )
