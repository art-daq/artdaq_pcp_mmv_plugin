# ======================================================================
#  artdaq_pcp_mmv_plugin main build file
#
#  cd .../path/to/build/directory
#  source .../path/to/artdaq/ups/setup_for_development
#  cmake [-DCMAKE_INSTALL_PREFIX=/install/path]
#        -Dqualifier:STRING=$CETPKG_QUAL
#        -DCMAKE_BUILD_TYPE=$CETPKG_TYPE
#        $CETPKG_SOURCE
#  make
#  make test
#  make install
#  make package
# ======================================================================

cmake_minimum_required (VERSION 3.17)

find_package(cetbuildtools)

project(artdaq_pcp_mmv_plugin VERSION 1.01.00)
# cetbuildtools contains our cmake modules
SET ( CETBUILDTOOLS_VERSION $ENV{CETBUILDTOOLS_VERSION} )
IF (NOT CETBUILDTOOLS_VERSION)
    MESSAGE (FATAL_ERROR "ERROR: setup cetbuildtools to get the cmake modules")
ENDIF()

set(CMAKE_MODULE_PATH $ENV{CETBUILDTOOLS_DIR}/Modules
                      $ENV{ARTDAQ_UTILITIES_DIR}/Modules
		      ${CMAKE_MODULE_PATH})

include(CetCMakeEnv)
cet_cmake_env()

cet_set_compiler_flags(DIAGS VIGILANT
  WERROR
  NO_UNDEFINED
  EXTRA_FLAGS -pedantic
)

string(TOUPPER ${CMAKE_BUILD_TYPE} BTYPE_UC )
if( ${BTYPE_UC} MATCHES "DEBUG" )
	if(DEFINED ENV{USE_ASAN})
		if($ENV{USE_ASAN} GREATER 0)
			message("!!! Address Sanitizer Activated!!!")
			cet_add_compiler_flags(-fsanitize=address -lasan -Wno-unused-command-line-argument)
		endif()
	endif()
	if(DEFINED ENV{USE_TSAN})
		if($ENV{USE_TSAN} GREATER 0)
			message("!!! Thread Sanitizer Activated!!!")
			cet_add_compiler_flags(-fsanitize=thread -ltsan -Wno-unused-command-line-argument)
		endif()
	endif()
	if(DEFINED ENV{USE_GCOV})
		cet_add_compiler_flags(-fprofile-arcs -ftest-coverage --coverage)
	endif()
endif()

cet_report_compiler_flags()

# these are minimum required versions, not the actual product versions
find_ups_product( artdaq_utilities v1_00_00 )
#find_ups_product( gcc v4_0_0 )
find_ups_boost(v1_53_0)
find_ups_product (cetbuildtools v4_09_02)
find_ups_product( TRACE v3_03_00 )
find_ups_product( messagefacility v1_14_01 )

# as of art v2_11_00, messagefacility has a single library
check_ups_version(messagefacility $ENV{MESSAGEFACILITY_VERSION} v2_02_00 PRODUCT_MATCHES_VAR ONE_MF_LIBRARY)
if(ONE_MF_LIBRARY)
    set( MESSAGEFACILITY_LIBS MF_MessageLogger)
else()
    set( MESSAGEFACILITY_LIBS MF_MessageLogger MF_Utilities)
endif()

# Debug streamer.
string(TOUPPER ${CMAKE_BUILD_TYPE} BTYPE_UC)
if(BTYPE_UC STREQUAL DEBUG)
  add_definitions(-DDEBUGME)
endif()

# macros for artdaq_dictionary and simple_plugin
include(BasicPlugin)

# Source
add_subdirectory(artdaq-pcp-mmv-plugin)

# doc - Documentation
add_subdirectory(doc)

# ups - table and config files
add_subdirectory(ups)

# packaging utility
include(UseCPack)
