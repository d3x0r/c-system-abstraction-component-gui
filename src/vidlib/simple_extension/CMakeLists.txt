cmake_minimum_required(VERSION 2.8)

if( NOT MAKING_SACK_CORE )
if( NOT SACK_SDK_ROOT_PATH )
GET_FILENAME_COMPONENT(SACK_SDK_ROOT_PATH "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Freedom Collective\\SACK;Install_Dir]" ABSOLUTE CACHE)
else( NOT SACK_SDK_ROOT_PATH )
set(SACK_SDK_ROOT_PATH ${SACK_SDK_ROOT_PATH} CACHE STRING "SACK SDK Root" )
endif( NOT SACK_SDK_ROOT_PATH )

include( ${SACK_SDK_ROOT_PATH}/CMakePackage )

include_directories( ${SACK_INCLUDE_DIR} )
LINK_DIRECTORIES( ${SACK_LIBRARY_DIR} )

INSTALL_SACK( bin )
endif( NOT MAKING_SACK_CORE )


PROJECT( bag.video.frame )

ADD_DEFINITIONS( -DMAKE_RCOORD_SINGLE )

set( PROJECT bag.video.frame.lock )
if( UNIX )
set( MORE_SOURCES keymap_linux.c )
endif( UNIX )
sack_add_library( ${PROJECT} SHARED ${FIRST_GCC_LIBRARY_SOURCE} Android_ANativeWindow.c ${MORE_SOURCES} ${LAST_GCC_LIBRARY_SOURCE})
target_link_libraries( ${PROJECT} ${SACK_LIBRARIES} )
install_default_dest( ${PROJECT} )
  SET_TARGET_PROPERTIES( ${PROJECT} PROPERTIES	FOLDER "Video Renderer"   
       COMPILER_DEFINITIONS TARGET_LABEL=bag_video_frame_lock 
   )


