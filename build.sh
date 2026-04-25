set -x
clang++ -I . -std=c++20 test.cpp Json.cpp -o tstclang 
g++     -I . -std=c++20 test.cpp Json.cpp -o tstg     
