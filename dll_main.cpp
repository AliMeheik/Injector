#include <windows.h>
#include <iostream>

DWORD __stdcall Init( LPVOID param ) {
    //std::cout << "hello there!!!\n";
	MessageBox( NULL, L"hello there!!!", NULL, MB_OK );
	return TRUE;
}

BOOL __stdcall DllMain( HANDLE dll_handle, DWORD dw_Reason, LPVOID lpreserved ) {
	switch( dw_Reason ) {
	case DLL_PROCESS_ATTACH:
		HANDLE thread = CreateThread( nullptr, NULL, Init, nullptr, NULL, nullptr );
		CloseHandle( thread );
		break;
	}
	return TRUE;
}
