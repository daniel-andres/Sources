/*
 * This was automatically generated by modgen
 * version 0.2
 * module python
 * Don't edit this file
 */


#line 10 "python.cc"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <unistd.h>
#include <sys/stat.h>
#include <boost/python.hpp>
#include <Python.h>
#include <kernel/mod2.h>
#include <tok.h>
#include <structs.h>
#include <ipid.h>

#include <locals.h>
#include <omalloc.h>
#include "python.h"
#line 19 "python.cc"

idhdl enter_id(char *name, char *value, idtyp t)
{
  idhdl h;

  h=enterid(omStrDup(name),0, t, &(currPack->idroot), TRUE/*FALSE*/);
  if(h!=NULL) {
     switch(t) {
         case STRING_CMD: 
              omFree(IDSTRING(h));
              IDSTRING(h) = omStrDup(value);
              break;
         case PACKAGE_CMD: break;
         case PROC_CMD: break;
     }
  } else 
      Warn("Cannot create '%s'\n", name);
  return(h);
}

#line 37 "python.cc"

idhdl add_singular_proc(FILE* binfp, char *procname,int line,
                       long pos, long end, BOOLEAN pstatic)
{
  idhdl h;
  procinfov pi;
  char *tempstr;
  h = enter_id(procname, NULL, PROC_CMD);
  if(h == NULL) return NULL;

  pi = IDPROC(h);

  pi->libname = omStrDup(currPack->libname);
  pi->procname = omStrDup(procname);
  pi->language = LANG_SINGULAR;
  pi->ref = 1;
  pi->is_static = pstatic;
  pi->data.s.proc_start = pos;
  pi->data.s.def_end    = pos;
  pi->data.s.help_start = 0L;
  pi->data.s.help_end   = 0L;
  pi->data.s.body_start = pos;
  pi->data.s.body_end   = end;
  pi->data.s.proc_end   = end;
  pi->data.s.example_start = 0L;
  pi->data.s.proc_lineno = line;
  pi->data.s.body_lineno = line;
  pi->data.s.example_lineno = 0;
  pi->data.s.help_chksum = 0;
  
  rewind(binfp);
  fseek(binfp,pos,SEEK_CUR);
  tempstr=(char *)omAlloc(end-pos+2);
  memset(tempstr,0,end-pos+2);
  fread(tempstr,sizeof(char),end-pos+1,binfp);
  pi->data.s.body = omStrDup(tempstr);
  omFree(tempstr);

  return(h);
}

unsigned long crccheck(
  char *file
  ) 
{
  unsigned char buf[BUFLEN1];
  unsigned long crc = 0;
  long length = 0;
  long bytes_read;
  register FILE *fp;

  // open the file
  fp = fopen (file, "rb");
  if (fp == NULL) return 0;

  // read the file chunk by chunk, determine the lenght and
  // start computing the checksum
  while ((bytes_read = fread (buf, 1, BUFLEN1, fp)) > 0)
  {
    unsigned char *cp = buf;
    length += bytes_read;
    while (bytes_read--)
      crc = (crc << 8) ^ crctab[((crc >> 24) ^ *(cp++)) & 0xFF];
    }

  // check if something went wrong and close the file
  if (ferror (fp)) return 0; 
  if (fclose(fp)==EOF) return 0;

  // second part of checksum computation
  bytes_read = length;
  while (bytes_read > 0) {
    crc = (crc << 8) ^ crctab[((crc >> 24) ^ bytes_read) & 0xFF];
    bytes_read >>= 8;
  }

  crc = ~crc & 0xFFFFFFFF;

  return(crc);
}

void fill_help_package();
void fill_example_package();

#line 2 "python.mod"
/*
 *  $Id$
 *
 *  Test mod fuer modgen
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <Python.h>
#include "wrapper.h"

void mbpython(char* in);
#line 17 "python.mod"


#line 137 "python.cc"
extern "C" {
//int mod_init(
//  int (*iiAddCproc)(char *libname, char *procname, BOOLEAN pstatic,
//              BOOLEAN(*func)(leftv res, leftv v))
//  )
  int mod_init(
    SModulFunctions* psModulFunctions
    )
{
  idhdl h;
  char * tempstr;
  FILE * binfp; 
  int ret;
  struct stat sb; 

  tempstr = (char *)omAlloc(strlen(currPack->libname)+5);
  memset(tempstr,0,strlen(currPack->libname)+1);
  memcpy(tempstr,currPack->libname,strlen(currPack->libname));
  memcpy(tempstr+strlen(currPack->libname)-3,".bin",4);
  ret=stat(tempstr,&sb);
  if(ret==0) { 
    if ((sb.st_mode & S_IFMT) == S_IFREG) { 
      if (crccheck(tempstr)!=crcsum)
      {   Warn("file %s does not agree with module version - ignoring file",tempstr);
          ret=-1;
      }
      if ((binfp = fopen(tempstr,"r")) == NULL) return -1;
    } 
    else 
      ret=-1; 
  }

  fill_help_package();
  fill_example_package();

#line 20 "python.mod"
  enter_id("category","tests", STRING_CMD);
#line 22 "python.mod"
  enter_id("version","$Id$", STRING_CMD);
#line 23 "python.mod"
  enter_id("info","LIBRARY: kernel.lib  PROCEDURES OF GENERAL TYPE WRITEN IN C python(input); eval a string  in python", STRING_CMD);
#line 26 "python.mod"
#line 27 "python.mod"
  Py_Initialize();
  PyRun_SimpleString("from sys import path\n\
path.insert(0,'.')\n");
  initSingular();
  init_Singular();

#line 36 "python.mod"
  psModulFunctions->iiAddCproc(currPack->libname,"python",FALSE, mod_python);
//psModulFunctions->iiAddCproc("packagename","procname",FALSE, procedure);
  if(ret!=-1) fclose(binfp);
  return 0;
}
}

#line 187 "python.cc"
/* Help section */
void fill_help_package() {

#line 192 "python.cc"
}  /* End of Help section */

/* Example section */
void fill_example_package() {
#line 43 "python.mod"
  enter_id("python_example",
 "\
  python(\"print 1+1\");\
", STRING_CMD);


#line 206 "python.cc"
} /* End of Example section */

#line 36 "python.mod"
BOOLEAN mod_python(leftv __res, leftv __h)
{
#line 37 "python.mod"
#line 214 "python.cc"
 
#line 38 "python.mod"
#line 217 "python.cc"
  leftv __v = __h, __v_save;
  int __tok = NONE, __index = 0;
  sleftv __sa; leftv __za = &__sa;
  char * a;
#line 38 "python.mod"


#line 39 "python.mod"
#line 226 "python.cc"
  if(__v==NULL) goto mod_python_error;
  __tok = __v->Typ();
  if((__index=iiTestConvert(__tok, STRING_CMD))==0)
     goto mod_python_error;
  __v_save = __v->next;
  __v->next = NULL;
  if(iiConvert(__tok, STRING_CMD, __index, __v, __za))
     goto mod_python_error;
  __v = __v_save;
  a = (char *)__za->Data();
  if(__v!=NULL) { __tok = __v->Typ(); goto mod_python_error; }

#line 39 "python.mod"

  mbpython(a);
#line 41 "python.mod"
#line 243 "python.cc"
  __res->data = NULL;
  __res->rtyp = END_RING;
  return FALSE;

  mod_python_error:
    Werror("python(`%s`) is not supported", Tok2Cmdname(__tok));
    Werror("expected python('string')");
    return TRUE;
}


#line 254 "python.cc"
#line 53 "python.mod"
void mbpython(char* inp){
  PyRun_SimpleString(inp);
}
