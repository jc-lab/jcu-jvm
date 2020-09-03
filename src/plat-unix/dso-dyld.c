/*
   Licensed to the Apache Software Foundation (ASF) under one or more
   contributor license agreements.  See the NOTICE file distributed with
   this work for additional information regarding copyright ownership.
   The ASF licenses this file to You under the Apache License, Version 2.0
   (the "License"); you may not use this file except in compliance with
   the License.  You may obtain a copy of the License at
 
       http://www.apache.org/licenses/LICENSE-2.0
 
   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#include <stdlib.h>

#include "dso.h"

#ifdef DSO_DYLD

#include <mach-o/dyld.h>

/* Print an error message and abort all if a specified symbol wasn't found */
static void nosymbol(const char *s) {
//  INTL_CFUNC(log_error)("Cannot find symbol '%s' in library", s);
  abort();
}

/* We found two symbols for the same name in two different modules */
static NSModule multiple(NSSymbol s, NSModule om, NSModule nm) {
  NSModule ret = nm;

//  INTL_CFUNC(log_debug)("Symbol \"%s\" found in modules \"%s\" and \"%s\" (using %s)",
//                        NSNameOfSymbol(s), NSNameOfModule(om), NSNameOfModule(nm), NSNameOfModule(ret));

  return (ret);
}

/* We got an error while linking a module, and if it's not a warning we have
   to abort the whole program */
static void linkedit(NSLinkEditErrors category, int number, const char *file, const char *message) {
//  INTL_CFUNC(log_error)("Errors during link edit of file \"%s\" (error=%d): %s", file, number, message);
  /* Check if this error was only a warning */
  if (category != NSLinkEditWarningError) {
//    INTL_CFUNC(log_error)("Cannot continue");
    abort();
  }
}

/* Initialize all DSO stuff */
bool INTL_CFUNC(dso_init)() {
  NSLinkEditErrorHandlers h;

  h.undefined = nosymbol;
  h.multiple = multiple;
  h.linkEdit = linkedit;

  NSInstallLinkEditErrorHandlers(&h);
  return (true);
}

/* Attempt to link a library from a specified filename */
dso_handle INTL_CFUNC(dso_link)(const char *path) {
  /* We need to load the library publicly as NSModuleFileImage is not
     yet implemented (at least for non MH_BUNDLE libraries */
  if (NSAddLibrary(path) != TRUE)
    return (NULL);
  /* We need to return a non-null value, even if it has no meaning. One day
     this whole crap will be fixed */
  return ((void *) !NULL);
}

/* Attempt to unload a library */
bool INTL_CFUNC(dso_unlink)(dso_handle libr) {
  /* Check the handle */
  if (libr == NULL) {
//    INTL_CFUNC(log_error)("Attempting to unload a module without handle");
    return (false);
  }

  /* We don't have a module, so, we don't really have to do anything */
  return (true);
}

/* Get the address for a specifed symbol */
void *INTL_CFUNC(dso_symbol)(dso_handle hdl, const char *nam) {
  NSSymbol sym = NULL;
  NSModule mod = NULL;
  char *und = NULL;
  void *add = NULL;
  int x = 0;

  /* Check parameters */
  if (hdl == NULL) {
//    INTL_CFUNC(log_error)("Invalid library handler specified");
    return (NULL);
  }

  if (nam == NULL) {
//    INTL_CFUNC(log_error)("Invalid symbol name specified");
    return (NULL);
  }

  /* Process the correct name (add a _ before the name) */
  while (nam[x] != '\0')
    x++;
  und = (char *) malloc(sizeof(char) * (x + 2));
  while (x >= 0)
    und[x + 1] = nam[x--];
  und[0] = '_';

  /* Find the symbol */
  sym = NSLookupAndBindSymbol(und);
  free(und);
  if (sym == NULL)
    return (NULL);

  /* Dump some debugging output since this part is shaky */
  mod = NSModuleForSymbol(sym);
  add = NSAddressOfSymbol(sym);
//  INTL_CFUNC(log_debug)("Symbol \"%s\" found in module \"%s\" at address \"0x%08X\"",
//                        NSNameOfSymbol(sym), NSNameOfModule(mod), add);

  /* We want to return the address of the symbol */
  return (add);
}

/* Return the error message from dlopen: Well we already print it */
char *INTL_CFUNC(dso_error)() {
  return ("no additional message");
}

#endif /* ifdef DSO_DYLD */
