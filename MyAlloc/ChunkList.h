#pragma once
#include <stddef.h>

typedef enum{
  false,
  true,
}bool;

typedef struct tag_blockDescPointers{
  struct tag_blockDescPointers *Next;
  struct tag_blockDescPointers *Prev;

}blockDescPointers;

typedef struct{
  size_t size;
  bool isFree;
}blockDescFlagSize;

void DestroyList();
void Delete(void *el);
void Add(void *el);
void CreateList(void *area, size_t size);
void InitListElement(void *area, size_t size, void *listEl);
blockDescPointers *Find(bool(*Comparator)(void *element, size_t size), size_t size);

size_t GetCurSizeOfFreeClientArea();
void InitHeapBoards(void *area, size_t areaSize);
void PrepareClientChunk(void *chunkDesc);
void SplitChunk(size_t newSize, void *curChunkDesc /*in*/, void *leftChunkDesc /*out*/, void *rightChunkDesc /*out*/);
bool IsPossibleToSplit(blockDescFlagSize *deskOfChunk, size_t desirableSize);
bool IsAppropriateChunk(void *chunkDesc, size_t size);
bool IsLeftChunkEmpty(void *chunkDesc);
bool IsRightChunkEmpty(void *chunkDesc);
void MergeWithLeftChunk(void *chunkDesc, void **leftChunk /*out*/);
void MergeWithRightChunk(void *chunkDesc, void **rightChunk /*out*/);
void SetFlagFree(void *chunkDesc);
void *GetDescNextChunk(void *chunkDesc);