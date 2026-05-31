
Motivation
----------

I needed a Json parser and my criteria was -

				1. Works.
				2. Easy to use.
				3. In a pinch, I could fix/change.

After reviewing the Json spec and considering the offerings, I thought it more straightforward to write one from
scratch.

Looking at the Json spec, I saw strings, numbers, booleans, null, key/value parings and arrays.  These can easily be
mapped to std::string, long, bool, double, std::map/std::unordered_map.

With modern c++ the bridge between associative concepts of Json can readily be
realised. The means of bridging the cap between modern c++ and Json are -

				using I     = long;
				using F     = double;
				using Ja    = std::vector<Json>;
				using Jk    = std::map<std::string,Json>;
				using DataT = std::variant<std::monostate, long, double, bool, std::string, Ja, Jk>;

At this one place, can change long to int, double to float and std::map to std::unordered_map and std::vector. These
types could have been templated, however it's easy to change and the goal keep it was as simple as possible.

The DataT object inside the Json class can be readily copied/moved in and out. The reason for this is, can harness the
full expressive power of the stl with the benefit of keeping Json.h as simple as possible. 

This is designed to take as input and output either a string or some std::stream. From below, can see the stream
versions are must slower. Still, the option is there for either case.

Examples
--------
Use is exemplified within test.cpp.

Exceptions
----------

Exceptions are mainly thrown by misuse of DataT, ie you try to access a type which is not there, but this can be easily
mitigated by properly interrogating the components. Also, if the json text is invalid. Again, see test.cpp for examples.

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

| Library        | Parse % | Write % | Total % | real (s) | user (s) | sys (s) |
|----------------|---------|---------|---------|----------|----------|---------|
| myJsonStrings  | 100%    | 100%    | 100%    | 9.643    | 7.474    | 2.135   |
| myJsonStreams  | 328%    | 415%    | 388%    | 31.291   | 26.744   | 4.446   |
| nlohmann       | 111%    | 89%     | 96%     | 9.357    | 8.312    | 1.022   |
| rapidjson      | 49%     | 41%     | 44%     | 5.425    | 4.375    | 1.037   |
| simdjson       | 12%     | 18%     | 16%     | 3.374    | 2.125    | 1.241   |
| AI             | 106%    | 310%    | 247%    | 20.739   | 16.666   | 4.021   |

| Library        | real (s) | real % | Δ real | user (s) | user % | Δ user | sys (s) | sys % | Δ sys |
|----------------|----------|--------|--------|----------|--------|--------|---------|-------|-------|
| myJsonStrings  | 9.643    | 100%   |   0%   | 7.474    | 100%   |   0%   | 2.135   | 100%  |   0%  |
| myJsonStreams  | 31.291   | 325%   | +225%  | 26.744   | 358%   | +258%  | 4.446   | 208%  | +108% |
| nlohmann       | 9.357    |  97%   |   −3%  | 8.312    | 111%   |  +11%  | 1.022   |  48%  |  −52% |
| rapidjson      | 5.425    |  56%   |  −44%  | 4.375    |  59%   |  −41%  | 1.037   |  49%  |  −51% |
| simdjson       | 3.374    |  35%   |  −65%  | 2.125    |  28%   |  −72%  | 1.241   |  58%  |  −42% |
| AI             | 20.739   | 215%   | +115%  | 16.666   | 223%   | +123%  | 4.021   | 188%  |  +88% |

The AI version is an AI generated version which is conformant to std::c98.

Can run the performance tests yourself, go to subfolder performance/json_bench.  Here there are two scripts,
configure.sh and run.sh.
 
