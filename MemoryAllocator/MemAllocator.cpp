#include "MemAllocator.h"
#include <Windows.h>
#define CHUNK_MAX_SIZE ~(1 << 31)
#define HEADER_SIZE sizeof(unsigned int)
#define HEAP_HEADER_SIZE sizeof(HeapHeader)

MemAllocator::MemAllocator() {
	poolSize = 100;

	memoryPool = VirtualAlloc(NULL, poolSize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);


	if (!memoryPool) {
		throw std::exception("Failed to allocate memory");
	}

	HeapHeader h;
	memcpy(memoryPool, &h, sizeof(HeapHeader));

	poolStart = (char*)memoryPool + sizeof(HeapHeader);

	setSize(poolStart, poolSize - 2 * HEADER_SIZE - HEAP_HEADER_SIZE);
	setAllocated(poolStart, 0);

}

MemAllocator::~MemAllocator() {
	VirtualFree(memoryPool, poolSize, MEM_RELEASE);
}

void* MemAllocator::MyMalloc(size_t size) {

	if (size > CHUNK_MAX_SIZE) {
		return NULL;
	}

	char* start = (char*)poolStart;
	char* end = (char*)memoryPool + poolSize - 1;
	void* ptrToRequestedMemory = NULL;

	while ( !(ptrToRequestedMemory = findFreeChunk(start, end, size)) ) {
		if (nextHeapHeader(memoryPool) != NULL) {
			start = (char*)nextHeapHeader(memoryPool) + HEAP_HEADER_SIZE;
			end = start + poolSize - 1;
		}
		else{
			void* newPool = VirtualAlloc(NULL, poolSize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

			if (!newPool){
				return NULL;
			}

			HeapHeader h;
			h.nextHeapHeader = NULL;
			h.prevHeapHeader = heapHeader(memoryPool);
			memcpy(newPool, &h, sizeof(HeapHeader));

			char* newPoolStart = (char*)newPool + HEAP_HEADER_SIZE;
			setSize(newPoolStart, poolSize - 2 * HEADER_SIZE - HEAP_HEADER_SIZE);
			setAllocated(newPoolStart, 0);

			HeapHeader* thisHeapHeader = heapHeader(memoryPool);
			thisHeapHeader->nextHeapHeader = (HeapHeader*)newPool;

			start = newPoolStart;
			end = (char*)newPool + poolSize - 1;
		}
	}

	return ptrToRequestedMemory;
}

void MemAllocator::MyFree(void* ptr) {
	//char* poolEnd = (char*)memoryPool + poolSize - 1;
	char* poolEnd = findPoolEnd(ptr);
	char* start = findPoolStart(ptr);

	if (!poolEnd || !start) {
		return;
	}

	unsigned int thisChunkSize = chunkSize((char*)ptr - HEADER_SIZE);

	char* thisChunkHeader = (char*)ptr - HEADER_SIZE;
	char* rightChunkHeader = (char*)ptr + thisChunkSize + HEADER_SIZE;
	char* leftChunkHeader = (char*)ptr - 2 * HEADER_SIZE - chunkSize((char*)ptr - 2 * HEADER_SIZE) - HEADER_SIZE;

	setAllocated(thisChunkHeader, 0);

	//merge if there is right chunk and if it's free
	if ((char*)ptr + thisChunkSize + HEADER_SIZE - 1 != poolEnd && !isAllocated(rightChunkHeader)) {
		mergeChunks(thisChunkHeader, rightChunkHeader);
	}

	//merge if there is left chunk and it's free
	if ((char*)ptr - HEADER_SIZE - HEAP_HEADER_SIZE != start && !isAllocated(leftChunkHeader)) {
		mergeChunks(leftChunkHeader, thisChunkHeader);
	}
}

void* MemAllocator::findFreeChunk(char* poolStart, char* poolEnd, size_t size) {
	for (;; poolStart += chunkSize(poolStart) + 2 * HEADER_SIZE) {
		if (!isAllocated(poolStart) && chunkSize(poolStart) >= size) {

			//split
			if (chunkSize(poolStart) != size) {
				setSize(poolStart + 2 * HEADER_SIZE + size, chunkSize(poolStart) - size - 2 * HEADER_SIZE);
				setAllocated(poolStart + 2 * HEADER_SIZE + size, 0);
			}

			setSize(poolStart, size);
			setAllocated(poolStart, 1);

			return poolStart + HEADER_SIZE;
		}

		if (poolStart + 2 * HEADER_SIZE + chunkSize(poolStart) - 1 == poolEnd) {
			break;
		}
	}
	return NULL;
}

void MemAllocator::setSize(void* ptr, unsigned int size) {
	if (!ptr) {
		return;
	}

	*(unsigned int*)ptr = size;
	*(unsigned int*)((char*)ptr + HEADER_SIZE + size) = size;
}

void MemAllocator::setAllocated(void* ptr, char isAllocated) {
	if (!ptr){
		return;
	}

	unsigned int size = chunkSize(ptr);

	if (isAllocated) {
		*(unsigned int*)ptr |= (1 << 31);
		*(unsigned int*)((char*)ptr + HEADER_SIZE + size ) |= (1 << 31);
	}
	else {
		*(unsigned int*)ptr &= ~(1 << 31);
		*(unsigned int*)((char*)ptr + HEADER_SIZE + size ) &= ~(1 << 31);
	}
}

void MemAllocator::mergeChunks(void* l, void* r) {
	if (!l || !r) {
		return;
	}

	setSize(l, chunkSize(l) + chunkSize(r) + 2 * HEADER_SIZE);
	setAllocated(l, 0);
}

unsigned int MemAllocator::chunkSize(void* ptr) {
	return (*(unsigned int*)ptr & ~(1 << 31));
}

bool MemAllocator::isAllocated(void* ptr) {
	return (*(unsigned int*)ptr & (1 << 31));
}

HeapHeader* MemAllocator::heapHeader(void* poolStart) const {
	return (HeapHeader*)poolStart;
}

HeapHeader* MemAllocator::nextHeapHeader(void* poolStart) const {
	return heapHeader(poolStart)->nextHeapHeader;
}

char* MemAllocator::findPoolEnd(void* ptr) {
	char* p = (char*)ptr;
	char* start = (char*)memoryPool;
	char* end = start + poolSize - 1;

	while (start != NULL){
		if (start <= p && p <= end) {
			return end;
		}
		start = (char*)(heapHeader(start)->nextHeapHeader);
		end = start + poolSize - 1;
	}

	return NULL;
}

char* MemAllocator::findPoolStart(void* ptr) {
	char* p = (char*)ptr;
	char* start = (char*)memoryPool;
	char* end = start + poolSize - 1;

	while (1){
		if (start <= p && p <= end) {
			return start;
		}

		start = (char*)(heapHeader(start)->nextHeapHeader);
		end = start + poolSize - 1;
	}

	return NULL;
}