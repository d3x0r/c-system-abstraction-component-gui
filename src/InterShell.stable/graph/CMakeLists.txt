
set( PROJECT network_graph.isp )

set( SOURCES
${FIRST_GCC_LIBRARY_SOURCE} 
    global.c linegraph.c connect_stat.c packet_stat.c
${LAST_GCC_LIBRARY_SOURCE}
)

add_library(${PROJECT} SHARED ${SOURCES} )

string( REPLACE "." "_" TARGET_LABEL ${PROJECT} )
SET_TARGET_PROPERTIES(${PROJECT} PROPERTIES
                  COMPILE_FLAGS  "-DTARGET_LABEL=${TARGET_LABEL} ${LibraryExtraFlags}"
                  FOLDER InterShell/plugins
)
if( NOT __ANDROID__ )
SET_TARGET_PROPERTIES(${PROJECT} PROPERTIES
                  SUFFIX ""
                  PREFIX ""
)
endif( NOT __ANDROID__ )

target_link_libraries( ${PROJECT} ${SACK_LIBRARIES} )
target_link_libraries( ${PROJECT} ${SACK_PLATFORM_LIBRARIES} )
install_plugin( ${PROJECT} plugins ) 

add_subdirectory( frames )

