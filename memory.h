#pragma once

#include <chrono>
#include <Windows.h>
#include <vector>
#include <tlhelp32.h>
#include <unordered_map>
#include <winternl.h>
#include <Psapi.h>

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

struct RTTIClassHierarchyDescriptor {
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

struct funcExport
{
	std::string name;
	uintptr_t address;
};

namespace mem {
    inline std::vector<processSnapshot> processes;
    inline HANDLE memHandle;
    inline DWORD g_pid;
    inline std::vector<moduleInfo> moduleList;
    inline std::unordered_map<uintptr_t, std::string> g_ExportMap;
    inline bool x32 = false;

    bool getProcessList();
    HANDLE openHandle(DWORD pid);
    uintptr_t getPEB();
    bool getModuleInfo(DWORD pid, const wchar_t* moduleName, moduleInfo* info);
    void getModules();
    void getSections(const moduleInfo& info, std::vector<moduleSection>& dest);
    bool isPointer(uintptr_t address, pointerInfo* info);
    bool rttiInfo(uintptr_t address, std::string& out);
    std::vector<funcExport> gatherRemoteExports(uintptr_t moduleBase);
    void gatherExports();
    uintptr_t getExport(const std::string& moduleName, const std::string& exportName);

    bool read(uintptr_t address, void* buf, uintptr_t size);
    bool write(uintptr_t address, const void* buf, uintptr_t size);
    bool initProcess(DWORD pid);
    bool isX32(HANDLE handle);

    inline bool activeProcess = false;
    inline std::chrono::steady_clock::time_point lastCheck = std::chrono::steady_clock::now();
    inline constexpr std::chrono::milliseconds PROCESS_CHECK_INTERVAL{ 1000 };

    bool isProcessAlive();
    void cleanDeadProcess();
}

inline bool mem::isX32(HANDLE handle) {
    BOOL wow64 = FALSE;
    if (!IsWow64Process(handle, &wow64)) {
        return false;
    }

    return wow64;
}

template <typename T>
T Read(uintptr_t address);
inline bool mem::rttiInfo(uintptr_t address, std::string& out) {
    uintptr_t objectLocatorPtr = Read<uintptr_t>(address - sizeof(void*));
    if (!objectLocatorPtr) {
        return false;
    }

    auto objectLocator = Read<RTTICompleteObjectLocator>(objectLocatorPtr);
    auto baseModule = objectLocatorPtr - objectLocator.selfOffset;

    auto hierarchy = Read<RTTIClassHierarchyDescriptor>(baseModule + objectLocator.hierarchyDescriptorOffset);
    uintptr_t classArray = baseModule + hierarchy.pBaseClassArray;

    for (DWORD i = 0; i < hierarchy.numBaseClasses; i++) {
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

DECLSPEC_NOINLINE bool mem::isPointer(uintptr_t address, pointerInfo* info) {
	for (auto& module : moduleList) {
		if (module.base <= address && address <= module.base + module.size) {
			info->moduleName = module.name;

			// default to unknown as pointers to places like the pe header don't get caught by any of these cases
			strcpy_s(info->section, 8, "UNK");

			for (auto& section : module.sections) {
				if (section.base <= address && address < section.base + section.size) {
					memcpy(info->section, section.name, 8);
					break;
				}
			}
			return true;
		}
	}

    MEMORY_BASIC_INFORMATION mbi;
    if (VirtualQueryEx(memHandle, reinterpret_cast<LPCVOID>(address), &mbi, sizeof(mbi))) {
        return (mbi.Type == MEM_PRIVATE && mbi.State == MEM_COMMIT);
    }

    return false;
}

inline bool mem::getProcessList() {
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

inline void mem::getModules() {
	moduleList.clear();

	uintptr_t pebAddress = getPEB();
	if (pebAddress == NULL)
		return;

	uintptr_t PEBldrAddress = pebAddress + offsetof(PEB, PEB::Ldr);
	uintptr_t PEBldr = 0;
	read(PEBldrAddress, &PEBldr, sizeof(uintptr_t));

	uintptr_t moduleListHead = PEBldr + offsetof(PEB_LDR_DATA, PEB_LDR_DATA::InMemoryOrderModuleList);
	uintptr_t currentLink = 0;
	read(moduleListHead, &currentLink, sizeof(uintptr_t));

	while (currentLink != moduleListHead)
	{
		uintptr_t entryBase = currentLink - offsetof(LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks);

		// BaseDllName is immediately after FullDllName
		UNICODE_STRING dllString;
		uintptr_t linkDllNameAddress = entryBase + offsetof(LDR_DATA_TABLE_ENTRY, LDR_DATA_TABLE_ENTRY::FullDllName) + sizeof(UNICODE_STRING);
		read(linkDllNameAddress, &dllString, sizeof(UNICODE_STRING));

		size_t charCount = (dllString.Length / sizeof(wchar_t)) + 1;
		std::vector<wchar_t> dllName(charCount, L'\0');
		read(reinterpret_cast<uintptr_t>(dllString.Buffer), dllName.data(), dllString.Length);

		uintptr_t baseAddress = 0;
		read(entryBase + offsetof(LDR_DATA_TABLE_ENTRY, LDR_DATA_TABLE_ENTRY::DllBase), &baseAddress, sizeof(baseAddress));

		ULONG moduleSize = 0;
		// reserved in winternl but SizeOfImage
		read(entryBase + offsetof(LDR_DATA_TABLE_ENTRY, LDR_DATA_TABLE_ENTRY::DllBase) + 0x10, &moduleSize, sizeof(moduleSize));

		std::wstring lDllNameW(dllName.begin(), dllName.end());
		moduleInfo info;
		info.name = std::string(lDllNameW.begin(), lDllNameW.end());
		info.base = baseAddress;
		info.size = moduleSize;

		getSections(info, info.sections);
		moduleList.push_back(info);

		read(currentLink, &currentLink, sizeof(currentLink));
	}

	return;
}

inline void mem::getSections(const moduleInfo& info, std::vector<moduleSection>& dest) {
    BYTE buf[4096];
    read(info.base, buf, sizeof(buf));

    auto dosHeader = (IMAGE_DOS_HEADER*)buf;
    auto ntHeader = (IMAGE_NT_HEADERS*)(buf + dosHeader->e_lfanew);
    auto sectionHeader = IMAGE_FIRST_SECTION(ntHeader);

    for (WORD i = 0; i < ntHeader->FileHeader.NumberOfSections; i++) {
        auto section = sectionHeader[i];
        moduleSection sectionInfo;
        sectionInfo.base = info.base + section.VirtualAddress;
        sectionInfo.size = section.Misc.VirtualSize;
        memcpy(sectionInfo.name, section.Name, 8);
        dest.push_back(sectionInfo);
    }
}



typedef NTSTATUS(*_NtQueryInformationProcess)(IN HANDLE ProcessHandle,
	IN PROCESSINFOCLASS ProcessInformationClass,
	OUT PVOID ProcessInformation,
	IN ULONG ProcessInformationLength,
	OUT PULONG ReturnLength OPTIONAL);

inline uintptr_t mem::getPEB()
{
    PROCESS_BASIC_INFORMATION processInformation;
    ULONG written = 0;

    HMODULE hNtdll = GetModuleHandleA("ntdll.dll");

    static _NtQueryInformationProcess query = (_NtQueryInformationProcess)GetProcAddress(hNtdll, "NtQueryInformationProcess");

    NTSTATUS result = query(memHandle, ProcessBasicInformation, &processInformation, sizeof(PROCESS_BASIC_INFORMATION), &written);

    return reinterpret_cast<uintptr_t>(processInformation.PebBaseAddress);
}

inline bool mem::getModuleInfo(DWORD pid, const wchar_t* moduleName, moduleInfo* info) {

	uintptr_t pebAddress = getPEB();

	if (pebAddress == NULL)
		return false;

	uintptr_t PEBldrAddress = pebAddress + offsetof(PEB, PEB::Ldr);
	uintptr_t PEBldr = 0;
	read(PEBldrAddress, &PEBldr, sizeof(uintptr_t));

	uintptr_t moduleListHead = PEBldr + offsetof(PEB_LDR_DATA, PEB_LDR_DATA::InMemoryOrderModuleList);

	uintptr_t currentLink = 0;
	read(moduleListHead, &currentLink, sizeof(uintptr_t));

	while (currentLink != moduleListHead)
	{
		uintptr_t entryBase = currentLink - offsetof(LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks);


		// https://www.geoffchappell.com/studies/windows/km/ntoskrnl/inc/api/ntldr/ldr_data_table_entry.htm
		UNICODE_STRING dllString;
		// base dll name is directly after full dll name
		uintptr_t linkDllNameAddress = entryBase + offsetof(LDR_DATA_TABLE_ENTRY, LDR_DATA_TABLE_ENTRY::FullDllName) + sizeof(UNICODE_STRING);

		// this is going to give an incorrect pointer to the string located inside dllString
		// however the length will be correct, so the length will be used to construct our own string with the contents
		// of the original
		read(linkDllNameAddress, &dllString, sizeof(UNICODE_STRING));

		size_t charCount = (dllString.Length / sizeof(wchar_t)) + 1;
		std::vector<wchar_t> dllName(charCount, L'\0');

		read(reinterpret_cast<uintptr_t>(dllString.Buffer), dllName.data(), dllString.Length);

		uintptr_t baseAddress = 0;
		read(entryBase + offsetof(LDR_DATA_TABLE_ENTRY, LDR_DATA_TABLE_ENTRY::DllBase), &baseAddress, sizeof(baseAddress));

		ULONG moduleSize = 0;

		// reserved in winternl but SizeOfImage
		read(entryBase + offsetof(LDR_DATA_TABLE_ENTRY, LDR_DATA_TABLE_ENTRY::DllBase) + 0x10, &moduleSize, sizeof(moduleSize));

		std::wstring lDllNameW(dllName.begin(), dllName.end());
		std::string lDllName(lDllNameW.begin(), lDllNameW.end());

		if (wcscmp(moduleName, dllName.data()) == 0)
		{
			// found it!!!

			info->base = baseAddress;
			info->size = moduleSize;
			info->name = lDllName;

			getSections(*info, info->sections);

			return true;
		}
		read(currentLink, &currentLink, sizeof(currentLink));
	}

	return false;
}

inline HANDLE mem::openHandle(const DWORD pid) {
    memHandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | PROCESS_VM_WRITE, FALSE, pid);
    return memHandle;
}

inline bool mem::read(uintptr_t address, void* buf, uintptr_t size) {
    SIZE_T sizeRead;
    return ReadProcessMemory(memHandle, reinterpret_cast<LPCVOID>(address), buf, size, &sizeRead);
}

inline bool mem::write(uintptr_t address, const void* buf, uintptr_t size) {
    SIZE_T sizeWritten;
    return WriteProcessMemory(memHandle, reinterpret_cast<LPVOID>(address), buf, size, &sizeWritten);
}

inline std::vector<funcExport> mem::gatherRemoteExports(uintptr_t moduleBase)
{
	std::vector<funcExport> exports;
	IMAGE_DOS_HEADER dosHeader;

	if (!read(moduleBase, &dosHeader, sizeof(IMAGE_DOS_HEADER)) || dosHeader.e_magic != IMAGE_DOS_SIGNATURE) {
		return exports;
	}

	IMAGE_NT_HEADERS ntHeaders;

	if (!read(moduleBase + dosHeader.e_lfanew, &ntHeaders, sizeof(IMAGE_NT_HEADERS)) ||
		ntHeaders.Signature != IMAGE_NT_SIGNATURE) {
		return exports;
	}

	DWORD exportDirRVA = ntHeaders.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
	DWORD exportDirSize = ntHeaders.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size;

	if (!exportDirRVA || !exportDirSize) {
		return exports;
	}

	IMAGE_EXPORT_DIRECTORY exportDir;

	if (!read(moduleBase + exportDirRVA, &exportDir, sizeof(IMAGE_EXPORT_DIRECTORY))) {
		return exports;
	}

	std::vector<DWORD> functionRVAs(exportDir.NumberOfFunctions);

	if (!read(moduleBase + exportDir.AddressOfFunctions, functionRVAs.data(),
		exportDir.NumberOfFunctions * sizeof(DWORD))) {
		return exports;
	}

	std::vector<DWORD> nameRVAs(exportDir.NumberOfNames);

	if (!read(moduleBase + exportDir.AddressOfNames, nameRVAs.data(),
		exportDir.NumberOfNames * sizeof(DWORD))) {
		return exports;
	}

	std::vector<WORD> ordinals(exportDir.NumberOfNames);

	if (!read(moduleBase + exportDir.AddressOfNameOrdinals, ordinals.data(),
		exportDir.NumberOfNames * sizeof(WORD))) {
		return exports;
	}


	exports.reserve(exportDir.NumberOfNames);

    // TODO: refactor this to use less individual read calls (most names end up in the same pages anyway...)
	for (DWORD i = 0; i < exportDir.NumberOfNames; ++i) {
		char nameBuffer[256] = { 0 };
		DWORD nameRVA = nameRVAs[i];

        DWORD readSize = 255;

        if (i + 1 < exportDir.NumberOfNames) {
            DWORD nextNameRVA = nameRVAs[i + 1];
            DWORD NameDelta = nextNameRVA - nameRVA;
            if (NameDelta > 0 && NameDelta < 255)
                readSize = NameDelta; // don't need to read more than this delta
        }


		if (!read(moduleBase + nameRVA, nameBuffer, readSize)) {
			continue;
		}

		std::string exportName = nameBuffer;
		if (exportName.empty()) {
			continue;
		}

		WORD ordinal = ordinals[i];
		if (ordinal >= exportDir.NumberOfFunctions) {
			continue;
		}

		DWORD functionRVA = functionRVAs[ordinal];
		if (functionRVA >= exportDirRVA && functionRVA < (exportDirRVA + exportDirSize)) {
			continue;
		}

		uintptr_t functionAddress = moduleBase + functionRVA;
		funcExport info;
		info.name = exportName;
		info.address = functionAddress;
		exports.push_back(std::move(info));
	}

	return exports;
}

inline void mem::gatherExports()
{
	g_ExportMap.clear();

	for (auto& module : moduleList) {
		char modulePath[MAX_PATH] = { 0 };
		if (K32GetModuleFileNameExA(memHandle, reinterpret_cast<HMODULE>(module.base), modulePath, MAX_PATH)) {
			auto exports = gatherRemoteExports(module.base);

			for (const auto& exp : exports) {
				g_ExportMap[exp.address] = module.name + "!" + exp.name;
			}
		}
	}
}

inline uintptr_t mem::getExport(const std::string& moduleName, const std::string& exportName)
{
	for (auto& module : moduleList) {
		if (_stricmp(module.name.c_str(), moduleName.c_str()) == 0) {
			for (auto& pair : g_ExportMap) {
				std::string fullExport = module.name + "!" + exportName;
				if (pair.second == fullExport) {
					return pair.first;
				}
			}
			return 0;
		}
	}
	return 0;
}

inline bool mem::isProcessAlive()
{
	if (!memHandle || memHandle == INVALID_HANDLE_VALUE)
        return false;

    auto curTime = std::chrono::steady_clock::now();

    if (curTime - lastCheck < PROCESS_CHECK_INTERVAL)
        return activeProcess;
    
    lastCheck = curTime;

    DWORD exitCode;
	if (!GetExitCodeProcess(memHandle, &exitCode) || exitCode != STILL_ACTIVE) {
		activeProcess = false;
		return false;
	}

	activeProcess = true;
	return true;
}

// used internally by ui::cleanDeadProcess
inline void mem::cleanDeadProcess() {
	if (memHandle && memHandle != INVALID_HANDLE_VALUE) {
		CloseHandle(memHandle);
		memHandle = nullptr;
	}

	moduleList.clear();
	g_ExportMap.clear();
	g_pid = 0;
	activeProcess = false;
}

extern void initClasses(bool);
inline bool mem::initProcess(DWORD pid) {
    mem::g_pid = pid;
    if (openHandle(pid)) {
        getModules();
        bool newX32 = isX32(memHandle);
        initClasses(newX32);
        gatherExports();
        return true;
    }
    return false;
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