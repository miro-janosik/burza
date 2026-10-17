# =============================================================================
#  Burza - build configuration
#
#  Program: Burza
#  Autor: Miro Janosik
#  Vytvorene pre MC Srdiecko Trencin
#
#  Usage (out-of-source, as before):
#      ./configure ; cd build ; make
#  or directly:
#      cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug && cmake --build build
#
#  Platforms: Linux and Windows.
#  The UI is gtkmm 4.0 loaded from a Cambalache .ui file through Gtk::Builder.
#  libglademm is no longer used.
#
#  NOTE ON OLD TARGETS  (please read before chasing a failed build)
#  ----------------------------------------------------------------
#  This file deliberately stays within CMake 3.10 syntax so that it still
#  configures on Ubuntu 18.04 LTS (which ships cmake 3.10.2) and on old MSVC
#  generators. No command newer than 3.10 is used; newer features are enabled
#  only behind version guards.
#
#  That keeps the *build system* usable on those machines. It cannot, however,
#  make the *program* build there any more, for reasons outside this file:
#
#    * gtkmm 4.0 requires GTK 4, which requires glib >= 2.66. Ubuntu 18.04 has
#      glib 2.56 and no GTK 4 package at all. The oldest Ubuntu that can build
#      this program after the gtkmm4 port is 22.04. For 18.04 you would need
#      the last gtkmm-2.4 commit (before the Cambalache branch).
#    * Windows XP cannot run GTK 4 - it needs APIs introduced in Vista, and the
#      MSVC v141_xp toolset caps out at C++17 while this code is C++20. Set
#      BURZA_CXX_STANDARD=17 and BURZA_TARGET_WINDOWS_XP=ON if you want to try
#      anyway, but expect GTK 4 itself to be the blocker, not this file.
#
#  Set BURZA_CXX_STANDARD to 17 (or 14) to build with an older compiler.
# =============================================================================

cmake_minimum_required(VERSION 3.10)

project(Burza VERSION 1.3 LANGUAGES CXX)

# Honour <PackageName>_ROOT variables when looking for dependencies (3.12+).
if(POLICY CMP0074)
    cmake_policy(SET CMP0074 NEW)
endif()
# Let the normal variable form of option() take precedence (3.13+).
if(POLICY CMP0077)
    cmake_policy(SET CMP0077 NEW)
endif()

set(PROJECT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/..")
get_filename_component(PROJECT_ROOT "${PROJECT_ROOT}" ABSOLUTE)

# -----------------------------------------------------------------------------
#  Options
# -----------------------------------------------------------------------------

# See compatibility.md: tier 2 needs 20 (std::format), tier 1 targets 17,
# tier 0 (the frozen gtkmm 2.4 build) is 11.
set(BURZA_CXX_STANDARD "20" CACHE STRING
    "C++ standard to build with (20, 17, 14 or 11). Lower it for old toolchains.")
set_property(CACHE BURZA_CXX_STANDARD PROPERTY STRINGS 11 14 17 20)

option(BURZA_WITH_POSTGRES
    "Build the PostgreSQL storage backend (needs libpqxx)" ON)
option(BURZA_USE_BUNDLED_LIBHARU
    "Use the libharu shipped in lib/libharu instead of a system one" OFF)
option(BURZA_WIN32_CONSOLE
    "Windows: keep a console window so the log is visible" ON)
option(BURZA_TARGET_WINDOWS_XP
    "Windows: target Windows XP (_WIN32_WINNT=0x0501, XP subsystem)" OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type" FORCE)
endif()

# Binaries land next to their data, as before.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_ROOT}/run_linux")
foreach(_cfg DEBUG RELEASE RELWITHDEBINFO MINSIZEREL)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${_cfg} "${PROJECT_ROOT}/run_linux")
endforeach()

# -----------------------------------------------------------------------------
#  Sources
# -----------------------------------------------------------------------------
# Globbed, as this project always did. On CMake 3.12+ the glob is re-checked on
# every build so a new file does not need a manual cmake re-run.

if(CMAKE_VERSION VERSION_LESS 3.12)
    set(_glob_mode "")
else()
    set(_glob_mode CONFIGURE_DEPENDS)
endif()

file(GLOB BURZA_SRC          ${_glob_mode} "${PROJECT_ROOT}/src/*.cpp")
file(GLOB BURZA_SRC_IRRXML   ${_glob_mode} "${PROJECT_ROOT}/src/irrxml/*.cpp")
file(GLOB BURZA_SRC_EXCEL    ${_glob_mode} "${PROJECT_ROOT}/src/BasicExcelFormat/*.cpp")

if(NOT BURZA_SRC)
    message(FATAL_ERROR
        "No sources found in ${PROJECT_ROOT}/src.\n"
        "burza_kdev_4/src is expected to be a symlink to ../src/ - if it was "
        "replaced by a real directory or deleted, restore it with:\n"
        "    ln -s ../src/ burza_kdev_4/src")
endif()

add_executable(burza
    ${BURZA_SRC}
    ${BURZA_SRC_IRRXML}
    ${BURZA_SRC_EXCEL}
)

target_include_directories(burza PRIVATE
    "${PROJECT_ROOT}/src"
    "${PROJECT_ROOT}/src/irrxml"
    "${PROJECT_ROOT}/src/BasicExcelFormat"
)

set_target_properties(burza PROPERTIES
    CXX_STANDARD          ${BURZA_CXX_STANDARD}
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS        OFF
)

# -----------------------------------------------------------------------------
#  Platform definitions
# -----------------------------------------------------------------------------
# convfncs.h / convfile.h branch on _UNIX vs _WIN32, so _UNIX must NOT be
# defined on Windows (the old build defined it unconditionally).

if(WIN32)
    target_compile_definitions(burza PRIVATE
        WIN32_LEAN_AND_MEAN
        NOMINMAX                    # keep std::min / std::max usable
    )
    if(BURZA_TARGET_WINDOWS_XP)
        target_compile_definitions(burza PRIVATE _WIN32_WINNT=0x0501)
        message(WARNING
            "BURZA_TARGET_WINDOWS_XP is ON. GTK 4 does not support Windows XP; "
            "this only sets the Windows API level and subsystem version.")
    else()
        target_compile_definitions(burza PRIVATE _WIN32_WINNT=0x0601)
    endif()
    if(MSVC)
        target_compile_definitions(burza PRIVATE _CRT_SECURE_NO_WARNINGS)
        # The sources are UTF-8 and contain Slovak text in string literals.
        if(NOT MSVC_VERSION LESS 1900)
            target_compile_options(burza PRIVATE /utf-8)
        endif()
    endif()
else()
    target_compile_definitions(burza PRIVATE _UNIX)
endif()

# -----------------------------------------------------------------------------
#  Dependencies
# -----------------------------------------------------------------------------

find_package(PkgConfig QUIET)
find_package(Threads REQUIRED)

# --- gtkmm 4.0 (required) ----------------------------------------------------
if(PKG_CONFIG_FOUND)
    pkg_check_modules(GTKMM REQUIRED gtkmm-4.0)
    message(STATUS "gtkmm: ${GTKMM_VERSION}")
    target_include_directories(burza SYSTEM PRIVATE ${GTKMM_INCLUDE_DIRS})
    # _LDFLAGS carries the -L paths together with the -l names, so no
    # link_directories() is needed (which would not apply to an already
    # created target anyway).
    target_link_libraries(burza PRIVATE ${GTKMM_LDFLAGS})
    target_compile_options(burza PRIVATE ${GTKMM_CFLAGS_OTHER})
else()
    message(FATAL_ERROR
        "pkg-config not found, and gtkmm 4.0 has no CMake package config.\n"
        "On Windows use MSYS2:\n"
        "    pacman -S mingw-w64-x86_64-gtkmm-4.0 mingw-w64-x86_64-pkgconf\n"
        "or vcpkg (which installs pkgconf and .pc files):\n"
        "    vcpkg install gtkmm:x64-windows\n"
        "then re-run cmake with that environment's pkg-config on PATH.")
endif()

# --- ICU (required) ----------------------------------------------------------
# find_package(ICU) exists since CMake 3.7 and works on Windows too; fall back
# to pkg-config, which is what the Linux build has always used.
set(_icu_handled FALSE)
if(NOT CMAKE_VERSION VERSION_LESS 3.7)
    find_package(ICU QUIET COMPONENTS uc i18n io data)
    if(ICU_FOUND)
        message(STATUS "ICU: ${ICU_VERSION} (find_package)")
        target_link_libraries(burza PRIVATE ICU::uc ICU::i18n ICU::io ICU::data)
        set(_icu_handled TRUE)
    endif()
endif()
if(NOT _icu_handled)
    if(NOT PKG_CONFIG_FOUND)
        message(FATAL_ERROR "ICU not found and pkg-config unavailable.")
    endif()
    pkg_check_modules(ICU_PC REQUIRED icu-io icu-i18n icu-uc)
    message(STATUS "ICU: ${ICU_PC_icu-uc_VERSION} (pkg-config)")
    target_include_directories(burza SYSTEM PRIVATE ${ICU_PC_INCLUDE_DIRS})
    # The old build listed -licuio -licui18n -licuuc -licudata by hand because
    # the pkg-config libraries "did not come through" - that was the missing
    # -L path. _LDFLAGS includes it, so the workaround is not needed.
    target_link_libraries(burza PRIVATE ${ICU_PC_LDFLAGS})
endif()

# --- libpqxx / PostgreSQL (optional) ----------------------------------------
if(BURZA_WITH_POSTGRES)
    set(_pqxx_handled FALSE)
    if(PKG_CONFIG_FOUND)
        pkg_check_modules(PQXX QUIET libpqxx)
        if(PQXX_FOUND)
            message(STATUS "libpqxx: ${PQXX_VERSION}")
            target_include_directories(burza SYSTEM PRIVATE ${PQXX_INCLUDE_DIRS})
            target_link_libraries(burza PRIVATE ${PQXX_LDFLAGS})
            set(_pqxx_handled TRUE)
        endif()
    endif()
    if(NOT _pqxx_handled)
        # Windows / MSVC: the prebuilt import libraries under lib/pqxx.
        find_path(PQXX_INCLUDE_DIR pqxx/pqxx
            HINTS "${PROJECT_ROOT}/lib/pqxx/include")
        find_library(PQXX_LIBRARY NAMES pqxx libpqxx
            HINTS "${PROJECT_ROOT}/lib/pqxx")
        find_library(PQ_LIBRARY NAMES pq libpq
            HINTS "${PROJECT_ROOT}/lib/pqxx")
        if(PQXX_INCLUDE_DIR AND PQXX_LIBRARY)
            message(STATUS "libpqxx: ${PQXX_LIBRARY} (bundled)")
            target_include_directories(burza SYSTEM PRIVATE "${PQXX_INCLUDE_DIR}")
            target_link_libraries(burza PRIVATE "${PQXX_LIBRARY}")
            if(PQ_LIBRARY)
                target_link_libraries(burza PRIVATE "${PQ_LIBRARY}")
            endif()
            if(WIN32)
                # Static libpqxx needs the socket libraries; see docs/install.txt.
                target_compile_definitions(burza PRIVATE PQXX_PQ_STATIC)
                target_link_libraries(burza PRIVATE ws2_32 wsock32)
            endif()
        else()
            message(FATAL_ERROR
                "libpqxx not found. Install libpqxx-dev, or configure with "
                "-DBURZA_WITH_POSTGRES=OFF to build with the text-file "
                "storage backend only.")
        endif()
    endif()
else()
    message(STATUS "libpqxx: disabled (BURZA_WITH_POSTGRES=OFF)")
endif()

# --- libharu / hpdf (required) ----------------------------------------------
# The bundled archives in lib/libharu/lib and lib/libharu/lib64 are BOTH
# x86-64; there is no 32-bit build in the tree. The old 'libhpdfs64.a'
# else-branch pointed at a file that never existed.

set(_hpdf_handled FALSE)

if(NOT BURZA_USE_BUNDLED_LIBHARU)
    if(PKG_CONFIG_FOUND)
        pkg_check_modules(HARU QUIET libharu)
    endif()
    if(HARU_FOUND)
        message(STATUS "libharu: ${HARU_VERSION} (system)")
        target_include_directories(burza SYSTEM PRIVATE ${HARU_INCLUDE_DIRS})
        target_link_libraries(burza PRIVATE ${HARU_LDFLAGS})
        set(_hpdf_handled TRUE)
    else()
        find_path(HPDF_INCLUDE_DIR hpdf.h)
        find_library(HPDF_LIBRARY NAMES hpdf)
        if(HPDF_INCLUDE_DIR AND HPDF_LIBRARY)
            message(STATUS "libharu: ${HPDF_LIBRARY} (system)")
            target_include_directories(burza SYSTEM PRIVATE "${HPDF_INCLUDE_DIR}")
            target_link_libraries(burza PRIVATE "${HPDF_LIBRARY}")
            set(_hpdf_handled TRUE)
        endif()
    endif()
endif()

if(NOT _hpdf_handled)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(_hpdf_dirs "${PROJECT_ROOT}/lib/libharu/lib64"
                       "${PROJECT_ROOT}/lib/libharu/lib")
    else()
        message(FATAL_ERROR
            "A 32-bit build was requested, but the libharu archives bundled in "
            "lib/libharu are x86-64 only. Install a 32-bit libharu, or build "
            "64-bit.")
    endif()

    find_library(HPDF_BUNDLED_LIBRARY
        NAMES hpdfs hpdf
        PATHS ${_hpdf_dirs}
        NO_DEFAULT_PATH)
    find_path(HPDF_BUNDLED_INCLUDE_DIR hpdf.h
        PATHS "${PROJECT_ROOT}/lib/libharu/include"
        NO_DEFAULT_PATH)

    if(NOT HPDF_BUNDLED_LIBRARY OR NOT HPDF_BUNDLED_INCLUDE_DIR)
        message(FATAL_ERROR
            "libharu not found, neither on the system nor in lib/libharu.\n"
            "Install libhpdf-dev, or unpack lib/libharu/libharu-RELEASE_2_3_0.zip "
            "and build it.")
    endif()

    message(STATUS "libharu: ${HPDF_BUNDLED_LIBRARY} (bundled)")
    target_include_directories(burza SYSTEM PRIVATE "${HPDF_BUNDLED_INCLUDE_DIR}")
    target_link_libraries(burza PRIVATE "${HPDF_BUNDLED_LIBRARY}")
endif()

# --- zlib (libharu's deflate) ------------------------------------------------
find_package(ZLIB REQUIRED)
if(TARGET ZLIB::ZLIB)
    target_link_libraries(burza PRIVATE ZLIB::ZLIB)
else()
    target_include_directories(burza SYSTEM PRIVATE ${ZLIB_INCLUDE_DIRS})
    target_link_libraries(burza PRIVATE ${ZLIB_LIBRARIES})
endif()

# --- threads -----------------------------------------------------------------
target_link_libraries(burza PRIVATE Threads::Threads)

# -----------------------------------------------------------------------------
#  Linking / packaging details
# -----------------------------------------------------------------------------

if(UNIX AND NOT APPLE)
    # $ORIGIN means "relative to the executable", so a shipped build picks its
    # libraries out of run_linux/lib without LD_LIBRARY_PATH.
    set_target_properties(burza PROPERTIES
        BUILD_RPATH   "\$ORIGIN/lib"
        INSTALL_RPATH "\$ORIGIN/lib")
endif()

if(WIN32)
    if(NOT BURZA_WIN32_CONSOLE)
        set_target_properties(burza PROPERTIES WIN32_EXECUTABLE TRUE)
    endif()
    if(BURZA_TARGET_WINDOWS_XP AND MSVC)
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            set(_xp_subsystem "5.02")   # XP x64 / Server 2003
        else()
            set(_xp_subsystem "5.01")   # XP 32-bit
        endif()
        if(BURZA_WIN32_CONSOLE)
            set(_xp_flags "/SUBSYSTEM:CONSOLE,${_xp_subsystem}")
        else()
            set(_xp_flags "/SUBSYSTEM:WINDOWS,${_xp_subsystem}")
        endif()
        set_target_properties(burza PROPERTIES LINK_FLAGS "${_xp_flags}")
    endif()
endif()

# -----------------------------------------------------------------------------
#  Install
# -----------------------------------------------------------------------------

include(GNUInstallDirs OPTIONAL)
if(NOT CMAKE_INSTALL_BINDIR)
    set(CMAKE_INSTALL_BINDIR "bin")
endif()
if(NOT CMAKE_INSTALL_DATADIR)
    set(CMAKE_INSTALL_DATADIR "share")
endif()

install(TARGETS burza RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
# The UI file lives in resources/ (version controlled); run_linux/data/burza.ui
# is only a symlink to it, so install from the real location.
install(FILES "${PROJECT_ROOT}/resources/burza.ui"
        DESTINATION "${CMAKE_INSTALL_DATADIR}/burza/data")
install(DIRECTORY "${PROJECT_ROOT}/run_linux/data/icons"
        DESTINATION "${CMAKE_INSTALL_DATADIR}/burza/data" OPTIONAL)
install(FILES "${PROJECT_ROOT}/run_linux/data/logo_mc.gif"
        DESTINATION "${CMAKE_INSTALL_DATADIR}/burza/data" OPTIONAL)

# -----------------------------------------------------------------------------
#  Summary
# -----------------------------------------------------------------------------

message(STATUS "---------------------------------------------")
message(STATUS "Burza ${PROJECT_VERSION}")
message(STATUS "  build type   : ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ standard : ${BURZA_CXX_STANDARD}")
message(STATUS "  toolchain    : ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "  pointer size : ${CMAKE_SIZEOF_VOID_P} bytes")
message(STATUS "  postgres     : ${BURZA_WITH_POSTGRES}")
message(STATUS "  output       : ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/burza")
message(STATUS "---------------------------------------------")
