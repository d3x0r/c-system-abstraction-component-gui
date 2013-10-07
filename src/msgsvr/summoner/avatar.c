#include <stdhdrs.h>
#include <network.h>
#include <sharemem.h>
#include <controls.h>
#include <timers.h>

#include "avatar_prot.h"
#include "resource.h"

//enum {
//	LST_TASKS = 1000
//		, BTN_STOP
//      , BTN_START
//} control_ids;


typedef struct network_data_tag
{
   PCLIENT pc; // self reference...
	_32 *msg; // default to 32 bit message elements...
   _32 toread; // amount of data to read for next read
	_32 command;
	_32 state;
	_32 task_id;
   _32 system_id;
   PCOMMON list; // quick handle to the listbox...
   PLISTITEM pliSystem;// system's list item..
   PLIST tasks;
} NETWORK_DATA, *PNETWORK_DATA;

typedef struct taskinfo_tag
{
	struct {
		_32 create : 1; // set to create in the server...
		_32 sync : 1; // set to update the summoner with this
	} flags;
   _32 id;
	char *name;
	char *state;
	PLIST depends;
   PLISTITEM pli;

	/********
	 * this info reflects what the summoner knows
	 * this allows the avatar to create and edit existing tasks
    ********/
	_32 ready_timeout; // how much time before the task is assumed ready
	_32 launch_width, launch_height; // special characteristics like height/width

	char *pTask; // the program itself
	char *pPath; // the path where it starts
	char **pArgs; // arguments to pass to it

} TASKINFO, *PTASKINFO;

typedef struct itemdata_tag
{
	struct {
		_32 bSystem : 1;
		_32 bTask : 1;
	} flags;
	//union
	//{
		PNETWORK_DATA pnd;  // system ID
		PTASKINFO task;
	//};
} ITEMDATA, *PITEMDATA;

typedef struct local_tag
{
	struct {
		_32 bDefineMe : 1;
	} flags;
	TEXTSTR addr;
	PLIST pSummoners;
   PCLIENT pDiscoverClient;
	POINTER buffer;
   PLIST tasks;
   PLISTITEM pli_current;
   int bOkay, bDone;
   PCOMMON pFrame;
} LOCAL;

static LOCAL l;

//---------------------------------------------------------------------

static void GetTaskDepends( PCLIENT pSummoner, PTASKINFO task )
{
	if( task )
	{
		struct {
			_32 op;
			_32 task_id;
		} msg;
		msg.op = LIST_TASK_DEPENDS;
		msg.task_id = task->id;
      lprintf( WIDE("Requesting dependancies for %")_32f WIDE(""), task->id );
		SendTCP( pSummoner, &msg, sizeof( msg ) );
	}
}


//---------------------------------------------------------------------

static void GetTaskList( PCLIENT pSummoner )
{
	_32 op = LIST_TASKS;
	SendTCP( pSummoner, &op, sizeof( op ) );
}


//---------------------------------------------------------------------------

// return a ppStr - releasing a prior if passed in.
char **MakeTaskArguments( char * *pArgs, char * progname, char * args )
{
	char * p;
	char **pp;
	pp = pArgs;
	while( pp && pp[0] )
	{
		Release( (POINTER)pp[0] );
		pp++;
	}
	Release( (POINTER)pArgs );

	{
		int count = 0;
		int lastchar;
		lastchar = ' '; // auto continue spaces...
		//lprintf( WIDE("Got args: %s"), args );
		p = args;
		while( p && p[0] )
		{
			if( !count ) // 1 at least...
				count++;
			if( lastchar != ' ' && p[0] == ' ' ) // and there's a space
				count++;

			lastchar = p[0];
			p++;
		}
		if( count )
		{
			char * start;
			lastchar = ' '; // auto continue spaces...
			pp = pArgs = (char**)Allocate( sizeof( char * ) * ( count + 2 ) );
			p = args;
			count = 0;
			pp[count++] = StrDup( progname  ); // setup arg to equal program (needed for linux stuff)
			start = p;
			while( p[0] )
			{
				//lprintf( WIDE("check character %c %c"), lastchar, p[0] );
				lastchar = p[0];
				if( lastchar != ' ' && p[0] == ' ' ) // and there's a space
				{
               int len;
					pp[count] = (char *)Allocate( len = (strlen( start ) + 1) );
					MemCpy( (TEXTSTR)pp[count], start, len-1 );
               pp[count++][len-1] = 0;
               start = NULL;
				}
				else if( lastchar == ' ' && p[0] != ' ' )
				{
					start = p;
				}
				p++;
			}
			//lprintf( WIDE("Setting arg %d to %s"), count, start );
			if( start )
				pp[count++] = StrDup( start );
			pp[count] = NULL;

		}
		else
			pArgs = NULL;
	}
   return 0;
}


//---------------------------------------------------------------------------

void EditTask( PSI_CONTROL parent_frame, PTASKINFO pTask )
{
	PCOMMON frame = LoadXMLFrame( WIDE("frames/CreateTask.frame") );
//cpg27dec2006 c:\work\sack\src\msgsvr\summoner\avatar.c(188): Warning! W202: Symbol 'created' has been defined, but not referenced
//cpg27dec2006    int created = 0;
	int okay = 0;
	int done = 0;
	char menuname[256];
	if( !pTask )
	{
		pTask = (PTASKINFO)Allocate( sizeof( *pTask ) );
		MemSet( pTask, 0, sizeof( *pTask ) );
      pTask->flags.create = 1; // comes from self, not summoner
      AddLink( &l.tasks, pTask );
	}
	{
		SetCommonButtons( frame, &done, &okay );
		SetControlText( GetControl( frame, TXT_CONTROL_TEXT ), pTask->name );
		SetControlText( GetControl( frame, TXT_TASK_NAME ), pTask->pTask );
		SetControlText( GetControl( frame, TXT_TASK_PATH ), pTask->pPath );
//cpg27dec2006 c:\work\sack\src\msgsvr\summoner\avatar.c(204): Warning! W1179: Parameter 1, type qualifier mismatch
//cpg27dec2006 c:\work\sack\src\msgsvr\summoner\avatar.c(204): Note! N2003: source conversion type is 'char **'
//cpg27dec2006 c:\work\sack\src\msgsvr\summoner\avatar.c(204): Note! N2004: target conversion type is 'char const *const *'
//cpg27dec2006 c:\work\sack\src\msgsvr\summoner\avatar.c(204): Note! N2002: 'GetArgsString' defined in: c:\work\sack\include\system.h(60)
		SetControlText( GetControl( frame, TXT_TASK_ARGS ), GetArgsString(pTask->pArgs) );
		{
			PSI_CONTROL list = GetControl( frame, LIST_TASKS );
			if( list )
			{
			}
         list = GetControl( frame, LIST_REQUIRED );
		}
	}
	DisplayFrameOver( frame, parent_frame );
   EditFrame( frame, TRUE );
	CommonWait( frame );
   lprintf( WIDE("Wait complete... %d %d"), okay, done );
	if( okay )
	{
      char args[256];
		// Get info from dialog...
		GetControlText( GetControl( frame, TXT_TASK_NAME ), pTask->pTask, sizeof( pTask->pTask ) );
		GetControlText( GetControl( frame, TXT_TASK_PATH ), pTask->pPath, sizeof( pTask->pPath ) );
		GetControlText( GetControl( frame, TXT_CONTROL_TEXT ), menuname, sizeof( menuname ) );
      if( pTask->name ) Release( pTask->name );
      pTask->name = StrDup( menuname );

		GetControlText( GetControl( frame, TXT_TASK_ARGS )
						  , args, sizeof( args ) );
		pTask->pArgs = MakeTaskArguments( pTask->pArgs, pTask->pTask, args );
	}
	else
	{
		//if( created )
      //   DestroyTask( &pTask );
	}
	DestroyFrame( &frame );
}



void CPROC SummonerReadComplete( PCLIENT pc, POINTER buffer, int nLen )
{
	// always get this member...
   // first read this should also be NULL like buffer
	PNETWORK_DATA pnd = (PNETWORK_DATA)GetNetworkLong( pc, 0 );
	if( !buffer )
	{
		{  // also setup private network-data structure
			pnd = (PNETWORK_DATA)Allocate( sizeof( NETWORK_DATA ) );
			MemSet( pnd, 0, sizeof( NETWORK_DATA ) );
			SetNetworkLong( pc, 0, (PTRSZVAL)pnd );
         pnd->pc = pc;
         pnd->list = GetControl( l.pFrame, LST_TASKS );
			pnd->pliSystem = AddListItem( pnd->list
												 , WIDE("System ....\t???") );
			{
				PITEMDATA pid = (PITEMDATA)Allocate( sizeof( ITEMDATA ) );
				pid->flags.bTask = 0;
				pid->flags.bSystem = 1;
				pid->pnd = pnd;
            SetItemData( pnd->pliSystem, (PTRSZVAL)pid );
			}
		}
		pnd->msg = (_32*)Allocate( 4096 );
      pnd->state = NET_STATE_RESET;
      pnd->toread = 4;
	}
	else
	{
		lprintf( "Received..." );
      LogBinary( (P_8)buffer, nLen );
	process_new_state:
		switch( pnd->state )
		{
		case NET_STATE_RESET:
			{
				pnd->state = NET_STATE_COMMAND;
				// because of the rarety of this operation,
				// having a loop or other control variable is somewhat
				// useless overhead, for following 'rules'
				goto process_new_state;
			}
		case NET_STATE_COMMAND:
			{
				pnd->command = pnd->msg[0];
				// on new command... do we need more data?
				// or do we perform an action ?
            lprintf( "Command received..." );
				switch( pnd->command )
				{
				case SYSTEM_STATUS:
               lprintf( WIDE("Get sytem status..") );
               pnd->state = NET_STATE_GET_SYSTEM_STATUS;
               pnd->toread = 8; // system_id, status
               break;
				case TASK_NAME:
               lprintf( "Read name" );
					pnd->state = NET_STATE_GET_TASKNAME_LENGTH;
					pnd->toread = 8; // TaskID, namelen
					break;
				case TASK_LIST_DONE:
               // task list is done, now get more information, such as dependancies
					{
						INDEX idx;
						PTASKINFO pti;
						LIST_FORALL( pnd->tasks, idx, PTASKINFO, pti )
                     GetTaskDepends( pc, pti );
					}
				   // no more, that's alright, don't really care yet.
					break;
				case TASK_DEPEND:
					pnd->toread = 8; // TaskID, Count
               pnd->state = NET_STATE_GET_DEPEND_COUNT;
               break;
				}
			}
			break;
		case NET_STATE_GET_SYSTEM_STATUS:
			{
				char line[80];
            pnd->system_id = pnd->msg[0];
				snprintf( line, sizeof( line ), WIDE("System %")_32f WIDE("\t%s")
						  , pnd->system_id // system_id
						  , (pnd->msg[1] & 1)?"Suspended":"Active"); // status
				SetItemText( pnd->pliSystem, line );
            // enque next state command.
				GetTaskList( pc );
				pnd->state = NET_STATE_COMMAND;
            pnd->toread = 4;
			}
         break;
		case NET_STATE_GET_DEPEND_COUNT:
         pnd->task_id = pnd->msg[0];
			pnd->toread = 4 * pnd->msg[1];
			lprintf( WIDE("Depends for %")_32f WIDE(" == %")_32f WIDE(":%")_32f WIDE(""), pnd->task_id, pnd->msg[1], pnd->toread / 4);
         pnd->state = NET_STATE_GET_DEPEND_DATA;
			break;
		case NET_STATE_GET_DEPEND_DATA:
			{
				INDEX idx;
            PTASKINFO pti = (PTASKINFO)GetLink( &pnd->tasks, pnd->task_id );
            INDEX *deps = (INDEX*)buffer;
								// toready will still ahve been set from GET_DEPEND_COUNT
            lprintf( WIDE("Got depends for task..") );
				for( idx = 0; idx < pnd->toread / 4; idx++ )
				{
               char line[80];
					PTASKINFO ptiDepend = (PTASKINFO)GetLink( &pnd->tasks, deps[idx] );
					AddLink( &pti->depends, ptiDepend );
					snprintf( line, sizeof( line ), WIDE("%s"), ptiDepend->name );
               lprintf( WIDE("Taskname %s"), ptiDepend->name );
               InsertListItemEx( pnd->list, pti->pli, 2, line );
				}
			}
			pnd->state = NET_STATE_RESET;
         pnd->toread = 4;
         break;
		case NET_STATE_GET_TASKNAME_LENGTH:
			{
            pnd->task_id = pnd->msg[0];
				pnd->toread = pnd->msg[1];
            lprintf( WIDE("Got lengths %")_32f WIDE(",%")_32f WIDE(""), pnd->task_id, pnd->toread );
				pnd->state = NET_STATE_GET_TASKNAME_DATA;
				break;
			}
		case NET_STATE_GET_TASKNAME_DATA:
			switch( pnd->command )
			{
			case TASK_NAME:
				{
               PTASKINFO pti = (PTASKINFO)Allocate( sizeof( TASKINFO ) );
					TEXTSTR line = (TEXTSTR)pnd->msg;
               char *p;
					line[nLen] = 0; // make sure there's a nul terminator
					lprintf( "Add task name" );
					pti->pli = InsertListItemEx( pnd->list, pnd->pliSystem, 1, line );
					{
						PITEMDATA pid = (PITEMDATA)Allocate( sizeof( ITEMDATA ) );
						pid->flags.bTask = 1;
						pid->flags.bSystem = 0;
                  pid->pnd = pnd;
						pid->task = pti;
						SetItemData( pti->pli, (PTRSZVAL)pid );
					}

					p = strchr( line, '\t' );
					if( p )
                  p[0] = 0;
					pti->name = StrDup( line );
					if( p )
						pti->state = StrDup( p + 1 );
               pti->id = pnd->task_id;
               pti->depends = NULL;
					SetLink( &pnd->tasks, pnd->task_id, pti );
					pnd->state = NET_STATE_RESET;
					pnd->toread = 4;
				}
            break;
			}
         break;
		}
	}
   ReadTCPMsg( pc, pnd->msg, pnd->toread );
}

//---------------------------------------------------------------------

void CPROC SummonerClose( PCLIENT pc )
{

}

//---------------------------------------------------------------------

void CPROC StopThing( PTRSZVAL psv, PCOMMON pc )
{
	PITEMDATA pid = (PITEMDATA)GetItemData( l.pli_current );
	if( pid )
	{
		if( pid->flags.bSystem )
		{
			_32 op;
			op = SUMMONER_SYSTEM_STOP;
         SendTCP( pid->pnd->pc, &op, sizeof( op ) );
		}
		else if( pid->flags.bTask )
		{
			struct {
				_32 op;
				_32 task_id;
			}msg;

			msg.op = SUMMONER_TASK_STOP;
         msg.task_id = pid->task->id;
         SendTCP( pid->pnd->pc, &msg, sizeof( msg ) );
		}
	}
}

//---------------------------------------------------------------------

void CPROC SuspendThing( PTRSZVAL psv, PCOMMON pc )
{
	PITEMDATA pid = (PITEMDATA)GetItemData( l.pli_current );
	if( pid )
	{
		if( pid->flags.bSystem )
		{
			_32 op;
			op = SUMMONER_SYSTEM_SUSPEND;
         SendTCP( pid->pnd->pc, &op, sizeof( op ) );
		}
		else if( pid->flags.bTask )
		{
			struct {
				_32 op;
				_32 task_id;
			}msg;

			msg.op = SUMMONER_TASK_SUSPEND;
         msg.task_id = pid->task->id;
         SendTCP( pid->pnd->pc, &msg, sizeof( msg ) );
		}
	}
}

//---------------------------------------------------------------------

void CPROC StartThing( PTRSZVAL psv, PCOMMON pc )
{
	PITEMDATA pid = (PITEMDATA)GetItemData( l.pli_current );
	if( pid )
	{
		if( pid->flags.bSystem )
		{
			_32 op;
			op = SUMMONER_SYSTEM_RESUME;
         SendTCP( pid->pnd->pc, &op, sizeof( op ) );
		}
		else if( pid->flags.bTask )
		{
			struct {
				_32 op;
				_32 task_id;
			}msg;

			msg.op = SUMMONER_TASK_START;
         msg.task_id = pid->task->id;
         SendTCP( pid->pnd->pc, &msg, sizeof( msg ) );
		}
	}
}

//---------------------------------------------------------------------

int OpenSummoner( char *addr )
{
   PCLIENT pSummoner;
	{
		if( !addr )
         addr = "127.0.0.1";
		pSummoner = OpenTCPClientEx( addr, 3015, SummonerReadComplete, SummonerClose, NULL );
		if( pSummoner )
		{
			AddLink( &l.pSummoners, pSummoner );
			// this tests outgoing commands...
         // on connect, a series of commands is also received...
			return TRUE;
		}
	}
   return FALSE;
}

//---------------------------------------------------------------------

void CPROC TaskSelectionChanged( PTRSZVAL psv, PCOMMON pcList, PLISTITEM pli )
{
// update task information
   l.pli_current = pli;
}

void CPROC CreateTask( PTRSZVAL psv, PSI_CONTROL button )
{
   EditTask( button, NULL );
}

//---------------------------------------------------------------------

int OpenDialog( void )
{
//cpg27dec2006 c:\work\sack\src\msgsvr\summoner\avatar.c(536): Warning! W202: Symbol 'pc' has been defined, but not referenced
//cpg27dec2006 	l.bDone = 0;
	l.bOkay = 0;
	l.pFrame = LoadXMLFrame( "SummonerControl.Frame" );
	if( l.pFrame )
	{
//cpg27dec2006 c:\work\sack\src\msgsvr\summoner\avatar.c(543): Warning! W202: Symbol 'pc' has been defined, but not referenced
//cpg27dec2006 		PCOMMON pc;
		//pc = MakeControl( l.pFrame, LISTBOX_CONTROL, 5, 5, 400, 310, LST_TASKS );
      SetListboxIsTree( GetControl( l.pFrame, LST_TASKS), TRUE );
		{
			int stops[3];
			stops[0] = 0;
			stops[1] = 200;
         stops[2] = 250;
			SetListBoxTabStops( GetControl( l.pFrame, LST_TASKS), 3, stops );
		}
      SetCommonButtons( l.pFrame, &l.bDone, &l.bOkay );
		SetSelChangeHandler( GetControl( l.pFrame, LST_TASKS), TaskSelectionChanged, 0 );
		SetButtonPushMethod( GetControl( l.pFrame, BTN_STOP), StopThing, 0 );
      SetButtonPushMethod( GetControl( l.pFrame, BTN_SUSPEND), SuspendThing, 0 );
      SetButtonPushMethod( GetControl( l.pFrame, BTN_START), StartThing, 0 );
		SetButtonPushMethod( GetControl( l.pFrame, BTN_CREATE_TASK), CreateTask, 0 );

      return TRUE;
	}
   return FALSE;
}

//---------------------------------------------------------------------

int main( int argc, char **argv )
{
	int nArg;
   NetworkWait(NULL,16,32);
	if( !OpenDialog() )
	{
		fprintf( stderr, WIDE("Failed to open the interface dialog?!\n") );
      return 1;
	}
	for( nArg = 1; nArg < argc; nArg++ )
	{
		if( argv[nArg][0] == '-' )
		{
		}
		else
		{
         OpenSummoner( argv[nArg] );
		}
	}
	//if( !OpenSummoner() )
	//{
	//	fprintf( stderr, WIDE("Failed to connect to summoner at %s\n"), l.addr );
   //   return 1;
	//}
	{
		INDEX idx;
		PCLIENT pc;
      LIST_FORALL( l.pSummoners, idx, PCLIENT, pc )
		{
		}
	}

	DisplayFrame( l.pFrame );
	EditFrame( l.pFrame, TRUE );
	CommonWait( l.pFrame );
	DestroyFrame( &l.pFrame );
   return 0;
}


PRELOAD( register_control_ids )
{
SimpleRegisterResource( BTN_STOP, NORMAL_BUTTON_NAME );
SimpleRegisterResource(BTN_START,NORMAL_BUTTON_NAME)   ;
SimpleRegisterResource(BTN_SUSPEND,NORMAL_BUTTON_NAME)  ;
SimpleRegisterResource( LST_TASKS    , LISTBOX_CONTROL_NAME );
SimpleRegisterResource( BTN_CREATE_TASK    , NORMAL_BUTTON_NAME );
SimpleRegisterResource( TXT_CONTROL_TEXT    , EDIT_FIELD_NAME )     ;
SimpleRegisterResource( TXT_TASK_NAME       , EDIT_FIELD_NAME )      ;
SimpleRegisterResource( TXT_TASK_PATH       , EDIT_FIELD_NAME )       ;
SimpleRegisterResource( TXT_TASK_ARGS       , EDIT_FIELD_NAME )        ;
}
