#ifndef UNDER_CE
#define NEED_REAL_IMAGE_STRUCTURE

#define NO_OPEN_MACRO
#define FIX_RELEASE_COM_COLLISION
#include <stdhdrs.h>
//#include <windows.h>    // Header File For Windows
#include <sharemem.h>
#include <timers.h>
#include <logging.h>
#include <vectlib.h>



#include <imglib/imagestruct.h>
#include <vidlib/vidstruc.h>
#include <render.h>
#include <render3d.h>
//#define LOG_OPENGL_CONTEXT
//#define ENABLE_ON_DC_LAYERED hDCFakeWindow
//#define ENABLE_ON_DC_LAYERED hDCFakeWindow
#define ENABLE_ON_DC_NORMAL hDCOutput

//#define ENABLE_ON_DC hDCBitmap
//#define ENABLE_ON_DC hDCOutput
//hDCBitmap

#include "local.h"
RENDER_NAMESPACE



void KillGLWindow( void )
{
   // do something here...
}

extern RENDER3D_INTERFACE Render3d;


int SetActiveD3DDisplayView( struct display_camera *hVideo, int nFracture )
{
	static CRITICALSECTION cs;
	static struct display_camera *_hVideo; // last display with a lock.
	if( hVideo )
	{
		if( nFracture )
		{
			nFracture -= 1;
		}
		else
		{
			_hVideo = hVideo;
			Render3d.current_device = hVideo->d3ddev;
			if( ! hVideo->d3ddev )
			{
				LeaveCriticalSec( &cs );
				return FALSE;
			}
			Render3d.current_device->BeginScene();    // begins the 3D scene
		}
	}
	else
	{
		if( _hVideo )
		{
#ifdef LOG_OPENGL_CONTEXT
			lprintf( "Prior GL Context being released." );
#endif
			//lprintf( "swapping buffer..." );
			Render3d.current_device->EndScene();    // ends the 3D scene

			Render3d.current_device->Present(NULL, NULL, NULL, NULL);   // displays the created frame on the screen
			Render3d.current_device = NULL;

		}
		LeaveCriticalSec( &cs );
	}
	return TRUE;
}

int SetActiveD3DDisplay( struct display_camera *hVideo )
{
   return SetActiveD3DDisplayView( hVideo, 0 );
}


void EndActive3D( struct display_camera *camera ) // does appropriate EndActiveXXDisplay
{
	SetActiveD3DDisplay( NULL );
}


//----------------------------------------------------------------------------

static int CreatePartialDrawingSurface (PVIDEO hVideo, int x, int y, int w, int h )
{
	int nFracture = -1;
	// can use handle from memory allocation level.....
	if (!hVideo)         // wait......
		return FALSE;
	//lprintf( WIDE("And here I might want to update the video, hope someone else does for me.") );
	return nFracture + 1;
}



#define MODE_UNKNOWN 0
#define MODE_PERSP 1
#define MODE_ORTHO 2
static int mode = MODE_UNKNOWN;


static void BeginVisPersp( struct display_camera *camera )
{
	D3DXMATRIX  dmx;
	/* init fProjection */
	//D3DXMatrixPerspectiveFovLH( &dmx, 90.0/180.0*3.1415926, camera->aspect, 1.0f, 30000.0f );
	MygluPerspective(90.0f,camera->aspect,1.0f,30000.0f);
	{
		int n;
		for( n = 0; n < 16; n++ )
			dmx.m[0][n] = l.fProjection[0][n];
	}
	//D3DXMatrixPerspectiveLH( &dmx, camera->hVidCore->pWindowPos.cx, camera->hVidCore->pWindowPos.cy, 0.1f, 30000.0f );
	camera->d3ddev->SetTransform( D3DTS_PROJECTION, &dmx );

	{
		D3DXMATRIX dmx2;
		D3DXMatrixIdentity( &dmx2 );
		camera->d3ddev->SetTransform( D3DTS_VIEW, &dmx2 );
	}
}


int Init3D( struct display_camera *camera )										// All Setup For OpenGL Goes Here
{
	if( !SetActiveD3DDisplay( camera ) )  // BeginScene()
		return FALSE;

	BeginVisPersp( camera );

	Render3d.current_device->Clear(0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER
											, D3DCOLOR_XRGB(0, 40, 100)
											, 1.0f
											, 0);

	/* clear scene, do enable context to simplify renderloop */
	return TRUE;
}

void SetupPositionMatrix( struct display_camera *camera )
{
	// camera->origin_camera is valid eye position matrix
}

void DisableD3d( struct display_camera *camera )
{
}

int EnableD3d( struct display_camera *camera )
{
    camera->d3d = Direct3DCreate9(D3D_SDK_VERSION);    // create the Direct3D interface

    D3DPRESENT_PARAMETERS d3dpp;    // create a struct to hold various device information

    ZeroMemory(&d3dpp, sizeof(d3dpp));    // clear out the struct for use
    d3dpp.Windowed = TRUE;    // program windowed, not fullscreen
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;    // discard old frames
	//d3dpp.SwapEffect = D3DSWAPEFFECT_COPY;
	d3dpp.hDeviceWindow = camera->hVidCore->hWndOutput;    // set the window to be used by Direct3D

    d3dpp.BackBufferFormat = D3DFMT_A8R8G8B8;    // set the back buffer format to 32-bit
	d3dpp.BackBufferWidth = camera->w;    // set the width of the buffer
    d3dpp.BackBufferHeight = camera->h;    // set the height of the buffer

	d3dpp.EnableAutoDepthStencil = TRUE;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D16;

    // create a device class using this information and information from the d3dpp stuct
    camera->d3d->CreateDevice(D3DADAPTER_DEFAULT,
                      D3DDEVTYPE_HAL,
					  camera->hWndInstance,
					  D3DCREATE_HARDWARE_VERTEXPROCESSING,
                      &d3dpp,
                      &camera->d3ddev);

	camera->hVidCore->flags.bD3D = 1;
	return TRUE;
}


int EnableOpenD3dView( struct display_camera *camera, int x, int y, int w, int h )
{

	// enable a partial opengl area on a single window surface
	// actually turns out it's just a memory context anyhow...
	int nFracture;

	if( !camera->hVidCore->flags.bD3D )
	{
		if( !EnableD3d( camera ) )
         return 0;
	}
	//nFracture = CreatePartialDrawingSurface( camera, x, y, w, h );
	nFracture = 0;
	if( nFracture )
	{
		nFracture -= 1;
		return nFracture + 1;
	}
	return 0;
}



RENDER_NAMESPACE_END

#endif