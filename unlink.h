#pragma once

#if defined(_WIN64) 
// ---------- x64 ----------
#define PEBoff 0x60
#define LDRoff 0x18

#define BaseDllNameOff 0x60 // BaseDllName is at 0x58, +0x8 for the UNICODE_STRING Buffer field → 0x60
#define Flink 0x0
#define Blink 0x8

#define InLoadOrderModuleList    0x10
#define InMemoryOrderModuleList  0x20
#define InInitOrderModuleList    0x30

#define InLoadOrderLinksOffset   0x00
#define InMemoryOrderLinksOffset 0x10
#define InInitOrderLinksOffset   0x20

#else
// ---------- x86 ----------
#define PEBoff 0x30
#define LDRoff 0xc 

#define BaseDllNameOff 0x30 // BaseDllName is at 0x2C, +0x4 for the UNICODE_STRING Buffer field → 0x30

#define Flink 0x0
#define Blink 0x4

#define InLoadOrderModuleList    0xc
#define InMemoryOrderModuleList  0x14
#define InInitOrderModuleList    0x1c

#define InLoadOrderLinksOffset   0x00
#define InMemoryOrderLinksOffset 0x8
#define InInitOrderLinksOffset   0x10
#endif


#include <iostream>
#include <winternl.h>

void Test() {
#if defined(_WIN64)
	uintptr_t peb = (uintptr_t)__readgsqword(PEBoff);
#else
	uintptr_t peb = (uintptr_t)__readfsdword(PEBoff);
#endif

	uintptr_t ldr = *(uintptr_t*)(peb + LDRoff);
	uintptr_t ListHead = (uintptr_t)(ldr + InLoadOrderModuleList);
	uintptr_t ListHead2 = (uintptr_t)(ldr + InMemoryOrderModuleList);
	uintptr_t ListHead3 = (uintptr_t)(ldr + InInitOrderModuleList);
	uintptr_t CurrentModule = *(uintptr_t*)(ListHead);

	std::cout << " ----------- InLoadOrderModuleList -----------" << std::endl;
	while (ListHead != CurrentModule) {
		const wchar_t* CurrentModuleName = *(const wchar_t**)(CurrentModule + BaseDllNameOff);		
		std::wcout << CurrentModuleName << std::endl;
		CurrentModule = *(uintptr_t*)(CurrentModule);
	}
	std::cout << " ----------- InMemoryOrderModuleList -----------" << std::endl;
	CurrentModule = *(uintptr_t*)(ListHead2);
	while (ListHead2 != CurrentModule) {
		const wchar_t* CurrentModuleName = *(const wchar_t**)(CurrentModule + BaseDllNameOff - InMemoryOrderLinksOffset);
		std::wcout << CurrentModuleName << std::endl;
		CurrentModule = *(uintptr_t*)(CurrentModule);
	}
	std::cout << " ----------- InInitOrderModuleList -----------" << std::endl;
	CurrentModule = *(uintptr_t*)(ListHead3);
	while (ListHead3 != CurrentModule) {
		const wchar_t* CurrentModuleName = *(const wchar_t**)(CurrentModule + BaseDllNameOff - InInitOrderLinksOffset);
		std::wcout << CurrentModuleName << std::endl;
		CurrentModule = *(uintptr_t*)(CurrentModule);
	}

	std::cout << " ----------- END -----------" << std::endl;
}


int dllUnlinking(const wchar_t* moduleName, int InLdr, int SubOffset) {
#if defined(_WIN64)
	uintptr_t peb = (uintptr_t)__readgsqword(PEBoff);
#else
	uintptr_t peb = (uintptr_t)__readfsdword(PEBoff);
#endif
	uintptr_t ldr = *(uintptr_t*)(peb + LDRoff);
	uintptr_t ListHead = (uintptr_t)(ldr + InLdr);
	uintptr_t CurrentModule = *(uintptr_t*)(ListHead);

	while (ListHead != CurrentModule) {
		const wchar_t* CurrentModuleName = *(const wchar_t**)(CurrentModule + BaseDllNameOff - SubOffset);
		if (_wcsicmp(moduleName, CurrentModuleName) == 0) {
			uintptr_t FlinkPrev = *(uintptr_t*)(CurrentModule + Blink);
			uintptr_t flink = *(uintptr_t*)(CurrentModule + Flink);
			uintptr_t BlinkNext = flink + Blink; 

			*(uintptr_t*)FlinkPrev = flink;
			*(uintptr_t*)BlinkNext = FlinkPrev;


			std::cout << "[+] The DLL was successfully unlinked!\n";
			return 0;


		}
		CurrentModule = *(uintptr_t*)(CurrentModule); // Flink to next module
	}
	std::cout << "[!] The entered module was not found!\n";
	return 1;

}

int dispatcher(const wchar_t* moduleName) {

	if (dllUnlinking(moduleName, InLoadOrderModuleList, InLoadOrderLinksOffset))     return 1;
	if (dllUnlinking(moduleName, InMemoryOrderModuleList, InMemoryOrderLinksOffset)) return 1;
	if (dllUnlinking(moduleName, InInitOrderModuleList, InInitOrderLinksOffset))     return 1;

	return 0;
}
