set -x
make
time json_bench -f huge.json -c myJsonStrings
time json_bench -f huge.json -c myJsonStreams
time json_bench -f huge.json -c nlohmann
time json_bench -f huge.json -c rapidjson
time json_bench -f huge.json -c simdjson
time json_bench -f huge.json -c AI
