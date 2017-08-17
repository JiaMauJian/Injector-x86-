#include <windows.h>
#include <ImageHlp.h>

using namespace std;

#pragma comment(lib, "ImageHlp.lib")

bool Flag(LPCSTR path, bool ASLR, bool DEP)
{
	LOADED_IMAGE PE;

	if (MapAndLoad(path, 0, &PE, 0, 0))
	{
		if (ASLR)
			PE.FileHeader->OptionalHeader.DllCharacteristics = IMAGE_DLLCHARACTERISTICS_DYNAMIC_BASE;
		else
			PE.FileHeader->OptionalHeader.DllCharacteristics = NULL;

		if (DEP)
			PE.FileHeader->OptionalHeader.DllCharacteristics = IMAGE_DLLCHARACTERISTICS_NX_COMPAT;
		else
			PE.FileHeader->OptionalHeader.DllCharacteristics = NULL;

		UnMapAndLoad(&PE);
		return true;
	}

	return false;
}

int main(void)
{
	Flag("testApp.exe", false, false);

	return 0;
}