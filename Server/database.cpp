#include "database.h"
#include "logging.h"

#include <fstream>
#include <sstream>

namespace server {

	Database::Database(const std::string& dbRoot):
		dbRoot(dbRoot),
		pathToUsers(dbRoot + "\\" + usersFilename) {
		std::error_code errCode;
		std::filesystem::create_directories(dbRoot, errCode);
		if (errCode.value()) {
			server::logger.logError("Creating users database failed.", errCode);
			return;
		}
		std::ofstream file{ pathToUsers, std::ios::app };
		if (!file) {
			server::logger.logError("Cannot create user database file.");
			return;
		}
	}

	std::string Database::getUser(DBUser& user) const {
		std::ifstream file{ pathToUsers, std::ios::in };
		if (!file) {
			server::logger.logError("Error! Cannot find users database!");
			return "Error! Cannot open users database!";
		}
		
		for (std::string line; std::getline(file, line);) {
			auto row = parseRow(line);
			if (row[0] == user.username) {
				if (row[1][row[1].size() - 1] == '\n') {
					row[1].erase(row[1].size() - 1);
				}
				user.password = row[1];
				return "";
			}
		}
		server::logger.logDebug("User " + user.username + " has not been found");
		return "User " + user.username + " has not been found";
	}

	std::string Database::putUser(const DBUser& user) {
		DBUser query;
		query.username = user.username;
		auto err = getUser(query);
		if (err.empty()) {
			server::logger.logDebug("User " + query.username + " already exists!");
			return "User " + query.username + " already exists!";
		}
		std::ofstream file{ pathToUsers, std::ios::app };
		if (!file) {
			server::logger.logError("Error! Cannot open users database!");
			return "Error! Cannot open users database!";
		}
		std::string row = user.username + "," + user.password + "\n";
		file << row;
		return "";
	}

	std::string Database::getDoc(DBDocument& doc) const {
		std::filesystem::path path = std::string(dbRoot + "\\" + doc.username + "\\" + doc.filename);
		if (!std::filesystem::is_regular_file(path)) {
			return "Cannot find document " + doc.filename;
		}
		std::ifstream file{ path, std::ios::in };
		if (!file) {
			server::logger.logError("Error! Cannot open document database!");
			return "Error! Cannot open document database!";
		}
		std::stringstream ss;
		ss << file.rdbuf();
		doc.text = ss.str();
		return "";
	}

	std::string Database::putDoc(const DBDocument& doc) {
		std::filesystem::path path = std::string(dbRoot + "\\" + doc.username + "\\" + doc.filename);
		if (!std::filesystem::is_directory(path.parent_path())) {
			std::error_code errCode;
			std::filesystem::create_directories(path.parent_path(), errCode);
			if (errCode.value()) {
				server::logger.logError("Creating user's document database failed.", errCode);
				return "Creating user's document database failed.";
			}
		}

		std::ofstream file{ path, std::ios::out };
		if (!file) {
			server::logger.logError("Error! Cannot save document!");
			return "Error! Cannot save document!";
		}
		file << doc.text;
		return "";
	}

	std::string Database::deleteDoc(const DBDocument& doc) {
		std::filesystem::path path = std::string(dbRoot + "\\" + doc.username + "\\" + doc.filename);
		std::error_code errCode;
		if (!std::filesystem::is_directory(path.parent_path(), errCode) || !std::filesystem::is_regular_file(path)) {
			return "Specified doc '" + doc.filename + "' does not exists in the database";
		}
		std::filesystem::remove(path, errCode);
		if (errCode.value()) {
			server::logger.logError("Error during doc deletion!", errCode);
			return "Something went wrong during document deletion";
		}
		return "";
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
