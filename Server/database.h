#pragma once
#include <vector>
#include <string>
#include <filesystem>

namespace server {

	class DBUser {
	public:
		std::string username;
		std::string password;
	};

	class DBDocument {
	public:
		std::string username;
		std::string filename;
		std::string text;
	};


	static constexpr const char* dbRootDefault = "./db";
	class Database {
	public:
		Database(const std::string& dbRoot);
		std::string getUser(DBUser& user) const;
		std::string putUser(const DBUser& user);

		std::string getDoc(DBDocument& doc) const;
		std::string putDoc(const DBDocument& doc);
		std::string deleteDoc(const DBDocument& doc);
	private:
		std::vector<std::string> parseRow(const std::string& line) const;
		std::string dbRoot;
		std::filesystem::path pathToUsers;
		constexpr static const char* usersFilename = "users";
	};
}
