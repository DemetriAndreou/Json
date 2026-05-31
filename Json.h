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

	Json( const Json::DataT  &in ) :data( in )             {}
	Json( Json::DataT       &&in ) :data( std::move(in) )  {}

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

	operator Json::DataT() const &           { return data;                       }
	operator Json::DataT() &&                { return std::move(data);            }

	void setFmt( std::chars_format in )      { fmt         = in;                  }
	std::chars_format getFmt()    const      { return fmt;                        }
	void setPrecision( int in )              { precision   = in;                  }
	int  getPrecision()           const      { return precision;                  }
	void setPrettyPrint( bool in )           { prettyPrint = in;                  } 
	bool getPrettyPrint()         const      { return prettyPrint;                }
	std::string str( std::string::size_type reserve = 0) const
	{
		std::string out;
		if( reserve )
			out.reserve( reserve );
		write(out);
		return out;
	}

	friend std::ostream &operator<<( std::ostream &out, const Json &that )
	{
		std::ostream_iterator<char> it(out);
		that.write( it );
		*it++ = '\n';
		return out;
	}

	void parse( std::istream_iterator<char> it, std::istream_iterator<char> end );
	void parse( std::string::const_iterator it, std::string::const_iterator end );
	void parse( const std::string &str ) { parse( str.begin(), str.end() );       }

	friend std::istream &operator>>( std::istream &in, Json &that )
	{
		that.setNull();

		in >> std::noskipws;

		std::istream_iterator<char> begin(in);
		std::istream_iterator<char> end;

		that.parse( begin, end );
		return in;
	}

	void setNull()      { reset(); }
	bool isNull() const { return std::holds_alternative<std::monostate>(data);    }

private:
	void reset()        { data = std::monostate{};                                }

	void write( std::ostream_iterator<char> it ) const;
	void write( std::string &out               ) const;

	std::chars_format fmt         = std::chars_format::scientific;
	//std::chars_format fmt         = std::chars_format::fixed;
	int               precision   = 5;
	bool              prettyPrint = true;

	DataT data;
};
}

#endif /* Json_h */
