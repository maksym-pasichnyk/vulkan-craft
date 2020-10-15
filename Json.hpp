#pragma once

#include <map>
#include <string_view>
#include <vector>
#include <variant>

namespace json {
	struct Value;
	using Span = std::span<Value>;
	using Array = std::vector<Value>;
	using Object = std::map<std::string, Value>;
	using Null = struct {};

	struct Value : private std::variant<int64_t, uint64_t, double, std::string, bool, Array, Object, Null> {
		using variant::variant;

		int64_t i64() {
			switch (index()) {
			case 0:
				return std::get<int64_t>(*this);
			case 1:
				return (int64_t) std::get<uint64_t>(*this);
			default:
				return (int64_t) std::get<double>(*this);
			}
		}

		double f64() {
			switch (index()) {
			case 0:
				return (double) std::get<int64_t>(*this);
			case 1:
				return (double) std::get<uint64_t>(*this);
			default:
				return std::get<double>(*this);
			}
		}

		std::string& string() {
			return std::get<std::string>(*this);
		}

		std::string_view string_view() {
			return std::get<std::string>(*this);
		}

		bool as_bool() {
			return std::get<bool>(*this);
		}

		Array& array() {
			return std::get<Array>(*this);
		}

		Span array_view() {
			return std::get<Array>(*this);
		}

		Object& object() {
			return std::get<Object>(*this);
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
			switch (_token.type) {
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
			while (_token.type != TokenType::ObjectEnd) {
				expect(TokenType::String);

				auto name = _token.string;

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
			while (!_token.is(TokenType::ArrayEnd)) {
				ret.push_back(parse().value());
				if (!skip(TokenType::Comma))
					break;
			}
			consume(TokenType::ArrayEnd);
			return std::move(ret);
		}

		float readFloat() {
			auto ret = std::stof(std::string(_token.string));
			next();
			return ret;
		}

		bool readBool() {
			auto ret = _token.boolean;
			next();
			return ret;
		}

		int readInteger() {
			auto ret = std::stoi(std::string(_token.string));
			next();
			return ret;
		}

		std::string readString() {
			auto ret = _token.string;
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
			_token.type = TokenType::Eof;

			while (!eof()) {
				switch (*data) {
				case '\n':
					line++;
					getc();
					continue;
				case ' ':  case '\t': case '\r': getc(); continue;
				case '"': {
					getc();
					auto s = data;
					while (!eof() && !is('"')) {
						getc();
					}
					getc();

					_token.type = TokenType::String;
					_token.string = std::string_view(s, data - s - 1);
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
						_token.type = TokenType::Bool;
						_token.boolean = true;
					} else if (string == "false") {
						_token.type = TokenType::Bool;
						_token.boolean = false;
					} else if (string == "null") {
						_token.type = TokenType::Null;
					} else {
						_token.type = TokenType::Ident;
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

					_token.type = flag ? TokenType::Float : TokenType::Integer;
					_token.string = std::string_view(s, data - s);
					break;
				}
				case '{':
					_token.type = TokenType::ObjectStart;
					getc();
					break;
				case '}':
					_token.type = TokenType::ObjectEnd;
					getc();
					break;
				case '[':
					_token.type = TokenType::ArrayStart;
					getc();
					break;
				case ']':
					_token.type = TokenType::ArrayEnd;
					getc();
					break;
				case ':':
					_token.type = TokenType::Colon;
					getc();
					break;
				case ',':
					_token.type = TokenType::Comma;
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
			if (_token.type != tt) {
//				std::cout << "unexpected token: " << line << std::endl;
				error();
			}
		}

		bool skip(TokenType tt) {
			if (_token.type == tt) {
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
		Token _token;
		int line = 0;
	};
}