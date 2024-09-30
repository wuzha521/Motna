#include "预编译.h"
#include "dx11.h"
#include <iostream>

BOOL DeleteSizeOfHeaders(HMODULE MyImageBase)
{
    
    if (!*(BYTE*)MyImageBase)
        return false;

    PIMAGE_DOS_HEADER PDosHeader = NULL;
    PIMAGE_NT_HEADERS PNtHeaders = NULL;
    PIMAGE_FILE_HEADER PFileHeader = NULL;
    PIMAGE_OPTIONAL_HEADER64 POptionHeader = NULL;
    PDosHeader = (PIMAGE_DOS_HEADER)MyImageBase;
    PNtHeaders = (PIMAGE_NT_HEADERS)((DWORD64)PDosHeader + PDosHeader->e_lfanew);
    PFileHeader = PIMAGE_FILE_HEADER((DWORD64)PNtHeaders + 0x04);
    POptionHeader = (PIMAGE_OPTIONAL_HEADER64)((DWORD64)PFileHeader + IMAGE_SIZEOF_FILE_HEADER);
  
    DWORD DllProtect=NULL;
    VirtualProtect(MyImageBase,4,PAGE_EXECUTE_READWRITE, &DllProtect);
    memset(MyImageBase, 0, POptionHeader->SizeOfHeaders);
    VirtualProtect(MyImageBase, 4, DllProtect, &DllProtect);
    return true;
   
}


BOOL WINAPI DllMain(HMODULE hModule,DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    VMProtectBegin("DllMain");
    DeleteSizeOfHeaders(hModule);

    if (ul_reason_for_call == DLL_PROCESS_ATTACH)
    {
       
        
#if DEBUG
        AllocConsole();
        SetConsoleTitle(L"debug console");
        freopen("CONOUT$", "w", stdout);
        
#endif
        
        
        _beginthreadex(NULL, 0, (_beginthreadex_proc_type)Dx11Hook, NULL, 0, NULL);
  

    }

    return TRUE;
    VMProtectEnd();
}

