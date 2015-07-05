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

	bool done = false;
	char* currentPos = (char*)poolStart;
	char* poolEnd = (char*)memoryPool + poolSize - 1;

	for (;; currentPos += chunkSize(currentPos) + 2 * HEADER_SIZE) {
		if (!isAllocated(currentPos) && chunkSize(currentPos) >= size) {

			if (chunkSize(currentPos) != size) {
				setSize(currentPos + 2 * HEADER_SIZE + size, chunkSize(currentPos) - size - 2 * HEADER_SIZE);
				setAllocated(currentPos + 2 * HEADER_SIZE + size, 0);
			}

			setSize(currentPos, size);
			setAllocated(currentPos, 1);
			
			return currentPos + HEADER_SIZE;
		}

		if (currentPos + 2 * HEADER_SIZE + chunkSize(currentPos) - 1 == poolEnd) {
			break;
		}
	}

	return NULL;
}

void MemAllocator::MyFree(void* ptr) {
	char* poolEnd = (char*)memoryPool + poolSize - 1;
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
	if ((char*)ptr - HEADER_SIZE != poolStart && !isAllocated(leftChunkHeader)) {
		mergeChunks(leftChunkHeader, thisChunkHeader);
	}
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