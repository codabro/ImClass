#pragma once

#include <Windows.h>
#include <vector>
#include <tlhelp32.h>

struct processSnapshot {
    std::wstring name;
    DWORD pid;
};

struct moduleInfo {
    uintptr_t base;
    DWORD size;
};

namespace mem {
    std::vector<processSnapshot> processes;
    HANDLE memHandle;
    DWORD pid;

    bool getProcessList();
    HANDLE openHandle(DWORD pid);
    bool getModuleInfo(DWORD pid, const wchar_t* moduleName, moduleInfo* info);
    bool read(uintptr_t address, void* buf, uintptr_t size);
    bool write(uintptr_t address, const void* buf, uintptr_t size);
    bool initProcess(const wchar_t* processName);
    bool initProcess(DWORD pid);
}

bool mem::getProcessList() {
    processes.clear();

    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        return false;
    }

    PROCESSENTRY32 processEntry;
    processEntry.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(hSnapshot, &processEntry)) {
        do {
            if (processEntry.th32ProcessID == 0) {
                continue;
            }

            if (!wcscmp(processEntry.szExeFile, L"System") || !wcscmp(processEntry.szExeFile, L"Registry")) {
                continue;
            }

            processSnapshot snapshot{ processEntry.szExeFile, processEntry.th32ProcessID };
            processes.push_back(snapshot);
        } while (Process32Next(hSnapshot, &processEntry));
    }

    CloseHandle(hSnapshot);
    return true;
}

bool mem::getModuleInfo(DWORD pid, const wchar_t* moduleName, moduleInfo* info) {
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
    if (snapshot == INVALID_HANDLE_VALUE) {
        return false;
    }

    MODULEENTRY32W entry = {};
    entry.dwSize = sizeof(MODULEENTRY32W);

    if (Module32FirstW(snapshot, &entry)) {
        do {
            if (wcsstr(moduleName, entry.szModule)) {
                info->base = (uintptr_t)entry.modBaseAddr;
                info->size = entry.modBaseSize;
                return true;
            }
        } while (Module32NextW(snapshot, &entry));
    }

    CloseHandle(snapshot);

    return true;
}

HANDLE mem::openHandle(DWORD pid) {
    memHandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | PROCESS_VM_WRITE, FALSE, pid);
    return memHandle;
}

bool mem::read(uintptr_t address, void* buf, uintptr_t size) {
    size_t sizeRead;
    return ReadProcessMemory(memHandle, (void*)address, buf, size, &sizeRead);
}

bool mem::write(uintptr_t address, const void* buf, uintptr_t size) {
    size_t sizeWritten;
    return WriteProcessMemory(memHandle, (void*)address, buf, size, &sizeWritten);
}

bool mem::initProcess(const wchar_t* processName) {
    if (!getProcessList()) {
        return false;
    }

    for (auto& proc : processes) {
        if (!wcscmp(proc.name.c_str(), processName)) {
            if (openHandle(proc.pid)) {
                return true;
            }
        }
    }

    return false;
}

bool mem::initProcess(DWORD pid) {
    mem::pid = pid;
    return openHandle(pid);
}

template <typename T>
T Read(uintptr_t address) {
    T response{};
    if (!address) {
        return response;
    }

    mem::read(address, &response, sizeof(T));

    return response;
}

template <typename T>
bool Write(uintptr_t address, const T& value) {
    return mem::write(address, &value, sizeof(T));
}

template <int n>
struct readBuf {
    BYTE data[n] = {};

    readBuf() {
        memset(data, 0, n);
    }
};