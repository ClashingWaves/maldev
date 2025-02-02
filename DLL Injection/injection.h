#pragma once
#include <windows.h>
#include <stdio.h>

#define DLL L"dll\\crow.dll"
#define OKAY(MSG, ...) printf("[+] " MSG "\n", ##__VA_ARGS__)
#define INFO(MSG, ...) printf("[*] " MSG "\n", ##__VA_ARGS__)
#define WARN(MSG, ...) printf("[-] " MSG "\n", ##__VA_ARGS__)

/*!
 * @brief
 *  Prints out a function's error code with the function's name for easier debugging.
 *  This function is just a wrapper for GetLastError().
 *
 * @param FunctionName
 *  Name of the function.
 *
 * @note
 *  GetLastError() just reads from the _TEB at the LastError member/offset (_TEB->LastError),
 *  so *any* function/routine that does this can be used in place of GetLastError().
 *
 * @return Void.
 */
VOID PrettyFormat(_In_ LPCSTR FunctionName);

/*!
 * @brief
 *  Injects a target process with a specified dynamic link library (DLL).
 *
 *  Similar to the standard shellcode injection method, most of the steps
 *  we perform in this method are the same. The only difference is that
 *  we're injecting a library/module/DLL/whatever into a target process. Once a
 *  DLL is loaded in a process, its DllMain() function is executed automagically.
 *
 *  To do this injection, we get a valid handle on the target process, allocate
 *  some memory into it, write the path to our DLL into that memory region. Then,
 *  before anything else, we need to find the address of LoadLibrary() which takes
 *  in a module to load (or the path to one) as an argument. LoadLibrary() is
 *  exported from Kernel32.dll. So, we first need to get a handle on Kernel32,
 *  afterwhich, we look inside of this module and find the address of LoadLibrary()
 *  with the GetProcAddress() function.
 *
 *  With the address of LoadLibrary() now obtained, we can create a thread with it as
 *  our starting address. Also, we must supply the rBuffer (which holds our DLL path)
 *  as an argument for LoadLibrary(); which we do by setting CreateRemoteThread's
 *  lpParameter to. Effectively, we're doing p_LoadLibrary(rBuffer) which is the same
 *  as LoadLibrary(DllPath);
 *
 * @param DLLPath
 *  The path of the specified DLL.
 *
 * @param PID
 *  The pid of the target process.
 *
 * @param PathSize
 *  The size of the DLLPath parameter.
 *
 * @return Bool
 *  True if successful, false if not.
 */
BOOL DLLInjection(
    _In_ LPCWSTR DLLPath,
    _In_ CONST DWORD PID,
    _In_ CONST SIZE_T PathSize
);
