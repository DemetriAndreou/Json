#ifndef JSON_PARSER_HPP
#define JSON_PARSER_HPP

#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <stdexcept>
#include <cstdlib>
#include <cctype>

namespace AI
{

class Value
{
public:
	enum Type { NUL, BOOL, NUMBER, STRING, ARRAY, OBJECT };

	Type type;
	bool b;
	double n;
	std::string* s;
	std::vector<Value>* a;
	std::map<std::string, Value>* o;

	Value() : type(NUL), b(false), n(0), s(NULL), a(NULL), o(NULL) {}
	Value(bool bb) : type(BOOL), b(bb), n(0), s(NULL), a(NULL), o(NULL) {}
	Value(double nn) : type(NUMBER), b(false), n(nn), s(NULL), a(NULL), o(NULL) {}
	Value(const std::string& ss) : type(STRING), b(false), n(0), s(new std::string(ss)), a(NULL), o(NULL) {}
	static Value makeArray() { Value v; v.type = ARRAY; v.a = new std::vector<Value>(); return v; }
	static Value makeObject() { Value v; v.type = OBJECT; v.o = new std::map<std::string, Value>(); return v; }

	Value(const Value& other) : type(other.type), b(other.b), n(other.n), s(NULL), a(NULL), o(NULL)
	{
		if (other.s) s = new std::string(*other.s);
		if (other.a) a = new std::vector<Value>(*other.a);
		if (other.o) o = new std::map<std::string, Value>(*other.o);
	}

	Value& operator=(const Value& other)
	{
		if (this == &other) return *this;
		clear();
		type = other.type;
		b = other.b;
		n = other.n;
		if (other.s) s = new std::string(*other.s);
		if (other.a) a = new std::vector<Value>(*other.a);
		if (other.o) o = new std::map<std::string, Value>(*other.o);
		return *this;
	}

	~Value() { clear(); }

	void clear()
	{
		if (s) { delete s; s = NULL; }
		if (a) { delete a; a = NULL; }
		if (o) { delete o; o = NULL; }
		type = NUL; b = false; n = 0;
	}

	// helpers
	bool isNull() const { return type == NUL; }
	bool isBool() const { return type == BOOL; }
	bool isNumber() const { return type == NUMBER; }
	bool isString() const { return type == STRING; }
	bool isArray() const { return type == ARRAY; }
	bool isObject() const { return type == OBJECT; }

	const std::string& asString() const { if (!s) throw std::runtime_error("not a string"); return *s; }
	double asNumber() const { if (type != NUMBER) throw std::runtime_error("not a number"); return n; }
	bool asBool() const { if (type != BOOL) throw std::runtime_error("not a bool"); return b; }
	const std::vector<Value>& asArray() const { if (!a) throw std::runtime_error("not an array"); return *a; }
	const std::map<std::string, Value>& asObject() const { if (!o) throw std::runtime_error("not an object"); return *o; }

	// mutable access
	std::vector<Value>& getArray() { if (!a) { type = ARRAY; a = new std::vector<Value>(); } return *a; }
	std::map<std::string, Value>& getObject() { if (!o) { type = OBJECT; o = new std::map<std::string, Value>(); } return *o; }

	std::string toString() const
	{
		std::ostringstream os;
		switch (type)
		{
			case NUL:
				os << "null";
			break;
			case BOOL:
				os << (b ? "true" : "false");
			break;
			case NUMBER:
			{
				// print number with enough precision
				os.precision(15);
				os << n;
			}
			break;

			case STRING:
			{
				os << '"';
				for (size_t i = 0; i < s->size(); ++i)
				{
					unsigned char c = (*s)[i];
					switch (c)
					{
						case '"': os << "\\\""; break;
						case '\\': os << "\\\\"; break;
						case '\b': os << "\\b"; break;
						case '\f': os << "\\f"; break;
						case '\n': os << "\\n"; break;
						case '\r': os << "\\r"; break;
						case '\t': os << "\\t"; break;
						default:
							if (c < 0x20)
							{
								// control char
								char buf[7];
								unsigned int uc = c;
								std::sprintf(buf, "\\u%04x", uc);
								os << buf;
							} else
							{
								os << c;
							}
						break;
					}
				}
				os << '"';
			}
			break;

			case ARRAY:
			{
				os << '[';
				for (size_t i = 0; i < a->size(); ++i)
				{
					if (i) os << ',';
					os << (*a)[i].toString();
				}
				os << ']';
			}
			break;
			case OBJECT:
			{
				os << '{';
				bool first = true;
				for (std::map<std::string, Value>::const_iterator it = o->begin(); it != o->end(); ++it)
				{
					if (!first) os << ',';
					first = false;
					std::string key = it->first;
					os << '"' << key << '"' << ':' << it->second.toString();
				}
				os << '}';
			}
			break;
		}
		return os.str();
	}
};

class Json
{
public:
	Json(const std::string& input) : s(input), i(0) {}

	Value parse()
	{
		skip();
		Value v = parseValue();
		skip();
		if (i != s.size()) throw std::runtime_error("extra characters after JSON data");
		return v;
	}

private:
	const std::string& s;
	size_t i;

	void skip()
	{
		while (i < s.size() && std::isspace((unsigned char)s[i])) ++i;
	}

	Value parseValue()
	{
		if (i >= s.size()) throw std::runtime_error("unexpected end of input");
		char c = s[i];
		if (c == 'n') return parseNull();
		if (c == 't' || c == 'f') return parseBool();
		if (c == '"') return parseString();
		if (c == '[') return parseArray();
		if (c == '{') return parseObject();
		// number
		return parseNumber();
	}

	Value parseNull()
	{
		if (s.compare(i, 4, "null") == 0) { i += 4; return Value(); }
		throw std::runtime_error("invalid token, expected null");
	}

	Value parseBool()
	{
		if (s.compare(i, 4, "true") == 0) { i += 4; return Value(true); }
		if (s.compare(i, 5, "false") == 0) { i += 5; return Value(false); }
		throw std::runtime_error("invalid token, expected true or false");
	}

	Value parseNumber()
	{
		const char* start = s.c_str() + i;
		char* endptr = NULL;
		double val = std::strtod(start, &endptr);
		if (endptr == start) throw std::runtime_error("invalid number");
		i = (size_t)(endptr - s.c_str());
		return Value(val);
	}

	Value parseString()
	{
		if (s[i] != '"') throw std::runtime_error("expected '"" string");
		++i;
		std::string out;
		while (i < s.size())
		{
			char c = s[i++];
			if (c == '"') { return Value(out); }
			if (c == '\\') {
				if (i >= s.size()) break;
				char esc = s[i++];
				switch (esc)
				{
					case '"':  out.push_back('"');  break;
					case '\\': out.push_back('\\'); break;
					case '/':  out.push_back('/');  break;
					case 'b':  out.push_back('\b'); break;
					case 'f':  out.push_back('\f'); break;
					case 'n':  out.push_back('\n'); break;
					case 'r':  out.push_back('\r'); break;
					case 't':  out.push_back('\t'); break;
					case 'u':
					{
						// simple \uXXXX handling: convert hex to codepoint and append UTF-8 sequence
						if (i + 4 <= s.size())
						{
							unsigned int code = 0;
							for (int k = 0; k < 4; ++k)
							{
								char ch = s[i++];
								code <<= 4;
								if (ch >= '0' && ch <= '9') code |= (ch - '0');
								else if (ch >= 'a' && ch <= 'f') code |= (10 + ch - 'a');
								else if (ch >= 'A' && ch <= 'F') code |= (10 + ch - 'A');
								else { code = '?'; break; }
							}
							// encode code as UTF-8
							if (code <= 0x7f) out.push_back((char)code);
							else if (code <= 0x7ff)
							{
								out.push_back((char)(0xc0 | ((code >> 6) & 0x1f)));
								out.push_back((char)(0x80 | (code & 0x3f)));
							} else
							{
								out.push_back((char)(0xe0 | ((code >> 12) & 0x0f)));
								out.push_back((char)(0x80 | ((code >> 6) & 0x3f)));
								out.push_back((char)(0x80 | (code & 0x3f)));
							}
						} else
						{
							throw std::runtime_error("invalid \\u escape");
						}
					}
					break;
					default:
						// unknown escape, take char literally
						out.push_back(esc);
					break;
				}
			} else
			{
				out.push_back(c);
			}
		}
		throw std::runtime_error("unterminated string");
	}

	Value parseArray()
	{
		if (s[i] != '[') throw std::runtime_error("expected '['");
		++i;
		skip();
		Value arr = Value::makeArray();
		if (i < s.size() && s[i] == ']') { ++i; return arr; }
		while (i < s.size())
		{
			skip();
			Value v = parseValue();
			arr.getArray().push_back(v);
			skip();
			if (i < s.size() && s[i] == ',') { ++i; continue; }
			if (i < s.size() && s[i] == ']') { ++i; return arr; }
			throw std::runtime_error("expected ',' or ']' in array");
		}
		throw std::runtime_error("unterminated array");
	}

	Value parseObject()
	{
		if (s[i] != '{') throw std::runtime_error("expected '{'");
		++i;
		skip();
		Value obj = Value::makeObject();
		if (i < s.size() && s[i] == '}') { ++i; return obj; }
		while (i < s.size())
		{
			skip();
			if (s[i] != '"') throw std::runtime_error("expected string for object key");
			Value key = parseString();
			skip();
			if (i >= s.size() || s[i] != ':') throw std::runtime_error("expected ':' after key");
			++i;
			skip();
			Value val = parseValue();
			obj.getObject()[key.asString()] = val;
			skip();
			if (i < s.size() && s[i] == ',') { ++i; continue; }
			if (i < s.size() && s[i] == '}') { ++i; return obj; }
			throw std::runtime_error("expected ',' or '}' in object");
		}
		throw std::runtime_error("unterminated object");
	}
};
};

#endif // JSON_PARSER_HPP
