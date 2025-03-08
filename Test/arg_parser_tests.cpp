#include "pch.h"
#include "args.h"

static constexpr int invalidInt = -134320423;
struct Arg {
	Arg(const char* key, const char* val) :
		cfgKey(key),
		inputKey("--" + std::string{key}),
		inputVal(val) {
			try {
				intVal = std::stoi(val);
			}
			catch (...) {}
		}
	int addToArgs(char* argv[], int pos) const {
		argv[pos] = (char*)inputKey.c_str();
		argv[pos + 1] = (char*)inputVal.c_str();
		return pos + 2;
	}
	std::string cfgKey;
	std::string inputKey;
	std::string inputVal;
	int intVal = invalidInt;
};

bool test(Args& args, Arg& arg) {
	if (arg.intVal == invalidInt) {
		std::string val = args.get<std::string>(arg.cfgKey);
		EXPECT_EQ(val, arg.inputVal);
		return val == arg.inputVal;
	}
	int val = args.get<int>(arg.cfgKey);
	EXPECT_EQ(val, arg.intVal);
	return val == arg.intVal;
}

template<typename... Args>
void addArgs(char* argv[], int pos, Args&&... args) {
	([&] {
		pos = args.addToArgs(argv, pos);
		} (), ...);
}
static constexpr const char* cmd1 = "help";
static constexpr const char* exe = "executable";
static Arg strArg{"a1", "val1"};
static Arg intArg{"a2", "54"};
static Arg invalidIntArg{"a3", "gdfgdf"};
static Arg emptyStrArg{"a4", ""};

TEST(ArgParserTests, HappyTest) {
	constexpr int argc = 5;
	char* argv[argc] = { (char*)exe };
	addArgs(argv, 1, strArg, intArg);
	Args::ArgsMap argsConfig {
		{ strArg.cfgKey, Args::Arg{ Args::Type::string } },
		{ intArg.cfgKey, Args::Arg{ Args::Type::integer } }
	};
	Args args{ std::move(argsConfig) };
	args.parse(argc, argv);
	EXPECT_TRUE(args.isValid());
	test(args, strArg);
	test(args, intArg);
}

TEST(ArgParserTests, EmptyArgTest) {
	constexpr int argc = 5;
	char* argv[argc] = { (char*)exe };
	addArgs(argv, 1, strArg, emptyStrArg);
	Args::ArgsMap argsConfig {
		{ strArg.cfgKey, Args::Arg{ Args::Type::string } },
		{ emptyStrArg.cfgKey, Args::Arg{ Args::Type::string } }
	};
	Args args{ std::move(argsConfig) };
	auto errMsg = args.parse(argc, argv);
	EXPECT_TRUE(args.isValid());
	test(args, strArg);
	test(args, emptyStrArg);
}

TEST(ArgParserTests, NoArgValueProvidedTest) {
	constexpr int argc = 2;
	char* argv[argc] = { (char*)exe, (char*)intArg.inputKey.c_str()};
	Args::ArgsMap argsConfig {
		{ strArg.cfgKey, Args::Arg{ Args::Type::string } },
		{ intArg.cfgKey, Args::Arg{ Args::Type::integer } }
	};
	Args args{ std::move(argsConfig) };
	auto errMsg = args.parse(argc, argv);
	EXPECT_FALSE(args.isValid());
	EXPECT_EQ(errMsg, "No value provied to argument '--a2'\n\n");
}

TEST(ArgParserTests, InvalidIntValueProvidedTest) {
	constexpr int argc = 5;
	char* argv[argc] = { (char*)exe };
	addArgs(argv, 1, strArg, invalidIntArg);
	Args::ArgsMap argsConfig {
		{ strArg.cfgKey, Args::Arg{ Args::Type::string } },
		{ invalidIntArg.cfgKey, Args::Arg{ Args::Type::integer } }
	};
	Args args{ std::move(argsConfig) };
	auto errMsg = args.parse(argc, argv);
	EXPECT_FALSE(args.isValid());
	EXPECT_EQ(errMsg, "Caught exception for argument '--a3': invalid stoi argument\n\n");
}

TEST(ArgParserTests, CommandFirstTest) {
	constexpr int argc = 2;
	char* argv[argc] = { (char*)exe, (char*)cmd1 };
	Args::ArgsMap argsConfig {
		{ strArg.cfgKey, Args::Arg{ Args::Type::string } },
		{ intArg.cfgKey, Args::Arg{ Args::Type::integer } }
	};
	Args::Commands commands {Args::Command{ cmd1 }};
	Args args{ std::move(argsConfig), std::move(commands) };
	args.parse(argc, argv);
	EXPECT_TRUE(args.isValid());
	EXPECT_EQ(args.getCommand(), cmd1);
}

TEST(ArgParserTests, CommandFirstAndThenArgsTest) {
	constexpr int argc = 4;
	char* argv[argc] = { (char*)exe, (char*)cmd1 };
	addArgs(argv, 2, strArg);
	Args::ArgsMap argsConfig {
		{ strArg.cfgKey, Args::Arg{ Args::Type::string } },
		{ intArg.cfgKey, Args::Arg{ Args::Type::integer } }
	};
	Args::Commands commands {Args::Command{ cmd1 }};
	Args args{ std::move(argsConfig), std::move(commands) };
	auto errMsg = args.parse(argc, argv);
	EXPECT_TRUE(errMsg.empty());
	EXPECT_TRUE(args.isValid());
	EXPECT_EQ(args.getCommand(), cmd1);
}