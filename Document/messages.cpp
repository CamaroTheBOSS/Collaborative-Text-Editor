#include "messages.h"

namespace msg {
	Buffer::Buffer(const int capacity) :
		data(std::make_unique<char[]>(capacity)),
		size(0),
		capacity(capacity) {}

	Buffer::Buffer(const Buffer& other) :
		data(std::make_unique<char[]>(other.capacity)),
		size(other.size),
		capacity(other.capacity) {
		if (other.size > 0) {
			memcpy(data.get(), other.get(), other.size);
		}
	}
	Buffer::Buffer(Buffer&& other) noexcept :
		data(std::move(other.data)),
		size(other.size),
		capacity(other.capacity) {}

	char* Buffer::get() const {
		return data.get();
	}

	void Buffer::clear() {
		if (size > 0) {
			memset(data.get(), 0, size);
		}
		size = 0;
	}
	bool Buffer::empty() const {
		return size == 0;
	}

	void Buffer::reserve(const int newCapacity) {
		auto newData = std::make_unique<char[]>(newCapacity);
		memcpy(newData.get(), data.get(), size);
		data = std::move(newData);
		capacity = newCapacity;
	}

	Buffer enrich(Buffer& buffer) {
		Buffer newBuffer{ buffer.capacity + 4 };
		serializeTo(newBuffer, 0, static_cast<unsigned int>(buffer.size), buffer);
		return newBuffer;
	}

	int parseObj(std::string& obj, Buffer& buffer, const int offset) {
		obj = std::string{ buffer.get() + offset };
		return obj.size() + 1;
	}
	int parseObj(unsigned int& obj, Buffer& buffer, const int offset) {
		u_long objBuff;
		memcpy(&objBuff, buffer.get() + offset, sizeof(u_long));
		obj = static_cast<unsigned int>(ntohl(objBuff));
		return sizeof(u_long);
	}
	int parseObj(Type& obj, Buffer& buffer, const int offset) {
		OneByteInt objBuff;
		memcpy(&objBuff, buffer.get() + offset, sizeof(OneByteInt));
		obj = static_cast<Type>(objBuff);
		return sizeof(OneByteInt);
	}
	int parseObj(MoveSide& obj, Buffer& buffer, const int offset) {
		OneByteInt objBuff;
		memcpy(&objBuff, buffer.get() + offset, sizeof(OneByteInt));
		obj = static_cast<MoveSide>(objBuff);
		return sizeof(OneByteInt);
	}

	constexpr std::array<const char*, 15> typeToStr = { "REGISTRATION", "LOGIN", "CREATE" , "LOAD" ,
	"JOIN" , "GETFILES", "SAVEFILE", "ERROR", "WRITE", "ERASE", "MOVEVERTICAL", "MOVEHORIZONTAL", "SYNC",
	"CONNECT", "DISCONNECT"};

	constexpr std::array<const char*, 4> sideToStr = { "LEFT", "RIGHT", "UP", "DOWN" };

	std::ostream& operator<<(std::ostream& stream, const Type& type) {
		stream << typeToStr[static_cast<int>(type)];
		return stream;
	}

	std::ostream& operator<<(std::ostream& stream, const MoveSide side) {
		stream << sideToStr[static_cast<int>(side)];
		return stream;
	}
}