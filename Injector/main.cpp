#define _CRT_SECURE_NO_WARNINGS

#include <Windows.h>
#include <TlHelp32.h>
#include <string>

using namespace std;

DWORD FindProcessId(wstring processName)
{
	PROCESSENTRY32 processInfo;
	processInfo.dwSize = sizeof(processInfo);

	HANDLE processSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	if (processSnapshot == INVALID_HANDLE_VALUE)
		return 0;

	Process32First(processSnapshot, &processInfo);
	if (!processName.compare(processInfo.szExeFile))
	{
		CloseHandle(processSnapshot);
		return processInfo.th32ProcessID;
	}

	while (Process32Next(processSnapshot, &processInfo))
	{
		if (!processName.compare(processInfo.szExeFile))
		{
			CloseHandle(processSnapshot);
			return processInfo.th32ProcessID;
		}
	}

	CloseHandle(processSnapshot);
	return 0;
}

int main(void)
{
	// 記得要先關防毒		 

	LPCSTR DllPath = "D:\\MIF00-AUTO\\Injector(x86)\\Debug\\TargetDll.dll";

	DWORD processID = FindProcessId(L"testApp.exe");

	printf("PID=%d\n", processID);

	HANDLE processHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processID);

	if (processHandle == NULL)
	{
		printf("Error unable to open process. Error code: %d", GetLastError());		
		return -1;
	}

	printf("Process handle %d is ready\n", processID);

	HMODULE dllHandle = GetModuleHandle(L"Kernel32");

	if (dllHandle == NULL)
	{
		printf("Error unable to allocate kernel32 handle..Error code: %d. Press any key to exit...", GetLastError());
		return -1;
	}

	printf("kernel32 handle is ready\n");

	FARPROC loadLibraryAddress = GetProcAddress(dllHandle, "LoadLibraryA");

	if (loadLibraryAddress == NULL)
	{
		printf("Cannot get LoadLibraryA() address. Error code: %d. Press any key to exit", GetLastError());
		return -1;
	}

	printf("LoadLibrary() address is ready, %p\n", loadLibraryAddress);

	LPVOID pDllPath = VirtualAllocEx(processHandle, NULL, strlen(DllPath) + 1, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

	if (pDllPath == NULL)
	{
		printf("Error unable to alocate memmory in remote process. Error code: %d. Press any key to exit", GetLastError());
		return -1;
	}

	printf("Memory allocation succeeded\n");

	BOOL isSucceeded = WriteProcessMemory(processHandle, pDllPath, (LPVOID)DllPath, strlen(DllPath) + 1, NULL);

	if (isSucceeded == 0)
	{
		printf("Error unable to write memory . Error code: %d Press any key to exit...", GetLastError());
		return -1;
	}

	printf("Argument has been written\n");


	//HANDLE threadHandle = CreateRemoteThread(processHandle, NULL, 0, (LPTHREAD_START_ROUTINE)loadLibraryAddress, pDllPath, NULL, 0);
	// 用OllyDbg也可以看Injector Kerner32.dll LoadLibrary()的位址 = 0x76924977
	// 因為底層的dll位址都一樣(不同平台可能不同)，所以可以在testApp用LoadLibrary的位址來呼叫被Inject的Dll(TargetDll)
	HANDLE threadHandle = CreateRemoteThread(processHandle, NULL, 0, (LPTHREAD_START_ROUTINE)0x76924977, pDllPath, NULL, 0);

	if (threadHandle != NULL)
	{
		printf("Remote thread has been created\n");
	}

	WaitForSingleObject(threadHandle, INFINITE);

	printf("Dll path allocated at: %p", pDllPath);	

	getchar();

	// Free the memory allocated for our dll path
	VirtualFreeEx(processHandle, pDllPath, strlen(DllPath) + 1, MEM_RELEASE);
}