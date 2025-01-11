#pragma once
#include <string_view>
#include <string>
#include <vector>
#include <optional>

namespace v3 {

	struct Pos {
	public:
		int x;
		int y;
		Pos& operator=(const Pos& other);
		Pos operator+(const Pos& other) const;
		Pos operator-(const Pos& other) const;
	};
	bool operator>=(const Pos& first, const Pos& other);
	bool operator>(const Pos& first, const Pos& other);
	bool operator<=(const Pos& first, const Pos& other);
	bool operator<(const Pos& first, const Pos& other);
	bool operator==(const Pos& first, const Pos& other);


	//class Action {
	//
	//};
	//
	//class History {
	//
	//};

	class Cursor {
	public:
		Cursor() = default;
		Cursor(const Pos& pos);
		Cursor(const Pos& pos, const int offset, const char label);
		void updateAfterEdit(const Pos& newPos, const char newLabel);
		Pos pos;
		int offset;
		char label;
	};
	bool operator==(const Cursor& first, const Cursor& other);

	struct User {
		Cursor cursor;
		std::optional<Cursor> anchor;
		//History history;
	};

	class Line {
	public:
		Line() = default;
		Line(const std::string_view& text);
		int insert(const int pos, const std::string_view newText);
		int append(std::string_view newText);
		int erase(const int pos, const int n);
		std::string cut(int left, int right);
		std::string_view get(const int left, const int right) const;
		int size() const;
	private:
		int effectiveLeft(const int left) const;
		int effectiveRight(const int right) const;

		std::string text;
	};

	class NewTextContainer {
	public:
		NewTextContainer() = default;
		NewTextContainer(const std::string& text);
		Pos write(const Cursor& cursor, const std::string& text);
		Pos erase(const Cursor& cursor, int n);

		Pos& moveLeft(Cursor& cursor);
		Pos& moveRight(Cursor& cursor);
		Pos& moveUp(Cursor& cursor, const int width);
		Pos& moveDown(Cursor& cursor, const int width);
		Pos& moveTo(Cursor& cursor, const Pos& endPos);

		std::string get(const Pos& left, const Pos& right) const;
		std::string_view getLine(const int line) const;
		char getLabel(const Pos& pos) const;
		Pos startPos() const;
		Pos endPos() const;
		int size() const;
	private:
		std::vector<std::string_view> parseText(const std::string& text) const;
		Pos insertText(Pos pos, const std::vector<std::string_view>& parsedLines);
		Line& addNewLine(const int col, const std::string_view initText);
		int eraseLine(const int col);
		bool isCursorValid(const Cursor& cursor) const;

		std::vector<Line> lines = { Line() };
	};


	class Document {
	public:
		Document(const std::string& txt, const int nUsers, const int myUserIdx);
		Pos write(const int userIdx, std::string& text);
		Pos erase(const int userIdx, const int n);

		Pos moveLeft(const int userIdx, const bool select);
		Pos moveRight(const int userIdx, const bool select);
		Pos moveUp(const int userIdx, const int width, const bool select);
		Pos moveDown(const int userIdx, const int width, const bool select);
		Pos moveTo(const int userIdx, const Pos& pos);

		std::string getText() const;
		const std::string_view getLine(const int n) const;
		std::string getSelectedText(const int userIdx) const;

		std::vector<Cursor&> getCursors();
		Cursor getCursor(const int userIdx) const;
		int height() const;
	private:
		void moveCursors(User& movedUser, const Pos startPos, const Pos& endPos);
		void moveCursor(Cursor& cursor, const Pos& startPos, const Pos& posDiff);

		NewTextContainer container;
		std::vector<User> users;
		int myUserIdx;
	};

}