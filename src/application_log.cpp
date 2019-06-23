#include "application_log.hpp"

#include "imgui.h"

void logDebug(const std::string& msg) {
	#ifdef ENABLE_LOG
	logEntries.push_back({ logType::DEBUG, msg });
	#endif
}

void logWarning(const std::string& msg) {
	#ifdef ENABLE_LOG
	logEntries.push_back({ logType::WARNING, "Warning : " + msg });
	#endif
}

void logError(const std::string& msg) {
	#ifdef ENABLE_LOG
	logEntries.push_back({ logType::ERROR, "ERROR : " + msg });
	#endif
}

void logFatal(const std::string& msg) {
	#ifdef ENABLE_LOG
	logEntries.push_back({ logType::FATAL, "FATAL : " + msg });
	#endif
}

void showDebugConsole() {
	ImGui::SetNextWindowSize(ImVec2(500, 500), ImGuiCond_FirstUseEver);
	if(!ImGui::Begin("Application log")) {
		ImGui::End();
		return;
	}

	for (logEntry entry : logEntries) {
		ImVec4 color;
		switch (entry.type) {
		case logType::ERROR:
			color = ImColor(255, 0, 0, 255);
			break;
		case logType::FATAL:
			color = ImColor(180, 0, 0, 255);
			break;
		case logType::DEBUG:
		default:
			color = ImColor(IM_COL32_WHITE);
		}
		ImGui::PushStyleColor(ImGuiCol_Text, color);
		ImGui::TextUnformatted(entry.message.c_str());
		ImGui::PopStyleColor();
	}

	ImGui::End();
}