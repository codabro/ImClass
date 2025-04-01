#pragma once

#include <cstdint>
#include <list>
#include <string>


namespace AddressParser
{
	uintptr_t parseLibrary(const char* str);
}

inline std::list<std::string> g_fileEndings = {
	".dll",
	".exe"
};


inline uintptr_t AddressParser::parseLibrary(const char* str)
{
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

	for (std::string& curToken : tokens)
	{
		size_t startPos = curToken.find_first_not_of(' ');
		if (startPos != std::string::npos) {
			curToken.erase(0, startPos);

			size_t endPos = curToken.find_last_not_of(' ');
			while (endPos + 1 < curToken.length())
			{
				curToken.pop_back();
			}
		}
		else {
			curToken = ""; // all spaces maybe?
		}

		uintptr_t value = 0;

		bool isDll = false;

		for (const std::string& ending : g_fileEndings)
		{
			if (curToken.find(ending) != std::string::npos)
			{
				value = 0x10000;
				isDll = true;
				break; // break out of file endings loop, continue in main
			}
		}

		if (!isDll)
		{
			value = std::stoull(curToken, nullptr, 16);
			// assumes all inputs will be hexadecimal, maybe change later
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
