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
		disconnect
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
			assert(capacity >= size + str->size() + 1 && "Error, buffer size excedeed!");
			memcpy(data.get() + size, str->c_str(), str->size() + 1);
			size += str->size() + 1;
		}
		void add(const Buffer* other) {
			assert(capacity >= size + other->size && "Error, buffer size excedeed!");
			memcpy(data.get() + size, other->get(), other->size);
			size += other->size;
		}
		void add(const Buffer* other, const int start, const int cpsize) {
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
		bool operator=(const Buffer& other);

		std::unique_ptr<char[]> data;
		int size;
		int capacity;
	};
	Buffer enrich(Buffer& buffer);

	template<typename T>
	int parseObj(T& obj, Buffer& buffer, const int offset) {
		memcpy(&obj, buffer.get() + offset, sizeof(T));
		return sizeof(T);
	}
	int parseObj(std::string& obj, Buffer& buffer, const int offset);
	int parseObj(unsigned int& obj, Buffer& buffer, const int offset); 
	int parseObj(Type& obj, Buffer& buffer, const int offset);
	int parseObj(MoveSide& obj, Buffer& buffer, const int offset);
	template<typename T>
	int parseObj(std::vector<T>& arr, Buffer& buffer, const int offset) {
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
	int parse(Buffer& buffer, int pos, Args&... args) {
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
		OneByteInt version;
	};

	struct ConnectResponse {
		Type type = Type::connect;
		OneByteInt version;
		OneByteInt user; // Which user idx you are in doc
		std::string text; // Whole current state of the document
		std::vector<unsigned int> cursorPositions;
	};

	struct Disconnect {
		Type type = Type::disconnect;
		OneByteInt version;
	};

	struct DisconnectResponse {
		Type type = Type::disconnect;
		OneByteInt version;
		OneByteInt user;
	};

	struct Write {
		Type type = Type::write;
		OneByteInt version;
		std::string token;
		std::string text;
	};

	struct WriteResponse {
		Type type = Type::write;
		OneByteInt version;
		OneByteInt user;
		std::string text;
	};

	struct Erase {
		Type type = Type::erase;
		OneByteInt version;
		std::string token;
		unsigned int eraseSize;
	};

	struct EraseResponse {
		Type type = Type::erase;
		OneByteInt version;
		OneByteInt user;
		unsigned int eraseSize;
	};

	struct MoveHorizontal {
		Type type = Type::moveHorizontal;
		OneByteInt version;
		std::string token;
		MoveSide side;
	};

	struct MoveHorizontalResponse {
		Type type = Type::moveHorizontal;
		OneByteInt version;
		OneByteInt user;
		unsigned int X;
		unsigned int Y;
	};

	struct MoveVertical {
		Type type = Type::moveVertical;
		OneByteInt version;
		std::string token;
		MoveSide side;
		unsigned int clientWidth;
	};

	struct MoveVerticalResponse {
		Type type = Type::moveVertical;
		OneByteInt version;
		OneByteInt user;
		unsigned int X;
		unsigned int Y;
	};

}
