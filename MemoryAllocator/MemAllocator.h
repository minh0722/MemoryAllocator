#pragma once
#include <iostream>

struct HeapHeader{
	HeapHeader() : nextHeapHeader(NULL), prevHeapHeader(NULL){}
	HeapHeader* nextHeapHeader, *prevHeapHeader;
};

class MemAllocator {
public:
	MemAllocator();
	~MemAllocator();

	void* MyMalloc(size_t size);
	
	void MyFree(void* ptr);

private:

	void* findFreeChunk(char* poolStart, char* poolEnd, size_t size);

	/* set size of the chunk
	*/
	void setSize(void* ptr, unsigned int size);

	void setAllocated(void* ptr, char isAllocated = 1);

	/* merge free chunks together
	* l and r must points to valid headers
	* l precedes r
	*/
	void mergeChunks(void* l, void* r);

	/* return size of the chunk
	*  must point to valid header
	*/
	unsigned int chunkSize(void* ptr);

	/* check if chunk is allocated
	*  must point to valid header
	*/
	bool isAllocated(void* ptr);

	/* return heap header of pool
	*  poolStart must point to first byte of pool
	*/
	HeapHeader* heapHeader(void* poolStart) const;

	/* return heap header of next pool of poolStart
	*  poolStart must point to first byte of pool
	*/
	HeapHeader* nextHeapHeader(void* poolStart) const;

	/* finds last byte in the pool that contains ptr
	*/
	char* findPoolEnd(void* ptr);

	/* finds first byte in the pool that contains ptr
	*/
	char* findPoolStart(void* ptr);

	/* free the pool and all pool after it
	*/
	void freePool(void* ptr);

	/* find offset bytes
	*  must point to valid header
	*/
	unsigned int offsetCount(void* ptr);

	void* memoryPool;
	void* poolStart;
	size_t poolSize;
};

