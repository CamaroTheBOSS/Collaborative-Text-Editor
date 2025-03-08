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
		static constexpr const char* showAcCode = "show accode";
		static constexpr const char* disconnect = "disconnect";
		static constexpr const char* createDocWindow = "create doc window";
		static constexpr const char* createDoc = "create doc";

		static constexpr const char* loadDocWindow = "load doc window";
		static constexpr const char* loadItemClicked = "load item clicked";
		static constexpr const char* loadItemAccepted = "load item accepted";
		static constexpr const char* loadItemDeleted = "load item deleted";

		static constexpr const char* joinDocWindow = "join doc window";
		static constexpr const char* joinDoc = "join doc";
		static constexpr const char* help = "help";
		static constexpr const char* exit = "exit";

		// Login and register main menu options events
		static constexpr const char* showLoginWindow = "show login";
		static constexpr const char* acceptLoginPassword = "accept login/password";
		static constexpr const char* logout = "logout";
	}
}

namespace windows::login {
	static constexpr const char* name = "Login";
}

namespace windows::registration {
	static constexpr const char* name = "Register";
}

namespace windows::password {
	static constexpr const char* name = "Password";
}

namespace windows::mainmenu {
	static constexpr const char* name = "Main Menu";
}
