#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#define ENABLE_LOG

enum class logType {
	DEBUG, WARNING, ERROR, FATAL
};

struct logEntry {
	logType type;
	std::string message;
};

static std::vector<logEntry> logEntries;

void logDebug(const std::string& msg);
void logWarning(const std::string& msg);
void logError(const std::string& msg);
void logFatal(const std::string& msg);

void showDebugConsole();