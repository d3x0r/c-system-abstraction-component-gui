
foreach(flag_var
        CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
        CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO
        CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE
        CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO
        )
   if(${flag_var} MATCHES "/MD")
      string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
   endif(${flag_var} MATCHES "/MD")
   if(${flag_var} MATCHES "-br")
      string(REGEX REPLACE "-br" "" ${flag_var} "${${flag_var}}")
   endif(${flag_var} MATCHES "-br")
endforeach(flag_var)

if( WATCOM )
string( REGEX REPLACE "library clbrdll.lib" "" CMAKE_C_STANDARD_LIBRARIES "${CMAKE_C_STANDARD_LIBRARIES}" )
string( REGEX REPLACE "library plbrdll.lib" "" CMAKE_C_STANDARD_LIBRARIES "${CMAKE_C_STANDARD_LIBRARIES}" )
endif( WATCOM )

set( project sqlcmd.portable )

set( SOURCES_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../../.. )

if( WIN32 )
set( MORE_SYSTEM_SOURCES   ${SOURCES_ROOT}/src/systemlib/oswin.c )
set(NET_SOURCES   ${SOURCES_ROOT}/src/netlib/net_winsock2.c )
endif( WIN32 )

if( SNPRINTF_SOURCES )
set( SNPRINTF_SOURCES    ${SOURCES_ROOT}/src/../${SNPRINTF_SOURCES} )
endif(SNPRINTF_SOURCES)


SET( SQLITE_VERSION 3.8.7.4 )
SET( SQLITE_SOURCES ${BAG_SQLITE_INTERFACE} ${SOURCES_ROOT}/src/sqlite/${SQLITE_VERSION}/sqlite3.c )
include_directories( ${SOURCES_ROOT}/src/sqlite/${SQLITE_VERSION} )

set(BASE_SOURCES
${FIRST_GCC_PROGRAM_SOURCE} 
    ../sqlcmd
#    ${SOURCES_ROOT}/src/netlib/network.c 
#    ${SOURCES_ROOT}/src/netlib/udpnetwork.c 
#    ${SOURCES_ROOT}/src/netlib/tcpnetwork.c 
#    ${NET_SOURCES} 
#    ${SOURCES_ROOT}/src/msgsvr/client/client.c
    ${SOURCES_ROOT}/src/SQLlib/sqlstub.c
    ${SOURCES_ROOT}/src/SQLlib/sqlutil.c
    ${SOURCES_ROOT}/src/SQLlib/sqlwrap.c
    ${SOURCES_ROOT}/src/SQLlib/sqlparse3.c
    ${SQLITE_SOURCES}
    ${SOURCES_ROOT}/src/memlib/sharemem.c
    ${SOURCES_ROOT}/src/memlib/memory_operations.c
    ${SOURCES_ROOT}/src/timerlib/timers.c
    ${SOURCES_ROOT}/src/idlelib/idle.c
    ${SOURCES_ROOT}/src/procreglib/names.c
    ${SOURCES_ROOT}/src/configlib/configscript.c
    ${SOURCES_ROOT}/src/sysloglib/syslog.c
    ${SOURCES_ROOT}/src/systemlib/system.c
    ${MORE_SYSTEM_SOURCES}
    ${SOURCES_ROOT}/src/typelib/typecode.c
    ${SOURCES_ROOT}/src/typelib/text.c
    ${SOURCES_ROOT}/src/typelib/sets.c
    ${SOURCES_ROOT}/src/typelib/binarylist.c
    ${SOURCES_ROOT}/src/typelib/msgqueue.c
    ${SOURCES_ROOT}/src/filesyslib/filescan.c
    ${SOURCES_ROOT}/src/filesyslib/pathops.c
    ${SOURCES_ROOT}/src/filesyslib/winfiles.c
    ${SNPRINTF_SOURCES}
${LAST_GCC_PROGRAM_SOURCE}
    ${SOURCES_ROOT}/src/deadstart/deadstart_core.c 
)

if( NOT __NO_ODBC__ )
    set( MoreDefinitions USE_ODBC )
else( NOT __ODBC__ )
    set( MoreDefinitions __NO_ODBC__ )
endif( NOT __NO_ODBC__ )


set_source_files_properties( ${BASE_SOURCES} PROPERTIES 
		COMPILE_DEFINITIONS "SACK_BAG_EXPORTS;__STATIC__;__NO_GUI__;__NO_OPTIONS__;__NO_DEFAULT_INTERFACES__;${MoreDefinitions};USE_SQLITE;__DISABLE_UDP_SYSLOG__;__NO_MSGSVR__;NO_DEADSTART_DLLMAIN" )

sack_add_executable(${project} ${BASE_SOURCES} )

SET_TARGET_PROPERTIES(${project} PROPERTIES        FOLDER utils   )

target_link_libraries( ${project}  ${SACK_PLATFORM_LIBRARIES} )
if( WIN32 )
#target_link_libraries( ${project} winmm wsock32 ws2_32 )
target_link_libraries( ${project} winmm odbc32 )
else( WIN32 )
if( NOT __NO_ODBC__ )
   target_link_libraries( ${project} odbc  )
endif( NOT __NO_ODBC__ )
endif( WIN32 )
install_default_dest_binary( ${project} )





