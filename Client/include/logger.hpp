#pragma once
#include <iostream>

struct Logger
{
	Logger() = delete;

	static void Log(const std::string& text)
	{
#ifdef WITH_CONSOLE
		std::cout << text << '\n';
#endif
	}

	static void LogDebug(const std::string& text)
	{
#ifdef WITH_CONSOLE
		std::cout << "[DEBUG]" << text << '\n';
#endif
	}

	static void LogDebug(const std::string& text, const std::string& variableName)
	{
#ifdef WITH_CONSOLE
		std::cout << "[DEBUG] [VARIABLE] " << text << '\n';
#endif
	}

	static void LogError(const std::string& text, const std::string& location)
	{
#ifdef WITH_CONSOLE
		std::cout << "[" << location << "] " << text << '\n';
#endif
	}
};