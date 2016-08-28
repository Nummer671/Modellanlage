#ifndef _GLOBAL_SWITCH_H_
#define _GLOBAL_SWITCH_H_

#define _WIN_PIPE_					// force Windows Pipe instead of UDP

// included in "FreeRTOSConfig.h"
// included in "logTRaceMacros.h"
// included in "main.c"

#ifdef _WIN_PIPE_
  #include "frtos_pipe.h"
#endif

#endif // #ifndef _GLOBAL_SWITCH_H_
