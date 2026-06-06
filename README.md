
Motivation
----------

I needed a JSON parser and my criteria was -

				1. Works.
				2. Easy to use.
				3. In a pinch, I could fix/change.

After reviewing the JSON spec and considering the offerings, I thought it more straightforward to write one from scratch.

Looking at the JSON spec, I saw strings, numbers, booleans, null, key/value parings and arrays. These can easily be mapped to std::string, long, bool, double, std::map/std::unordered_map.

With modern C++ can bridge between associative concepts of JSON. The means of bridging the cap between modern c++ and JSON are -

				using I     = long;
				using F     = double;
				using Ja    = std::vector<Json>;
				using Jk    = std::map<std::string,Json>;
				using DataT = std::variant<std::monostate, long, double, bool, std::string, Ja, Jk>;

At this one place, can change long to int, double to float and std::map to std::unordered_map and std::vector. These types could have been templated, however it's easy to change and the goal was to keep it as simple as possible.

The DataT object inside the Json class can be readily copied/moved in and out. The reason for this is, can harness the full expressive power of the Standard Templace Library with the benefit of keeping Json.h as simple as possible. 

This is designed to take as input and output either a string or some std::stream. From below, can see the stream versions are much slower. Still, the option is there for either case.

Examples
--------
Use is exemplified within test.cpp.

Exceptions
----------

Exceptions are mainly thrown by misuse of DataT, for example you try to access a type which is not there, but this can be easily mitigated by properly interrogating the components. Also, if the JSON text is invalid. Again, see test.cpp for examples.

Tests
-----

Coded tests are within test.cpp.  Several Json text files were taken from https://www.json.org/example.html, parsed in
and parsed out, the meaning in the net result must be the same.

Lint
----
clang-tidy18 Json.cpp Json.h test.cpp
```
[1/3] Processing file ./Json.cpp.
[2/3] Processing file ./Json.h.
[3/3] Processing file ./test.cpp.
```

Checking with valgrind
----------------------
```
==73873== Memcheck, a memory error detector
==73873== Copyright (C) 2002-2024, and GNU GPL'd, by Julian Seward et al.
==73873== Using Valgrind-3.26.0 and LibVEX; rerun with -h for copyright info
==73873== Command: tst
==73873==
db:passed
db:passed
db:passed
db:passed
db:passed
testOperatorAccess1:passed
changed:passed
testOperatorAccess2:Null test passed
testOperatorAccess2:Null test passed
trad:passed
testOperatorAccess4:passed
test6:passed
==73873==
==73873== HEAP SUMMARY:
==73873==     in use at exit: 4,096 bytes in 1 blocks
==73873==   total heap usage: 677 allocs, 676 frees, 352,504 bytes allocated
==73873==
==73873== LEAK SUMMARY:
==73873==    definitely lost: 0 bytes in 0 blocks
==73873==    indirectly lost: 0 bytes in 0 blocks
==73873==      possibly lost: 0 bytes in 0 blocks
==73873==    still reachable: 0 bytes in 0 blocks
==73873==         suppressed: 4,096 bytes in 1 blocks
==73873==
==73873== For lists of detected and suppressed errors, rerun with: -s
==73873== ERROR SUMMARY: 0 errors from 0 contexts (suppressed: 0 from 0)
```

Performance
-----------

Performance is comparable to nlohmann.

## 📊 Performance (huge.json — 827,420,037 bytes)
### Round‑trip parse + write performance  
Baseline = myJsonStrings = 8012.54 ms = 100%
| Library          | Parse (ms) | Write (ms) | Total (ms) | Relative Speed vs myJsonStrings |
|------------------|------------|------------|------------|---------------------------------|
| myJsonStrings    | 2097.47    | 5915.07    | 8012.54    | 100%                             |
| myJsonStreams    | 6946.30    | 20435.50   | 27381.80   | 341.7% (3.41× slower)            |
| nlohmann/json    | 2358.62    | 4466.79    | 6825.42    | 85.2% (1.17× faster)             |
| RapidJSON        | 1295.97    | 2315.19    | 3611.16    | 45.1% (2.22× faster)             |
| simdjson DOM     | 281.33     | 981.66     | 1262.99    | 15.8% (6.34× faster)             |
| AI (C++98)       | 2434.60    | 16597.40   | 19032.00   | 237.5% (2.37× slower)            |



## 🕒 Unix `time` results (real/user/sys)
Baseline = myJsonStrings real = 10.089 s = 100%
| Library          | real (s) | user (s) | sys (s) | real % vs baseline |
|------------------|----------|----------|---------|---------------------|
| myJsonStrings    | 10.089   | 7.163    | 2.886   | 100%                |
| myJsonStreams    | 29.524   | 24.533   | 4.810   | 292.6%              |
| nlohmann/json    | 8.899    | 7.757    | 1.125   | 88.2%               |
| RapidJSON        | 5.718    | 4.698    | 1.016   | 56.6%               |
| simdjson DOM     | 3.379    | 2.251    | 1.126   | 33.5%               |
| AI (C++98)       | 21.178   | 16.376   | 4.785   | 210.0%              |



The AI version is an AI generated version which is conformant to std::c98.

You can run the performance tests yourself. In subfolder performance/json_bench,  there are two scripts, configure.sh and run.sh.
 
