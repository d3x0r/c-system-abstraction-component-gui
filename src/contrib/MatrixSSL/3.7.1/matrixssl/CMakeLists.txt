	
set( MatrixSSL_Sources
		prf.c
		sslv3.c
		tls.c
		psk.c
		cipherSuite.c
		hsHash.c
		matrixssl.c
		matrixsslApi.c
		sslDecode.c
sslEncode.c )

# if SHARED; on windows, declspec...
ADD_DEFINITIONS( -D_USRDLL -DMATRIX_SSL_SOURCE )

add_library( MatrixSSL SHARED ${MatrixSSL_Sources} )

target_link_libraries( MatrixSSL mssl_crypto mssl_core )

SET_TARGET_PROPERTIES( MatrixSSL PROPERTIES
                  DEFINE_SYMBOL _USRDLL
                  COMPILE_FLAGS  "${SYS_C_FLAGS}"
                  COMPILE_DEFINITIONS "${LIBFLAGS}"
                  FOLDER "Matrix SSL" 
                  )

if( WIN32 )
   install( TARGETS MatrixSSL RUNTIME DESTINATION bin LIBRARY DESTINATION bin ARCHIVE DESTINATION lib )
else( WIN32 )
   include( GNUInstallDirs )
   install( TARGETS MatrixSSL DESTINATION ${CMAKE_INSTALL_LIBDIR} )
endif( WIN32 )
