/***********************************************************************
   Project:

   Module:  bprint.h
   
   Purpose: header file for buffered print
            provides external declarations for buffered print functions

   Author/Group: N. Drescher

   Date: 10/23/2002

***********************************************************************/
extern char* bpinit(int bufsize);
extern int bprintf(const char * fmt, ...);
extern int bpreset();
extern void bpDelete(void);

