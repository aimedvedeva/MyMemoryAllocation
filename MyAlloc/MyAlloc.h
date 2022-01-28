/*#ifndef MYALLOC_H_INCLUDED
#define MYALLOC_H_INCLUDED
#pragma once

#ifdef MYALLOC_EXPORTS
#define MYALLOC_CALL __declspec(dllexport)
#else
#define MYALLOC_CALL __declspec(dllimport)
#endif /* MYALLOC_EXPORTS */

//#ifdef __cplusplus
//extern "C" {
//#endif // __cplusplus
  #include <stddef.h>
  void MyAllocInit(void *area, size_t areaSize);
  void MyAllocDestroy();

  void *MyMalloc(size_t size);
  void MyFree(void *ptr);
/*
#ifdef __cplusplus
}
#endif // __cplusplus*/



//#endif /* MYALLOC_H_INCLUDED */
