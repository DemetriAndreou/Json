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
#include<cstdlib>
#include<charconv>

namespace
{

void indent( auto &out, unsigned int depth )
{
	for( auto idx = 0u; idx < depth; ++idx )
	{
		*out++ = '\t';
	}
}

inline void checkEnd( auto &idx, auto &end )
{
	if( idx == end ) throw std::runtime_error("unexpected end");
}

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

void ws( auto &it, auto &end)
{
	while( it != end && std::isspace(static_cast<unsigned char>(*it)) )
	{
		//char ch = *it;
		++it;
	}
}

const std::string readTillWs( auto &it, auto &end)
{
	std::string ret;

	while( it != end
	       && false == std::isspace(static_cast<unsigned char>(*it))
	       && *it != '}'
	       && *it != ']'
	       && *it != ',' )
	{
		char ch = *it;
		ret += ch;
		++it;
	}
	return ret;
}

DaJson::Json readArray( auto &idx, auto &end );
DaJson::Json readValue( auto &idx, auto &end );

std::string readString( auto &idx, auto &end )
{
	std::string ret;

	while( idx != end && '"' != *idx )
	{
		auto ch = *idx++;
		ret += ch;
	}
	if( *idx == '"' )
	{
		idx++;
	}

	return ret;
}

DaJson::Json readObject( auto &idx, auto &end )
{
	DaJson::Json ret;
	DaJson::Json::Jk jk;

	ws( idx, end );
	while( idx != end )
	{
		ws( idx, end );
		if( idx == end ) { ret = jk; return ret; }
		char ch = *idx;
		switch( ch )
		{
			case ',':
				++idx;
				ws( idx, end );
			break;
			case '}':
				++idx;
				ret = jk;
				ws( idx, end );
				return ret;
			case '"':
			{
				++idx;
				std::string key = readString( idx, end );
				ws( idx, end );
				ch = *idx;
				if( ':' == ch )
				{
					++idx;
					ws( idx, end );
					jk.emplace( std::make_pair( key, readValue( idx, end ) ));
					ws( idx, end );
				}
				else
				{
					throw std::runtime_error("expected :");
				}
			}
			break;

			default:
				throw std::runtime_error("unexpected character");
			break;
		}
	}

	ret = jk;
	return ret;
}

DaJson::Json readValue( auto &idx, auto &end )
{
	DaJson::Json ret;

	ws( idx, end );
	char ch = *idx;
	switch( ch )
	{
		case '"':
		{
			++idx;
			ch = *idx;
			std::string string;
			while( idx != end )
			{
				if( '\\' == ch )
				{
					++idx;
					checkEnd( idx, end );
					ch = *idx;
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

							++idx; checkEnd( idx, end ); chs[0] = *idx;
							++idx; checkEnd( idx, end ); chs[1] = *idx;
							++idx; checkEnd( idx, end ); chs[2] = *idx;
							++idx; checkEnd( idx, end ); chs[3] = *idx;

							string += codePointToUTF8( std::stoi( chs, nullptr, 16 ));
						}
						break;
					}
				}
				else
				{
					if( '"' == ch )
					{
						++idx;
						checkEnd( idx, end );
						break;
					}
					else
					{
						string += ch;
					}
				}
				++idx;
				checkEnd( idx, end );
				ch = *idx;
			}
			ret = string;
			ws( idx, end );
			return ret;
		}
		break;

		break;

		case '{':
		{
			++idx;
			ret = readObject( idx, end );
		}
		break;

		case '[':
		{
			++idx;
			ret = readArray( idx, end );
		}
		break;

		case 't': case 'T':
		{
			ret = true;
			readTillWs( idx, end );
		}
		break;

		case 'f': case 'F':
		{
			ret = false;
			readTillWs( idx, end );
		}
		break;

		case 'n': case 'N':
		{
			readTillWs( idx, end );
			ret.setNull();
		}
		break;

		default:
		{
			ws( idx, end );
			double d;
			long   l;
			char   *p_l_end{}, *p_d_end{};
			const std::string num = readTillWs( idx, end );

			{
				d = std::strtod( num.c_str(), &p_d_end );
				if( num.c_str() == p_d_end )
				{
					throw std::runtime_error("not a number");
				}
			}

			{
				l = std::strtol( num.c_str(), &p_l_end, 10 );
				if( num.c_str() == p_l_end )
				{
					throw std::runtime_error("not a number");
				}
			}
	
			if( p_l_end < p_d_end )
			{
				ret = static_cast<DaJson::Json::F>(d);
			}
			else
			{
				ret = static_cast<DaJson::Json::I>(l);
			}
		}
		break;
	}

	ws( idx, end );
	return ret;
}

DaJson::Json readArray( auto &idx, auto &end )
{
	DaJson::Json ret;
	DaJson::Json::Ja ja;

	while( idx != end )
	{
		ws( idx, end );
		checkEnd( idx, end );
		char ch = *idx;
		switch( ch )
		{
			case ',':
				++idx;
			break;

			case ']':
				++idx;
				ws( idx, end );
				return ret = ja;
			break;

			default:
			{
				ws( idx, end );
				ja.emplace_back( readValue( idx, end ) );
			}
			break;
		}
	}

	ws( idx, end );
	ret = ja;
	return ret;
}

DaJson::Json readJson( auto &idx, auto &end )
{
	DaJson::Json ret;

	while( idx != end )
	{
		ws( idx, end );
		if( idx == end ) return ret;
		char ch = *idx;
		switch( ch )
		{
			case '{':
				++idx;
				ret = readObject( idx, end );
				ws( idx, end );
			break;
			case '[':
				++idx;
				ret = readArray( idx, end );
				ws( idx, end );
			break;
			case ',':
				++idx;
				ws( idx, end );
			break;
		}
	}

	return ret;
}


void writeCh( auto &out, char ch )
{
	char ret[2]; ret[0] = ret[1] = 0;
	switch( ch )
	{
		case '\"': *out++ = '\\'; *out++ = '"';  break;
		case '\\': *out++ = '\\'; *out++ = '\\'; break;
		case '/' : *out++ =  '/';                break;
		case '\b': *out++ = '\\'; *out++ = 'b';  break;
		case '\f': *out++ = '\\'; *out++ = 'f';  break;
		case '\n': *out++ = '\\'; *out++ = 'n';  break;
		case '\r': *out++ = '\\'; *out++ = 'r';  break;
		case '\t': *out++ = '\\'; *out++ = 't';  break;
		default:
			*out++ = ch;
		break;
	}
}

void writeIt( auto &out, const DaJson::Json::DataT &data, bool prettyPrint, int depth, bool specialIdent = false )
{
	if( std::holds_alternative<std::string>( data ) )
	{
		const std::string &str = std::get<std::string>( data );
		if( specialIdent ) { if(prettyPrint) { *out++ = '\n'; indent( out, depth );} }
		*out++ = '"';
		for( auto idx = 0u; idx < str.length(); ++idx )
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
						strm << "\\u" << std::setfill('0') << std::setw(4) << std::hex << utf8ToCodePoint( uni );
						std::string some = strm.str();
						std::copy( some.begin(), some.end(), out );
					}
					else
					{
						*out++ = ch;
						*out++ = ch2;
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
		*out++ = '"';
	}
	else if( std::holds_alternative<DaJson::Json::I>( data ) )
	{
		std::string s = std::to_string( std::get<DaJson::Json::I>( data ) );
		std::copy( s.begin(), s.end(), out );
	}
	else if( std::holds_alternative<DaJson::Json::F>( data ) )
	{
		const size_t buf_size = 64;
		char buf[buf_size]{};
		DaJson::Json::F n = std::get<DaJson::Json::F>( data );
		std::to_chars_result result = std::to_chars( buf, buf+buf_size, n, std::chars_format::scientific, 5);
		if (result.ec != std::errc())
		{
			throw std::runtime_error("Number format error");
		}
		std::copy( buf, result.ptr, out );
	}
	else if( std::holds_alternative<bool>( data ) )
	{
		constexpr std::string trueStr{"true"}, falseStr{"false"};
		if( std::get<bool>( data ) )
		{
			std::copy( trueStr.begin(), trueStr.end(), out );
		}
		else
		{
			std::copy( falseStr.begin(), falseStr.end(), out );
		}
	}
	else if( std::holds_alternative<DaJson::Json::Jk>( data ) )
	{
		std::size_t count = 0;
		const DaJson::Json::Jk &jk      = std::get<DaJson::Json::Jk>( data );
		if( jk.size() )
		{
			if(prettyPrint)
			{
				if( depth ) *out++ = '\n';
				indent(out,  depth );
			}
			*out++ = '{';
			if(prettyPrint)
				*out++ = '\n';
		}
		for( const auto &[key, value] : jk )
		{
			if(prettyPrint) indent( out,  depth+1 );
			*out++ = '"';
			std::copy( key.begin(), key.end(), out );
			constexpr std::string tmp{ "\": "};
			std::copy( tmp.begin(), tmp.end(), out );
			writeIt( out, value, prettyPrint, depth+1 );

			if( jk.size() - 1 > count )
			{
				*out++ = ',';
				if(prettyPrint)
					*out++ = '\n';
			}

			count++;
		}
		if( jk.size() ) { if(prettyPrint) {*out++ = '\n'; indent( out, depth );} *out++ = '}'; };
	}
	else if( std::holds_alternative<DaJson::Json::Ja>( data ) )
	{
		std::size_t count = 0;
		const DaJson::Json::Ja &ja = std::get<DaJson::Json::Ja>( data );
		if( ja.size() ) { if(prettyPrint) {*out++ = '\n'; indent( out, depth );} *out++ = '['; };
		for( auto &idx : ja )
		{
			writeIt( out, idx, prettyPrint, depth+1, true );
			if( ja.size() - 1 > count ) *out++ = ',';
			count++;
		}
		if( ja.size() ) { if(prettyPrint) {*out++ = '\n'; indent( out, depth );} *out++ = ']'; };
	}
	else
	{
		if(prettyPrint)
		{
			*out++ = '\n';
			indent( out, depth );
		}
		constexpr std::string tmp{ "null"};
		std::copy( tmp.begin(), tmp.end(), out );
	}
}

}

namespace DaJson
{

void Json::parse( std::istream_iterator<char> begin, std::istream_iterator<char> end )
{
	setNull();
	*this =  readJson( begin, end );
}

void Json::parse( std::string::const_iterator begin, std::string::const_iterator end )
{
	setNull();
	*this =  readJson( begin, end );
}

void Json::write( std::ostream_iterator<char> it ) const
{
	writeIt( it, *this, prettyPrint, 0, false );
}

void Json::write( std::string &out ) const
{
	auto it = std::back_inserter( out );
	writeIt( it, *this, prettyPrint, 0, false );
	if( prettyPrint ) out += '\n';
}

}
