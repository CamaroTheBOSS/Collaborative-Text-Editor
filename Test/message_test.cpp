#include "pch.h"
#include "messages.h"

constexpr msg::OneByteInt oneByteInt = 2;
const std::string str = "txt";
constexpr unsigned int uint = 42;
constexpr msg::Type msgType = msg::Type::write;
constexpr msg::MoveSide moveSide = msg::MoveSide::up;

struct ExampleStruct {
	msg::OneByteInt oneByteInt;
	std::string str;
	unsigned int value;
	msg::Type type;
	msg::MoveSide side;
};

TEST(BufferTests, SerializeOneByteIntTest) {
	msg::Buffer buffer{8};
	msg::serializeTo(buffer, 0, oneByteInt);
	EXPECT_EQ(buffer.get()[0], oneByteInt);
	EXPECT_EQ(buffer.size, 1);
}

TEST(BufferTests, SerializeStringTest) {
	msg::Buffer buffer{8};
	msg::serializeTo(buffer, 0, str);
	std::string parsed{buffer.get(), str.size()};
	EXPECT_EQ(parsed, str);
	EXPECT_EQ(buffer.size, 4);
}

TEST(BufferTests, SerializeUnsignedIntTest) {
	msg::Buffer buffer{8};
	msg::serializeTo(buffer, 0, uint);
	EXPECT_EQ(buffer.get()[0], 0);
	EXPECT_EQ(buffer.get()[1], 0);
	EXPECT_EQ(buffer.get()[2], 0);
	EXPECT_EQ(buffer.get()[3], uint);
	EXPECT_EQ(buffer.size, 4);
}

TEST(BufferTests, SerializeTypeTest) {
	msg::Buffer buffer{8};
	msg::serializeTo(buffer, 0, msgType);
	EXPECT_EQ(buffer.get()[0], static_cast<int>(msgType));
	EXPECT_EQ(buffer.size, 1);
}

TEST(BufferTests, SerializeMoveSideTest) {
	msg::Buffer buffer{8};
	msg::serializeTo(buffer, 0, moveSide);
	EXPECT_EQ(buffer.get()[0], static_cast<int>(moveSide));
	EXPECT_EQ(buffer.size, 1);
}

TEST(BufferTests, SerializeOBufferTest) {
	msg::Buffer bigger{16};
	msg::Buffer smaller{8};
	msg::serializeTo(smaller, 0, 'a');
	msg::serializeTo(bigger, 0, smaller);
	EXPECT_EQ(bigger.get()[0], 'a');
	EXPECT_EQ(bigger.size, 1);
}

TEST(BufferTests, SerializeMultipleTypes) {
	msg::Buffer buffer{128};
	auto container = ExampleStruct{ oneByteInt, str, uint, msgType, moveSide };
	msg::serializeTo(buffer, 0, container.oneByteInt, container.side, container.str, container.type, container.value);
	EXPECT_EQ(buffer.get()[0], container.oneByteInt);
	EXPECT_EQ(buffer.get()[1], static_cast<int>(container.side));
	std::string parsed{buffer.get() + 2, container.str.size()};
	EXPECT_EQ(parsed, "txt");
	EXPECT_EQ(buffer.get()[5], 0);
	EXPECT_EQ(buffer.get()[6], static_cast<int>(container.type));
	EXPECT_EQ(buffer.get()[7], 0);
	EXPECT_EQ(buffer.get()[8], 0);
	EXPECT_EQ(buffer.get()[9], 0);
	EXPECT_EQ(buffer.get()[10], container.value);
	EXPECT_EQ(buffer.size, 11);
}

TEST(BufferTests, SerializeOneByteIntRValueTest) {
	msg::Buffer buffer{8};
	msg::serializeTo(buffer, 0, 'a');
	EXPECT_EQ(buffer.get()[0], 'a');
	EXPECT_EQ(buffer.size, 1);
}

TEST(BufferTests, SerializeMultipleTypesRValueAndLValue) {
	msg::Buffer buffer{128};
	auto container = ExampleStruct{ oneByteInt, str, uint, msgType, moveSide };
	msg::serializeTo(buffer, 0, 'a', container.side, container.str, container.type, container.value);
	EXPECT_EQ(buffer.get()[0], 'a');
	EXPECT_EQ(buffer.get()[1], static_cast<int>(container.side));
	std::string parsed{buffer.get() + 2, container.str.size()};
	EXPECT_EQ(parsed, "txt");
	EXPECT_EQ(buffer.get()[5], 0);
	EXPECT_EQ(buffer.get()[6], static_cast<int>(container.type));
	EXPECT_EQ(buffer.get()[7], 0);
	EXPECT_EQ(buffer.get()[8], 0);
	EXPECT_EQ(buffer.get()[9], 0);
	EXPECT_EQ(buffer.get()[10], container.value);
	EXPECT_EQ(buffer.size, 11);
}

TEST(BufferTests, ParseOneByteIntTest) {
	msg::Buffer buffer{8};
	buffer.get()[0] = oneByteInt;
	buffer.size = 1;
	msg::OneByteInt value;
	msg::parse(buffer, 0, value);
	EXPECT_EQ(value, oneByteInt);
}

TEST(BufferTests, ParseStringTest) {
	msg::Buffer buffer{8};
	memcpy(buffer.get(), str.c_str(), str.size() + 1);
	buffer.size = str.size() + 1;
	std::string myStr;
	msg::parse(buffer, 0, myStr);
	EXPECT_EQ(myStr, str);
}

TEST(BufferTests, ParseUnsignedIntTest) {
	msg::Buffer buffer{8};
	buffer.get()[0] = 0;
	buffer.get()[1] = 0;
	buffer.get()[2] = 0;
	buffer.get()[3] = uint;
	buffer.size = 4;
	unsigned int myUint;
	msg::parse(buffer, 0, myUint);
	EXPECT_EQ(myUint, uint);
}

TEST(BufferTests, ParseTypeTest) {
	msg::Buffer buffer{8};
	buffer.get()[0] = static_cast<int>(msgType);
	buffer.size = 1;
	msg::Type type;
	msg::parse(buffer, 0, type);
	EXPECT_EQ(type, msgType);
}

TEST(BufferTests, ParseMoveSideTest) {
	msg::Buffer buffer{8};
	buffer.get()[0] = static_cast<int>(moveSide);
	buffer.size = 1;
	msg::MoveSide side;
	msg::parse(buffer, 0, side);
	EXPECT_EQ(side, moveSide);
}

TEST(BufferTests, ParseMultipleTypesTest) {
	msg::Buffer buffer{11};
	buffer.get()[0] = oneByteInt;
	memcpy(buffer.get() + 1, str.c_str(), str.size() + 1);
	buffer.get()[5] = 0;
	buffer.get()[6] = 0;
	buffer.get()[7] = 0;
	buffer.get()[8] = uint;
	buffer.get()[9] = static_cast<int>(msgType);
	buffer.get()[10] = static_cast<int>(moveSide);
	buffer.size = 11;
	auto myStruct = ExampleStruct{};
	msg::parse(buffer, 0, myStruct.oneByteInt, myStruct.str, myStruct.value, myStruct.type, myStruct.side);
	EXPECT_EQ(myStruct.oneByteInt, oneByteInt);
	EXPECT_EQ(myStruct.str, str);
	EXPECT_EQ(myStruct.value, uint);
	EXPECT_EQ(myStruct.type, msgType);
	EXPECT_EQ(myStruct.side, moveSide);
}

TEST(BufferTests, EnrichBufferTest) {
	msg::Buffer buffer{8};
	msg::serializeTo(buffer, 0, oneByteInt);
	msg::Buffer enriched = msg::enrich(buffer);
	EXPECT_EQ(enriched.size, 5);
	EXPECT_EQ(enriched.get()[0], 0);
	EXPECT_EQ(enriched.get()[1], 0);
	EXPECT_EQ(enriched.get()[2], 0);
	EXPECT_EQ(enriched.get()[3], 1);
	EXPECT_EQ(enriched.get()[4], oneByteInt);
}

TEST(BufferTests, ReserveMemoryTest) {
	msg::Buffer buffer{1};
	msg::serializeTo(buffer, 0, oneByteInt);
	buffer.reserve(13);
	msg::serializeTo(buffer, 0, str);
	EXPECT_EQ(buffer.size, 5);
	EXPECT_EQ(buffer.capacity, 13);
	EXPECT_EQ(buffer.get()[0], oneByteInt);
	EXPECT_EQ(buffer.get()[1], 't');
	EXPECT_EQ(buffer.get()[2], 'x');
	EXPECT_EQ(buffer.get()[3], 't');
	EXPECT_EQ(buffer.get()[4], 0);
}

TEST(BufferTests, SerializeWriteMsg) {
	msg::Buffer buffer{128};
	const int key = 97;
	msg::serializeTo(buffer, 0, msg::Type::write, oneByteInt, std::string{""}, std::string(1, key));
	EXPECT_EQ(buffer.size, 5);
	EXPECT_EQ(buffer.get()[1], oneByteInt);
	EXPECT_EQ(buffer.get()[2], 0);
	EXPECT_EQ(buffer.get()[3], key);
	EXPECT_EQ(buffer.get()[4], 0);
}