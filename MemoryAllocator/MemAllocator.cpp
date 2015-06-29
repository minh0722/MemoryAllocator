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



}

MemAllocator::~MemAllocator() {
	VirtualFree(memoryPool, poolSize, MEM_RELEASE);
}

void* MemAllocator::MyMalloc(size_t size) {
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
}

void MemAllocator::setAllocated(void* ptr, char isAllocated) {
	if (!ptr){
		return;
	}

	if (isAllocated) {
		*(char*)ptr |= (1 << 7);
	}
	else {
		*(char*)ptr |= 0;
	}
}

unsigned int MemAllocator::getSize(void* ptr) {
	return  (*(int*)ptr & ~(1 << 31));
}