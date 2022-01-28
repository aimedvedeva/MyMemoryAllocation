#include <stddef.h>
#include <string.h>
#include "ChunkList.h"

static blockDescPointers *s_listHead; //need to add new elements after listHead
static blockDescPointers *s_curElement; //that pointer allows to begin searching another one from the past stop-->next fit

static void *s_heapBeginning; 
static void *s_heapEnd;

void _removePointerDescFromChunk(void *chunkDesc);
void _setFlagNotFree(void *chunkDesc);
bool _isChunkFree(void *chunkDesc);
bool _isChunkHeapBeginning(void *chunkDesc);
bool _isChunkHeapEnd(void *chunkDesc);
bool _isChunkSizeBiggerOrEqual(blockDescPointers *chunkDesc, size_t size);
void _initChunk(void *tmp, size_t areaSize, blockDescPointers **pointerDesc /*out*/);
size_t _descSumSize();
void *_shiftLeftFlagSizeDesc(void *data);
void *_shiftRightFlagSizeDesc(void *data);
void *_shiftRightPointersDesc(void *data);
void *_shiftLeftPointersDesc(void *data);
void *_shiftRight(void *data, size_t shiftSize);
void *_shiftLeft(void *data, size_t shiftSize);
void _initDescFlagSize(blockDescFlagSize *desc, bool isFree, size_t size);
void _initDescPointers(blockDescPointers *desc, void *Next, void *Prev);

size_t _getSizeOfChunk(blockDescPointers *chunkDesc){
  size_t chunkSize;

  chunkDesc = _shiftLeftFlagSizeDesc(chunkDesc);
   chunkSize = ((blockDescFlagSize*)chunkDesc)->size;
  return chunkSize;
}

size_t GetCurSizeOfFreeClientArea(){
  blockDescPointers *each = s_listHead;
  size_t curChunkSize;
  size_t sumSizeOfClientFreeArea = 0;

  while (each != NULL){
    curChunkSize = _getSizeOfChunk(each);
    sumSizeOfClientFreeArea += curChunkSize;
    each = each->Next;
  }
  return sumSizeOfClientFreeArea;
}

void DestroyList(){
  s_curElement = NULL;
  s_listHead = NULL;
}

void InitHeapBoards(void *area, size_t areaSize){
  s_heapBeginning = area;
  s_heapEnd = (void*)((char*)area + areaSize);
}

void CreateList(void *area, size_t size){
  InitListElement(area, size, &s_listHead);
  
  s_curElement = s_listHead;
  //_initChunk has already initialized s_listHead by NULL values
}

void InitListElement(void *area, size_t size, void **listEl){
  _initChunk(area, size, (blockDescPointers**)listEl);
}

void *GetDescNextChunk(void *chunkDesc){
  size_t curChunkSize;

  chunkDesc = _shiftLeftFlagSizeDesc(chunkDesc);

  curChunkSize = ((blockDescFlagSize*)chunkDesc)->size;

  chunkDesc = _shiftRightFlagSizeDesc(chunkDesc);
  chunkDesc = _shiftRight(chunkDesc, curChunkSize);
  chunkDesc = _shiftRightFlagSizeDesc(chunkDesc);
  chunkDesc = _shiftRightFlagSizeDesc(chunkDesc);

  return chunkDesc;
}

void Delete(void *el) {
  if (el == s_listHead){
    s_listHead = s_listHead->Next;
    if (s_listHead != NULL) {
      s_listHead->Prev = NULL;
    }
    return;
  }
  if (((blockDescPointers*)el)->Prev != NULL){
    ((blockDescPointers*)el)->Prev->Next = ((blockDescPointers*)el)->Next;
  }
  if (((blockDescPointers*)el)->Next != NULL){
    ((blockDescPointers*)el)->Next->Prev = ((blockDescPointers*)el)->Prev;
  }
}

void Add(void *el) {
  if (s_listHead == NULL){
    _initDescPointers(el, NULL, NULL);

    s_listHead = el;
    s_curElement = el;
    return;
  }

  ((blockDescPointers*)el)->Next = s_listHead;
  s_listHead->Prev = el;
  ((blockDescPointers*)el)->Prev = NULL;
  
  s_listHead = el;
}

blockDescPointers *Find(bool(*Comparator)(void *element, size_t size), size_t size){
  blockDescPointers *each = s_curElement;

  if (s_curElement == NULL){
    return NULL;
  }

  if (each == s_curElement && Comparator(each, size)){
    s_curElement = each->Next;
    return each;
  }

  do{
    if (Comparator(each, size)){
      if (each->Next == NULL){
        s_curElement = s_listHead;
      }
      else{
        s_curElement = each->Next;
      }
      return each;
    }
    each = each->Next;

    if (each == NULL){
      each = s_listHead;
    }
   } while (each != s_curElement);

  return NULL;
}

void SetFlagFree(void *chunkDesc){
  size_t curChunkSize;

  chunkDesc = _shiftLeftFlagSizeDesc(chunkDesc);
  ((blockDescFlagSize*)chunkDesc)->isFree = true;

  curChunkSize = ((blockDescFlagSize*)chunkDesc)->size;

  chunkDesc = _shiftRightFlagSizeDesc(chunkDesc);
  chunkDesc = _shiftRight(chunkDesc, curChunkSize);

  ((blockDescFlagSize*)chunkDesc)->isFree = true;
}

bool IsAppropriateChunk(void *chunkDesc, size_t size) {
  
  if (_isChunkSizeBiggerOrEqual(chunkDesc, size) == true) {
    return true;
  }
  else {
    return false;
  }
}

bool IsPossibleToSplit(blockDescFlagSize *deskOfChunk, size_t desirableSize){
  deskOfChunk = _shiftLeftFlagSizeDesc(deskOfChunk);

  if (deskOfChunk->size > _descSumSize() + desirableSize) {
    return true;
  }
  else {
    return false;
  }
}

void SplitChunk(size_t newSize, void *curChunkDesc /*in*/, void **leftChunkDesc /*out*/, void **rightChunkDesc /*out*/) {
  blockDescFlagSize oldFlagSizeData;
  blockDescPointers oldPointersData;
  size_t balance;

  //remember pointer info
  oldPointersData.Next = ((blockDescPointers*)curChunkDesc)->Next;
  oldPointersData.Prev = ((blockDescPointers*)curChunkDesc)->Prev;

  //shift void* pointer to the flagSize desc
  curChunkDesc = _shiftLeftFlagSizeDesc(curChunkDesc);

  //remember flagsize info
  oldFlagSizeData.isFree = ((blockDescFlagSize*)curChunkDesc)->isFree;
  oldFlagSizeData.size = ((blockDescFlagSize*)curChunkDesc)->size;

  //balance - defference between old and new sizes
  balance = oldFlagSizeData.size - newSize - 2 * sizeof(blockDescFlagSize);

  _initChunk(curChunkDesc, newSize + 2 * sizeof(blockDescFlagSize), (blockDescPointers**)leftChunkDesc);

  curChunkDesc = _shiftRight(curChunkDesc, newSize + 2 * sizeof(blockDescFlagSize));

  _initChunk(curChunkDesc, balance + 2 * sizeof(blockDescFlagSize), (blockDescPointers**)rightChunkDesc);
  
  //recover old pointers
  ((blockDescPointers*)(*leftChunkDesc))->Next = *rightChunkDesc;
  ((blockDescPointers*)(*leftChunkDesc))->Prev = oldPointersData.Prev;
  
  
  ((blockDescPointers*)(*rightChunkDesc))->Prev = *leftChunkDesc;
  ((blockDescPointers*)(*rightChunkDesc))->Next = oldPointersData.Next;

  s_curElement = *rightChunkDesc;
}

void PrepareClientChunk(void *chunkDesc){
  _removePointerDescFromChunk(chunkDesc);
  _setFlagNotFree(chunkDesc);
}

bool IsRightChunkEmpty(void *chunkDesc) {
  size_t chunkSize;

  if (_isChunkHeapEnd(chunkDesc)){
    return false;
  }
  
  chunkDesc = _shiftLeftFlagSizeDesc(chunkDesc);

  chunkSize = ((blockDescFlagSize*)chunkDesc)->size;
  chunkDesc = _shiftRightFlagSizeDesc(chunkDesc);
  chunkDesc = _shiftRight(chunkDesc, chunkSize);
  chunkDesc = _shiftRightFlagSizeDesc(chunkDesc);

  if (((blockDescFlagSize*)chunkDesc)->isFree == true) {
    return true;
  }
  else {
    return false;
  }
}

bool IsLeftChunkEmpty(void *chunkDesc) {
  if (_isChunkHeapBeginning(chunkDesc)){
    return false;
  }

  chunkDesc = _shiftLeftFlagSizeDesc(chunkDesc);
  chunkDesc = _shiftLeftFlagSizeDesc(chunkDesc);

  if (((blockDescFlagSize*)chunkDesc)->isFree == true) {
    return true;
  }
  else {
    return false;
  }
}

void MergeWithLeftChunk(void *chunkDesc, void **finalChunk /*out*/) {
  bool isChunkFree = _isChunkFree(chunkDesc);
  bool isChunkDescListHead = false;
  bool isChunkDescCurEl = false;

  size_t curChunkSize;
  size_t leftChunkSize;

  blockDescPointers tmpPointers;
  
  tmpPointers.Next = tmpPointers.Prev = NULL;
  
  if (isChunkFree == true){
    //remember pointers of the left block in temporery variable
    tmpPointers.Next = ((blockDescPointers*)chunkDesc)->Next;
    tmpPointers.Prev = ((blockDescPointers*)chunkDesc)->Prev;
    
    Delete(chunkDesc);
    if (chunkDesc == s_listHead){
      isChunkDescListHead = true;
    }
    if (chunkDesc == s_curElement){
      isChunkDescCurEl = true;
    }
  }
  
  chunkDesc = _shiftLeftFlagSizeDesc(chunkDesc);
  curChunkSize = ((blockDescFlagSize*)chunkDesc)->size;

  //consider last desc of the left block
  chunkDesc = _shiftLeftFlagSizeDesc(chunkDesc);
  leftChunkSize = ((blockDescFlagSize*)chunkDesc)->size;

  chunkDesc = _shiftLeft(chunkDesc, leftChunkSize);

  if (isChunkFree == false){
    //remember pointers of the left block in temporery variable
    tmpPointers.Next = ((blockDescPointers*)chunkDesc)->Next;
    tmpPointers.Prev = ((blockDescPointers*)chunkDesc)->Prev;
  }
  
  chunkDesc = _shiftLeftFlagSizeDesc(chunkDesc);

  _initChunk(chunkDesc, 4 * sizeof(blockDescFlagSize) + leftChunkSize + curChunkSize, (blockDescPointers**)finalChunk);

  //init by previous pointers
  ((blockDescPointers*)*finalChunk)->Next = tmpPointers.Next;
  ((blockDescPointers*)*finalChunk)->Prev = tmpPointers.Prev;

  //if s_listHead or s_curElement ara the elements that were merged to another
  if (isChunkFree == true){
    Add(*finalChunk);
    if (isChunkDescListHead) {
      s_listHead = *finalChunk;
    }
    if (isChunkDescCurEl) {
      s_curElement = *finalChunk;
    }
  }
}

void MergeWithRightChunk(void *chunkDesc, void **finalChunk /*out*/) {
  void *finalChunkArea = NULL;
  void *rightPointerDesc = NULL;
  size_t curChunkSize;
  size_t rightChunkSize;
  blockDescPointers tmpPointers;

  chunkDesc = _shiftLeftFlagSizeDesc(chunkDesc);

  //remember pointer to the area for final Chunk
  finalChunkArea = chunkDesc;

  curChunkSize = ((blockDescFlagSize*)chunkDesc)->size;

  chunkDesc = _shiftRightFlagSizeDesc(chunkDesc);
  chunkDesc = _shiftRight(chunkDesc, curChunkSize);
  chunkDesc = _shiftRightFlagSizeDesc(chunkDesc);

  //remember right block size
  rightChunkSize = ((blockDescFlagSize*)chunkDesc)->size;

  chunkDesc = _shiftRightFlagSizeDesc(chunkDesc);
  
  //remember *pointerDesc to right chunk to check that listHead has correct value
  rightPointerDesc = chunkDesc;

  //remember right block pointers in temporary variable
  tmpPointers.Next = ((blockDescPointers*)chunkDesc)->Next;
  tmpPointers.Prev = ((blockDescPointers*)chunkDesc)->Prev;

  _initChunk(finalChunkArea, 4 * sizeof(blockDescFlagSize) + curChunkSize + rightChunkSize, (blockDescPointers**)finalChunk);

  ((blockDescPointers*)(*finalChunk))->Next = tmpPointers.Next;
  ((blockDescPointers*)(*finalChunk))->Prev = tmpPointers.Prev;
  
  //if s_listHead or s_curElement ara the elements that were merged to another
  if (rightPointerDesc == s_listHead){
    s_listHead = *finalChunk;
  }
  if (rightPointerDesc == s_curElement){
    s_curElement = *finalChunk;
  }
}


void _removePointerDescFromChunk(void *chunkDesc) {
  ((blockDescPointers*)chunkDesc)->Next = NULL;
  ((blockDescPointers*)chunkDesc)->Prev = NULL;
}

void _setFlagNotFree(void *chunkDesc){
  size_t curChunkSize;

  chunkDesc = _shiftLeftFlagSizeDesc(chunkDesc);
  ((blockDescFlagSize*)chunkDesc)->isFree = false;

  curChunkSize = ((blockDescFlagSize*)chunkDesc)->size;

  chunkDesc = _shiftRightFlagSizeDesc(chunkDesc);
  chunkDesc = _shiftRight(chunkDesc, curChunkSize);

  ((blockDescFlagSize*)chunkDesc)->isFree = false;
}

bool _isChunkHeapBeginning(void *chunkDesc){
  chunkDesc = _shiftLeftFlagSizeDesc(chunkDesc);
  if (chunkDesc == s_heapBeginning){
    return true;
  }
  return false;
}

bool _isChunkHeapEnd(void *chunkDesc){
  size_t chunkSize;

  chunkDesc = _shiftLeftFlagSizeDesc(chunkDesc);

  chunkSize = ((blockDescFlagSize*)chunkDesc)->size;
  chunkDesc = _shiftRightFlagSizeDesc(chunkDesc);
  chunkDesc = _shiftRight(chunkDesc, chunkSize);
  chunkDesc = _shiftRightFlagSizeDesc(chunkDesc);

  if (chunkDesc == s_heapEnd){
    return true;
  }
  return false;
}

bool _isChunkFree(void *chunkDesc){
  chunkDesc = _shiftLeftFlagSizeDesc(chunkDesc);
  if (((blockDescFlagSize*)chunkDesc)->isFree == true){
    return true;
  }
  else{
    return false;
  }
}

size_t _descSumSize() {
  return 2 * sizeof(blockDescFlagSize) + sizeof(blockDescPointers);
}

void *_shiftLeftFlagSizeDesc(void *data) {
  return  data = (blockDescFlagSize*)data - 1;
}

void *_shiftRightFlagSizeDesc(void *data) {
  return data = (blockDescFlagSize*)data + 1;
}

void *_shiftRightPointersDesc(void *data) {
  return data = (blockDescPointers*)data + 1;
}

void *_shiftLeftPointersDesc(void *data) {
  return data = (blockDescPointers*)data - 1;
}

void *_shiftRight(void *data, size_t shiftSize) {
  return data = (char*)data + shiftSize;
}

void *_shiftLeft(void *data, size_t shiftSize) {
  return data = (char*)data - shiftSize;
}

void _initDescFlagSize(blockDescFlagSize *desc, bool isFree, size_t size) {
  desc->isFree = isFree;
  desc->size = size;
}

void _initDescPointers(blockDescPointers *desc, void *Next, void *Prev) {
  desc->Next = Next;
  desc->Prev = Prev;
}

void _initChunk(void *area, size_t areaSize, blockDescPointers **pointerDesc /*out*/) {
  size_t descSumSize = _descSumSize();
  size_t clientMemorySize = areaSize - descSumSize + sizeof(blockDescPointers);
  size_t freeMemorySize = areaSize - descSumSize;

  _initDescFlagSize((blockDescFlagSize*)area, true, clientMemorySize);

  area = _shiftRightFlagSizeDesc(area);

  _initDescPointers((blockDescPointers*)area, NULL, NULL);

  *pointerDesc = area;

  area = _shiftRightPointersDesc(area);
  area = _shiftRight(area, freeMemorySize);

  _initDescFlagSize((blockDescFlagSize*)area, true, clientMemorySize);
}

bool _isChunkSizeBiggerOrEqual(blockDescPointers *chunkDesc, size_t size) {
  chunkDesc = _shiftLeftFlagSizeDesc(chunkDesc);

  if (((blockDescFlagSize*)chunkDesc)->size >= size) {
    return true;
  }
  else {
    return false;
  }
}
