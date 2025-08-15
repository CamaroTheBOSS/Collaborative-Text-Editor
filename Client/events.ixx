module;

export module events;

export namespace windows::text_editor {
	export constexpr const char* name = "TextEditorWindow";
	namespace events {
		constexpr const char* find = "find";
		constexpr const char* findReset = "find reset";
		constexpr const char* findNext = "find next";
		constexpr const char* replace = "replace";
	}
}

export namespace windows::app {
	constexpr const char* name = "App";
	namespace events {
		constexpr const char* showAcCode = "show accode";
		constexpr const char* disconnect = "disconnect";
		constexpr const char* createDocWindow = "create doc window";
		constexpr const char* createDoc = "create doc";

		constexpr const char* loadDocWindow = "load doc window";
		constexpr const char* loadItemClicked = "load item clicked";
		constexpr const char* loadItemAccepted = "load item accepted";
		constexpr const char* loadItemDeleted = "load item deleted";

		constexpr const char* joinDocWindow = "join doc window";
		constexpr const char* joinDoc = "join doc";
		constexpr const char* help = "help";
		constexpr const char* exit = "exit";

		// Login and register main menu options events
		constexpr const char* showLoginWindow = "show login";
		constexpr const char* acceptLoginPassword = "accept login/password";
		constexpr const char* logout = "logout";
	}
}

export namespace windows::login {
	constexpr const char* name = "Login";
}

export namespace windows::registration {
	constexpr const char* name = "Register";
}

export namespace windows::password {
	constexpr const char* name = "Password";
}

export namespace windows::mainmenu {
	constexpr const char* name = "Main Menu";
}
