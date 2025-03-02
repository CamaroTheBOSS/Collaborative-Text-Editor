#pragma once
#include <vector>
#include <string>
#include <filesystem>
#include <concepts>
#include <functional>

namespace server {

	class DBUser {
	public:
		DBUser() = default;
		DBUser(const std::string& username, const std::string& password, const std::vector<std::string>& documentIds):
			username(username),
			password(password),
			documentIds(documentIds) {}
		void parseRow(std::vector<std::string>& row);
		std::vector<std::string> serialize() const;

		std::string username;
		std::string password;
		std::vector<std::string> documentIds;
		static const std::string dbName;
		static const std::string objName;
	};

	class DBDocument {
	public:
		void parseRow(std::vector<std::string>& row);
		std::vector<std::string> serialize() const;

		std::string id;
		std::string filename;
		std::vector<std::string> usernames;
		static const std::string dbName;
		static const std::string objName;
	};

	class Database {
	public:
		Database(const std::string& dbRoot = "./db");
		std::optional<DBUser> getUserWithUsername(const std::string& username);
		std::optional<DBUser> extractUserWithUsername(const std::string& username);
		bool addUser(const DBUser& user);
		bool addDocToUser(const DBDocument& doc, const DBUser& user);
		bool delDocFromUser(const DBDocument& doc, const DBUser& user);

		std::optional<DBDocument> getDocWithId(const std::string& id);
		std::optional<DBDocument> getDocWithUsernameAndFilename(const std::string& username, const std::string& filename);
	
		std::string getLastError();
	private:
		template <typename T, typename Functor>
		std::optional<T> getObjFromDb(Functor&& functor) {
			auto db = getDbForRead(T::dbName);
			if (!db) {
				return {};
			}

			T obj{};
			for (std::string line; std::getline(db, line);) {
				bool success = functor(line, obj);
				if (success) {
					return std::make_optional(std::move(obj));
				}
			}
			lastError = "Specified " + DBUser::objName + " does not exists";
			return {};
		}
		bool _addUserWithoutCheckingIfExists(const DBUser& user);
		std::ifstream getDbForRead(const std::string& dbName);
		std::ofstream getDbForAdd(const std::string& dbName);
		std::ofstream getDbForReplace(const std::string& dbName);
		std::vector<std::string> parseRow(const std::string& line) const;

		std::string lastError;
		std::string dbRoot;
	};
}
