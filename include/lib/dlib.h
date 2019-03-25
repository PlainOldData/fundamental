/*
  dlib
  --
  Very simple wrapper around dynamic libraries for Windows / Nix platforms

  public domain 2017
*/
#ifndef DLIB_INCLUDED_CC96CA25_2327_4042_B491_4CED597F1541
#define DLIB_INCLUDED_CC96CA25_2327_4042_B491_4CED597F1541


#ifdef __cplusplus
extern "C" {
#endif 


/* ---------------------------------------------------- [ DLIB interface ] -- */


void*
lib_dlib_open(
  const char *name);


void*
lib_dlib_get_address(
  void *handle,
  const char *name);


void
lib_dlib_close(
  void *handle);


#ifdef __cplusplus
} /* extern */
#endif 

#endif /* inc guard */