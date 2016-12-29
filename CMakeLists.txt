
cmake_minimum_required(VERSION 3.0)
cmake_policy(VERSION 3.0)

project(bmdc
	VERSION 0.1.16
	LANGUAGES C CXX)
set(PROJECT_BUGREPORT "https://bugs.launchpad.net/bmdc++")
set(PROJECT_URL "http://freedcppmv.sourceforge.net/doku.php/mainpage")
#for now always use stacktrace
#-Wall 
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -D_LARGEFILE_SOURCE -DUSE_STACKTRACE -D_FILE_OFFSET_BITS=64 -D_REENTRANT")
# Required for FindIntltool module
set(GETTEXT_PACKAGE ${PROJECT_NAME})
set(GETTEXT_PO_DIR ${CMAKE_SOURCE_DIR}/po)

# Keep these two definitions in agreement.
set(glib_minimum_version 2.46)
set(glib_encoded_version GLIB_VERSION_2_46)

# Keep these two definitions in agreement.
set(gdk_minimum_version 3.10)
set(gdk_encoded_version GDK_VERSION_3_10)

# Warn about API usage that violates our minimum requirements.
#add_definitions(-DGLIB_VERSION_MAX_ALLOWED=${glib_encoded_version})
#add_definitions(-DGDK_VERSION_MAX_ALLOWED=${gdk_encoded_version})
set(gdk_pixbuf_minimum_version 2.24.0)
set(gladeui_minimum_version 3.10.0)

# Load modules from the source tree
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_SKIP_BUILD_RPATH OFF)
set(CMAKE_BUILD_WITH_INSTALL_RPATH OFF)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH ON)
# CMAKE_INSTALL_RPATH is set below

# CMake modules
include(CheckCCompilerFlag)
include(CheckCSourceCompiles)
include(CheckCSourceRuns)
include(CheckFunctionExists)
include(CheckIncludeFile)
include(CheckLibraryExists)
include(FindMiniupnpc)

# Project custom modules
include(PrintableOptions)


add_printable_variable(LIB_SUFFIX "Library directory suffix, usually defined to '64' for x86_64 systems" "")
add_printable_variable_bare(CMAKE_INSTALL_PREFIX)
add_printable_variable_path(BIN_INSTALL_DIR "Install directory for binary files, defaults to CMAKE_INSTALL_PREFIX/bin" "")
add_printable_variable_path(INCLUDE_INSTALL_DIR "Install directory for header files, defaults to CMAKE_INSTALL_PREFIX/include" "")
add_printable_variable_path(LIB_INSTALL_DIR "Install directory for library files, defaults to CMAKE_INSTALL_PREFIX/lib{LIB_SUFFIX}" "")
add_printable_variable_path(LIBEXEC_INSTALL_DIR "Install directory for library executable files, defaults to CMAKE_INSTALL_PREFIX/libexec" "")
add_printable_variable_path(SHARE_INSTALL_PREFIX "Install directory for shared files, defaults to CMAKE_INSTALL_PREFIX/share" "")
add_printable_variable_path(LOCALE_INSTALL_DIR "Install directory for locale files, defaults to SHARE_INSTALL_PREFIX/locale" "")
add_printable_variable_path(SYSCONF_INSTALL_DIR "Install directory for system configuration files, defaults to CMAKE_INSTALL_PREFIX/etc" "")

macro(ensure_default_value _var _defvalue)
	if(${_var} STREQUAL "")
		set(${_var} ${_defvalue})
	endif(${_var} STREQUAL "")
endmacro(ensure_default_value)

ensure_default_value(BIN_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/bin")
ensure_default_value(INCLUDE_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/include")
ensure_default_value(LIB_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}")
ensure_default_value(LIBEXEC_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/libexec")
ensure_default_value(SHARE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}/share")
ensure_default_value(LOCALE_INSTALL_DIR "${SHARE_INSTALL_PREFIX}/locale")
ensure_default_value(SYSCONF_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/etc")


include (CheckIncludeFile)
include (CheckIncludeFiles)
include (CheckFunctionExists)
include (FindPkgConfig)
include (CheckCXXSourceCompiles)


include(CodeCoverageGCOV)
include(CheckTarget)

include(GLibTools)
include(GtkDoc)
include(IconCache)

include(PkgConfigEx)
include(SetupBuildFlags)

include(FindIntltool)


add_printable_option(ENABLE_MAINTAINER_MODE "Enable maintainer mode" OFF)
add_printable_variable(VERSION_SUBSTRING "Version substring, for packagers" "")
add_printable_variable(VERSION_COMMENT "Define if you want a comment appended to the version number" "")


if(ENABLE_MAINTAINER_MODE)
	set(BUILD_TESTING ON)
endif(ENABLE_MAINTAINER_MODE)

# Setup compiler/linker flags
setup_build_flags(${ENABLE_MAINTAINER_MODE})

set(MATH_LDFLAGS -lm)

CHECK_INCLUDE_FILE(sys/wait.h HAVE_SYS_WAIT_H)
CHECK_INCLUDE_FILE(X11/XF86keysym.h HAVE_XFREE)
CHECK_FUNCTION_EXISTS(mkdtemp HAVE_MKDTEMP)
CHECK_FUNCTION_EXISTS(nl_langinfo HAVE_NL_LANGINFO)

 #******************************
# Check for nl_langinfo features
# ******************************

CHECK_C_SOURCE_COMPILES("#include <langinfo.h>
			int main(void) { char *detail = nl_langinfo (_NL_MEASUREMENT_MEASUREMENT); return 0; }" HAVE__NL_MEASUREMENT_MEASUREMENT)

# ******************************
# Dependencies
# ******************************

if(WIN32)
	set(GIO_UNIX_REQUIREMENT "gio-windows-2.0")
else(WIN32)
	set(GIO_UNIX_REQUIREMENT "gio-unix-2.0")
endif(WIN32)


pkg_check_modules(GTK_PLATFORM REQUIRED cairo-gobject
	gdk-pixbuf-2.0>=${gdk_pixbuf_minimum_version}
	gio-2.0>=${glib_minimum_version}
	${GIO_UNIX_REQUIREMENT}
	gmodule-2.0>=${glib_minimum_version}
	gtk+-3.0>=${gdk_minimum_version}
	glib-2.0
	
)
#INCLUDE(FindPackageHandleStandardArgs)

find_package (PkgConfig)
find_package (BZip2 REQUIRED)
find_package (ZLIB REQUIRED)
find_package (OpenSSL REQUIRED)
find_package (GeoIP REQUIRED)
find_package (Xattr)
find_package(Pcre)

if (USE_MINIUPNP)
    message (STATUS "Building with UPNP support (miniupnpc)")
    set (MINIUPNP_INCLUDE_DIR)
    set (MINIUPNP_LIBRARY)
  if (LOCAL_MINIUPNP)
    add_subdirectory(upnp)
    set (MINIUPNP_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/upnp")
    set (MINIUPNP_LIBRARY "miniupnpc")
  else (LOCAL_MINIUPNP)
    find_package (Miniupnpc REQUIRED)
  endif (LOCAL_MINIUPNP)

elseif (NOT USE_MINIUPNP)
    unset (MINIUPNP_INCLUDE_DIR CACHE)
    unset (MINIUPNP_LIBRARY CACHE)
    message (STATUS "Building without UPNP support (miniupnpc)")
endif(USE_MINIUPNP)

# ******************************
# Canberra / Canberra-GTK Sound
# ******************************

add_printable_option(ENABLE_CANBERRA "Enable Canberra and Canberra-GTK sound" ON)

if(ENABLE_CANBERRA)
	pkg_check_modules_for_option(ENABLE_CANBERRA "Canberra and Canberra-GTK sound" CANBERRA libcanberra-gtk3>=${libcanberra_gtk_minimum_version})
	set(HAVE_CANBERRA ON)
endif(ENABLE_CANBERRA)

# ******************************
# iconv checking
# ******************************

#set(CMAKE_REQUIRED_LIBRARIES "-liconv")
#CHECK_C_SOURCE_COMPILES("#include <iconv.h>
#			#include <stdlib.h>
#			int main(void) { iconv_t cd; cd = iconv_open (\"UTF-8\", \"ISO-8859-1\"); return 0; }" HAVE_LIBICONV)
#unset(CMAKE_REQUIRED_LIBRARIES)

#if(HAVE_LIBICONV)
#	set(ICONV_LIBS "-liconv")
#	set(HAVE_ICONV ON)
#else(HAVE_LIBICONV)
#	set(ICONV_LIBS "")
#	CHECK_FUNCTION_EXISTS(iconv HAVE_ICONV)
#endif(HAVE_LIBICONV)

#if(NOT HAVE_ICONV)
#	message(FATAL_ERROR "You need to install a working iconv implementation, such as ftp://ftp.gnu.org/pub/gnu/libiconv")
#endif(NOT HAVE_ICONV)

#set(CMAKE_REQUIRED_LIBRARIES ${ICONV_LIBS})
#CHECK_C_SOURCE_RUNS("#include \"${CMAKE_SOURCE_DIR}/iconv-detect.c\"" _correct_iconv)
#unset(CMAKE_REQUIRED_LIBRARIES)

#if(NOT _correct_iconv)
#	message(FATAL_ERROR "You need to install a working iconv implementation, such as ftp://ftp.gnu.org/pub/gnu/libiconv")
#endif(NOT _correct_iconv)

# ******************************
# Libnotify
# ******************************

#pkg_check_modules(LIBNOTIFY libnotify>=${libnotify_minimum_version})
#set(HAVE_LIBNOTIFY ${LIBNOTIFY_FOUND})

# Generate the ${PROJECT_NAME}-config.h file
#CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/config.h.in ${CMAKE_BINARY_DIR}/${PROJECT_NAME}-config.h)
#IF (WIN32)
#empty
#else(WIN32)
	message(STATUS "Building needed files...")
	execute_process(COMMAND "${CMAKE_SOURCE_DIR}/linux/gen-cmake.sh" "${CMAKE_SOURCE_DIR}/linux")
#ENDIF(WIN32)
print_build_options()

add_subdirectory(natpmp)

add_subdirectory(data)
add_subdirectory(po)
add_subdirectory(dcpp)
add_subdirectory(linux)
install(DIRECTORY icons country emoticons  DESTINATION share/bmdc)
install(DIRECTORY extensions/Scripts/ DESTINATION share/bmdc/extensions/Scripts)
#add_subdirectory(tests)

#if(ENABLE_GTK_DOC)
#	add_subdirectory(docs)
#endif(ENABLE_GTK_DOC)

