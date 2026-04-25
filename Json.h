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

#ifndef Json_h
#define Json_h

#include<iostream>
#include<sstream>
#include<algorithm>
#include<iterator>
#include<vector>
#include<map>
#include<variant>

namespace DaJson
{

class Json
{
public:
	using I     = long;
	using F     = double;
	using Ja    = std::vector<Json>;
	using Jk    = std::map<std::string,Json>;
	using DataT = std::variant<std::monostate, F, I, bool, std::string, Ja, Jk>;

	Ja::reference        operator[]( Ja::size_type      pos  )       { return std::get<Json::Ja>( data )[pos]; }
	Ja::const_reference  operator[]( Ja::size_type      pos  ) const { return std::get<Json::Ja>( data )[pos]; }
	Jk::mapped_type     &operator[]( const Jk::key_type &key )       { return std::get<Json::Jk>( data )[key]; }

	Json()                               = default;
	Json( const Json &that )             = default;
	Json &operator=( const Json  &that ) = default;
	Json &operator=( Json       &&that ) = default;
	Json( Json &&that )                  = default;
	~Json()                              = default;

	Json( const Json::DataT &in )
		:data( in )
	{
	}

	Json( Json::DataT &&in )
		:data( std::move(in) )
	{
	}

	Json &operator=( const Json::DataT &in )
	{
		if( &data != &in )
		{
			data = in;
		}
		return *this;
	}

	Json &operator=( Json::DataT &&in )
	{
		if( &data != &in )
		{
			data = std::move(in);
		}
		return *this;
	}

	operator Json::DataT() const &    { return data;            }
	operator Json::DataT() &&         { return std::move(data); }

	friend std::ostream &operator<<( std::ostream &out, const Json &that )
	{
		that.write( out, 0 );
		out << '\n';
		return out;
	}

	friend std::istream &operator>>( std::istream &in, Json &that )
	{
		that = that.readObject( in );
		return in;
	}

	void setNull()      { reset(); }
	bool isNull() const { return std::holds_alternative<std::monostate>(data);      }

private:
	void   reset()      { data = std::monostate{};                                  }
	enum   State        { Object, String, Value, PostKey, PostValue, PostValueArray };
	void   writeCh( std::ostream &out, char ch ) const
	{
		switch( ch )
		{
			case '\"': out << '\\' << '"';  break;
			case '\\': out << '\\' << '\\'; break;
			case '/' : out         << '/';  break;
			case '\b': out << '\\' << 'b';  break;
			case '\f': out << '\\' << 'f';  break;
			case '\n': out << '\\' << 'n';  break;
			case '\r': out << '\\' << 'r';  break;
			case '\t': out << '\\' << 't';  break;
			default:
				out << ch;
			break;
		}
	}
	static State changeState( State, State to )
	{
		return to;
	}

	static const char *stateStr( State state );
	static Json    readObject(   std::istream &in );
	static Json    readArray(    std::istream &in );
	static Json    readValue(    std::istream &in );
	void           write(        std::ostream &out, int depth, bool specialIndent = false ) const;

	DataT data;
};
}

#endif /* Json_h */
