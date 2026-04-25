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

#include<fstream>
#include<sstream>
#include<time.h>
#include<ctime>
#include<memory>


using namespace DaJson;

class Uid
{
public:
	Uid()
	{
		id = std::time(nullptr);
	}

	explicit Uid( const Uid &that ) : id( that.id )
	{
	}

	explicit Uid( long in ) : id(in)
	{
	}

	void setId( long in ) { id = in;   }
	long getId() const    { return id; }

	friend std::ostream &operator<<( std::ostream &strm, const Uid &that )
	{
		strm << "id:" << that.id;
		return strm;
	}

	Uid &operator=(  const Uid &that)       { id =  that.id; return *this; }
	bool operator<(  const Uid &that) const { return id <  that.id; }
	bool operator>(  const Uid &that) const { return id >  that.id; }
	bool operator==( const Uid &that) const { return id == that.id; }
private:
	long id;
};

class Person
{
public:
	Person( const std::string &in ) : name(in)
	{
	}
	Person()
	{
	}
	const std::string &getName() const                  { return name;  }
	void               setName( const std::string &in ) { name = in;    }

	const Uid         &getUid()  const                  { return uid;   }
	void              setUid( const Uid &in )           { uid = in;     }

	struct Address
	{
		std::string line;
	};
	using AddressesT = std::vector<Address>;
	AddressesT addresses;

private:
	std::string name;
	Uid uid;
};

class Team
{
public:
	using PersonsT = std::map<Uid, Person>;

	std::unique_ptr<Person> getLeader( const Uid & )
	{
		if( leader )
		{
			return std::unique_ptr<Person>( std::make_unique<Person>( *leader ) );
		}
		return std::unique_ptr<Person>( std::make_unique<Person>() );
	}

	void setLeader( std::unique_ptr<Person> person    ) { leader = std::move( person );                                       }
	void removeStaff( const Person &                  ) {}
	void getStaff(    PersonsT     &                  ) {}
	void setStaff(    std::unique_ptr<Person>         ) {}
	std::unique_ptr<Person> getPerson( const Uid &    ) { return std::unique_ptr<Person>( std::make_unique<Person>() );       }
	void clearTaff()                                    { staff.clear();                                                      }

private:
	std::unique_ptr<Person> leader;
	PersonsT                staff;
	Uid                     uid;
};

//
// The rest is left as an exercise
//
class Db
{
public:
	using TeamsT   = std::map<Uid, Team>;
	using PersonsT = Team::PersonsT;

	Db() : version(0)
	{
	}
	Db( const Db &that ) : version( that.version ), persons( that.persons )
	{
	}

	std::unique_ptr<Person> getPerson( const Uid &    )   { return std::unique_ptr<Person>( std::make_unique<Person>() );       }
	void setPerson(    std::unique_ptr<Person>          );
	void removePerson( const Person &                   ) {}
	void getPersons(   PersonsT &                       ) {}
	void setPersons(   PersonsT &                       ) {}
	void clearPersons()                                   {}

	std::unique_ptr<Team>   getTeam( const Uid &        ) { return std::unique_ptr<Team>(   std::make_unique<Team>()   );       }
	void setTeam(    std::unique_ptr<Team>              ) {}
	void getTeams(   TeamsT &                           ) {}
	void removeTeam( Team   &                           ) {}
	void setTeams(   TeamsT &                           ) {}
	void clearTeams()                                     {}

	int  getVersion() const                               { return version;                                                     }
	bool readDb(  const std::string &file );
	bool writeDb( const std::string &file );
private:
	int version;
	PersonsT    persons;
};

void Db::setPerson( std::unique_ptr<Person> person )
{
	persons[person->getUid()] = Person( *person );
}

bool Db::readDb(const std::string &file )
{
	version = {};
	persons.clear();

	std::ifstream in( file );
	Json json;
	in >> json;
	Json::DataT data = std::move(json);

	version = {};
	persons.clear();

	const Json::Jk &jk = std::get<Json::Jk>( data );
	for( const auto &[key, value] : jk )
	{
		if( "persons" == key )
		{
			const Json::DataT &personArray = value;
			const Json::Ja    &personJvt   = std::get<Json::Ja>( personArray );
			Person person;
			Uid    uid;
			bool   haveUid = false;
			for( const auto &personIdx : personJvt )
			{
				const Json::DataT &personData      = personIdx;
				const Json::Jk    &personKeyValue  = std::get<Json::Jk>( personData );
				for( const auto &[personKey, personDataIdx] : personKeyValue )
				{
					if( "address" == personKey )
					{
						const Json::DataT &data      = personDataIdx;
						const Json::Ja    &addresses = std::get<Json::Ja>(data);
						Person::AddressesT ads;
						for( const auto &address : addresses )
						{
							const Json::DataT &a = address;
							ads.emplace_back(std::get<std::string>(a));
						}
						person.addresses = ads;
					}
					else if( "name" == personKey )
					{
						person.setName( std::get<std::string>(static_cast<const Json::DataT&>(personDataIdx)));
					}
					else if( "uid" == personKey )
					{
						person.setUid( Uid( std::get<long>(static_cast<const Json::DataT&>(personDataIdx))));
						haveUid = true;
					}
				}
			}
			if( haveUid )
			{
				persons[person.getUid()] = person;
			}
		}
		else if( "version" == key )
		{
			version = std::get<long>(static_cast<const Json::DataT&>(value));
		}
	}

	return true;
}

bool Db::writeDb( const std::string &file )
{
	Json::Jk jvm;
	
	Json::Ja personsJson;
	for( const auto &[uid, person] : persons )
	{
		Json::Ja addressesJson;
		Json::Jk personJson;
		for( const auto &address : person.addresses )
		{
			addressesJson.emplace_back( Json( address.line ) );
		}
		personJson["address"] = addressesJson;
		personJson["name"]    = person.getName();
		personJson["uid"]     = person.getUid().getId();
		personsJson.emplace_back( personJson );
	}

	jvm["persons"] = personsJson;
	jvm["version"] = 1;

	Json json( std::move(jvm) );
	std::ofstream out( file );
	out << json;

	return true;
}

void createJson()
{
	Db dbOut;
	{
		std::unique_ptr<Person> p1( std::make_unique<Person>( "p1" ));
		Person::AddressesT a{ { "add1_1" }, { "add1_2" }, { "add1_3" }};
		p1->addresses= a;
		p1->setUid( Uid( 1 ) );
		dbOut.setPerson( std::move(p1) );
	}
	{
		std::unique_ptr<Person> p2( std::make_unique<Person>( "p2" ));
		Person::AddressesT a{ { "ADD2_1" }, { "ADD2_2" }, { "ADD2_3" }};
		p2->addresses= a;
		p2->setUid( Uid( 2 ) );
		dbOut.setPerson( std::move(p2) );
	}
	dbOut.writeDb( "db.out.json" );
}

void readJson()
{
	Db dbIn;
	dbIn.readDb(  "db.out.json" );
	dbIn.writeDb( "db.in.json"  );
}

void testJson( const std::string &file )
{
	std::ifstream out( file + ".out.json" );
	std::ifstream in(  file + ".in.json"  );

	if( false == out.is_open() || false == in.is_open() )
	{
		std::cerr << file << ":failed to open\n";
	}

	std::ostringstream outStrm;
	outStrm << out.rdbuf();

	std::ostringstream inStrm;
	inStrm << in.rdbuf();

	const std::string outStr = outStrm.str();
	const std::string inStr  = inStrm.str();

	if( outStr != inStr )
	{
		std::cerr << file  << ":failed" << '\n';
	}
	else
	{
		std::cout << file << ":passed"  << '\n';
	}
}

void readWriteJson( const std::string &file )
{
	std::ofstream out( file + ".out.json" );
	std::ifstream in(  file + ".in.json"  );

	Json json;
	in  >> json;
	out << json;
}

void testOperatorAccess1()
{
	{
		std::ifstream in(  "test3.in.json" );
		std::ofstream out( "changed.out.json" );
		Json json;
		in >> json;

		std::size_t size = std::get<Json::Ja>(static_cast<const Json::DataT &>(json["menu"]["popup"]["menuitem"])).size();
		std::size_t pos = 0;
		if( pos < size )
		{
			json["menu"]["popup"]["menuitem"][pos]["onclick"] = "changed";
		}

		const std::string s = std::get<std::string>(static_cast<const Json::DataT &>(json["menu"]["value"]));
		const std::string matchOn{"FileΛHereThere\"AndWhere"};
		if( s == matchOn )
		{
			std::cout << "testOperatorAccess1:passed\n";
		}
		else
		{
			std::cerr << "testOperatorAccess1:failed\n";
		}

		out << json;
	}
	{
		testJson( "changed" );
	}
}

void testOperatorAccess2()
{
	// This is not advised -
	//		std::string str;
	//		str = nullptr
	// For the same reason, the below is not advised.
	//		Json::DataT data
	// 		data = nullptr;
	// 		Json j;
	// 		j = nullptr;
	// 		Json j( nullptr );
	// Reason being is the above decays to std::string.
	{
		Json json;
		bool shouldBeNull = json.isNull();

		std::ifstream in(  "test3.in.json" );
		in >> json;
		bool shouldNotBeNull = json.isNull();

		if( shouldBeNull && false == shouldNotBeNull )
		{
			std::cout << "testOperatorAccess2:Null test passed\n";
		}
		else
		{
			std::cerr << "testOperatorAccess2:Null test failed\n";
		}
	}
	{
		Json json;
		std::ifstream in(  "test3.in.json" );
		in >> json;
		bool shouldNotBeNull = json.isNull();
		json.setNull();
		bool shouldBeNull = json.isNull();

		if( shouldBeNull && false == shouldNotBeNull )
		{
			std::cout << "testOperatorAccess2:Null test passed\n";
		}
		else
		{
			std::cerr << "testOperatorAccess2:Null test failed\n";
		}
	}
}

void testOperatorAccess3()
{
	{
		std::ofstream out( "trad.out.json" );
		Json::Ja ja;
		Json::Jk jk;
		Json json( jk );

		ja.emplace_back( "data1" );
		ja.emplace_back( "data2" );

		json["start"]    = "begin";
		json["middle"]   = "half way";
		json["data"]     = ja;
		json["end"]      = "finished";

		out << json;
	}
	{
		testJson( "trad" );
	}
}

void testOperatorAccess4()
{
	bool passed = true;
	std::ifstream in( "test5.in.json" );
	Json json;
	in >> json;

	std::map<std::string,std::string> tests =
	{
		{ "key_01", "data\" complete"  },
		{ "key_02", "data\\ complete"  },
		{ "key_03", "data/ complete"   },
		{ "key_04", "data complete"   },
		{ "key_05", "data complete"   },
		{ "key_06", "data\x0A complete" },
		{ "key_07", "data\x0D complete" },
		{ "key_08", "data\x09 complete" },
		{ "key_09", "dataΛ complete" },
		{ "key_11", "\"" },
		{ "key_12", "\\" },
		{ "key_13", "/" },
		{ "key_14", "\b" },
		{ "key_15", "\f" },
		{ "key_16", "\n" },
		{ "key_17", "\r" },
		{ "key_18", "\t" },
		{ "key_19", "Λ" }
	};

	for( const auto &[key, value] : tests )
	{
		const std::string str  = std::get<std::string>(static_cast<const Json::DataT&>(json[key]));
		if( value != str )
		{
			std::cerr << "Failed:" << "Length:" <<  value.size() << ':' << str.size() << '\n';
			std::cerr << "Failed:" << key << '(' << value << ") (" << str << ')' << '\n';
			std::cout << "value:\n";
			for( auto ch : value )
			{
				std::cout << ch << ':' << int(ch) << '\n';
			}
			std::cout << std::endl;
			std::cout << "read:\n";
			for( auto ch : str )
			{
				std::cout << ch << ':' << int(ch) << '\n';
			}
			std::cout << std::endl;
			passed = false;
		}
	}
	if( passed )
	{
		std::cout << "testOperatorAccess4:passed\n";
	}
	else
	{
		std::cerr << "testOperatorAccess4:failed\n";
	}
}

void testOperatorAccess5()
{
	std::istringstream strm( R"({
	                                 "start": "begin",
	                                 "data":
	                                 [
	                                         "data1",
	                                         "data2"
	                                 ],
	                                 "end": "finished",
	                                 "middle": "half way"
	                         })");
	Json json;
	strm >> json;
	{
		std::ofstream out( "test6.out.json" );
		out << json;
	}
	testJson( "test6" );
}


int main()
{
	createJson();
	readJson();

	for( std::string file : { "db", "test1", "test2", "test3", "test4" } )
	for( std::string file : { "db" } )
	{
		readWriteJson( file );
		testJson(      file );
	}

	testOperatorAccess1();
	testOperatorAccess2();
	testOperatorAccess3();
	testOperatorAccess4();
	testOperatorAccess5();

	return 0;
}
