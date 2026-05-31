
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

| Library         | Parse % | Write % | Total % |
|-----------------|---------|---------|---------|
| myJsonStrings   | 100%    | 100%    | 100%    |
| myJsonStreams   | 299%    | 381%    | 355%    |
| nlohmann        | 95%     | 82%     | 86%     |
| rapidjson       | 41%     | 39%     | 39%     |
| simdjson        | 9%      | 17%     | 15%     |
| AI              | 91%     | 265%    | 208%    |


| Library         | real (s) | real % | Δ real | user (s) | user % | Δ user | sys (s) | sys % | Δ sys |
|-----------------|----------|--------|--------|----------|--------|--------|---------|-------|-------|
| myJsonStrings   | 10.177   | 100%   | 0%     | 8.431    | 100%   | 0%     | 1.731   | 100%  | 0%    |
| myJsonStreams   | 30.977   | 304%   | +204%  | 26.253   | 311%   | +211%  | 4.397   | 254%  | +154% |
| nlohmann        | 9.004    | 88%    | −12%   | 8.127    | 96%    | −4%    | 0.865   | 50%   | −50%  |
| rapidjson       | 5.334    | 52%    | −48%   | 4.360    | 52%    | −48%   | 0.865   | 50%   | −50%  |
| simdjson        | 3.245    | 32%    | −68%   | 2.123    | 25%    | −75%   | 1.117   | 65%   | −35%  |
| AI              | 19.061   | 187%   | +87%   | 16.776   | 199%   | +99%   | 2.258   | 130%  | +30%  |


The AI version is an AI generated version which is conformant to std::c98.

Can run the performance tests yourself, go to subfolder performance/json_bench.  Here there are two scripts,
configure.sh and run.sh.
 
