
cmake_minimum_required(VERSION 2.8)


PROJECT( layout_board )

include_directories( ${SACK_INCLUDE_DIR} )
LINK_DIRECTORIES( ${SACK_LIBRARY_DIR} )
add_definitions( -DPEICE_SOURCE -DTOOLBIN_SOURCE)

ADD_LIBRARY( ${PROJECT_NAME} SHARED
${FIRST_GCC_LIBRARY_SOURCE} 
board_db.c board.cpp layer.cpp peice.cpp toolbin.cpp 
${LAST_GCC_LIBRARY_SOURCE} 
)
TARGET_LINK_LIBRARIES( ${PROJECT_NAME} ${SACK_LIBRARIES} )

string( REPLACE "." "_" TARGET_LABEL ${PROJECT_NAME} )
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES
                  COMPILE_FLAGS  "-DTARGET_LABEL=${TARGET_LABEL} ${LibraryExtraFlags}"
                  FOLDER "InterShell/plugins"
)
install_literal_product( ${PROJECT_NAME} plugins ) 
