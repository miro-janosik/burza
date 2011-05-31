PROJECT(Burza)

cmake_minimum_required(VERSION 2.8)
set(CMAKE_MODULE_PATH  ${PROJECT_SOURCE_DIR})
MESSAGE("CMAKE_MODULE_PATH: ${CMAKE_MODULE_PATH}")

set(Burza_VERSION_MAJOR 0)
set(Burza_VERSION_MINOR 5)
ADD_DEFINITIONS(-D_UNIX)

#get source files in the subdirectories instead of writing them all by hand.
AUX_SOURCE_DIRECTORY(kdev_src KDEV_SRC_FILES)
AUX_SOURCE_DIRECTORY(src SRC_FILES)

ADD_EXECUTABLE(burza_kdev 
${KDEV_SRC_FILES}
${SRC_FILES}
)

include_directories(src; kdev_src)

find_package(PkgConfig)
pkg_check_modules(GTKMM REQUIRED gtkmm-2.4)
pkg_check_modules(GLADEMM REQUIRED libglademm-2.4)

MESSAGE("gtkmm: ${GTKMM_FOUND}, ${GTKMM_INCLUDE_DIRS} , ${GTKMM_LIBRARY_DIRS}.")
MESSAGE("Glade: ${GLADEMM_FOUND}, ${GLADEMM_LIBRARY_DIRS} , ${GLADEMM_INCLUDE_DIRS} , ${GLADEMM_LIBRARIES} .")

set(PRJ_LIB_DIR "${PROJECT_SOURCE_DIR}/../lib")

include_directories(
  ${GLADEMM_INCLUDE_DIRS}
  ${GTKMM_INCLUDE_DIRS}
  "${PRJ_LIB_DIR}/libharu/include"
  "${PRJ_LIB_DIR}/libXL/include"
  )

link_directories( 
  ${GTKMM_LIBRARY_DIRS} 
  ${GLADEMM_LIBRARY_DIRS}
  ${PRJ_LIB_DIR}"/libXL"
  )

target_link_libraries(burza_kdev 
  ${GTKMM_LIBRARIES}
  ${GLADEMM_LIBRARIES}
  "${PRJ_LIB_DIR}/libharu/lib/libhpdf.a"
  "${PRJ_LIB_DIR}/libXL/libxl.so"
  )
