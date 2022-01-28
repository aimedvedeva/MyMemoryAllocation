#include "ChunkList.h"
#include "MyAlloc.h"

void MyAllocInit(void *area, size_t areaSize){
  CreateList(area, areaSize);
  InitHeapBoards(area, areaSize);
}

void MyAllocDestroy(){
  DestroyList();
}

void *MyMalloc(size_t size){
  void *leftChunk = NULL;
  void *rightChunk = NULL;
  void *clientChunk = NULL;

  //system can't give less than descriptors require
  if (size < sizeof(blockDescPointers)){
    size = sizeof(blockDescPointers);
  }

  clientChunk = Find(IsAppropriateChunk, size);

  if (clientChunk != NULL){
    if (IsPossibleToSplit(clientChunk, size)){
      SplitChunk(size, clientChunk, &leftChunk, &rightChunk);
      clientChunk = leftChunk;
    }
    Delete(clientChunk);
    PrepareClientChunk(clientChunk);
    return clientChunk;
  }
  else {
    return NULL;
  }
}

void MyFree(void *ptr){
  void *leftChunk = NULL;
  void *rightChunk = NULL;
  bool isLeftChunkEmpty;
  bool isRightChunkEmpty;

  if (ptr == NULL){
    return;
  }

  isLeftChunkEmpty = IsLeftChunkEmpty(ptr);
  isRightChunkEmpty = IsRightChunkEmpty(ptr);

  if (isLeftChunkEmpty && !isRightChunkEmpty){
    MergeWithLeftChunk(ptr, &leftChunk);
  }
  else if (!isLeftChunkEmpty && isRightChunkEmpty){
    ptr = GetDescNextChunk(ptr);
    MergeWithLeftChunk(ptr, &leftChunk);
    //MergeWithRightChunk(ptr, &rightChunk);
  }
  else if (isLeftChunkEmpty && isRightChunkEmpty){
    MergeWithLeftChunk(ptr, &leftChunk);
    Delete(leftChunk);
    rightChunk = GetDescNextChunk(leftChunk);
    MergeWithLeftChunk(rightChunk, &leftChunk);
    //MergeWithRightChunk(leftChunk, &rightChunk);
  }
  else if (!isLeftChunkEmpty && !isRightChunkEmpty){
    SetFlagFree(ptr);
    Add(ptr);
  }
}


