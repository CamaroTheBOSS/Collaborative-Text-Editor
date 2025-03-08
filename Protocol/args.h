#include <string>
#include <unordered_map>
#include <vector>
#include <variant>

static constexpr const char* commandRun = "__run__";
static constexpr const char* commandHelp = "help";

template <typename E>
constexpr typename std::underlying_type<E>::type to_underlying(E e) noexcept {
	return static_cast<typename std::underlying_type<E>::type>(e);
}

class Args {
public:
	enum class Type { null = 0, integer = 1, string = 2 };
private:
	struct NullArg {};
	using Variant = std::variant<NullArg, int, std::string>;
public:
	struct Command {
	public:
		Command(const std::string& key, const std::string& description = "", std::vector<std::string>&& requiredArgKeys = {}) :
			key(key),
			description(description),
			requiredArgKeys(requiredArgKeys) {}
		bool isArgRequiredForCommand(const std::string& commandKey) {
			return std::find(requiredArgKeys.cbegin(), requiredArgKeys.cend(), commandKey) != requiredArgKeys.cend();
		}
		
		std::string key;
		std::string description;
		std::vector<std::string> requiredArgKeys;
	};
	struct Arg {
	public:
		Arg() = default;
		Arg(const Type type, const std::string& description = "") :
			type(type),
			description(description) {}
		template <typename T>
		Arg(const Type type, const T&& value, const std::string& description = "") :
			type(type),
			value(Variant{value}),
			description(description) {}
		Type type = Type::null;
		Variant value = Variant{ NullArg{} };
		std::string description;
	};
	using ArgsMap = std::unordered_map<std::string, Arg>;
	using Commands = std::vector<Command>;

	Args(ArgsMap&& config = {}, std::vector<Command>&& commands = {});
	std::string parse(int argc, char* argv[]);
	bool isValid() { return valid; }
	std::string getCommand() { return command; }
	template <typename T>
	const T& get(const std::string& key) {
		return std::get<T>(args[key].value);
	}
	std::string getDescription();
private:
	std::string parseArgument(char* argKey, char* argVal);
	std::string parseCommand(char* argVal);

	ArgsMap args;
	std::vector<Command> commands;
	int setCommandIdx = -1;
	std::string command = commandRun;
	bool valid = true;
};