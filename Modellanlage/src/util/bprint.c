/* Buffered Print Routines
* Purpose:
* Normal printf() output causes the task to be blocked
* while the data is printed. When the purpose of the
* program is to demonstrate the interplay of task priorities,
* preemption, and other blocking kernel calls, these
* buffered print routines may be used: they do not block
* the caller. Instead, all output data is copied to a
* single buffer thereby preserving the order of output but
* not perturbing the flow of execution.
*
* Usage:
* bpinit(BufSize); Acquire BufSize storage.
* bprintf(AsUsual); Add data to buffer.
* bpreset; resets buffer pointer
*
* Note:
* bpinit() and bpflush() are designed to be used ONCE,
* typically in the ROOT task during start-up or shutdown.
* bprintf(), on the other hand, may be used over and over
* by any task at any priority: it is re-entrant. (It should
* not, however, be called from ISRs.)
*/

#include "kernel/FreeRTOS.h"
#include "kernel/task.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>


static char * bstart; 		/* Buffer start address */
static int bsize = 0; 		/* Total buffer size */
static char * bptr = 0; 	/* Current buffer pointer */
static int bleft = 0; 		/* Buffer space remaining */

/* bpreset - Reset buffer pointers: Used herein only */

int bpreset()
{
  taskENTER_CRITICAL();
  bptr = bstart;
  bleft = bsize;
  *bptr = '\0';
  taskEXIT_CRITICAL();
  return(0);
}

/* bpinit - Initialize buffered print routines
* Purpose:
* Use this ONCE at the beginning of a test run to allocate
* buffer storage for bprintf() data.
*/

char* bpinit(int bufsize)
{
  bsize = bufsize;
  bstart = (char*)malloc(bsize);
  bpreset();
  return bstart;
}
/* bprintf - Add to buffered data
* Purpose:
* Use exactly like printf(): the formatted output will be
* stored in the internal buffer without blocking the task.
*/
int bprintf(const char * fmt, ...)
{
  va_list ap;
  char str[256], * cp;
  va_start(ap, fmt);
  vsprintf_s(str, 256, fmt, ap); /* Format the data */
  va_end(ap);
  cp = str;
  taskENTER_CRITICAL();
  while (*cp) {
   *bptr++ = *cp++; /* Copy data to buffer. */
   if (--bleft == 1) {
     *bptr = '\0'; /* Flush if full. */
     bpreset();
   }
  }
  *bptr = '\0';
  taskEXIT_CRITICAL();
  return(bleft);
}

void bpdelete(void)
{
	free(bstart);
	return;
}