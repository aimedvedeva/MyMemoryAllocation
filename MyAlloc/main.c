#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <crtdbg.h>
#include "MyAlloc.h"
#include "ChunkList.h"

int main(void){
  size_t areaSize = 448;
  size_t curSizeOfClientFreeArea;
  void *area = (void*)malloc(areaSize * sizeof(char));
  void *mem1 = NULL, *mem2 = NULL, *mem3 = NULL, *mem4 = NULL, *mem5 = NULL, *mem6 = NULL, *mem7 = NULL, *mem8 = NULL, *mem9 = NULL;

  memset(area, '!', areaSize);

  MyAllocInit(area, areaSize);

  mem1 = MyMalloc(40);
  mem2 = MyMalloc(40);
  mem3 = MyMalloc(40);
  mem4 = MyMalloc(40);
  mem5 = MyMalloc(40);
  mem6 = MyMalloc(40);
  mem7 = MyMalloc(40);
  mem8 = MyMalloc(40);
  mem9 = MyMalloc(40);
  curSizeOfClientFreeArea = GetCurSizeOfFreeClientArea();
  MyFree(mem2);
  curSizeOfClientFreeArea = GetCurSizeOfFreeClientArea();
  MyFree(mem9);
  curSizeOfClientFreeArea = GetCurSizeOfFreeClientArea();
  MyFree(mem5);
  curSizeOfClientFreeArea = GetCurSizeOfFreeClientArea();
  MyFree(mem3);
  curSizeOfClientFreeArea = GetCurSizeOfFreeClientArea();
  MyFree(mem7);
  curSizeOfClientFreeArea = GetCurSizeOfFreeClientArea();
  MyFree(mem8);
  curSizeOfClientFreeArea = GetCurSizeOfFreeClientArea();
  MyFree(mem1);
  curSizeOfClientFreeArea = GetCurSizeOfFreeClientArea();
  MyFree(mem4);
  curSizeOfClientFreeArea = GetCurSizeOfFreeClientArea();
  MyFree(mem6);
  curSizeOfClientFreeArea = GetCurSizeOfFreeClientArea();

  MyAllocDestroy();
  free(area);

  _CrtDumpMemoryLeaks();
}