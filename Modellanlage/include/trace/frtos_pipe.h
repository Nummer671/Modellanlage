// frtos_pipe.h - Header: Windows pipes

#define NAME_PIPE_OUT "\\\\.\\pipe\\frtos_logout"
#define NAME_PIPE_IN  "\\\\.\\pipe\\frtos_cmdin"

#define RET_PIPE_OK 0
#define RET_PIPE_FAIL -1

int pipeInit( void );					// initialize pipes
int pipeWrite( const char * buffer, const unsigned int size );
int pipeRead( char * byte );

// EOF frtos_pipe.h
