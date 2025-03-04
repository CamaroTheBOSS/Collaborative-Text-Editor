#pragma once
#include <vector>
#include <string>
#include <filesystem>
#include <concepts>
#include <functional>

#include "parser.h"
#include "server_document.h"

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
		DBDocument() = default;
		DBDocument(const std::string& id, const std::string& filename, const std::vector<std::string>& usernames) :
			id(id),
			filename(filename),
			usernames(usernames) {}
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
		std::optional<DBDocument> extractDocWithUsernameAndFilename(const std::string& username, const std::string& filename);
		std::optional<DBDocument> extractDocWithId(const std::string& id);
		bool addDoc(const DBDocument& doc);
		bool addUserToDoc(const DBUser& user, const DBDocument& doc);
		bool delUserFromDoc(const DBUser& user, const DBDocument& doc);

		bool addDocAndLink(const DBDocument& doc);
		bool linkUserAndDoc(const DBUser& user, const DBDocument& doc);
		bool unlinkUserAndDoc(const DBUser& user, const DBDocument& doc);

		std::optional<ServerSiteDocument> loadDoc(const std::string& username, const std::string& filename);
		bool saveDoc(const std::string& id, const std::string& newText);
		
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
				if (line.empty()) {
					continue;
				}
				bool success = functor(line, obj);
				if (success) {
					return std::make_optional(std::move(obj));
				}
			}
			lastError = "Specified " + DBUser::objName + " does not exists";
			return {};
		}
		template <typename T, typename Functor>
		std::optional<T> extractObjFromDb(Functor&& functor) {
			auto db = getDbForRead(T::dbName);
			if (!db) {
				return {};
			}
			std::stringstream ss;
			ss << db.rdbuf();
			auto lines = Parser::parseTextToVector(ss.str());
			std::optional<T> opt;
			T obj{};
			for (int i = 0; i < lines.size(); i++) {
				if (lines[i].empty()) {
					continue;
				}
				bool success = functor(lines[i], obj);
				if (success) {
					opt = std::make_optional(std::move(obj));
					lines.erase(lines.cbegin() + i);
				}
			}
			if (!opt) {
				setError("Specified " + T::objName + " does not exists");
				return {};
			}
			auto dbForReplace = getDbForReplace(T::dbName);
			if (!dbForReplace) {
				return {};
			}
			std::string newDbContent = Parser::parseVectorToText(lines);
			dbForReplace << newDbContent;
			return opt;
		}

		template<typename T>
		bool addObjToDbEvenIfExists(const T& obj) {
			auto db = getDbForAdd(T::dbName);
			if (!db) {
				return false;
			}
			db << Parser::parseVectorToText(obj.serialize(), ',') + "\n";
			return true;
		}
		void setError(const std::string& error);
		std::ifstream getDbForRead(const std::string& dbName);
		std::ofstream getDbForAdd(const std::string& dbName);
		std::ofstream getDbForReplace(const std::string& dbName);
		std::vector<std::string> parseRow(const std::string& line) const;

		std::string lastError;
		std::string dbRoot;
	};
}
