#pragma once

#include <list>
#include <string>
#include <vector>
#include "memory.h"

namespace ui {
	extern uintptr_t toAddress(std::string address);
	extern bool isValidHex(std::string& str);
}

namespace addressParser {
	uintptr_t parseExport(const std::string& expression);
	uintptr_t parseInput(const char* str);
}

inline std::list<std::string> g_fileEndings = {
	".dll",
	".exe"
};

inline uintptr_t addressParser::parseExport(const std::string& expression)
{
	size_t delimPos = expression.find('!');

	if (delimPos != std::string::npos) {
		std::string moduleName = expression.substr(0, delimPos);
		std::string exportName = expression.substr(delimPos + 1);

		return mem::getExport(moduleName, exportName);
	}

	return 0;
}

inline uintptr_t addressParser::parseInput(const char* str) {
	std::string expression(str);
	uintptr_t rtn = 0;

	std::vector<std::string> tokens;
	std::vector<char> operators;

	{ // separating scope to avoid confusion with token naming
		size_t lastPos = 0;
		std::string token;

		for (size_t pos = 0; pos < expression.length(); pos++)
		{
			char posToken = expression[pos];
			if (posToken == '+' || posToken == '-')
			{
				token = expression.substr(lastPos, pos - lastPos);
				tokens.push_back(token);
				operators.push_back(posToken);
				lastPos = pos + 1;
			}
		}

		token = expression.substr(lastPos);
		tokens.push_back(token);
	}

	size_t iterator = 0;

	for (std::string& curToken : tokens) {
		size_t startPos = curToken.find_first_not_of(' ');
		if (startPos != std::string::npos) {
			curToken.erase(0, startPos);

			size_t endPos = curToken.find_last_not_of(' ');
			while (endPos + 1 < curToken.length()) {
				curToken.pop_back();
			}
		}
		else {
			curToken = ""; // all spaces maybe?
		}

		uintptr_t value = 0;
		if (curToken.find('!') != std::string::npos) {
			value = parseExport(curToken);
		}
		else {
			bool isModule = false;

			for (const std::string& ending : g_fileEndings) {
				if (curToken.find(ending) != std::string::npos) {
					std::wstring token(curToken.begin(), curToken.end());
					moduleInfo info;
					if (mem::getModuleInfo(mem::g_pid, token.c_str(), &info)) {
						value = info.base;
						isModule = true;
					}
					break; // break out of file endings loop, continue in main
				}
			}

			if (!isModule) {
				if (ui::isValidHex(curToken)) {
					value = ui::toAddress(curToken);
				}
				else {
					value = 0;
				}
			}
		}

		if (iterator == 0) {
			rtn = value;
		}
		else {
			if (operators[iterator - 1] == '+') {
				rtn += value;
			}
			else if (operators[iterator - 1] == '-') {
				rtn -= value;
			}
		}
		iterator++;

	}

	return rtn;
}
