#include <iostream>
#include <Windows.h>
#include <bitset>

using namespace std;

int main(){

	void* ptr = VirtualAlloc(NULL, 100, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

	//cout << (int)(char*)ptr << endl;
	
	//check if address is multiple of 8
	if ((int)(char*)ptr % 8 == 0) {
		//cout << "aligned" << endl;
	}

	*(int*)ptr = ~(1 << 31);
	*(int*)ptr /= 2;
	bitset<32> x(*(int*)ptr);

	cout << x << " " << (*(int*)ptr & ~(1 << 31) ) << endl;

	*(int*)ptr |= (1 << 31);

	bitset<32> y(*(int*)ptr);
	cout << y << " " << (*(int*)ptr & ~(1 << 31) )<< endl;

	char q = *(char*)((char*)ptr+3);
	bitset<8> s(q);
	cout << s << endl;

	VirtualFree(ptr, 100, MEM_RELEASE);

	std::cin.get();
	return 0;
}