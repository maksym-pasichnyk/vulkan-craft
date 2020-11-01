#pragma once

#include <map>
#include <string>
#include <string_view>
#include <vector>
#include <variant>
#include <optional>
#include <span>

template<class... Ts> struct overload : Ts... { using Ts::operator()...; };
template<class... Ts> overload(Ts...) -> overload<Ts...>;

namespace json {
	struct Value;
	using Span = std::span<Value>;
	using Array = std::vector<Value>;
	using Object = std::map<std::string, Value>;
	using Null = struct {};

	template <typename T>
	struct Result {
		constexpr Result(std::nullopt_t) : storage(std::nullopt) {}
		constexpr Result(T& ref) : storage(std::addressof(ref)) {}

		template <typename U>
		constexpr T value_or(U&& arg) {
			if (storage.has_value()) {
				return **storage;
			}
			return std::forward<U>(arg);
		}

		constexpr operator bool() const {
			return storage.has_value();
		}

		constexpr bool has_value() const {
			return storage.has_value();
		}

		constexpr T& value() {
			return *storage.value();
		}

		constexpr const T& value() const {
			return *storage.value();
		}

		constexpr T* operator->() {
			return std::addressof(**storage);
		}

		constexpr const T* operator->() const {
			return std::addressof(**storage);
		}

		constexpr T& operator*() {
			return **storage;
		}

		constexpr const T& operator*() const {
			return **storage;
		}
	private:
		std::optional<T*> storage;
	};

	struct Value : public std::variant<int64_t, uint64_t, double, std::string, bool, Array, Object, Null> {
		using variant::variant;
		using variant::valueless_by_exception;

		std::optional<int64_t> as_i64() {
			switch (index()) {
			case 0:
				return std::get<int64_t>(*this);
			case 1:
				return (int64_t) std::get<uint64_t>(*this);
			default:
				return (int64_t) std::get<double>(*this);
			}
		}

		std::optional<double> as_f64() {
			switch (index()) {
			case 0:
				return (double) std::get<int64_t>(*this);
			case 1:
				return (double) std::get<uint64_t>(*this);
			default:
				return std::get<double>(*this);
			}
		}

		Result<std::string> as_string() & {
			return get_opt_ref<std::string>(*this);
		}

		std::optional<std::string> as_string() && {
			return get_opt<std::string>(std::move(*this));
		}

		Result<bool> as_bool() & {
			return get_opt_ref<bool>(*this);
		}

		Result<Array> as_array() & {
			return get_opt_ref<Array>(*this);
		}

		std::optional<Array> as_array() && {
			return get_opt<Array>(std::move(*this));
		}

		Result<Object> as_object() & {
			return get_opt_ref<Object>(*this);
		}

		std::optional<Object> as_object() && {
			return get_opt<Object>(std::move(*this));
		}

		Result<Value> get(const std::string& key) & {
			if (auto obj = as_object()) {
				auto it = obj->find(key);
				if (it != obj->end()) {
					return it->second;
				}
			}
			return std::nullopt;
		}

		bool is_number() const {
			switch (index()) {
			case 0: case 1: case 2:
				return true;
			default:
				return false;
			}
		}

		bool is_string() const {
			switch (index()) {
			case 3:
				return true;
			default:
				return false;
			}
		}

		bool is_bool() const {
			switch (index()) {
			case 4:
				return true;
			default:
				return false;
			}
		}

		bool is_array() const {
			switch (index()) {
			case 5:
				return true;
			default:
				return false;
			}
		}

		bool is_object() const {
			switch (index()) {
			case 6:
				return true;
			default:
				return false;
			}
		}

		bool is_null() const {
			switch (index()) {
			case 7:
				return true;
			default:
				return false;
			}
		}

		variant& to_variant() {
			return *static_cast<variant*>(this);
		}

		const variant& to_variant() const {
			return *static_cast<const variant*>(this);
		}

	private:
		template<typename _Tp, typename... _Types>
		inline static Result<_Tp> get_opt_ref(std::variant<_Types...>& __ptr) noexcept {
			static constexpr size_t _Np = std::__detail::__variant::__index_of_v<_Tp, _Types...>;
			if (__ptr.index() != _Np) {
				return std::nullopt;
			}
			return std::__detail::__variant::__get<_Np>(__ptr);
		}


		template<typename _Tp, typename... _Types>
		inline static std::optional<_Tp> get_opt(std::variant<_Types...>&& __ptr) noexcept {
			static constexpr size_t _Np = std::__detail::__variant::__index_of_v<_Tp, _Types...>;
			if (__ptr.index() != _Np) {
				return std::nullopt;
			}
			return std::move(std::__detail::__variant::__get<_Np>(__ptr));
		}
	};

	struct Parser {
		enum class TokenType {
			Eof,

			Ident,
			String,
			Integer,
			Float,
			Bool,
			Null,

			ObjectStart,
			ObjectEnd,

			ArrayStart,
			ArrayEnd,

			Colon,
			Comma
		};

		struct Token {
			TokenType type;
			std::string_view string;
			bool boolean;

			bool is(TokenType tp) {
				return type == tp;
			}
		};

		explicit Parser(std::span<const char> bytes) : data{bytes.data()}, last{bytes.data() + bytes.size()} {
			next();
		}

	public:
		std::optional<Value> parse() {
			switch (token.type) {
			case TokenType::Eof:
				return std::nullopt;
			case TokenType::String:
				return readString();
			case TokenType::Integer:
				return readInteger();
			case TokenType::Float:
				return readFloat();
			case TokenType::Bool:
				return readBool();
			case TokenType::Null:
				next();
				return Null{};
			case TokenType::ObjectStart:
				return readObject();
			case TokenType::ArrayStart:
				return readArray();
			default:
				return std::nullopt;
			}
		}

	private:
		Object readObject() {
			next();

			std::map<std::string, Value> ret;
			while (token.type != TokenType::ObjectEnd) {
				expect(TokenType::String);

				auto name = token.string;

				next();
				consume(TokenType::Colon);

				ret.emplace(name, parse().value());

				if (!skip(TokenType::Comma))
					break;
			}
			consume(TokenType::ObjectEnd);
			return std::move(ret);
		}

		std::vector<Value> readArray() {
			next();

			std::vector<Value> ret;
			while (!token.is(TokenType::ArrayEnd)) {
				ret.push_back(parse().value());
				if (!skip(TokenType::Comma))
					break;
			}
			consume(TokenType::ArrayEnd);
			return std::move(ret);
		}

		float readFloat() {
			auto ret = std::stof(std::string(token.string));
			next();
			return ret;
		}

		bool readBool() {
			auto ret = token.boolean;
			next();
			return ret;
		}

		int readInteger() {
			auto ret = std::stoi(std::string(token.string));
			next();
			return ret;
		}

		std::string readString() {
			auto ret = token.string;
			next();
			return std::string(ret);
		}

	private:
		bool is(char c) const {
			return *data == c;
		}

		void getc() {
			data++;
		}

		bool eof() {
			return data >= last;
		}

		void next() {
			token.type = TokenType::Eof;

			while (!eof()) {
				switch (*data) {
				case '\n':
					line++;
					getc();
					continue;
				case '/':
					getc();
					if (!is('/')) {
						break;
					}
					getc();
					while (!eof() && !is('\n')) {
						getc();
					}
					continue;
				case ' ':  case '\t': case '\r': getc(); continue;
				case '"': {
					getc();
					auto s = data;
					while (!eof() && !is('"')) {
						getc();
					}
					getc();

					token.type = TokenType::String;
					token.string = std::string_view(s, data - s - 1);
					break;
				}
				case 'A'...'Z':
				case 'a'...'z': {
					auto s = data;
					getc();
					while (!eof() && isalnum(*data)) {
						getc();
					}
					std::string_view string(s, data - s);
					if (string == "true") {
						token.type = TokenType::Bool;
						token.boolean = true;
					} else if (string == "false") {
						token.type = TokenType::Bool;
						token.boolean = false;
					} else if (string == "null") {
						token.type = TokenType::Null;
					} else {
						token.type = TokenType::Ident;
					}
					break;
				}
				case '+': case '-':
				case '0'...'9': {
					bool flag = false;
					auto s = data;
					getc();

					while (!eof()) {
						if (isdigit(*data)) {
							getc();
						} else if (!flag && is('.')) {
							flag = true;
							getc();
						} else {
							break;
						}
					}

					token.type = flag ? TokenType::Float : TokenType::Integer;
					token.string = std::string_view(s, data - s);
					break;
				}
				case '{':
					token.type = TokenType::ObjectStart;
					getc();
					break;
				case '}':
					token.type = TokenType::ObjectEnd;
					getc();
					break;
				case '[':
					token.type = TokenType::ArrayStart;
					getc();
					break;
				case ']':
					token.type = TokenType::ArrayEnd;
					getc();
					break;
				case ':':
					token.type = TokenType::Colon;
					getc();
					break;
				case ',':
					token.type = TokenType::Comma;
					getc();
					break;
				}
				break;
			}
		}

		void consume(TokenType tt) {
			expect(tt);
			next();
		}

		void expect(TokenType tt) {
			if (token.type != tt) {
//				std::cout << "unexpected token: " << line << std::endl;
				error();
			}
		}

		bool skip(TokenType tt) {
			if (token.type == tt) {
				next();
				return true;
			}
			return false;
		}

		void error() {
			abort();
		}
	private:
		const char *data;
		const char *last;
		Token token;
		int line = 0;
	};
}