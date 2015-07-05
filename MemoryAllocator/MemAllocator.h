#pragma once
#include <iostream>

struct HeapHeader{
	HeapHeader() : nextHeap(NULL), prevHeap(NULL){}
	char* nextHeap, *prevHeap;
};

class MemAllocator {
public:
	MemAllocator();
	~MemAllocator();

	void* MyMalloc(size_t size);
	
	void MyFree(void* ptr);

private:
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
	*/
	unsigned int chunkSize(void* ptr);

	/* check if chunk is allocated
	* must point to valid header
	*/
	bool isAllocated(void* ptr);

	HeapHeader* thisHeapHeader() const;

	char* nextHeap() const;

	void* memoryPool;
	void* poolStart;
	size_t poolSize;
};

