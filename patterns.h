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
	std::optional<PatternScanResult> scanPattern(PatternInfo& patternInfo, const std::string& dllName);
	std::optional<PatternScanResult> findBytePattern(uintptr_t baseAddress, size_t size, const uint8_t* signature, const char* mask);
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
	const std::string& signature = patternInfo.pattern;

	outBytes.clear();
	outMask.clear();

	if (patternInfo.type == PatternType::IDA_SIGNATURE) {

		std::string byteStr;
		size_t length = signature.length();
		for (size_t i = 0; i <= length; i++) {
			char c = signature[i];

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
		for (size_t i = 0; i < signature.length();) { // don't iterate here, it can be incremented by varying numbers

			auto currentCharacter = signature[i];

			if (currentCharacter == '\\' && i + 3 < signature.length() && signature[i + 1] == 'x')
			{
				std::string byteStr = signature.substr(i + 2, 2);
				outBytes.push_back((uint8_t)(std::stoi(byteStr, nullptr, 16)));
				outMask += 'x';
				i += 4;
			}
			else if (currentCharacter == '?') {
				outBytes.push_back(0);  // just a placeholder, never used
				outMask += '?';

				while (i < signature.length() && signature[i] == '?') {
					i++;
				}
			}
			else {
				i++;
			}
		}
		return true;
	}
	return false;
}

inline std::optional<PatternScanResult> pattern::findBytePattern(uintptr_t baseAddress, size_t size, const uint8_t* signature, const char* mask) {
	PatternScanResult result;

	size_t patternLength = strlen(mask);

	if (patternLength == 0)
		return std::nullopt;

	if (size < patternLength)
		return std::nullopt;

	std::vector<uint8_t> buffer(size);

	SIZE_T bytesRead;
	if (!mem::read(baseAddress, buffer.data(), size)) {
		return std::nullopt;
	}

	for (size_t i = 0; i <= size - patternLength; i++) {
		bool found = true;

		for (size_t j = 0; j < patternLength; j++) {

			if (mask[j] == '?')
				continue;

			if (buffer[i + j] != signature[j]) {
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


inline std::optional<PatternScanResult> pattern::scanPattern(PatternInfo& patternInfo, const std::string& dllName)
{
	auto patternType = detectPatternType(patternInfo.pattern);

	if (patternType != std::nullopt) {
		patternInfo.type = patternType->type;
	}

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