#include <fstream>
#include <string>

int main()
{
	std::ofstream out("huge.json");
	out << "{\n";
	out << "  \"metadata\": {\n";
	out << "    \"generated\": \"2026-05-09T12:00:00Z\",\n";
	out << "    \"description\": \"1GB synthetic JSON for performance testing\"\n";
	out << "  },\n";
	out << "  \"blocks\": [\n";

	const std::string long_str(4096, 'x');  // 4 KB string
	const int blocks = 1000;              // ~1 GB total

	for( int i = 0; i < blocks; ++i )
	{
		out << "    {\n";
		out << "      \"id\": " << i << ",\n";
		out << "      \"deep\": { \"l1\": { \"l2\": { \"l3\": { \"l4\": { \"l5\": { \"value\": \"deep\" }}}}}},\n";
		out << "      \"wide\": {\n";
		for( int k = 1; k <= 200; ++k )
		{
			out << "        \"k" << k << "\": \"" << long_str << "\"";
			if( k < 200 )
				out << ",";
			out << "\n";
		}
		out << "      },\n";
		out << "      \"array\": [1, \"two\", null, true, false, 3.14, {\"nested\": \"object\"}],\n";
		out << "      \"long_string\": \"" << long_str << "\"\n";
		out << "    }";
		if (i < blocks - 1) out << ",";
		out << "\n";
	}

	out << "  ]\n";
	out << "}\n";
}

