#pragma comment(lib,"vsynth-core")
#pragma comment(lib,"vsynth-stdlib")


#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

BOOL WINAPI DllMain(__in  HINSTANCE hinstDLL, __in  DWORD fdwReason, __in  LPVOID lpvReserved)
{
	return TRUE;
}
#endif
