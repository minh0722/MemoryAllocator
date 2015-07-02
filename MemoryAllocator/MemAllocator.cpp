#include "MemAllocator.h"
#include <iostream>
#include <Windows.h>
#define CHUNK_MAX_SIZE ~(1 << 31)

MemAllocator::MemAllocator() {
	poolSize = 10 * 1024;

	memoryPool = VirtualAlloc(NULL, poolSize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

	if (!memoryPool) {
		throw std::exception("Failed to allocate memory");
	}

	setSize(memoryPool, poolSize - 2 * sizeof(unsigned int));
	setAllocated(memoryPool, 0);

}

MemAllocator::~MemAllocator() {
	VirtualFree(memoryPool, poolSize, MEM_RELEASE);
}

void* MemAllocator::MyMalloc(size_t size) {

	bool done = false;
	char* currentPos = (char*)memoryPool;
	char* poolEnd = (char*)memoryPool + poolSize;

	for (;; currentPos += chunkSize(currentPos) + 2*sizeof(unsigned int) - 1) {
		if (!isAllocated(currentPos)) {
			int a;
			a = 4;
			//setAllocated(currentPos, 1);
		}

		if (currentPos + 2 * sizeof(unsigned int) + chunkSize(currentPos) == poolEnd) {
			break;
		}
	}

	return NULL;
}

void MemAllocator::MyFree(void* ptr) {

}

void MemAllocator::setSize(void* ptr, unsigned int size) {
	if (!ptr) {
		return;
	}

	if (size > CHUNK_MAX_SIZE) {
		throw std::exception("Chunk size is too big!");
	}

	*(unsigned int*)ptr = size;
	*(unsigned int*)((char*)ptr + 2*sizeof(unsigned int) + size - 1) = size;
}

void MemAllocator::setAllocated(void* ptr, char isAllocated) {
	if (!ptr){
		return;
	}

	unsigned int chunkSize = *(unsigned int*)ptr;

	if (isAllocated) {
		*(unsigned int*)ptr |= (1 << 31);
		*(unsigned int*)((char*)ptr + 2*sizeof(unsigned int) + chunkSize - 1) |= (1 << 31);
	}
	else {
		*(unsigned int*)ptr &= ~(1 << 31);
		*(unsigned int*)((char*)ptr + 2*sizeof(unsigned int) + chunkSize - 1) &= ~(1 << 31);
	}
}

void MemAllocator::mergeChunks(void* l, void* r) {
	if (!l || !r) {
		return;
	}

	setSize(l, chunkSize(l) + chunkSize(r));
	setAllocated(l, 0);
}

unsigned int MemAllocator::chunkSize(void* ptr) {
	return (*(unsigned int*)ptr & ~(1 << 31));
}

bool MemAllocator::isAllocated(void* ptr) {
	return (*(unsigned int*)ptr & (1 << 31));
}