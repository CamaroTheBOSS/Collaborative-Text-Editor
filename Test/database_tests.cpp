#include "pch.h"
#include "database.h"
#include <array>
#include <fstream>

using namespace server;

static constexpr const char* testDbRoot = "db_test";
static constexpr const char* dbInitialUsersStr = "user1,password,\n"
												 "user2,password2,id1\n"
												 "user3,password,id1;id2;id3\n";										
static std::array<DBUser, 3> dbInitialUsers = { 
	DBUser("user1", "password", {}),
	DBUser("user2", "password2", { "id1" }),
	DBUser("user3", "password", { "id1", "id2", "id3" }),
};

static constexpr const char* dbInitialDocsStr = "id1,filename2,user2;user3\n"
												"id2,filename,user3\n"
												"id3,filename,user3\n";
static std::array<DBDocument, 3> dbInitialDocs = {
	DBDocument("id1", "filename2", {"user2", "user3"}),
	DBDocument("id2", "filename", { "user3" }),
	DBDocument("id3", "filename", { "user3" })
};

template<typename T>
std::string getTestDbPath() {
	return testDbRoot + std::string{"\\"} + std::string{ T::dbName };
}

template <typename T>
std::string getDbContentAsString() {
	std::stringstream ss;
	std::ifstream userDb{ getTestDbPath<T>(), std::ios::in };
	ss << userDb.rdbuf();
	return ss.str();
}

Database prepareUserDb() {
	Database db{ testDbRoot };
	std::ofstream userDb{ getTestDbPath<DBUser>(), std::ios::out };
	userDb << dbInitialUsersStr;
	return db;
}

Database prepareDocDb() {
	Database db{ testDbRoot };
	std::ofstream userDb{ getTestDbPath<DBDocument>(), std::ios::out };
	userDb << dbInitialDocsStr;
	return db;
}

TEST(DatabaseTests, UserParse) {
	std::vector<std::string> row = { "username", "password", "id1;id2;id3" };
	std::vector<std::string> expectedIds{"id1", "id2", "id3"};
	server::DBUser user;
	user.parseRow(row);
	EXPECT_EQ(user.username, "username");
	EXPECT_EQ(user.password, "password");
	EXPECT_EQ(user.documentIds, expectedIds);
}

TEST(DatabaseTests, UserParseOneDoc) {
	std::vector<std::string> row = { "username", "password", "id1" };
	std::vector<std::string> expectedIds{"id1"};
	server::DBUser user;
	user.parseRow(row);
	EXPECT_EQ(user.username, "username");
	EXPECT_EQ(user.password, "password");
	EXPECT_EQ(user.documentIds, expectedIds);
}

TEST(DatabaseTests, UserParseEmptyDocs) {
	std::vector<std::string> row = { "username", "password", "" };
	std::vector<std::string> expectedIds{};
	server::DBUser user;
	user.parseRow(row);
	EXPECT_EQ(user.username, "username");
	EXPECT_EQ(user.password, "password");
	EXPECT_EQ(user.documentIds, expectedIds);
}

TEST(DatabaseTests, UserSerialization) {
	std::vector<std::string> expected = { "username", "password", "id1;id2;id3" };
	server::DBUser user;
	user.username = "username";
	user.password = "password";
	user.documentIds = {"id1", "id2", "id3"};
	auto row = user.serialize();
	EXPECT_EQ(row, expected);
}

TEST(DatabaseTests, UserSerializationOneDoc) {
	std::vector<std::string> expected = { "username", "password", "id1" };
	server::DBUser user;
	user.username = "username";
	user.password = "password";
	user.documentIds = { "id1" };
	auto row = user.serialize();
	EXPECT_EQ(row, expected);
}

TEST(DatabaseTests, UserSerializationEmptyDocs) {
	std::vector<std::string> expected = { "username", "password", "" };
	server::DBUser user;
	user.username = "username";
	user.password = "password";
	user.documentIds = {};
	auto row = user.serialize();
	EXPECT_EQ(row, expected);
}

TEST(DatabaseTests, DocumentParse) {
	std::vector<std::string> row = { "id1", "filename.txt", "user1;user2;user3" };
	std::vector<std::string> expectedUsernames{"user1", "user2", "user3"};
	server::DBDocument doc;
	doc.parseRow(row);
	EXPECT_EQ(doc.id, "id1");
	EXPECT_EQ(doc.filename, "filename.txt");
	EXPECT_EQ(doc.usernames, expectedUsernames);
}

TEST(DatabaseTests, DocumentOneUser) {
	std::vector<std::string> row = { "id1", "filename.txt", "user1" };
	std::vector<std::string> expectedUsernames{"user1"};
	server::DBDocument doc;
	doc.parseRow(row);
	EXPECT_EQ(doc.id, "id1");
	EXPECT_EQ(doc.filename, "filename.txt");
	EXPECT_EQ(doc.usernames, expectedUsernames);
}

TEST(DatabaseTests, DocumentEmptyUsers) {
	std::vector<std::string> row = { "id1", "filename.txt", "" };
	std::vector<std::string> expectedUsernames{};
	server::DBDocument doc;
	doc.parseRow(row);
	EXPECT_EQ(doc.id, "id1");
	EXPECT_EQ(doc.filename, "filename.txt");
	EXPECT_EQ(doc.usernames, expectedUsernames);
}

TEST(DatabaseTests, DocumentSerialization) {
	std::vector<std::string> expected = { "id1", "filename.txt", "user1;user2;user3" };
	server::DBDocument doc;
	doc.id = "id1";
	doc.filename = "filename.txt";
	doc.usernames = { "user1", "user2", "user3" };
	auto row = doc.serialize();
	EXPECT_EQ(row, expected);
}

TEST(DatabaseTests, DocumentSerializationOneUser) {
	std::vector<std::string> expected = { "id1", "filename.txt", "user1" };
	server::DBDocument doc;
	doc.id = "id1";
	doc.filename = "filename.txt";
	doc.usernames = { "user1" };
	auto row = doc.serialize();
	EXPECT_EQ(row, expected);
}

TEST(DatabaseTests, DocumentSerializationEmptyUsers) {
	std::vector<std::string> expected = { "id1", "filename.txt", "" };
	server::DBDocument doc;
	doc.id = "id1";
	doc.filename = "filename.txt";
	doc.usernames = { };
	auto row = doc.serialize();
	EXPECT_EQ(row, expected);
}

TEST(DatabaseTests, ReadUserDbTest) {
	auto db = prepareUserDb();
	for (const auto& expectedUser : dbInitialUsers) {
		auto userFromDb = db.getUserWithUsername(expectedUser.username);
		EXPECT_TRUE(userFromDb.has_value());
		if (!userFromDb.has_value()) {
			continue;
		}
		EXPECT_EQ(userFromDb.value().username, expectedUser.username);
		EXPECT_EQ(userFromDb.value().password, expectedUser.password);
		EXPECT_EQ(userFromDb.value().documentIds, expectedUser.documentIds);
	}
	prepareUserDb();
}

TEST(DatabaseTests, AddUserDbTest) {
	auto db = prepareUserDb();
	DBUser dbUser{ "newuser", "newpassword", {} };
	EXPECT_TRUE(db.addUser(dbUser));
	auto content = getDbContentAsString<DBUser>();
	EXPECT_EQ(content, dbInitialUsersStr + std::string{"newuser,newpassword,\n"});
	prepareUserDb();
}

TEST(DatabaseTests, AddExistingUserDbTest) {
	auto db = prepareUserDb();
	DBUser dbUser{ "user1", "newpassword", {} };
	EXPECT_FALSE(db.addUser(dbUser));
	auto content = getDbContentAsString<DBUser>();
	EXPECT_EQ(content, dbInitialUsersStr);
	prepareUserDb();
}

TEST(DatabaseTests, ExtractUserDbTest) {
	auto db = prepareUserDb();
	auto userOpt = db.extractUserWithUsername("user2");
	EXPECT_TRUE(userOpt.has_value());
	if (!userOpt.has_value()) {
		return;
	}
	EXPECT_EQ(userOpt.value().username, dbInitialUsers[1].username);
	EXPECT_EQ(userOpt.value().password, dbInitialUsers[1].password);
	EXPECT_EQ(userOpt.value().documentIds, dbInitialUsers[1].documentIds);
	auto content = getDbContentAsString<DBUser>();
	EXPECT_EQ(content, 
		"user1,password,\n"
		"user3,password,id1;id2;id3\n");
	prepareUserDb();
}

TEST(DatabaseTests, AddDocToUserDbTest) {
	auto db = prepareUserDb();
	DBDocument doc;
	doc.id = "someid";
	EXPECT_TRUE(db.addDocToUser(doc, dbInitialUsers[1]));
	auto content = getDbContentAsString<DBUser>();
	EXPECT_EQ(content,
		"user1,password,\n"
		"user3,password,id1;id2;id3\n"
		"user2,password2,id1;someid\n");
}

TEST(DatabaseTests, DeleteDocFromUserDbTest) {
	auto db = prepareUserDb();
	DBDocument doc;
	doc.id = "id2";
	EXPECT_TRUE(db.delDocFromUser(doc, dbInitialUsers[2]));
	auto content = getDbContentAsString<DBUser>();
	EXPECT_EQ(content,
		"user1,password,\n"
		"user2,password2,id1\n"
		"user3,password,id1;id3\n"
		);
	prepareUserDb();
}

TEST(DatabaseTests, GetDocByIdTest) {
	auto db = prepareDocDb();
	auto docOpt = db.getDocWithUsernameAndFilename(dbInitialDocs[0].usernames[0], dbInitialDocs[0].filename);
	EXPECT_TRUE(docOpt.has_value());
	if (docOpt.has_value()) {
		EXPECT_EQ(docOpt.value().id, dbInitialDocs[0].id);
		EXPECT_EQ(docOpt.value().filename, dbInitialDocs[0].filename);
		EXPECT_EQ(docOpt.value().usernames, dbInitialDocs[0].usernames);
	}
	prepareDocDb();
}

TEST(DatabaseTests, ExtractDocTest) {
	auto db = prepareDocDb();
	auto docOpt = db.extractDocWithUsernameAndFilename(dbInitialDocs[0].usernames[0], dbInitialDocs[0].filename);
	EXPECT_TRUE(docOpt.has_value());
	if (docOpt.has_value()) {
		EXPECT_EQ(docOpt.value().id, dbInitialDocs[0].id);
		EXPECT_EQ(docOpt.value().filename, dbInitialDocs[0].filename);
		EXPECT_EQ(docOpt.value().usernames, dbInitialDocs[0].usernames);
	}
	auto content = getDbContentAsString<DBDocument>();
	EXPECT_EQ(content, "id2,filename,user3\n"
					   "id3,filename,user3\n");
}

TEST(DatabaseTests, AddDocTest) {
	auto db = prepareDocDb();
	DBDocument doc("randomid", "filenameee", {"user1"});
	EXPECT_TRUE(db.addDoc(doc));
	auto content = getDbContentAsString<DBDocument>();
	EXPECT_EQ(content, "id1,filename2,user2;user3\n"
					   "id2,filename,user3\n"
					   "id3,filename,user3\n"
					   "randomid,filenameee,user1\n");
	prepareDocDb();
}

TEST(DatabaseTests, AddUserToDocTest) {
	auto db = prepareDocDb();
	DBUser user("user1", "", {});
	DBDocument doc("id2", "", {});
	EXPECT_TRUE(db.addUserToDoc(user, doc));
	auto content = getDbContentAsString<DBDocument>();
	EXPECT_EQ(content, "id1,filename2,user2;user3\n"
					   "id3,filename,user3\n"
					   "id2,filename,user3;user1\n");
	prepareDocDb();
}

TEST(DatabaseTests, DeleteUserFromDocTest) {
	auto db = prepareDocDb();
	DBUser user("user3", "", {});
	DBDocument doc("id1", "", {});
	EXPECT_TRUE(db.delUserFromDoc(user, doc));
	auto content = getDbContentAsString<DBDocument>();
	EXPECT_EQ(content, "id2,filename,user3\n"
					   "id3,filename,user3\n"
					   "id1,filename2,user2\n");
	prepareDocDb();
}

TEST(DatabaseTests, DeleteUserFromDocNoUsersLeftTest) {
	auto db = prepareDocDb();
	DBUser user("user3", "", {});
	DBDocument doc("id2", "", {});
	EXPECT_TRUE(db.delUserFromDoc(user, doc));
	auto content = getDbContentAsString<DBDocument>();
	EXPECT_EQ(content, "id1,filename2,user2;user3\n"
					   "id3,filename,user3\n");
	prepareDocDb();
}

TEST(DatabaseTests, UnlinkUserFromDocTest) {
	prepareUserDb();
	auto db = prepareDocDb();
	DBUser user("user3", "", {});
	DBDocument doc("id1", "", {});
	EXPECT_TRUE(db.unlinkUserAndDoc(user, doc));
	auto content = getDbContentAsString<DBDocument>();
	EXPECT_EQ(content, "id2,filename,user3\n"
					   "id3,filename,user3\n"
					   "id1,filename2,user2\n");
	content = getDbContentAsString<DBUser>();
	EXPECT_EQ(content, "user1,password,\n"
					   "user2,password2,id1\n"
					   "user3,password,id2;id3\n");
	prepareUserDb();
	prepareDocDb();
}

TEST(DatabaseTests, LinkUserFromDocTest) {
	prepareUserDb();
	auto db = prepareDocDb();
	DBUser user("user1", "", {});
	DBDocument doc("id3", "", {});
	EXPECT_TRUE(db.linkUserAndDoc(user, doc));
	auto content = getDbContentAsString<DBDocument>();
	EXPECT_EQ(content, "id1,filename2,user2;user3\n"
						"id2,filename,user3\n"
						"id3,filename,user3;user1\n");
	content = getDbContentAsString<DBUser>();
	EXPECT_EQ(content, "user2,password2,id1\n"
						"user3,password,id1;id2;id3\n"
						"user1,password,id3\n");
	prepareUserDb();
	prepareDocDb();
}

TEST(DatabaseTests, AddDocAndLinkTest) {
	prepareUserDb();
	auto db = prepareDocDb();
	DBDocument doc("newId", "f", {"user1"});
	EXPECT_TRUE(db.addDocAndLink(doc));
	auto content = getDbContentAsString<DBDocument>();
	EXPECT_EQ(content, "id1,filename2,user2;user3\n"
						"id2,filename,user3\n"
						"id3,filename,user3\n"
						"newId,f,user1\n");
	content = getDbContentAsString<DBUser>();
	EXPECT_EQ(content, "user2,password2,id1\n"
						"user3,password,id1;id2;id3\n"
						"user1,password,newId\n");
	prepareUserDb();
	prepareDocDb();
}
