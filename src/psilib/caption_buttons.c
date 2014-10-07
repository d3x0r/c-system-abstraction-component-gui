

#include "global.h"
#include <sharemem.h>
#include <controls.h>
#include "controlstruc.h"
#include <psi.h>

PSI_NAMESPACE 

//---------------------------------------------------------------------------

static void CPROC StopThisProgram( PSI_CONTROL pc )
{
	PSI_CONTROL button;
	if( pc->device && pc->parent  && ( !(pc->BorderType & BORDER_WITHIN ) ) )
	{
		button = GetControl( pc, pc->device->nIDDefaultCancel );
		if( button )
			InvokeButton( button );
		else
		{
			button = GetControl( pc, pc->device->nIDDefaultOK );
			if( button )
				InvokeButton( button );
			else
				HideCommon( pc );
		}
	}
	else
		BAG_Exit( (int)'exit' );
}

//---------------------------------------------------------------------------

void AddCaptionButton( PSI_CONTROL frame, Image normal, Image pressed, void (CPROC*event)(PSI_CONTROL) )
{
	if( !( frame->BorderType & BORDER_CAPTION_NO_CLOSE_BUTTON ) )
	{
		if( !frame->flags.bCloseButtonAdded 
			&& ( frame->BorderType & BORDER_CAPTION_CLOSE_BUTTON ) )
		{
			frame->flags.bCloseButtonAdded = 1;
			AddCaptionButton( frame, g.StopButton, g.StopButtonPressed, StopThisProgram );
		}
	}
	if( frame && event )
	{
		PCAPTION_BUTTON button = New( CAPTION_BUTTON );
		button->normal = normal;
		button->pressed = pressed;
		button->pressed_event = event;
		button->is_pressed = FALSE;
		AddLink( &frame->caption_buttons, button );
	}
}

//---------------------------------------------------------------------------


PSI_NAMESPACE_END
