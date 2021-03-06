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

set( project proxy.portable )

set( SOURCES_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../../.. )

if( WIN32 )
set( NET_SOURCES     ${SOURCES_ROOT}/src/netlib/net_winsock2.c  ) 
endif( WIN32 )

if( WIN32 )
set( MORE_SYSTEM_SOURCES   ${SOURCES_ROOT}/src/systemlib/oswin.c )
endif( WIN32 )

if( SNPRINTF_SOURCES )
set( SNPRINTF_SOURCES    ${SOURCES_ROOT}/../${SNPRINTF_SOURCES} )
endif(SNPRINTF_SOURCES)

if( MSVC ) 
SET( ExtraSources ${SOURCES_ROOT}/src/systemlib/args.c )
endif( MSVC )



sack_add_executable( ${project} ${FIRST_GCC_PROGRAM_SOURCE} 
    ../proxy.c 
    ${SOURCES_ROOT}/src/netlib/network.c 
    ${SOURCES_ROOT}/src/netlib/udpnetwork.c 
    ${SOURCES_ROOT}/src/netlib/tcpnetwork.c 
    ${NET_SOURCES}
    ${SOURCES_ROOT}/src/memlib/sharemem.c
    ${SOURCES_ROOT}/src/memlib/memory_operations.c
    ${SOURCES_ROOT}/src/timerlib/timers.c
    ${SOURCES_ROOT}/src/idlelib/idle.c
    ${SOURCES_ROOT}/src/procreglib/names.c
    ${SOURCES_ROOT}/src/configlib/configscript.c
    ${SOURCES_ROOT}/src/sysloglib/syslog.c
    ${ExtraSources}
    ${SOURCES_ROOT}/src/systemlib/system.c
    ${MORE_SYSTEM_SOURCES}
    ${SOURCES_ROOT}/src/typelib/typecode.c
    ${SOURCES_ROOT}/src/typelib/text.c
    ${SOURCES_ROOT}/src/typelib/sets.c
    ${SOURCES_ROOT}/src/typelib/binarylist.c
    ${SOURCES_ROOT}/src/filesyslib/filescan.c
    ${SOURCES_ROOT}/src/filesyslib/pathops.c
    ${SOURCES_ROOT}/src/filesyslib/winfiles.c
    ${SNPRINTF_SOURCES}
${LAST_GCC_PROGRAM_SOURCE} 
    ${SOURCES_ROOT}/src/deadstart/deadstart_core.c
)
target_link_libraries( ${project} ${SACK_PLATFORM_LIBRARIES} )

string( REPLACE "." "_" TARGET_LABEL ${project} )
SET_PROPERTY(TARGET ${project} APPEND PROPERTY COMPILE_DEFINITIONS "SACK_BAG_EXPORTS;__NO_GUI__;__NO_OPTIONS__;__NO_DEFAULT_INTERFACES__;__STATIC__;NO_DEADSTART_DLLMAIN" )
SET_TARGET_PROPERTIES(${project} PROPERTIES
                  FOLDER "Network Examples"
)
target_link_libraries( ${project} ${SOCKET_LIBRARIES} ${SACK_PLATFORM_LIBRARIES} )
install_default_dest( ${project} )

if( WIN32 )

set( project proxy.service )
if( WIN32 )
set( NET_SOURCES     ${SOURCES_ROOT}/src/netlib/net_winsock2.c  ) 
set( MORE_SYSTEM_SOURCES   ${SOURCES_ROOT}/src/systemlib/oswin.c )
endif( WIN32 )

if( MSVC ) 
SET( ExtraSources ${SOURCES_ROOT}/src/systemlib/args.c )
endif( MSVC )


sack_add_executable( ${project} ${FIRST_GCC_PROGRAM_SOURCE} 
    ../proxy.c 
    ${SOURCES_ROOT}/src/netlib/network.c 
    ${SOURCES_ROOT}/src/netlib/udpnetwork.c 
    ${SOURCES_ROOT}/src/netlib/tcpnetwork.c 
    ${NET_SOURCES}
    ${SOURCES_ROOT}/src/memlib/sharemem.c
    ${SOURCES_ROOT}/src/memlib/memory_operations.c
    ${SOURCES_ROOT}/src/timerlib/timers.c
    ${SOURCES_ROOT}/src/idlelib/idle.c
    ${SOURCES_ROOT}/src/procreglib/names.c
    ${SOURCES_ROOT}/src/configlib/configscript.c
    ${SOURCES_ROOT}/src/sysloglib/syslog.c
    ${ExtraSources}
    ${SOURCES_ROOT}/src/systemlib/system.c
    ${MORE_SYSTEM_SOURCES}
    ${SOURCES_ROOT}/src/typelib/typecode.c
    ${SOURCES_ROOT}/src/typelib/text.c
    ${SOURCES_ROOT}/src/typelib/sets.c
    ${SOURCES_ROOT}/src/typelib/binarylist.c
    ${SOURCES_ROOT}/src/filesyslib/filescan.c
    ${SOURCES_ROOT}/src/filesyslib/pathops.c
    ${SOURCES_ROOT}/src/filesyslib/winfiles.c
    ${SOURCES_ROOT}/src/utils/service/service_stub.c
    ${SOURCES_ROOT}/src/systemlib/args.c
    ${SOURCES_ROOT}/src/systemlib/spawntask.c
    ${SNPRINTF_SOURCES}
${LAST_GCC_PROGRAM_SOURCE} 
    ${SOURCES_ROOT}/src/deadstart/deadstart_core.c
)
target_link_libraries( ${project} ${SACK_PLATFORM_LIBRARIES} )

SET_PROPERTY(TARGET ${project} APPEND PROPERTY COMPILE_DEFINITIONS "SACK_BAG_EXPORTS;BUILD_SERVICE;__STATIC__;__NO_GUI__;__NO_DEFAULT_INTERFACES__;__NO_OPTIONS__;NO_DEADSTART_DLLMAIN" )
string( REPLACE "." "_" TARGET_LABEL ${project} )
SET_TARGET_PROPERTIES(${project} PROPERTIES
                  COMPILE_FLAGS  "-DTARGET_LABEL=${TARGET_LABEL}"
                  FOLDER "Network Examples"
)
install_default_dest( ${project} )

endif( WIN32 )
