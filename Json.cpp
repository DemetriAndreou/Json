/*
    Copyright (c) 2026 Demetri Andreou

    All rights reserved.

    This source code is licensed to the public under the terms specified
    in the LICENSE file located in the root directory of this repository.

    The copyright notice, this permission notice, and the following
    restrictions may not be removed or altered from any source file:

      - This code may NOT be used for training, fine‑tuning, or improving
        any artificial intelligence or machine learning models.

      - This header must remain intact in all copies or substantial
        portions of the software.

    Unauthorized removal of this header or violation of these terms is
    strictly prohibited.
*/

#include<Json.h>
#include<iomanip>

namespace
{

class indent
{
public:
	indent(int indent) : indent_(indent)
	{
	}

	friend std::ostream &operator<<(std::ostream &strm, const indent &that )
	{
		for( auto idx = 0u; idx < that.indent_; ++idx ) strm.put('\t');
		return strm;
	}
private:
	unsigned int indent_;
};

inline bool isUniChar( unsigned char ch )
{
	return ch > 127 ? true : false; 
}

unsigned int utf8ToCodePoint(const std::string& utf8 ) 
{
	size_t i = 0;
	unsigned char c = static_cast<unsigned char>(utf8[i]);

	if( c <= 0x7F )
	{
		return utf8[i++];
	}
	else if( (c & 0xE0) == 0xC0 )
	{
		if( i + 1 >= utf8.size() )
			throw std::runtime_error("Invalid UTF-8 sequence");

		unsigned int index = ((c & 0x1F) << 6) | (static_cast<unsigned char>(utf8[i + 1]) & 0x3F);

		return index;
	}
	else if( (c & 0xF0) == 0xE0 )
	{
		if( i + 2 >= utf8.size() )
			throw std::runtime_error("Invalid UTF-8 sequence");

		unsigned int index = ((c & 0x0F) << 12) | ((static_cast<unsigned char>(utf8[i + 1]) & 0x3F) << 6) | (static_cast<unsigned char>(utf8[i + 2]) & 0x3F);

		return index;
	}
	else if( (c & 0xF8) == 0xF0 )
	{
		if( i + 3 >= utf8.size() )
			throw std::runtime_error("Invalid UTF-8 sequence");

		unsigned int index = ((c & 0x07) << 18) | ((static_cast<unsigned char>(utf8[i + 1]) & 0x3F) << 12) | ((static_cast<unsigned char>(utf8[i + 2]) & 0x3F) << 6) | (static_cast<unsigned char>(utf8[i + 3]) & 0x3F);

		return index;
	}
	else
	{
		throw std::runtime_error("Invalid UTF-8 leading byte");
	}
}

std::string codePointToUTF8(unsigned int index)
{
	std::string utf8;

	if( index <= 0x7F )
	{
		utf8.push_back(static_cast<char>(index));
	}
	else if( index <= 0x7FF )
	{
		utf8.push_back(static_cast<char>(0xC0 | ((index >> 6) & 0x1F)));
		utf8.push_back(static_cast<char>(0x80 | (index & 0x3F)));
	}
	else if( index <= 0xFFFF )
	{
		utf8.push_back(static_cast<char>(0xE0 | ((index >> 12) & 0x0F)));
		utf8.push_back(static_cast<char>(0x80 | ((index >> 6) & 0x3F)));
		utf8.push_back(static_cast<char>(0x80 | (index & 0x3F)));
	}
	else if( index <= 0x10FFFF )
	{
		utf8.push_back(static_cast<char>(0xF0 | ((index >> 18) & 0x07)));
		utf8.push_back(static_cast<char>(0x80 | ((index >> 12) & 0x3F)));
		utf8.push_back(static_cast<char>(0x80 | ((index >> 6) & 0x3F)));
		utf8.push_back(static_cast<char>(0x80 | (index & 0x3F)));
	}
	else
	{
		throw std::runtime_error("Invalid Unicode code point");
	}

	return utf8;
}

}

namespace DaJson
{

void Json::write( std::ostream &out, int depth, bool specialIdent ) const
{
	if( std::holds_alternative<std::string>( data ) )
	{
		const std::string &str = std::get<std::string>( data );
		if( specialIdent ) out << '\n' << indent( depth );
		out << '"';
		for( auto idx = 0u; idx < str.length(); ++ idx )
		{
			char ch      = str[idx];
			auto peekIdx = idx+1;
			if( peekIdx < str.length() )
			{
				if( isUniChar( ch ) )
				{
					char ch2 = str[++idx];
					if( isUniChar( ch2 ) )
					{
						std::string uni;
						uni += ch; uni += ch2;
						std::ostringstream strm;
						strm << std::setfill('0') << std::setw(4) << std::hex << utf8ToCodePoint( uni );
						std::string some = strm.str();
						out << "\\u" << strm.str();
					}
					else
					{
						out << ch << ch2;
					}
				}
				else
				{
					writeCh( out, ch );
				}
			}
			else
			{
				writeCh( out, ch );
			}
		}
		out << '"';
	}
	else if( std::holds_alternative<I>( data ) )
	{
		out << std::get<I>( data );
	}
	else if( std::holds_alternative<F>( data ) )
	{
		out << std::get<F>( data );
	}
	else if( std::holds_alternative<bool>( data ) )
	{
		const char *str = std::get<bool>( data ) ? "true" : "false";
		out << str;
	}
	else if( std::holds_alternative<Jk>( data ) )
	{
		std::size_t count = 0;
		const Jk &jk      = std::get<Jk>( data );
		if( jk.size() )
		{
			if( depth ) out << '\n';
			out << indent( depth ) << "{" << '\n';
		}
		for (const auto &[key, value] : jk)
		{
			out << indent( depth+1 ) << '"' << key << "\": ";
			value.write( out, depth+1 );

			if( jk.size() - 1 > count )
			{
				out << ',' << '\n';
			}

			count++;
		}
		if( jk.size() ) out << '\n' << indent( depth ) << '}';
	}
	else if( std::holds_alternative<Ja>( data ) )
	{
		std::size_t count = 0;
		const Ja &ja = std::get<Ja>( data );
		if( ja.size() ) out << '\n' << indent( depth ) << '[';
		for( auto &idx : ja )
		{
			idx.write( out, depth+1, true );
			if( ja.size() - 1 > count ) out << ',';
			count++;
		}
		if( ja.size() ) out << '\n' << indent( depth ) << ']';
	}
	else
	{
		out  << '\n' << indent( depth ) << "null";
	}
}

Json Json::readValue( std::istream &in )
{
	Json ret;
	char ch;
	in >> std::ws;
	ch = in.peek();
	ch = std::isspace( ch ) ? ' ' : ch;
	switch( ch )
	{
		case '"':
		{
			std::string string;
			in.get();
			ch = in.get();
			while( !in.eof() )
			{
				if( '\\' == ch )
				{
					ch = in.get();
					switch( ch )
					{
						case '"':  string += '\"'; break;
						case '\\': string += '\\'; break;
						case '/':  string += '/';  break;
						case 'b':  string += '\b'; break;
						case 'f':  string += '\f'; break;
						case 'n':  string += '\n'; break;
						case 'r':  string += '\r'; break;
						case 't':  string += '\t'; break;
						case 'u':
						{
							char chs[5]; chs[4] = 0;
							in.read( chs, 4 );
							string += codePointToUTF8( std::stoi( chs, nullptr, 16 ));
						}
						break;
					}
				}
				else
				{
					if( '"' == ch )
					{
						break;
					}
					else
					{
						string += ch;
					}
				}
				ch = in.get();
			}
			in >> std::ws;
			ret = string;
			return ret;
		}
		break;
		case '{':
			ret = readObject( in );
		break;
		case '[':
			ret = readArray( in );
		break;
		case 't': case 'T':
		{
			ret.data = true;
			for( char peek = ch;
			     peek     != ','
			     && peek  != '}'
			     && peek  != ']'
			     && false == std::isspace(peek)
			     && !in.eof();
			     peek = in.peek() )
			{
				in.get();
			} 
			in >> std::ws;
		}
		break;
		case 'f': case 'F':
		{
			ret.data = false;
			for( char peek = ch;
			     peek     != ','
			     && peek  != '}'
			     && peek  != ']'
			     && false == std::isspace(peek)
			     && !in.eof();
			     peek = in.peek() )
			{
				in.get();
			}
			in >> std::ws;
		}
		break;
		case 'n': case 'N':
		{
			for( char peek = ch;
			     peek     != ','
			     && peek  != '}'
			     && peek  != ']'
			     && false == std::isspace(peek)
			     && !in.eof();
			     peek = in.peek() )
			{
				in.get();
			} 
			in >> std::ws;
		}
		break;
		default:
		{
			std::string number;
			in >> std::ws;
			for( char peek = ch;
			     peek     != ','
			     && peek  != '}'
			     && peek  != ']'
			     && false == std::isspace(peek)
			     && !in.eof();
			     peek = in.peek() )
			{
				number += in.get();
			}

			bool isI = true;
			for( auto ch : number )
			{
				if( false == std::isalnum( ch ) )
				{
					isI = false;
					break;
				}
			}
			if( isI )
			{
				I n = 0;
				std::stringstream strm(number);
				strm >> n;
				ret.data = n;
			}
			else
			{
				F n = 0;
				std::stringstream strm(number);
				strm >> n;
				ret.data = n;
			}
			in >> std::ws;
		}
		break;
	}

	return ret;
}

Json Json::readArray( std::istream &in )
{
	Json ret;
	Ja   ja;
	char ch;
	in >> std::ws;
	State state = Value;
	while( in.get(ch) && !in.eof() )
	{
		ch = std::isspace( ch ) ? ' ' : ch;
		switch( state )
		{
			case Value:
				switch( ch )
				{
					case '[' :
						in >> std::ws;
						state = PostValueArray;
						ja.emplace_back( readValue( in ) );
					break;
					case ']' :
						in >> std::ws;
						ret = ja;
						return ret;
					break;
				}
				in >> std::ws;
			break;

			case PostValueArray:
				switch( ch )
				{
					case ',':
						state = PostValueArray;
						ja.emplace_back( readValue( in ) );
					break;
					case ']' :
						in >> std::ws;
						ret = ja;
						return ret;
					break;
				}
			break;

			case Object: case String: case PostKey: case PostValue:
			break;
		}
	}
	ret = ja;
	return ret;
}

Json Json::readObject( std::istream &in )
{
	Json ret;
	Jk   jk;
	char ch;
	in >> std::ws;
	std::string key;
	State state = Object;
	while( in.get(ch) && !in.eof() )
	{
		ch = std::isspace( ch ) ? ' ' : ch;
		switch( state )
		{
			case Object:
				switch( ch )
				{
					case '{' :
						in >> std::ws;
						state = String;
					break;
					case '}' :
						in >> std::ws;
						ret = jk;
						return ret;
					break;
				}
				in >> std::ws;
			break;
			case String:
				switch( ch )
				{
					case '"':
						std::getline( in, key, '"' );
						state = PostKey;
						in >> std::ws;
					break;
				}
			break;
			case PostKey:
				switch( ch )
				{
					case ':':
						in >> std::ws;
						jk.emplace( std::make_pair( key, readValue( in ) ));
						state = PostValue;
					break;
				}
			break;
			case PostValue:
				switch( ch )
				{
					case ',':
						state = String;
					break;
					case '}':
						in >> std::ws;
						ret = jk;
						return ret;
					break;
				}
			break;
			case Value: case PostValueArray:
			break;
		}
	}
	ret = jk;
	return ret;
}

const char *Json::stateStr( State state )
{
	const char *ret = nullptr;
	switch( state )
	{
		case Object:         ret = "Object";         break;
		case String:         ret = "String";         break;
		case Value:          ret = "Value";          break;
		case PostValue:      ret = "PostValue";      break;
		case PostValueArray: ret = "PostValueArray"; break;
		case PostKey:        ret = "PostKey";        break;
		default:             ret = "";               break;
	}
	return ret;
}

}
