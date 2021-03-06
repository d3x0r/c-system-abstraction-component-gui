#include <stdhdrs.h>
#include <network.h>
#include <idle.h>
#define SACK_WEBSOCKET_CLIENT_SOURCE
#define WEBSOCKET_COMMON_SOURCE
#include "html5.websocket.common.h"

void SendWebSocketMessage( PCLIENT pc, int opcode, int final, int do_mask, P_8 payload, size_t length )
{
	P_8 msgout;
	P_8 use_mask;
	_32 zero = 0;
	size_t length_out = length + 2; // minimum additional is the opcode and tiny payload length (2 bytes)
	if( ( opcode & 8 ) && ( length > 125 ) )
	{
		lprintf( WIDE("Invalid send, control packet with large payload. (opcode %d  length %") _size_f WIDE(")"), opcode, length );
		return;
	}

	if( length > 125 )
	{
		if( length > 32767 )
		{
			if( 1 ) // auto fragment large packets
			{
				int block;
				for( block = 0; block < ( length / 8100 ); block++ )
				{
					SendWebSocketMessage( pc, block == 0 ?opcode:0, 0, do_mask, payload + block * 8100, 8100);
				}
				SendWebSocketMessage( pc, 0, final, do_mask, payload + block * 8100, length - block * 8100 );
				return;
			}
			length_out += 8; // need 8 more bytes for a really long length
		}
		else
			length_out += 2; // need 2 more bytes for a longer length
	}
	if( do_mask )
	{
		static _32 newmask;
		newmask ^= rand() ^ ( rand() << 13 ) ^ ( rand() << 26 );
		use_mask = (P_8)&newmask;
		length_out += 4; // need 4 more bytes for the mask
	}
	else
	{
		use_mask = (P_8)&zero;
	}

	msgout = NewArray( _8, length_out );
	MemSet( msgout, 0x12345678, length_out );
	msgout[0] = (final?0x80:0x00) | opcode;
	if( length > 125 )
	{
		if( length > 32767 )
		{
			msgout[1] = 127;
#if __64__
			// size_t will be 32 bits on non 64 bit builds
			msgout[2] = (_8)(length >> 56);
			msgout[3] = (_8)(length >> 48);
			msgout[4] = (_8)(length >> 40);
			msgout[5] = (_8)(length >> 32);
#else
			msgout[2] = 0;
			msgout[3] = 0;
			msgout[4] = 0;
			msgout[5] = 0;
#endif
			msgout[6] = (_8)(length >> 24);
			msgout[7] = (_8)(length >> 16);
			msgout[8] = (_8)(length >> 8);
			msgout[9] = (_8)length;
		}
		else
		{
			msgout[1] = 126;
			msgout[2] = (_8)(length >> 8);
			msgout[3] = (_8)(length);
		}
	}
	else
		msgout[1] = length;
	if( do_mask )
	{
		int mask_offset = (length_out-length) - 4;
		msgout[1] |= 0x80;
		msgout[mask_offset+0] = (_8)(use_mask[3]);
		msgout[mask_offset+1] = (_8)(use_mask[2]);
		msgout[mask_offset+2] = (_8)(use_mask[1]);
		msgout[mask_offset+3] = (_8)(use_mask[0]);
		use_mask = msgout + mask_offset;
	}
	{
		size_t n;
		P_8 data_out = msgout + (length_out-length);
		for( n = 0; n < length; n++ )
		{
			(*data_out++) = payload[n] ^ use_mask[n&3];
		}
	}
	SendTCP( pc, msgout, length_out );
	Deallocate( P_8, msgout );
}



static void ResetInputState( WebSocketInputState websock )
{
	websock->input_msg_state = 0;
	websock->final = 0;
	websock->mask = 0;
	websock->fragment_collection_index = 0; // mask index counter
	websock->input_type = 0; // assume text input; binary will set flag opposite

	// just always process the mask key, so set it to 0 for a no-op on XOR
	websock->mask_key[0] = 0;
	websock->mask_key[1] = 0;
	websock->mask_key[2] = 0;
	websock->mask_key[3] = 0;
}



/* opcodes
      *  %x0 denotes a continuation frame
      *  %x1 denotes a text frame
      *  %x2 denotes a binary frame
      *  %x3-7 are reserved for further non-control frames
      *  %x8 denotes a connection close
      *  %x9 denotes a ping
      *  %xA denotes a pong
      *  %xB-F are reserved for further control frames
*/
void ProcessWebSockProtocol( WebSocketInputState websock, PCLIENT pc, P_8 msg, size_t length )
{
	size_t n;

	for( n = 0; n < length; n++ )
	{
		switch( websock->input_msg_state )
		{
		case 0: // opcode/final
			if( msg[n] & 0x80 )
				websock->final = 1;
			websock->opcode = ( msg[n] & 0xF );
			websock->input_msg_state++;
			break;
		case 1: // mask bit, and 7 bits of frame_length(payload)
			websock->mask = (msg[n] & 0x80) != 0;
			websock->frame_length = (msg[n] & 0x7f );

			// control opcodes are limited to the one byte size limit; they can never be encoded with extended payload length
			if( websock->opcode & 0x8 )
			{
				if( websock->frame_length > 125 )
				{
					lprintf( WIDE("Bad length of control packet: %")_size_f, length );
					// RemoveClient( websock->pc );
					ResetInputState( websock );
					// drop the rest of the data, maybe the beginning of the next packet will make us happy
					return;
				}
			}

			if( websock->frame_length == 126 )
			{
				websock->frame_length = 0;
				websock->input_msg_state = 2;
			}
			else if( websock->frame_length == 127 )
			{
				websock->frame_length = 0;
				websock->input_msg_state = 4;
			}
			else
			{
				if( websock->mask )
					websock->input_msg_state = 12;
				else
					websock->input_msg_state = 16;
			}
			break;

		case 2: // byte 1, extended payload _16
			websock->frame_length = msg[n] << 8;
			websock->input_msg_state++;
			break;
		case 3: // byte 1, extended payload _16
			websock->frame_length |= msg[3];

			if( websock->mask )
				websock->input_msg_state = 12;
			else
				websock->input_msg_state = 16;
			break;

		case 4: // byte 1, extended payload _64
		case 5: // byte 2, extended payload _64
		case 6: // byte 3, extended payload _64
		case 7: // byte 4, extended payload _64
		case 8: // byte 5, extended payload _64
		case 9: // byte 6, extended payload _64
		case 10: // byte 7, extended payload _64
			websock->frame_length |= msg[n] << ( ( 11 - websock->input_msg_state ) * 8 );
			websock->input_msg_state++;
			break;
		case 11: // byte 8, extended payload _64
			websock->frame_length |= msg[n];

			if( websock->mask )
				websock->input_msg_state++;
			else
				websock->input_msg_state = 16;
			break;

		case 12: // mask data byte 1
		case 13: // mask data byte 2
		case 14: // mask data byte 3
		case 15: // mask data byte 4
			websock->mask_key[websock->input_msg_state-12] = msg[n];
			websock->input_msg_state++;
			break;

		case 16: // extended data or application data byte 1.
			// might have already collected fragments (non final packets, so increase the full buffer )
			// first byte of data, check we have enough room for the remaining bytes; the frame_length is valid now.
			if( websock->fragment_collection_avail < ( websock->fragment_collection_length + websock->frame_length ) )
			{
				P_8 new_fragbuf;
				websock->fragment_collection_avail += websock->frame_length;
				new_fragbuf = (P_8)Allocate( websock->fragment_collection_avail );
				if( websock->fragment_collection_length )
					MemCpy( new_fragbuf, websock->fragment_collection, websock->fragment_collection_length );
				Deallocate( P_8, websock->fragment_collection );
				websock->fragment_collection = new_fragbuf;
				websock->fragment_collection_index = 0; // start with mask byte 0 on this new packet
			}
			websock->input_msg_state++;
			// fall through, no break statement; add the byte to the buffer
		case 17:
			// if there was no data, then there's nothing to demask
			if( websock->fragment_collection )
			{
				if( websock->fragment_collection_length >= websock->fragment_collection_avail )
				{
					int a  =3 ;;
				}
				websock->fragment_collection[websock->fragment_collection_length++]
					= msg[n] ^ websock->mask_key[(websock->fragment_collection_index++) % 4];
			}

			// if final packet, and we have all the bytes for this packet
			// dispatch the opcode.
			if( websock->final && ( websock->fragment_collection_length == websock->frame_length ) )
			{

				//lprintf( WIDE("Final: %d  opcode %d  mask %d length %Ld ")
				//		 , websock->final, websock->opcode, websock->mask, websock->frame_length );
				websock->last_reception = timeGetTime();

				switch( websock->opcode )
				{
				case 0x02: //binary
					websock->input_type = 1;
				case 0x01: //text
				case 0x00: // continuation
					/// single packet, final...
					//LogBinary( websock->fragment_collection, websock->fragment_collection_length );
					if( websock->on_event )
						websock->on_event( pc, websock->psv_open, websock->fragment_collection, websock->fragment_collection_length );
					websock->fragment_collection_length = 0;
					break;
				case 0x08: // close
					// close may have app data with a reason.
					// if it has a reason, then the first two bytes are a code
					//  1000 - normal
					//  1001 - end point going away (page close, server shutdown)
					//  1002 - termination from protocol error
					//  1003 - binary/text mismatch (if supported)
					// 1004 - reserved
					// 1005 - No status code (reserved, must not send in close)
					// 1006 - reserved for not in clude; connection terminated unexpected ( guess there are local-only, not sent)
					// 1007 - inconsistant data for data in message
					// 1008 - ereceived a message that violates policy (generic message for nothing better)
					// 1009 - message too big
					// 1010 - server did not negotiate extension
					// 1011 - excpetion in handling message.
					// 1015 - reservd, must not send in close; failure to perform TLS handshake (or bad server verification)
					//  0-999 = not used;
					// 1000-2999 - reserved for this protocol, reserved for specification
					// 3000-3999 - libarry/framework/application.  Registered with IANA.  Defined by protocol.
					// 4000-4999 - reserved for private use; cannot be registerd;
					if( !websock->flags.closed )
					{
						struct web_socket_output_state *output = (struct web_socket_output_state *)GetNetworkLong(pc, 1);
						SendWebSocketMessage( pc, 0x08, 1, output->flags.expect_masking, websock->fragment_collection, websock->frame_length );
						websock->flags.closed = 1;
					}
					if( websock->on_close )
						websock->on_close( pc, websock->psv_open );
					websock->fragment_collection_length = 0;
					break;
				case 0x09: // ping
					{
						struct web_socket_output_state *output = (struct web_socket_output_state *)GetNetworkLong(pc, 1);
						SendWebSocketMessage( pc, 0x0a, 1, output->flags.expect_masking, websock->fragment_collection, websock->frame_length );
						websock->fragment_collection_length = 0;
					}
					break;
				case 0x0A: // pong
					{
						// this is for the ping routine to wait (or rather to end wait)
						websock->flags.received_pong = 1;
					}
					websock->fragment_collection_length = 0;
					break;
				default:
					lprintf( WIDE("Bad WebSocket opcode: %d"), websock->opcode );
					return;
				}
				// after processing any opcode (this is IN final, and length match) we're done, start next message
				ResetInputState( websock );
			}
			break;
		}
	}
}

void WebSocketPing( PCLIENT pc, _32 timeout )
{
	_32 start_at = timeGetTime();
	_32 target = start_at + timeout;
	_32 now;
	struct web_socket_input_state *input_state = (struct web_socket_input_state*)GetNetworkLong( pc, 2 );
	struct web_socket_output_state *output = (struct web_socket_output_state *)GetNetworkLong(pc, 1);
	SendWebSocketMessage( pc, 9, 1, output->flags.expect_masking, NULL, 0 );

	while( !input_state->flags.received_pong
			&& ( ( ( now=timeGetTime() ) - start_at ) < timeout ) )
		IdleFor( target-now );
	input_state->flags.received_pong = 0;
}


// there is a control bit for whether the content is text or binary or a continuation
void WebSocketSendText( PCLIENT pc, POINTER buffer, size_t length ) // UTF8 RFC3629
{
	struct web_socket_output_state *output = (struct web_socket_output_state *)GetNetworkLong(pc, 1);
	if( output )
	{
#ifdef _UNICODE
		char *outbuf = WcharConvertExx( (CTEXTSTR)buffer, length DBG_SRC );
		int real_len = CStrLen( outbuf );
		//lprintf( WIDE( "send %s"), buffer );
		SendWebSocketMessage( pc, output->flags.sent_type?0:1, 1, output->flags.expect_masking, (P_8)outbuf, length );
		Deallocate( char *, outbuf );
#else
		SendWebSocketMessage( pc, output->flags.sent_type?0:1, 1, output->flags.expect_masking, (P_8)buffer, length );
#endif

		output->flags.sent_type = 0;
	}
}

// there is a control bit for whether the content is text or binary or a continuation
void WebSocketBeginSendText( PCLIENT pc, POINTER buffer, size_t length ) // UTF8 RFC3629
{
   struct web_socket_output_state *output = (struct web_socket_output_state *)GetNetworkLong(pc, 1);
   SendWebSocketMessage( pc, 1, 0, output->flags.expect_masking, (P_8)buffer, length );
   output->flags.sent_type = 1;

}

// literal binary sending; this may happen to be base64 encoded too
void WebSocketSendBinary( PCLIENT pc, POINTER buffer, size_t length )
{
   struct web_socket_output_state *output = (struct web_socket_output_state *)GetNetworkLong(pc, 1);
	SendWebSocketMessage( pc, output->flags.sent_type?0:2, 1, output->flags.expect_masking, (P_8)buffer, length );
}

// literal binary sending; this may happen to be base64 encoded too
void WebSocketBeginSendBinary( PCLIENT pc, POINTER buffer, size_t length )
{
   struct web_socket_output_state *output = (struct web_socket_output_state *)GetNetworkLong(pc, 1);
   SendWebSocketMessage( pc, 2, 0, output->flags.expect_masking, (P_8)buffer, length );
   output->flags.sent_type = 1;
}




