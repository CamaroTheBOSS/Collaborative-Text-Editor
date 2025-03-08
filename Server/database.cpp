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
		return extractObjFromDb<DBUser>([&](const std::string& line, DBUser& user) {
			if (line.substr(0, line.find(',')) == username) {
				auto parsed = parseRow(line);
				user.parseRow(parsed);
				return true;
			}
			return false;
		});
	}

	bool Database::addUser(const DBUser& user) {
		if (getUserWithUsername(user.username).has_value()) {
			setError("User " + user.username + " already exists!");
			return false;
		}
		return addObjToDbEvenIfExists(user);
	}

	bool Database::addDocToUser(const DBDocument& doc, const DBUser& user) {
		auto userFromDbOpt = extractUserWithUsername(user.username);
		if (!userFromDbOpt) {
			return false;
		}
		auto it = std::find(userFromDbOpt.value().documentIds.cbegin(), userFromDbOpt.value().documentIds.cend(), doc.id);
		if (it != userFromDbOpt.value().documentIds.cend()) {
			logger.logDebug("Specified document", doc.id, "is present in user's (", user.username, ") database");
			return addObjToDbEvenIfExists(userFromDbOpt.value());
		}
		userFromDbOpt.value().documentIds.emplace_back(doc.id);
		return addObjToDbEvenIfExists(userFromDbOpt.value());
	}

	bool Database::delDocFromUser(const DBDocument& doc, const DBUser& user) {
		auto userFromDbOpt = extractUserWithUsername(user.username);
		if (!userFromDbOpt) {
			return false;
		}
		auto it = std::find(userFromDbOpt.value().documentIds.cbegin(), userFromDbOpt.value().documentIds.cend(), doc.id);
		if (it == userFromDbOpt.value().documentIds.cend()) {
			logger.logDebug("Specified document", doc.id, "does not exists in user's (", user.username, ") database");
			return addObjToDbEvenIfExists(userFromDbOpt.value());
		}
		userFromDbOpt.value().documentIds.erase(it);
		return addObjToDbEvenIfExists(userFromDbOpt.value());
	}

	std::vector<std::string> Database::getUserDocumentNames(const std::string& username) {
		auto userDb = getUserWithUsername(username);
		if (!userDb) {
			return {};
		}
		auto db = getDbForRead(DBDocument::dbName);
		if (!db) {
			return {};
		}
		std::stringstream ss;
		ss << db.rdbuf();
		std::vector<std::string> names;
		auto lines = Parser::parseTextToVector(ss.str());
		for (const auto& line : lines) {
			int pos = line.find(',');
			if (pos == std::string::npos) {
				continue;
			}
			auto id = std::string_view(line.cbegin(), line.cbegin() + pos);
			auto it = std::find(userDb.value().documentIds.cbegin(), userDb.value().documentIds.cend(), id);
			if (it != userDb.value().documentIds.cend()) {
				auto row = parseRow(line);
				names.emplace_back(std::move(row[1]));
			}
		}
		return names;
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

	bool Database::addDoc(const DBDocument& doc) {
		if (doc.usernames.size() != 1) {
			setError("Wrong parent assigned to the document!");
			return false;
		}
		if (doc.filename.empty()) {
			setError("Wrong name assigned to the document!");
			return false;
		}
		if (getDocWithUsernameAndFilename(doc.usernames[0], doc.filename).has_value()) {
			setError("Document with " + doc.filename + " is already assigned to the user " + doc.usernames[0]);
			return false;
		}
		return addObjToDbEvenIfExists(doc);
	}

	bool Database::addDocAndLink(const DBDocument& doc) {
		if (!addDoc(doc)) {
			return false;
		}
		DBUser user(doc.usernames[0], "", {});
		return linkUserAndDoc(user, doc);
	}

	bool Database::addUserToDoc(const DBUser& user, const DBDocument& doc) {
		auto docFromDbOpt = extractDocWithId(doc.id);
		if (!docFromDbOpt) {
			return false;
		}
		auto it = std::find(docFromDbOpt.value().usernames.cbegin(), docFromDbOpt.value().usernames.cend(), user.username);
		if (it != docFromDbOpt.value().usernames.cend()) {
			logger.logDebug("Specified user", user.username, "is present in doc's (", doc.id, ") database");
			return addObjToDbEvenIfExists(docFromDbOpt.value());
		}
		docFromDbOpt.value().usernames.emplace_back(user.username);
		return addObjToDbEvenIfExists(docFromDbOpt.value());
	}

	bool Database::delUserFromDoc(const DBUser& user, const DBDocument& doc) {
		auto docFromDbOpt = extractDocWithId(doc.id);
		if (!docFromDbOpt) {
			return false;
		}
		auto it = std::find(docFromDbOpt.value().usernames.cbegin(), docFromDbOpt.value().usernames.cend(), user.username);
		if (it == docFromDbOpt.value().usernames.cend()) {
			logger.logDebug("Specified user", user.username, "does not exists in doc's (", doc.id, ") database");
			return addObjToDbEvenIfExists(docFromDbOpt.value());
		}
		docFromDbOpt.value().usernames.erase(it);
		if (docFromDbOpt.value().usernames.size() == 0) {
			std::filesystem::remove(std::filesystem::path(dbRoot) / docFromDbOpt.value().id);
			return true;
		}
		return addObjToDbEvenIfExists(docFromDbOpt.value());
	}

	bool Database::linkUserAndDoc(const DBUser& user, const DBDocument& doc) {
		bool userLinked = addUserToDoc(user, doc);
		if (!userLinked) {
			return false;
		}
		bool docLinked = addDocToUser(doc, user);
		if (!docLinked) {
			logger.logError("Error when linking doc with user! Performing rollback...");
			assert(delUserFromDoc(user, doc));
			return false;
		}
		return true;
	}

	bool Database::unlinkUserAndDoc(const DBUser& user, const DBDocument& doc) {
		bool userUnlinked = delUserFromDoc(user, doc);
		if (!userUnlinked) {
			return false;
		}
		bool docUnlinked = delDocFromUser(doc, user);
		if (!docUnlinked) {
			logger.logError("Error when unlinking doc with user! Performing rollback...");
			assert(addUserToDoc(user, doc));
			return false;
		}
		return true;
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

	std::optional<DBDocument> Database::extractDocWithUsernameAndFilename(const std::string& username, const std::string& filename) {
		return extractObjFromDb<DBDocument>([&](const std::string& line, DBDocument& doc) {
			auto parsed = parseRow(line);
			doc.parseRow(parsed);
			if (doc.filename == filename && std::find(doc.usernames.cbegin(), doc.usernames.cend(), username) != doc.usernames.cend()) {
				return true;
			}
			return false;
		});
	}

	std::optional<DBDocument> Database::extractDocWithId(const std::string& id) {
		return extractObjFromDb<DBDocument>([&](const std::string& line, DBDocument& doc) {
			auto parsed = parseRow(line);
			doc.parseRow(parsed);
			if (doc.id == id) {
				return true;
			}
			return false;
			});
	}

	std::optional<ServerSiteDocument> Database::loadDoc(const std::string& username, const std::string& filename) {
		auto docFromDbOpt = getDocWithUsernameAndFilename(username, filename);
		if (!docFromDbOpt) {
			setError("Document " + filename + " does not exists in " + username + "'s database");
			return {};
		}
		std::ifstream file{ dbRoot + "\\" + docFromDbOpt.value().id, std::ios::in };
		if (!file) {
			setError("Error! Cannot document " + docFromDbOpt.value().id);
			return {};
		}
		std::stringstream ss;
		ss << file.rdbuf();
		ServerSiteDocument doc{ ss.str(), 0, 0, docFromDbOpt.value().id, docFromDbOpt.value().filename };
		return std::make_optional(std::move(doc));
	}

	bool Database::saveDoc(const std::string& id, const std::string& newText) {
		std::ofstream file{ dbRoot + "\\" + id, std::ios::out };
		if (!file) {
			setError("Error! Cannot document " + id);
			return false;
		}
		file << newText;
		return true;
	}

	void Database::setError(const std::string& error) {
		lastError = error;
		logger.logError(error);
	}

	std::string Database::getLastError() {
		std::string error = lastError;
		lastError.clear();
		return error;
	}

	std::ifstream Database::getDbForRead(const std::string& dbName) {
		std::ifstream file{ dbRoot + "\\" + dbName, std::ios::in };
		if (!file) {
			setError("Error! Cannot find " + DBUser::dbName + " database!");
		}
		return file;
	}

	std::ofstream Database::getDbForAdd(const std::string& dbName) {
		std::ofstream file{ dbRoot + "\\" + dbName, std::ios::app };
		if (!file) {
			setError("Error! Cannot find " + DBUser::dbName + " database!");
		}
		return file;
	}

	std::ofstream Database::getDbForReplace(const std::string& dbName) {
		std::ofstream file{ dbRoot + "\\" + dbName, std::ios::out };
		if (!file) {
			setError("Error! Cannot find " + DBUser::dbName + " database!");
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
