/*
 * SACK extension to define methods to render to javascript/HTML5 WebSocket event interface
 *
 * Crafted by: Jim Buckeyne
 *
 * Purpose: Provide a well defined, concise structure to
 *   provide websocket server support to C applications.
 *   
 *
 *
 * (c)Freedom Collective, Jim Buckeyne 2012+; SACK Collection.
 *
 */

#ifndef HTML5_WEBSOCKET_STUFF_DEFINED
#define HTML5_WEBSOCKET_STUFF_DEFINED

#include <procreg.h>
#include <controls.h>

// should consider merging these headers(?)
#include <html5.websocket.client.h>

#ifdef __cplusplus
#define _HTML5_WEBSOCKET_NAMESPACE namespace Html5WebSocket {
#define HTML5_WEBSOCKET_NAMESPACE SACK_NAMESPACE _NETWORK_NAMESPACE _HTML5_WEBSOCKET_NAMESPACE
#define HTML5_WEBSOCKET_NAMESPACE_END } _NETWORK_NAMESPACE_END SACK_NAMESPACE_END
#define USE_HTML5_WEBSOCKET_NAMESPACE using namespace sack::network::Html5WebSocket;
#else
#define _HTML5_WEBSOCKET_NAMESPACE
#define HTML5_WEBSOCKET_NAMESPACE
#define HTML5_WEBSOCKET_NAMESPACE_END
#define USE_HTML5_WEBSOCKET_NAMESPACE
#endif

HTML5_WEBSOCKET_NAMESPACE


#ifdef HTML5_WEBSOCKET_SOURCE
#define HTML5_WEBSOCKET_PROC(type,name) EXPORT_METHOD type CPROC name
#else
#define HTML5_WEBSOCKET_PROC(type,name) IMPORT_METHOD type CPROC name
#endif

// need some sort of other methods to work with an HTML5WebSocket...
// server side.
	HTML5_WEBSOCKET_PROC( PCLIENT, WebSocketCreate )( CTEXTSTR server_url
																	, web_socket_opened on_open
																	, web_socket_event on_event
																	, web_socket_closed on_closed
																	, web_socket_error on_error
																	, PTRSZVAL psv
																	);


/* define a callback which uses a HTML5WebSocket collector to build javascipt to render the control.
 * example:
 *       static int OnDrawToHTML("Control Name")(CONTROL, HTML5WebSocket ){ }
 */
//#define OnDrawToHTML(name)  \
//	__DefineRegistryMethodP(PRELOAD_PRIORITY,ROOT_REGISTRY,_OnDrawCommon,WIDE("control"),name WIDE("/rtti"),WIDE("draw_to_canvas"),int,(CONTROL, HTML5WebSocket ), __LINE__)




HTML5_WEBSOCKET_NAMESPACE_END
USE_HTML5_WEBSOCKET_NAMESPACE

#endif
