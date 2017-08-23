#define _CRT_SECURE_NO_WARNINGS

#define _WIN32_WINNT _WIN32_WINNT_WINXP
/*I just went through OpenProcess API in msdn, it says below for for PROCESS_ALL_ACCESS. 
I think this was the issue which you were facing.
Windows Server 2003 and Windows XP/2000:   
The size of the PROCESS_ALL_ACCESS flag increased on Windows Server 2008 and Windows Vista. 
If an application compiled for Windows Server 2008 and Windows Vista is run on Windows Server 2003 or Windows XP/2000, 
the PROCESS_ALL_ACCESS flag is too large and the function specifying this flag fails with ERROR_ACCESS_DENIED. 
To avoid this problem, specify the minimum set of access rights required for the operation. 
If PROCESS_ALL_ACCESS must be used, set _WIN32_WINNT to the minimum operating system targeted by your application 
(for example,#define _WIN32_WINNT _WIN32_WINNT_WINXP). For more information, see Using the Windows Headers .*/

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

	//LPCSTR DllPath = "D:\\MIF00-AUTO\\Injector(x86)\\Debug\\TargetDll.dll";
	LPCSTR DllPath = "C:\\injection\\TargetDll.dll";

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

	printf("Memory allocation succeeded, Dll path allocated at %p\n", pDllPath);

	BOOL isSucceeded = WriteProcessMemory(processHandle, pDllPath, (LPVOID)DllPath, strlen(DllPath) + 1, NULL);

	if (isSucceeded == 0)
	{
		printf("Error unable to write memory . Error code: %d Press any key to exit...", GetLastError());
		return -1;
	}

	printf("Argument has been written\n");


	HANDLE threadHandle = CreateRemoteThread(processHandle, NULL, 0, (LPTHREAD_START_ROUTINE)loadLibraryAddress, pDllPath, NULL, 0);

	if (threadHandle != NULL)
	{
		printf("Remote thread has been created\n");
	}

	WaitForSingleObject(threadHandle, INFINITE);

	getchar();

	// Free the memory allocated for our dll path
	VirtualFreeEx(processHandle, pDllPath, strlen(DllPath) + 1, MEM_RELEASE);
}