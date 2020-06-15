#include <windows.h>
#include <iostream>
#include <string>
#include <TlHelp32.h>

using std::wcin;
using std::cout;
using std::endl;
using std::wstring;



int Get_processID( const WCHAR* process_name ) {

    int processId = 0;

    HANDLE process_snap = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, NULL );
    if( process_snap != INVALID_HANDLE_VALUE ) {
        PROCESSENTRY32 p_Buffer;
        p_Buffer.dwSize = sizeof( PROCESSENTRY32 );
        if( Process32First( process_snap, &p_Buffer ) ) {
            do {
                if( !_wcsicmp( process_name, p_Buffer.szExeFile ) ) {
                    processId = p_Buffer.th32ProcessID;
                    break;
                }
            } while( Process32Next( process_snap, &p_Buffer ) );
        }
		CloseHandle( process_snap );
    }

    return processId;
}

int main() {

    do {

        cout << "Enter process name: ";
        wstring process_name;
        getline( wcin, process_name );
        DWORD process_id = Get_processID( process_name.c_str() );

        if( !process_id ) {
            cout << "Failed to find process!" << endl;
            break;
        }

        cout << "Enter name of the dll you whish to inject: ";
        wstring dll_name;
        getline( wcin, dll_name );

        HANDLE process_handle = OpenProcess( PROCESS_ALL_ACCESS, FALSE, process_id );
        if( !process_handle ) {
            cout << "Error getting process handle" << endl;
            break;
        }
        WCHAR nameBuffer[ MAX_PATH ];
        GetFullPathNameW( dll_name.c_str(), MAX_PATH, nameBuffer, NULL );
        int name_length = wcslen( nameBuffer );

        //allocate memory in remote process 
        LPVOID memAloc_address = VirtualAllocEx( process_handle, nullptr, name_length * 2, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE ); 
        
        if( !memAloc_address ) {
            cout << "Failed to allocate memory in target process" << endl;
            break;
        }

        if( !WriteProcessMemory( process_handle, memAloc_address, nameBuffer, name_length * 2, nullptr ) ) {
            cout << "Failed to write to target memory" << endl;
            break;
        }
        HMODULE k32_h = GetModuleHandle( L"kernel32.dll" );

        LPVOID	loadLibAddr = GetProcAddress( k32_h, "LoadLibraryW" );

        //invoke LoadLibrary 
        HANDLE hThread = CreateRemoteThread( process_handle, nullptr, NULL, ( LPTHREAD_START_ROUTINE )loadLibAddr, memAloc_address, NULL, NULL );

        if( !hThread ) {
            cout << "Failed to create remote thread in target process" << endl;
            break;
        }

        cout << "dll injected!" << endl;
        // let the thread finish and clean up
        WaitForSingleObject( hThread, INFINITE );

        //once done we close it
        CloseHandle( hThread );

    } while( 0 );

    std::cin.get();
    return 0;
}
