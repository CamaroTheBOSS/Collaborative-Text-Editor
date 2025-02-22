#pragma once

namespace windows::text_editor {
	static constexpr const char* name = "TextEditorWindow";
	namespace events {
		static constexpr const char* find = "find";
		static constexpr const char* findReset = "find reset";
		static constexpr const char* findNext = "find next";
		static constexpr const char* replace = "replace";
	}
}

namespace windows::app {
	static constexpr const char* name = "App";
	namespace events {
		static constexpr const char* createDocWindow = "create doc window";
		static constexpr const char* createDoc = "create doc";

		static constexpr const char* loadDocWindow = "load doc window";
		static constexpr const char* loadDoc = "load doc";
		static constexpr const char* help = "help";
		static constexpr const char* exit = "exit";
	}
}
