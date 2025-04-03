#pragma once

#include <Windows.h>
#include <vector>
#include <tlhelp32.h>

struct processSnapshot {
    std::wstring name;
    DWORD pid;
};

struct moduleSection {
    uintptr_t base;
    DWORD size;
    char name[8];
};

struct moduleInfo {
    uintptr_t base;
    DWORD size;
    std::vector<moduleSection> sections;
    std::string name;
};

struct pointerInfo {
    char section[8] = { 0 };
    std::string moduleName;
};

struct RTTICompleteObjectLocator {
    int signature;
    int offset;
    int cdOffset;
    DWORD typeDescriptorOffset;
    DWORD hierarchyDescriptorOffset;
    DWORD selfOffset;
};

typedef const struct RTTIClassHierarchyDescriptor {
    DWORD signature;
    DWORD attributes;
    DWORD numBaseClasses;
    DWORD pBaseClassArray;
};

struct TypeDescriptor {
    uintptr_t pVFTable;
    uintptr_t spare;
    char name[60];
};

namespace mem {
    std::vector<processSnapshot> processes;
    HANDLE memHandle;
    DWORD pid;
    std::vector<moduleInfo> moduleList;

    bool getProcessList();
    HANDLE openHandle(DWORD pid);
    bool getModuleInfo(DWORD pid, const wchar_t* moduleName, moduleInfo* info);
    void getModules();
    void getSections(moduleInfo& info, std::vector<moduleSection>& dest);
    bool isPointer(uintptr_t address, pointerInfo* info);
    bool rttiInfo(uintptr_t address, std::string& out);

    bool read(uintptr_t address, void* buf, uintptr_t size);
    bool write(uintptr_t address, const void* buf, uintptr_t size);
    bool initProcess(const wchar_t* processName);
    bool initProcess(DWORD pid);
}

template <typename T>
T Read(uintptr_t address);
bool mem::rttiInfo(uintptr_t address, std::string& out) {
    uintptr_t objectLocatorPtr = Read<uintptr_t>(address - sizeof(void*));
    if (!objectLocatorPtr) {
        return false;
    }

    auto objectLocator = Read<RTTICompleteObjectLocator>(objectLocatorPtr);
    auto baseModule = objectLocatorPtr - objectLocator.selfOffset;

    auto hierarchy = Read<RTTIClassHierarchyDescriptor>(baseModule + objectLocator.hierarchyDescriptorOffset);
    uintptr_t classArray = baseModule + hierarchy.pBaseClassArray;

    for (int i = 0; i < hierarchy.numBaseClasses; i++) {
        uintptr_t classDescriptor = baseModule + Read<DWORD>(classArray + i * sizeof(DWORD));
        DWORD typeDescriptorOffset = Read<DWORD>(classDescriptor);
        auto typeDescriptor = Read<TypeDescriptor>(baseModule + typeDescriptorOffset);

        std::string name = typeDescriptor.name;
        if (!name.ends_with("@@")) {
            return false;
        }

        name = name.substr(4);
        name = name.substr(0, name.size() - 2);

        out = out + " : " + name;
    }

    return true;
}

bool mem::isPointer(uintptr_t address, pointerInfo* info) {
    for (int i = 0; i < moduleList.size(); i++) {
        auto& module = moduleList[i];
        if (module.base <= address && address <= module.base + module.size) {
            info->moduleName = module.name;
            for (int j = 0; j < module.sections.size(); j++) {
                auto& section = module.sections[j];
                if (section.base <= address && address < section.base + section.size) {
                    memcpy(info->section, section.name, 8);
                    break;
                }
            }
            return true;
        }
    }

    MEMORY_BASIC_INFORMATION mbi;
    if (VirtualQueryEx(memHandle, (LPCVOID)address, &mbi, sizeof(mbi))) {
        return (mbi.Type == MEM_PRIVATE && mbi.State == MEM_COMMIT);
    }

    return false;
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

void mem::getModules() {
    moduleList.clear();

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
    if (snapshot == INVALID_HANDLE_VALUE) {
        return;
    }

    MODULEENTRY32W entry = {};
    entry.dwSize = sizeof(MODULEENTRY32W);

    if (Module32FirstW(snapshot, &entry)) {
        do {
            std::wstring entryName = entry.szModule;
            moduleInfo info;
            info.name = std::string(entryName.begin(), entryName.end());
            info.base = (uintptr_t)entry.modBaseAddr;
            info.size = entry.modBaseSize;
            getSections(info, info.sections);

            moduleList.push_back(info);
        } while (Module32NextW(snapshot, &entry));
    }

    CloseHandle(snapshot);

    return;
}

void mem::getSections(moduleInfo& info, std::vector<moduleSection>& dest) {
    BYTE buf[4096];
    read(info.base, buf, sizeof(buf));

    auto dosHeader = (IMAGE_DOS_HEADER*)buf;
    auto ntHeader = (IMAGE_NT_HEADERS*)(buf + dosHeader->e_lfanew);
    auto sectionHeader = IMAGE_FIRST_SECTION(ntHeader);

    for (int i = 0; i < ntHeader->FileHeader.NumberOfSections; i++) {
        auto section = sectionHeader[i];
        moduleSection sectionInfo;
        sectionInfo.base = info.base + section.VirtualAddress;
        sectionInfo.size = section.Misc.VirtualSize;
        memcpy(sectionInfo.name, section.Name, 8);
        dest.push_back(sectionInfo);
    }
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

    return false;
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
                getModules();
                return true;
            }
        }
    }

    return false;
}

bool mem::initProcess(DWORD pid) {
    mem::pid = pid;
    if (openHandle(pid)) {
        getModules();
        return true;
    }
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