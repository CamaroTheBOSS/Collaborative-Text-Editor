#include "database.h"
#include "logging.h"
#include "parser.h"

#include <fstream>
#include <sstream>

namespace server {
	const std::string DBUser::dbName{ "usersDb" };
	const std::string DBUser::objName{ "user" };
	const std::string DBDocument::dbName{ "docDb" };
	const std::string DBDocument::objName{ "document" };

	void DBUser::parseRow(std::vector<std::string>& row) {
		username = std::move(row[0]);
		password = std::move(row[1]);
		documentIds = Parser::parseLineToVector(row[2], ';');
	}

	std::vector<std::string> DBUser::serialize() const {
		return { username, password, Parser::parseVectorToText(documentIds, ';') };
	}

	void DBDocument::parseRow(std::vector<std::string>& row) {
		id = std::move(row[0]);
		filename = std::move(row[1]);
		usernames = Parser::parseLineToVector(row[2], ';');
	}

	std::vector<std::string> DBDocument::serialize() const {
		return { id, filename, Parser::parseVectorToText(usernames, ';')};
	}


	Database::Database(const std::string& dbRoot):
		dbRoot(dbRoot) {
		std::error_code errCode;
		std::filesystem::create_directories(dbRoot, errCode);
		if (errCode.value()) {
			logger.logError("Creating users database failed.", errCode);
			return;
		}

		for (const auto& name : { DBUser::dbName, DBDocument::dbName }) {
			std::ofstream db{ dbRoot + "\\" + std::string{ name }, std::ios::app };
			if (!db) {
				logger.logError("Cannot create user database file.");
			}
		}
	}

	std::optional<DBUser> Database::getUserWithUsername(const std::string& username) {
		return getObjFromDb<DBUser>([&](const std::string& line, DBUser& user) {
				auto row = parseRow(line);
				if (row[0] == username) {
					user.parseRow(row);
					return true;
				}
				return false;
		});
	}

	std::optional<DBUser> Database::extractUserWithUsername(const std::string& username) {
		auto db = getDbForRead(DBUser::dbName);
		if (!db) {
			return {};
		}
		std::stringstream ss;
		ss << db.rdbuf();
		auto lines = Parser::parseTextToVector(ss.str());
		std::optional<DBUser> opt;
		for (int i = 0; i < lines.size(); i++) {
			if (lines[i].substr(0, lines[i].find(',')) == username) {
				auto parsed = parseRow(lines[i]);
				DBUser user;
				user.parseRow(parsed);
				opt = std::move(user);
				lines.erase(lines.cbegin() + i);
				break;
			}
		}
		if (!opt) {
			lastError = "Specified " + DBUser::objName + " does not exists";
			return {};
		}
		auto dbForReplace = getDbForReplace(DBUser::dbName);
		if (!dbForReplace) {
			return {};
		}
		std::string newDbContent = Parser::parseVectorToText(lines);
		dbForReplace << newDbContent;
		return opt;
	}

	bool Database::addUser(const DBUser& user) {
		if (getUserWithUsername(user.username).has_value()) {
			lastError = "User " + user.username + " already exists!";
			return false;
		}
		return _addUserWithoutCheckingIfExists(user);
	}

	bool Database::_addUserWithoutCheckingIfExists(const DBUser& user) {
		auto db = getDbForAdd(DBUser::dbName);
		if (!db) {
			return false;
		}
		db << Parser::parseVectorToText(user.serialize(), ',');
		return true;
	}

	bool Database::addDocToUser(const DBDocument& doc, const DBUser& user) {
		auto userFromDbOpt = extractUserWithUsername(user.username);
		if (!userFromDbOpt) {
			return false;
		}
		auto it = std::find(userFromDbOpt.value().documentIds.cbegin(), userFromDbOpt.value().documentIds.cend(), doc.id);
		if (it != userFromDbOpt.value().documentIds.cend()) {
			logger.logDebug("Specified document", doc.id, "is present in user's (", user.username, ") database");
			return true;
		}
		userFromDbOpt.value().documentIds.emplace_back(doc.id);
		return _addUserWithoutCheckingIfExists(userFromDbOpt.value());
	}

	bool Database::delDocFromUser(const DBDocument& doc, const DBUser& user) {
		auto userFromDbOpt = extractUserWithUsername(user.username);
		if (!userFromDbOpt) {
			return false;
		}
		auto it = std::find(userFromDbOpt.value().documentIds.cbegin(), userFromDbOpt.value().documentIds.cend(), doc.id);
		if (it == userFromDbOpt.value().documentIds.cend()) {
			logger.logDebug("Specified document", doc.id, "does not exists in user's (", user.username, ") database");
			return true;
		}
		userFromDbOpt.value().documentIds.erase(it);
		return _addUserWithoutCheckingIfExists(userFromDbOpt.value());
	}

	std::optional<DBDocument> Database::getDocWithId(const std::string& id) {
		return getObjFromDb<DBDocument>([&](const std::string& line, DBDocument& doc) {
			auto row = parseRow(line);
			doc.parseRow(row);
			if (doc.filename == id) {
				return true;
			}
			return false;
		});
	}

	std::optional<DBDocument> Database::getDocWithUsernameAndFilename(const std::string& username, const std::string& filename) {
		return getObjFromDb<DBDocument>([&](const std::string& line, DBDocument& doc) {
			auto row = parseRow(line);
			doc.parseRow(row);
			if (doc.filename == filename && std::find(doc.usernames.cbegin(), doc.usernames.cend(), username) != doc.usernames.cend()) {
				return true;
			}
			return false;
		});
	}

	std::string Database::getLastError() {
		std::string error = lastError;
		lastError.clear();
		return error;
	}

	std::ifstream Database::getDbForRead(const std::string& dbName) {
		std::ifstream file{ dbRoot + "\\" + dbName, std::ios::in };
		if (!file) {
			lastError = "Error! Cannot find " + DBUser::dbName + " database!";
			logger.logError(lastError);
		}
		return file;
	}

	std::ofstream Database::getDbForAdd(const std::string& dbName) {
		std::ofstream file{ dbRoot + "\\" + dbName, std::ios::app };
		if (!file) {
			lastError = "Error! Cannot find " + DBUser::dbName + " database!";
			logger.logError(lastError);
		}
		return file;
	}

	std::ofstream Database::getDbForReplace(const std::string& dbName) {
		std::ofstream file{ dbRoot + "\\" + dbName, std::ios::out };
		if (!file) {
			lastError = "Error! Cannot find " + DBUser::dbName + " database!";
			logger.logError(lastError);
		}
		return file;
	}

	std::vector<std::string> Database::parseRow(const std::string& line) const {
		int prev = 0, curr = 0;
		std::vector<std::string> row;
		while (curr = line.find(',', prev), curr != std::string::npos) {
			row.emplace_back(line.substr(prev, curr - prev));
			prev = curr + 1;
		}
		row.emplace_back(line.substr(prev));
		return row;
	}
}
