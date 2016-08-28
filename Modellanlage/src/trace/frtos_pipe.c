#include <windows.h> 
#include <stdio.h>
#include "trace\frtos_pipe.h"
 
#define PIPE_TIMEOUT 1					// 1 ms
#define BUFSIZE_OUTB 4096				// logout instance
 
HANDLE hPipeOut = INVALID_HANDLE_VALUE, hPipeIn = INVALID_HANDLE_VALUE;
DWORD WINAPI InstanceThreadIn( LPVOID lpvParam );	// this thread handles incomming commands
#define BUFSIZE_INB  1					// cmd instance, read byte by byte
#define CMD_EMPTY 0						// no command available
static volatile char cmdFromPipe = CMD_EMPTY;	// command received from pipe


int pipeInit( void )			
{
	BOOL fConnected = FALSE; 
    HANDLE hThreadIn = NULL; 
    LPTSTR lpszPipeNameOut = TEXT( NAME_PIPE_OUT ); 
    LPTSTR lpszPipeNameIn = TEXT( NAME_PIPE_IN ); 

// create output pipe
	hPipeOut = CreateNamedPipe( 
		lpszPipeNameOut,				// pipe name 
        PIPE_ACCESS_OUTBOUND,			// write access 
        PIPE_TYPE_BYTE |				// byte-type pipe 
        PIPE_READMODE_BYTE |			// byte-read mode 
        PIPE_WAIT,						// blocking mode 
        1,								// number of instances 
        BUFSIZE_OUTB,					// output buffer size 
        0,								// input buffer size 
        PIPE_TIMEOUT,					// client time-out 
        NULL);							// default security attributes 

	if( hPipeOut == INVALID_HANDLE_VALUE ) 
    {
		printf( "Create PipeOut failed with %d.\n", GetLastError() );
        return RET_PIPE_FAIL;
    }

	hPipeIn = CreateNamedPipe( 
		lpszPipeNameIn,					// pipe name 
        PIPE_ACCESS_INBOUND,			// read access 
        PIPE_TYPE_BYTE |				// byte type pipe 
        PIPE_READMODE_BYTE |			// message-read mode 
        PIPE_WAIT,						// blocking mode 
        1,								// max. instances  
        0,								// output buffer size 
        BUFSIZE_INB,					// input buffer size 
        0,								// infinite client time-out 
        NULL );							// default security attribute 

    if( hPipeIn == INVALID_HANDLE_VALUE ) 
	{
		printf( "Create PipeIn failed with %d.\n", GetLastError() );
		CloseHandle( hPipeOut );
		hPipeOut = NULL;
        return RET_PIPE_FAIL;
    }

	printf( "Pipes created, waiting for Client connect.\n" ); 

// Wait for the client to connect; if it succeeds, the function returns a nonzero value.
// If the function returns zero, GetLastError returns ERROR_PIPE_CONNECTED. 
    fConnected = ConnectNamedPipe( hPipeIn, NULL ) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED); 
    if( fConnected ) 
    { 
		printf( "Client connected, creating a processing thread.\n" ); 
        hThreadIn = CreateThread( 
            NULL,						// no security attribute 
            0,							// default stack size 
            InstanceThreadIn,			// thread proc
            (LPVOID) hPipeIn,			// thread parameter 
            0,							// not suspended 
            NULL );						// returns thread ID 

		if( hThreadIn == NULL ) 
		{
			printf( "Create PipeInThread failed with %d.\n", GetLastError() );
			CloseHandle( hPipeOut );
			CloseHandle( hPipeIn );
			hPipeOut = INVALID_HANDLE_VALUE;
			hPipeIn = INVALID_HANDLE_VALUE;
			return RET_PIPE_FAIL;
		}
		else CloseHandle( hThreadIn ); 
    } 
    else
	{
		CloseHandle( hPipeOut );
		CloseHandle( hPipeIn );
		hPipeOut = INVALID_HANDLE_VALUE;
		hPipeIn = INVALID_HANDLE_VALUE;
        return RET_PIPE_FAIL;
	} 

// Client must connected to outbound pipe yet, check this.
    fConnected = ConnectNamedPipe( hPipeOut, NULL ) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED); 
    if( !fConnected ) 
    { 
		printf( "Client outbound not connected with %d.\n", GetLastError() );
		CloseHandle( hPipeOut );
		CloseHandle( hPipeIn );
		hPipeOut = INVALID_HANDLE_VALUE;
		hPipeIn = INVALID_HANDLE_VALUE;
        return RET_PIPE_FAIL;
	}

	return RET_PIPE_OK;
}


int pipeWrite( const char * buffer, const unsigned int size )
{
    BOOL fSuccess;
	DWORD cbWritten = 0;
	if(hPipeOut != INVALID_HANDLE_VALUE)
	{
		fSuccess = WriteFile( 
			hPipeOut,						// handle to pipe 
			buffer,							// buffer to write from 
			size,							// number of bytes to write 
			&cbWritten,						// number of bytes written 
			NULL );							// not overlapped I/O 
		if( ! fSuccess || (size != cbWritten) )
		{   
		printf( "WritePipe failed with %d.\n", GetLastError() );
			CloseHandle( hPipeOut );
			CloseHandle( hPipeIn );
			hPipeOut = INVALID_HANDLE_VALUE;
			hPipeIn = INVALID_HANDLE_VALUE;
			return RET_PIPE_FAIL;
		}
	}
	else
	{
		return RET_PIPE_FAIL;
	}
	return RET_PIPE_OK;
}


// This routine is a thread processing function to read from a client via the open pipe connection passed from the main loop.
DWORD WINAPI InstanceThreadIn( LPVOID lpvParam )
{ 
	char bufCmd[BUFSIZE_INB];
	DWORD cbBytesRead = 0;
	BOOL fSuccess = FALSE;
	HANDLE hPipe  = (HANDLE) lpvParam;	// The thread's parameter is a handle to a pipe object instance.

// Print verbose messages. In production code, this should be for debugging only.
	printf( "InstanceThread created, receiving and processing messages.\n" );

    for( ; ; ) 
    { 
		fSuccess = ReadFile( 
			hPipe,						// handle to pipe 
			bufCmd,						// buffer to receive data 
			1,							// size of buffer 
			&cbBytesRead,				// number of bytes read 
			NULL );						// not overlapped I/O 

		if( !fSuccess || (cbBytesRead == 0) )
		{   
			if( GetLastError() == ERROR_BROKEN_PIPE )
			{
				printf( "InstanceThread: PIPE BROKEN.\n" ); 
			}
			else
			{
				printf( "InstanceThread ReadFile failed with %d.\n", GetLastError() ); 
			}
			break;
		}

		while( cmdFromPipe != CMD_EMPTY ) { Sleep( 1 ); }	// sleep 1 ms, wait til command readed
		cmdFromPipe = bufCmd[0];
	}

	CloseHandle( hPipeOut );
	CloseHandle( hPipeIn );
	hPipeOut = INVALID_HANDLE_VALUE;
	hPipeIn = INVALID_HANDLE_VALUE;
	return (DWORD)RET_PIPE_FAIL;
}


int pipeRead( char * byte )
{
	int count = 0;

	if( cmdFromPipe != CMD_EMPTY )
	{
		*byte = cmdFromPipe;
		cmdFromPipe = 0;
		count++;
	}
	return count;
}

/*int main( VOID ) 
{ 
	int rc;
	char echo[] = "Echo=c\n";
	char cmd;
	rc = pipeInit();
	char logExample[] = "This is a valid log squenece\n";

	for( ; ; )
	{
		if( pipeRead( &cmd ) != 0 )
		{
			printf( "Command received %c\n", cmd );
			echo[5] = cmd;
			rc = pipeWrite( echo, sizeof( echo ) );
		}
		if( cmdFromPipe == 'e' ) { break; }
		rc = pipeWrite( logExample, sizeof( logExample ) );
	}
	CloseHandle( hPipeOut );
	CloseHandle( hPipeIn );
	return RET_PIPE_OK;
}
*/
