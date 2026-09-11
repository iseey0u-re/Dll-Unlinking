#include <iostream>
#include "unlink.h"

int main() {

	wchar_t dllName[256] = {};
	char YesNo = 0;

	std::cout << "Enter the name of the DLL you want to hide (example.dll): ";
	if (std::wcin >> dllName) {
		int status = dispatcher(dllName);
		if (status == 0) {
			std::cout << "[?] Do you want to check the result? (y/n): ";
			std::cin >> YesNo;
			if (YesNo == 'y') {
				Test();
			}
		}
	}

	
	return 0;

}