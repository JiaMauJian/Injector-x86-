// dllmain.cpp : Defines the entry point for the DLL application.
#include <Windows.h>
#include <stdio.h>

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH) {

		// Disbaled ASLR
		//num = 0018FF00, add() = 00411113, sub() = 00411118, fp() = 0018FEF4

		printf("\n-----------------------");
		printf("Hello, Injection\n");

		int *p = (int *)0x0018FF00;
		*p = 888;
		printf("change num = %d\n", *p);

		void(*pf)(int, int) = (void(*)(int, int))0x00411113;
		pf(50, 50);
		printf("Injection calls add() = %d\n", *p);

		void(**ppf)(int, int) = (void(**)(int, int))0x0018FEF4;
		(*ppf) = (void(*)(int, int))0x00411118;
		(*ppf)(-50, -50);
		printf("chnage add() to sub()\n");

		printf("-----------------------\n");

	}
	return TRUE;
}

