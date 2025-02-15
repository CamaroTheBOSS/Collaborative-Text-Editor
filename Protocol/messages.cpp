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
		if (size > 0) {
			memcpy(newData.get(), data.get(), size);
		}
		data = std::move(newData);
		capacity = newCapacity;
		
	}

	void Buffer::reserveIfNeeded(const int cpsize) {
		if (capacity < size + cpsize) {
			reserve(size + cpsize + 64);
		}
	}

	bool Buffer::operator=(const Buffer& other) {
		return other.data == data;
	}

	Buffer enrich(Buffer& buffer) {
		Buffer newBuffer{ buffer.capacity + 4 };
		serializeTo(newBuffer, 0, static_cast<unsigned int>(buffer.size), buffer);
		return newBuffer;
	}
	int parseObj(std::pair<COORD, COORD>& obj, const Buffer& buffer, const int offset) {
		unsigned int x1, y1, x2, y2;
		int pos = parseObj(x1, buffer, offset);
		pos += parseObj(y1, buffer, offset + pos);
		pos += parseObj(x2, buffer, offset + pos);
		pos += parseObj(y2, buffer, offset + pos);
		obj.first = COORD{ static_cast<SHORT>(x1), static_cast<SHORT>(y1) };
		obj.second = COORD{ static_cast<SHORT>(x2), static_cast<SHORT>(y2) };
		return pos;
	}
	int parseObj(std::string& obj, const Buffer& buffer, const int offset) {
		obj = std::string{ buffer.get() + offset };
		return obj.size() + 1;
	}
	int parseObj(unsigned int& obj, const Buffer& buffer, const int offset) {
		u_long objBuff;
		memcpy(&objBuff, buffer.get() + offset, sizeof(u_long));
		obj = static_cast<unsigned int>(ntohl(objBuff));
		return sizeof(u_long);
	}
	int parseObj(Type& obj, const Buffer& buffer, const int offset) {
		OneByteInt objBuff;
		memcpy(&objBuff, buffer.get() + offset, sizeof(OneByteInt));
		obj = static_cast<Type>(objBuff);
		return sizeof(OneByteInt);
	}
	int parseObj(MoveSide& obj, const Buffer& buffer, const int offset) {
		OneByteInt objBuff;
		memcpy(&objBuff, buffer.get() + offset, sizeof(OneByteInt));
		obj = static_cast<MoveSide>(objBuff);
		return sizeof(OneByteInt);
	}

	constexpr std::array<const char*, 22> typeToStr = { "MASTER NOTIFICATION", "MASTER CLOSE", "REGISTRATION", "LOGIN", "CREATE" , "LOAD" ,
	"JOIN" , "GETFILES", "SAVEFILE", "ERROR", "WRITE", "ERASE", "REPLACE", "MOVEVERTICAL", "MOVEHORIZONTAL", "MOVETO", "SYNC",
	"CONNECT", "DISCONNECT", "SELECT ALL", "UNDO", "REDO"};

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