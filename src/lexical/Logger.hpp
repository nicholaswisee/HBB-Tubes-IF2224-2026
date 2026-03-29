#pragma once

#include <iostream>
#include <string>

class Logger {
	public:
		static void error(int line, const std::string &message) {
				std::cerr << "[line " << line << "] Error: " << message << '\n';
		}
};
