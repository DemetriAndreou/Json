#include<chrono>
#include<fstream>
#include<iostream>
#include<sstream>
#include<string>
#include<vector>
#include<unistd.h>

// ------------------------------
//  Load JSON file into memory
// ------------------------------
std::string load_file(const std::string& path)
{
	std::ifstream f(path);
	if (!f)
	{
		std::cerr << "Error: cannot open file: " << path << "\n";
		std::exit(1);
	}
	return std::string( (std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>() );
}

using Clock = std::chrono::high_resolution_clock;

// ------------------------------
//  Benchmark wrapper
// ------------------------------
template <typename Parser>
double bench_parse(const std::string& data)
{
	auto start = Clock::now();
	Parser::parse(data);
	auto end = Clock::now();
	return std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() / 1e6;
}

template <typename Parser>
double bench_write(const std::string& data)
{
	auto start = Clock::now();
	Parser::write(data);
	auto end = Clock::now();
	return std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() / 1e6;
}

// ============================================================
//  Adapters for each JSON library
// ============================================================

// ------------------------------
//  Your parser (DemetriAndreou/Json)
// ------------------------------
#include "Json/Json.h"

struct MyJSONString
{
	static DaJson::Json parse_only(const std::string& s)
	{
		using namespace DaJson;
		Json j;
		j.parse( s.begin(), s.end() );
		return j;
	}

	static void parse(const std::string& s)
	{
		parse_only(s);
	}

	static void write(const std::string& s)
	{
		auto j = parse_only(s);
		j.setPretty( false );
		volatile auto sink = j.str(s.length());
		//volatile auto sink = j.str();
	}
};

struct MyJSONStream
{
	static DaJson::Json parse_only(const std::string& s)
	{
		using namespace DaJson;
		std::istringstream in(s);
		Json j;
		in >> j;
		return j;
	}

	static void parse(const std::string& s)
	{
		parse_only(s);
	}

	static void write(const std::string& s)
	{
		auto j = parse_only(s);
		std::ostringstream out;
		out << j;
		volatile auto sink = out.str();
	}
};

// ------------------------------
//  nlohmann/json
// ------------------------------
#include <nlohmann/json.hpp>

struct Nlohmann
{
	static nlohmann::json parse_only(const std::string& s)
	{
		return nlohmann::json::parse(s);
	}

	static void parse(const std::string& s)
	{
		parse_only(s);
	}

	static void write(const std::string& s)
	{
		auto j = parse_only(s);
		volatile auto sink = j.dump();
	}
};

// ------------------------------
//  RapidJSON
// ------------------------------
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

struct Rapid
{
	static rapidjson::Document parse_only(const std::string& s)
	{
		rapidjson::Document d;
		d.Parse(s.c_str());
		return d;
	}

	static void parse(const std::string& s)
	{
		parse_only(s);
	}

	static void write(const std::string& s)
	{
		auto d = parse_only(s);
		rapidjson::StringBuffer buffer;
		rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
		d.Accept(writer);
		volatile auto sink = buffer.GetString();
	}
};

// ------------------------------
//  simdjson DOM
// ------------------------------
#include <simdjson.h>

struct Simdjson
{
	static void parse(const std::string& s)
	{
		simdjson::dom::parser p;
		auto doc = p.parse(s);
		(void)doc;
	}

	static void write(const std::string& s)
	{
		simdjson::dom::parser p;
		auto doc = p.parse(s);

		// Safe: parser p is still alive here
		std::string out = simdjson::minify(doc);
		volatile auto sink = out;
	}
};

// ------------------------------
//  AI
// ------------------------------
#include "ai/Json.h"

struct AIT
{
	static void parse(const std::string& s)
	{
		AI::Json p(s);
		auto doc = p.parse();
		(void)doc;
	}

	static void write(const std::string& s)
	{
		AI::Json p(s);
		auto doc = p.parse();

		doc.toString();
	}
};

struct BenchEntry
{
	std::function<double(const std::string&)> parse;
	std::function<double(const std::string&)> write;
};

const std::map<std::string, BenchEntry> registry =
{
	{ "myJsonStrings",
		{
			[](const std::string& s){ return bench_parse<MyJSONString>(s); },
			[](const std::string& s){ return bench_write<MyJSONString>(s); }
		}
	},
	{ "myJsonStreams",
		{
			[](const std::string& s){ return bench_parse<MyJSONStream>(s); },
			[](const std::string& s){ return bench_write<MyJSONStream>(s); }
		}
	},
	{ "nlohmann",
		{
			[](const std::string& s){ return bench_parse<Nlohmann>(s); },
			[](const std::string& s){ return bench_write<Nlohmann>(s); }
		}
	},
	{ "rapidjson",
		{
			[](const std::string& s){ return bench_parse<Rapid>(s); },
			[](const std::string& s){ return bench_write<Rapid>(s); }
		}
	},
	{ "simdjson",
		{
			[](const std::string& s){ return bench_parse<Simdjson>(s); },
			[](const std::string& s){ return bench_write<Simdjson>(s); }
		}
	},
	{ "AI",
		{
			[](const std::string& s){ return bench_parse<AIT>(s); },
			[](const std::string& s){ return bench_write<AIT>(s); }
		}
	}
};

int main(int argc, char** argv)
{
	std::string which, file;
	int opt;

	while( (opt = getopt(argc, argv, "f:c:")) != -1)
	{
		switch( opt )
		{
			case 'c': which = optarg; break;
			case 'f': file  = optarg; break;
		}
	}

	if( file.empty())
	{
		std::cerr << "usage: " << argv[0] << " -f <FILE> -c [";
		bool first = true;
		for( auto &kv : registry )
		{
			if (!first) std::cerr << '|';
			std::cerr << kv.first;
			first = false;
		}
		std::cerr << "]\n";
		return 1;
	}

	const std::string data = load_file(file);

	std::cout << "Benchmarking file: " << file << "\n";
	std::cout << "Size: " << data.size() << " bytes\n\n";

	auto it = registry.find(which);
	if( it == registry.end() )
	{
		std::cerr << "Unknown test: " << which << "\n";
		return 1;
	}

	const auto &entry = it->second;

	double parse_ms = entry.parse(data);
	double write_ms = entry.write(data);
	double total    = parse_ms + write_ms;

	std::cout << which << ":\n";
	std::cout << "  parse: " << parse_ms << " ms\n";
	std::cout << "  write: " << write_ms << " ms\n";
	std::cout << "  total: " << total << " ms\n\n";

	return 0;
}

