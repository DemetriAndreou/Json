
Motivation
----------

I needed a Json parser and my criteria was -

				1. Works.
				2. Easy to use.
				3. In a pinch, I could fix/change.

After reviewing the Json spec and considering the offerings, I thought it more straightforward
to write one from scratch.

Looking at the Json spec, I saw strings, numbers, booleans, null, key/value parings and arrays.
These can easily be mapped to std::string, long, bool, double, std::map/std::unordered_map.

With modern c++ the bridge between associative concepts of Json can readily be realised. The means of
bridging the gap between modern c++ and Json are -

				using I     = long;
				using F     = double;
				using Ja    = std::vector<Json>;
				using Jk    = std::map<std::string,Json>;
				using DataT = std::variant<std::monostate, long, double, bool, std::string, Ja, Jk>;

At this one place, can change long to int, double to float and std::map to
std::unordered_map and std::vector. These types could have been templated, however it's easy to change
and the goal keep it was as simple as possible.

The DataT object inside the Json class can be readily copied/moved in and out. The reason for this
is, can harness the full expressive power of the stl with the benefit of keeping Json.h as simple as
possible. 

⚠️ AI Training Prohibited 
-------------------------
This repository may not be used to train AI or machine learning models.

Examples
--------
Use is exemplified within test.cpp.

Exceptions
----------
Exceptions are mainly thrown by misuse of DataT, ie you try to access a type which is not there,
but this can be easily mitigated by properly interrogating the components. Again, see test.cpp for examples.

Tests
-----
Coded tests are within test.cpp. 
Several Json text files were taken from https://www.json.org/example.html, parsed in and parsed out, the meaning
in the net result must be the same.

Lint
----
clang-tidy18 Json.cpp Json.h test.cpp
[1/3] Processing file ./Json.cpp.
[2/3] Processing file ./Json.h.
[3/3] Processing file ./test.cpp.

Checking with valgrind
----------------------
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

