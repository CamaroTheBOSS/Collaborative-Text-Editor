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
		masterNotification,
		registration,
		login,
		create,
		load,
		join,
		getFiles,
		saveFile,
		error,
		// Modifiers
		write,
		erase,
		moveVertical,
		moveHorizontal,
		sync,
		connect,
		disconnect,
		selectAll,
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
		void add(const unsigned int* val) {
			u_long uLongVal = htonl(*val);
			add(&uLongVal);
		}
		void add(const Type* type) {
			OneByteInt byteVal = static_cast<OneByteInt>(*type);
			add(&byteVal);
		}
		void add(const MoveSide* type) {
			OneByteInt byteVal = static_cast<OneByteInt>(*type);
			add(&byteVal);
		}
		void add(const std::string* str) {
			reserveIfNeeded(str->size() + 1);
			assert(capacity >= size + str->size() + 1 && "Error, buffer size excedeed!");
			memcpy(data.get() + size, str->c_str(), str->size() + 1);
			size += str->size() + 1;
		}
		void add(const Buffer* other) {
			reserveIfNeeded(other->size);
			assert(capacity >= size + other->size && "Error, buffer size excedeed!");
			memcpy(data.get() + size, other->get(), other->size);
			size += other->size;
		}
		void add(const Buffer* other, const int start, const int cpsize) {
			reserveIfNeeded(cpsize);
			assert(capacity >= size + cpsize && "Error, buffer size excedeed!");
			assert(start + cpsize <= other->size && "Error, buffer size excedeed!");
			memcpy(data.get() + size, other->get() + start, cpsize);
			size += cpsize;
		}
		template<typename T>
		void add(const std::vector<T>* arr) {
			unsigned int arrSize = arr->size();
			add(&arrSize);
			for (const auto& element : *arr) {
				add(&element);
			}
		}
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
	Buffer enrich(Buffer& buffer);

	template<typename T>
	int parseObj(T& obj, const Buffer& buffer, const int offset) {
		memcpy(&obj, buffer.get() + offset, sizeof(T));
		return sizeof(T);
	}
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
			arr.push_back(T{ 0 });
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

	struct Connect {
		Type type = Type::connect;
		OneByteInt version = 0;
	};

	struct ConnectResponse {
		Type type = Type::connect;
		OneByteInt version = 0;
		OneByteInt user = 0; // Which user idx you are in doc
		std::string text; // Whole current state of the document
		std::vector<unsigned int> cursorPositions;
	};

	struct Disconnect {
		Type type = Type::disconnect;
		OneByteInt version = 0;
	};

	struct DisconnectResponse {
		Type type = Type::disconnect;
		OneByteInt version = 0;
		OneByteInt user = 0;
	};

	struct Write {
		Type type = Type::write;
		OneByteInt version = 0;
		std::string token;
		std::string text;
	};

	struct WriteResponse {
		Type type = Type::write;
		OneByteInt version = 0;
		OneByteInt user = 0;
		std::string text;
	};

	struct Erase {
		Type type = Type::erase;
		OneByteInt version = 0;
		std::string token;
		unsigned int eraseSize = 0;
	};

	struct EraseResponse {
		Type type = Type::erase;
		OneByteInt version = 0;
		OneByteInt user = 0;
		unsigned int eraseSize = 0;
	};

	struct MoveHorizontal {
		Type type = Type::moveHorizontal;
		OneByteInt version = 0;
		std::string token;
		MoveSide side = MoveSide::left;
		OneByteInt withSelect = 0;
	};

	struct MoveVertical {
		Type type = Type::moveVertical;
		OneByteInt version = 0;
		std::string token;
		MoveSide side = MoveSide::up;
		unsigned int clientWidth = 0;
		OneByteInt withSelect = 0;
	};

	struct MoveSelectAll {
		Type type = Type::selectAll;
		OneByteInt version = 0;
		std::string token;
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
}
