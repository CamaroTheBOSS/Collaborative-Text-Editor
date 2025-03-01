#pragma once
#include <Winsock2.h>
#include <memory>
#include <assert.h>
#include <string>
#include <vector>
#include <sstream>
#include <type_traits>
#include <array>

#pragma comment(lib, "Ws2_32.lib")


namespace msg {
	using OneByteInt = unsigned char;
	enum class Type {
		// Commands
		masterForwardConnect,
		masterClose,
		registration,
		login,
		logout,
		create,
		load,
		join,
		getFiles,
		saveFile,
		error,
		// Modifiers
		write,
		erase,
		replace,
		moveVertical,
		moveHorizontal,
		moveTo,
		sync,
		connect,
		disconnect,
		selectAll,
		undo,
		redo
	};

	enum class MoveSide {
		left,
		right,
		up,
		down
	};

	std::ostream& operator<<(std::ostream& stream, const Type& type);
	std::ostream& operator<<(std::ostream& stream, const MoveSide side);


	class Buffer {
	public:
		Buffer(const int capacity);
		Buffer(const Buffer& other);
		Buffer(Buffer&& other) noexcept;

		template<typename T>
		void add(const T* val) {
			reserveIfNeeded(sizeof(T));
			assert(capacity >= size + sizeof(T) && "Error, buffer size excedeed!");
			memcpy(data.get() + size, val, sizeof(T));
			size += sizeof(T);
		}
		// Swap to BigEndian if system is LittleEndian
		void add(const unsigned int* val);
		void add(const Type* type);
		void add(const MoveSide* type);
		void add(const std::string* str);
		void add(const Buffer* other);
		void add(const Buffer* other, const int start, const int cpsize);
		void add(const std::pair<COORD, COORD>* val);
		template<typename T>
		void add(const std::vector<T>* arr) {
			unsigned int arrSize = arr->size();
			add(&arrSize);
			for (const auto& element : *arr) {
				add(&element);
			}
		}
		void replace(const int pos, const unsigned int val);
		void clear();
		char* get() const;
		bool empty() const;
		void reserve(const int capacity);
		void reserveIfNeeded(const int cpsize);
		bool operator=(const Buffer& other);

		std::unique_ptr<char[]> data;
		int size;
		int capacity;
	};
	Buffer enrich(const Buffer& buffer);

	template<typename T>
	int parseObj(T& obj, const Buffer& buffer, const int offset) {
		memcpy(&obj, buffer.get() + offset, sizeof(T));
		return sizeof(T);
	}
	int parseObj(std::pair<COORD, COORD>& obj, const Buffer& buffer, const int offset);
	int parseObj(std::string& obj, const Buffer& buffer, const int offset);
	int parseObj(unsigned int& obj, const Buffer& buffer, const int offset); 
	int parseObj(Type& obj, const Buffer& buffer, const int offset);
	int parseObj(MoveSide& obj, const Buffer& buffer, const int offset);
	template<typename T>
	int parseObj(std::vector<T>& arr, const Buffer& buffer, const int offset) {
		unsigned int arrSize;
		int pos = offset;
		pos += parseObj(arrSize, buffer, pos);
		for (int i = 0; i < arrSize; i++) {
			arr.push_back(T{});
			pos += parseObj(arr[arr.size() - 1], buffer, pos);
		}
		return pos;
	}

	template<typename... Args>
	int parse(const Buffer& buffer, int pos, Args&... args) {
		([&] {
			pos += parseObj(args, buffer, pos);
		} (), ...);
		return pos;
	}

	template<typename...Args>
	void serializeTo(Buffer& buffer, int pos, Args&&... args) {
		([&] {
			buffer.add(&args);
		} (), ...);
	}

	struct AckMsg {
		Type type = Type::logout;
		OneByteInt version = 0;
	};

	struct AckResponse {
		Type type = Type::logout;
		OneByteInt version = 0;
	};

	struct Login {
		Type type = Type::login;
		OneByteInt version = 0;
		std::string login;
		std::string password;
	};

	struct LoginResponse {
		Type type = Type::login;
		OneByteInt version = 0;
		std::string errMsg;
		std::string authToken;
	};

	struct Register {
		Type type = Type::registration;
		OneByteInt version = 0;
		std::string login;
		std::string password;
	};

	struct RegisterResponse {
		Type type = Type::registration;
		OneByteInt version = 0;
		std::string errMsg;
	};

	struct ConnectCreateDoc {
		Type type = Type::create;
		OneByteInt version = 0;
		unsigned int socket = 0;
		std::string filename;
	};

	struct ConnectJoinDoc {
		Type type = Type::join;
		OneByteInt version = 0;
		unsigned int socket = 0;
		std::string acCode;
	};

	struct ConnectResponse {
		Type type = Type::connect;
		OneByteInt version = 0;
		OneByteInt user = 0; // Which user idx you are in doc
		std::string error; // If not empty -> error happened
		std::string acCode; //Access code to document
		std::string text; // Whole current state of the document
		std::vector<unsigned int> cursorPositions;
	};

	struct Disconnect {
		Type type = Type::disconnect;
		OneByteInt version = 0;
		std::string authToken;
	};

	struct DisconnectResponse {
		Type type = Type::disconnect;
		OneByteInt version = 0;
		OneByteInt user = 0;
	};

	struct Write {
		Type type = Type::write;
		OneByteInt version = 0;
		std::string authToken;;
		std::string text;
	};

	struct WriteResponse {
		Type type = Type::write;
		OneByteInt version = 0;
		OneByteInt user = 0;
		std::string text;
		unsigned int X = 0;
		unsigned int Y = 0;
	};

	struct Erase {
		Type type = Type::erase;
		OneByteInt version = 0;
		std::string authToken;
		unsigned int eraseSize = 0;
	};

	struct EraseResponse {
		Type type = Type::erase;
		OneByteInt version = 0;
		OneByteInt user = 0;
		unsigned int eraseSize = 0;
		unsigned int X = 0;
		unsigned int Y = 0;
	};

	struct Replace {
		Type type = Type::replace;
		OneByteInt version = 0;
		std::string authToken;
		std::string text; // new text
		std::vector<std::pair<COORD, COORD>> segments;
	};

	struct ReplaceResponse {
		Type type = Type::replace;
		OneByteInt version = 0;
		OneByteInt user = 0;
		std::string text; // new text
		std::vector<std::pair<COORD, COORD>> segments;
	};

	struct MoveHorizontal {
		Type type = Type::moveHorizontal;
		OneByteInt version = 0;
		std::string authToken;
		MoveSide side = MoveSide::left;
		OneByteInt withSelect = 0;
	};

	struct MoveVertical {
		Type type = Type::moveVertical;
		OneByteInt version = 0;
		std::string authToken;
		MoveSide side = MoveSide::up;
		unsigned int clientWidth = 0;
		OneByteInt withSelect = 0;
	};

	struct MoveSelectAll {
		Type type = Type::selectAll;
		OneByteInt version = 0;
		std::string authToken;
	};

	struct MoveTo {
		Type type = Type::moveTo;
		OneByteInt version = 0;
		std::string authToken;
		unsigned int X = 0;
		unsigned int Y = 0;
	};

	struct MoveResponse {
		Type type = Type::moveVertical;
		OneByteInt version = 0;
		OneByteInt user = 0;
		unsigned int X = 0;
		unsigned int Y = 0;
		OneByteInt withSelect = 0;
		unsigned int anchorX = 0;
		unsigned int anchorY = 0;
	};

	struct ControlMessage {
		Type type;
		OneByteInt version;
		std::string authToken;
	};

	struct ControlMessageResponse {
		Type type;
		OneByteInt version;
		OneByteInt user;
	};
}
