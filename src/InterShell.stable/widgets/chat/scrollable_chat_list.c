#ifndef FORCE_NO_INTERFACE
#define USE_IMAGE_INTERFACE l.pii
#define USE_RENDER_INTERFACE l.pri
#endif

#include <stdhdrs.h>
#include <controls.h>
#include <psi.h>
#include <sqlgetoption.h>


#include "../include/buttons.h"

#include "../../intershell_registry.h"

#define CONTROL_NAME WIDE("Scrollable Button List")

typedef struct chat_message_tag
{
	struct {
		_8 hr,mn,sc;
		_8 mo,dy;
		_16 year;
	} received; // the time the message was received
	struct {
		_8 hr,mn,sc;
		_8 mo,dy;
		_16 year;
	} sent; // the time the message was sent
	CTEXTSTR text;
   LOGICAL sent; // if not sent, is received message - determine justification and decoration
} CHAT_MESSAGE;
typedef struct chat_message_tag *PCHAT_MESSAGE;

typedef struct chat_list_tag
{
	PLIST messages; //
   int first_button;
   int control_offset;
} CHAT_LIST;
typedef struct chat_list_tag *PCHAT_LIST;

#define l local_scollable_list_data
static struct scollable_list
{
	Image decoration;
	struct {
		S_32 back_x, back_y;
		_32 back_w, back_h;
		S_32 arrow_x, arrow_y;
		_32 arrow_w, arrow_h;
      S_32 div_x1, div_x2;
      S_32 div_y1, div_y2;
	} sent;
	struct {
		S_32 back_x, back_y;
		_32 back_w, back_h;
		S_32 arrow_x, arrow_y;
		_32 arrow_w, arrow_h;
      S_32 div_x1, div_x2;
      S_32 div_y1, div_y2;
	} received;
} l;

EasyRegisterControlWithBorder( CONTROL_NAME, sizeof( CHAT_LIST ), BORDER_NONE );



static PTRSZVAL CPROC SetBackgroundImage( PTRSZVAL psv, arg_list args )
{
	PARAM( args, CTEXTSTR, image_name );
   l.decoration = LoadImageFile( image_name );
   return psv;
}

static PTRSZVAL CPROC SetSentArrowArea( PTRSZVAL psv, arg_list args )
{
   PARAM( args, S_64, x );
   PARAM( args, S_64, y );
   PARAM( args, S_64, w );
   PARAM( args, S_64, h );
   l.sent.back_x = x;
   l.sent.back_y = y;
   l.sent.back_w = w;
   l.sent.back_h = h;
   return psv;
}

static PTRSZVAL CPROC SetReceiveArrowArea( PTRSZVAL psv, arg_list args )
{
   PARAM( args, S_64, x );
   PARAM( args, S_64, y );
   PARAM( args, S_64, w );
   PARAM( args, S_64, h );
   l.received.arrow_x = x;
   l.received.arrow_y = y;
   l.received.arrow_w = w;
   l.received.arrow_h = h;
   return psv;
}

static PTRSZVAL CPROC SetSentBackgroundArea( PTRSZVAL psv, arg_list args )
{
   PARAM( args, S_64, x );
   PARAM( args, S_64, y );
   PARAM( args, S_64, w );
   PARAM( args, S_64, h );
   l.sent.back_x = x;
   l.sent.back_y = y;
   l.sent.back_w = w;
   l.sent.back_h = h;
   return psv;
}

static PTRSZVAL CPROC SetReceiveBackgroundArea( PTRSZVAL psv, arg_list args )
{
   PARAM( args, S_64, x );
   PARAM( args, S_64, y );
   PARAM( args, S_64, w );
   PARAM( args, S_64, h );
   l.received.back_x = x;
   l.received.back_y = y;
   l.received.back_w = w;
   l.received.back_h = h;
   return psv;
}

static PTRSZVAL CPROC SetSentBackgroundDividers( PTRSZVAL psv, arg_list args )
{
   PARAM( args, S_64, x1 );
   PARAM( args, S_64, x2 );
   PARAM( args, S_64, y1 );
   PARAM( args, S_64, y2 );
   l.sent.div_x1 = x1;
   l.sent.div_x2 = x2;
   l.sent.div_y1 = y1;
   l.sent.div_y2 = y2;
   return psv;
}

static PTRSZVAL CPROC SetReceiveBackgroundDividers( PTRSZVAL psv, arg_list args )
{
   PARAM( args, S_64, x1 );
   PARAM( args, S_64, x2 );
   PARAM( args, S_64, y1 );
	PARAM( args, S_64, y2 );
   l.received.div_x1 = x1;
   l.received.div_x2 = x2;
   l.received.div_y1 = y1;
   l.received.div_y2 = y2;
   return psv;
}


static void OnLoadCommon( WIDE( "Chat Control" ) )( PCONFIG_HANDLER pch )
{
	AddConfigurationMethod( pch, "Background Image=%m", SetBackgroundImage );
   AddConfigurationMethod( pch, "Sent Arrow Area=%i,%i %i,%i", SetSentArrowArea );
   AddConfigurationMethod( pch, "Received Arrow Area=%i,%i %i,%i", SetReceiveArrowArea );
   AddConfigurationMethod( pch, "Sent Background Area=%i,%i %i,%i", SetSentBackgroundArea );
	AddConfigurationMethod( pch, "Received Background Area=%i,%i %i,%i", SetReceiveBackgroundArea );
	AddConfigurationMethod( pch, "Sent Background Dividers=%i,%i,%i,%i", SetSentBackgroundDividers );
	AddConfigurationMethod( pch, "Received Background Dividers=%i,%i,%i,%i", SetReceiveBackgroundDividers );
}



void AddSentMessage( PSI_CONTROL pc, CTEXTSTR text )
{

}


