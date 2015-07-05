#include <iostream>
#include <Windows.h>
#include <bitset>
#include "MemAllocator.h"

using namespace std;



int main(){

	MemAllocator m;
	char* ptr = (char*)m.MyMalloc(10);
	char* ptr1 = (char*)m.MyMalloc(66);
	char* ptr2 = (char*)m.MyMalloc(20);

	m.MyFree(ptr);
	m.MyFree(ptr1);
	m.MyFree(ptr2);

	//char* ptr3 = (char*)m.MyMalloc(20);
	//m.MyFree(ptr3);


	std::cin.get();
	return 0;
}