#pragma once

#include <ios>
#include <optional>
#include <regex>
#include <string>

#include "memory.h"


enum class PatternType {
	IDA_SIGNATURE,
	BYTE_PATTERN,
	UNKNOWN
};

struct PatternScanResult {
	std::vector<uintptr_t> matches; // include multiple matches to allow for user selection
};

class PatternInfo {
public:
	PatternType type;
	std::string pattern; // always trimmed on creation to not have whitespace


	inline std::string toString()
	{
		switch (this->type)
		{
		case PatternType::IDA_SIGNATURE:
			return "IDA Signature";
		case PatternType::BYTE_PATTERN:
			return "Byte Pattern";
		case PatternType::UNKNOWN:
		default:
			return "Unknown";
		}
	}
};

namespace pattern
{
	std::optional<PatternInfo> detectPatternType(const std::string& in);
	std::optional<PatternScanResult> scanPattern(const PatternInfo& patternInfo, const std::string& dllName);
	std::optional<PatternScanResult> findBytePattern(uintptr_t baseAddress, size_t size, uint8_t* pattern, const char* mask);
	bool patternToMask(const PatternInfo& patternInfo, std::vector<uint8_t>& outBytes, std::string& outMask);
}

inline std::optional<PatternInfo> pattern::detectPatternType(const std::string& in)
{
	auto trimmedInput = in;
	trimmedInput.erase(0, trimmedInput.find_first_not_of(" \t\n\r\f\v"));
	trimmedInput.erase(trimmedInput.find_last_not_of(" \t\n\r\f\v") + 1);

	if (trimmedInput.empty()) {
		return std::nullopt;
	}

	// double check these, I don't use byte patterns ever
	std::regex idaRegex(R"(^([0-9A-Fa-f]{2}|\?{1,2})(\s+([0-9A-Fa-f]{2}|\?{1,2}))*$)");
	std::regex byteRegex(R"(^(\\x[0-9A-Fa-f]{2}|\\x\?)*(\\x[0-9A-Fa-f]{2}|\\x\?)+$)");

	PatternInfo info;

	if (std::regex_match(trimmedInput, idaRegex)) {
		info.type = PatternType::IDA_SIGNATURE;
		info.pattern = trimmedInput;
		return info;
	}

	/*
	if (std::regex_match(trimmedInput, byteRegex)) {
		info.type = PatternType::BYTE_PATTERN;
		info.pattern = trimmedInput;
		return info;
	}
	*/

	// TODO: fix byte pattern regex, temporary solution below.
	if (trimmedInput.find("\\x") != std::string::npos) {
		info.type = PatternType::BYTE_PATTERN;
		info.pattern = trimmedInput;
		return info;
	}

	return std::nullopt;
}

inline bool pattern::patternToMask(const PatternInfo& patternInfo, std::vector<uint8_t>& outBytes, std::string& outMask)
{
	const std::string& pattern = patternInfo.pattern;

	outBytes.clear();
	outMask.clear();

	if (patternInfo.type == PatternType::IDA_SIGNATURE) {

		std::string byteStr;
		size_t length = pattern.length();
		for (size_t i = 0; i <= length; i++) {
			char c = pattern[i];

			if (c == ' ' || i == length) { // processes the last byte at spaces, but it will skip the last byte if not specified otherwise
				if (!byteStr.empty()) {
					if (byteStr == "?" || byteStr == "??") {
						outBytes.push_back(0);
						outMask += '?';
					}
					else {
						outBytes.push_back((uint8_t)(std::stoi(byteStr, nullptr, 16)));
						outMask += 'x';
					}
					byteStr.clear();
				}
			}
			else {
				byteStr += c;
			}
		}

		return true;

	}

	if (patternInfo.type == PatternType::BYTE_PATTERN) {
		for (size_t i = 0; i < pattern.length();) { // don't iterate here, it can be incremented by varying numbers

			auto currentCharacter = pattern[i];

			if (currentCharacter == '\\' && i + 3 < pattern.length() && pattern[i + 1] == 'x')
			{
				std::string byteStr = pattern.substr(i + 2, 2);
				outBytes.push_back((uint8_t)(std::stoi(byteStr, nullptr, 16)));
				outMask += 'x';
				i += 4;
			}
			else if (currentCharacter == '?') {
				outBytes.push_back(0);  // just a placeholder, never used
				outMask += '?';

				while (i < pattern.length() && pattern[i] == '?') {
					i++;
				}
			}
			else {
				i++;
			}
		}
		return true;
	}
}

inline std::optional<PatternScanResult> pattern::findBytePattern(uintptr_t baseAddress, size_t size, uint8_t* pattern, const char* mask) {
	PatternScanResult result;

	size_t patternLength = strlen(mask);

	if (patternLength == 0)
		return std::nullopt;

	if (size < patternLength)
		return std::nullopt;

	std::vector<uint8_t> buffer(size);

	SIZE_T bytesRead;
	if (!ReadProcessMemory(mem::memHandle, (LPCVOID)(baseAddress), buffer.data(), size, &bytesRead) || bytesRead < patternLength) {
		return std::nullopt;
	}

	for (size_t i = 0; i <= bytesRead - patternLength; i++) {
		bool found = true;

		for (size_t j = 0; j < patternLength; j++) {

			if (mask[j] == '?')
				continue;

			if (buffer[i + j] != pattern[j]) {
				found = false;
				break;
			}
		}

		if (found) {
			result.matches.push_back(baseAddress + i);
		}
	}

	if (!result.matches.empty()) {
		return result;
	}

	return std::nullopt;
}


inline std::optional<PatternScanResult> pattern::scanPattern(const PatternInfo& patternInfo, const std::string& dllName)
{

	std::wstring wideName(dllName.begin(), dllName.end()); // windows api requires widestring, maybe change input type?
	DWORD pid = mem::pid;

	if (!pid)
		return std::nullopt;

	moduleInfo moduleData;
	if (!mem::getModuleInfo(pid, wideName.c_str(), &moduleData))
	{
		return std::nullopt; // TODO: add failure reasons to the ui such as not finding the module
	}

	std::vector<uint8_t> patternBytes;
	std::string mask;

	if (!patternToMask(patternInfo, patternBytes, mask)) {
		return std::nullopt;
	}

	auto result = findBytePattern(
		moduleData.base,
		moduleData.size,
		patternBytes.data(),
		mask.c_str()
	);

	return result;
}

// gmod (get rid of this in prod. branch and replace with better testing)
inline void testPatternScanning() {

	const char* idaPattern = "48 89 5C 24 ? 57 48 83 EC ? 48 8B 01 41 B8";
	const char* bytePattern = "\\x48\\x89\\x5C\\x24\\?\\x57\\x48\\x83\\xEC\\?\\x48\\x8B\\x01\\x41\\xB8";

	auto idaPatternInfo = pattern::detectPatternType(idaPattern);
	auto bytePatternInfo = pattern::detectPatternType(bytePattern);

	if (!idaPatternInfo || !bytePatternInfo) {
		MessageBoxA(NULL, "failed to differentiate pattern types", "Error", MB_ICONERROR);
		return;
	}

	moduleInfo clientDllInfo;
	if (!mem::getModuleInfo(mem::pid, L"client.dll", &clientDllInfo)) {
		MessageBoxA(NULL, "failed to find client.dll", "Error", MB_ICONERROR);
		return;
	}

	std::vector<uint8_t> idaBytes;
	std::string idaMask;
	std::vector<uint8_t> bytePatternBytes;
	std::string bytePatternMask;

	if (!pattern::patternToMask(*idaPatternInfo, idaBytes, idaMask) ||
		!pattern::patternToMask(*bytePatternInfo, bytePatternBytes, bytePatternMask)) {
		MessageBoxA(NULL, "failed to convert patterns to generic format", "Error", MB_ICONERROR);
		return;
	}

	auto idaResult = pattern::findBytePattern(clientDllInfo.base, clientDllInfo.size,
                                           idaBytes.data(), idaMask.c_str());

	auto byteResult = pattern::findBytePattern(clientDllInfo.base, clientDllInfo.size,
                                            bytePatternBytes.data(), bytePatternMask.c_str());

	std::string idaResultStr;
	std::string byteResultStr;

	if (idaResult) {
		idaResultStr = "IDA pattern found at: 0x" + std::to_string(idaResult->matches.front()) +
			"\ntotal matches: " + std::to_string(idaResult->matches.size());
	}
	else {
		idaResultStr = "IDA pattern not found";
	}

	if (byteResult) {
		byteResultStr = "Byte pattern found at: 0x" + std::to_string(byteResult->matches.front()) +
			"\ntotal matches: " + std::to_string(byteResult->matches.size());
	}
	else {
		byteResultStr = "Byte pattern not found";
	}

	std::string comparisonStr;
	if (idaResult && byteResult) {
		if (idaResult->matches.front() == byteResult->matches.front()) {
			comparisonStr = "both patterns found the same address";
		}
		else {
			comparisonStr = "patterns found different addresses";
		}
	}
	else if (idaResult) {
		comparisonStr = "only IDA pattern found a match";
	}
	else if (byteResult) {
		comparisonStr = "only byte pattern found a match";
	}
	else {
		comparisonStr = "no matches found for either pattern";
	}

	std::string finalMessage = idaResultStr + "\n\n" + byteResultStr + "\n\n" + comparisonStr;
	MessageBoxA(NULL, finalMessage.c_str(), "sigscan test results", MB_ICONINFORMATION);
}