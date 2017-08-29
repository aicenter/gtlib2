cpplint - cpplint.py is a tool that reads a source file and identifies many (Google c++) style errors.
It is not perfect, and has both false positives and false negatives, but it is still a valuable tool.
False positives can be ignored by putting // NOLINT at the end of the line or // NOLINTNEXTLINE in the previous line.
You can install it via pip - pip install cpplint

pybind11 - pybind11 is a lightweight header-only library that exposes C++ types in Python and vice versa,
mainly to create Python bindings of existing C++ code. Documentation is here: http://pybind11.readthedocs.io/en/master/index.html
Example files are example.cpp and draw.py; example.cpp is compiled like this: c++ -O3 -shared -std=c++11 -fPIC -I <path>/pybind11/include `python-config --cflags --ldflags`  utils/example.cpp base.cpp efg.cpp pursuit.cpp -o utils/example.so
Then you can run the python script.


Google Test - Google Test is Google's C++ test framework. All Google Test source and pre-built packages are provided under the New BSD License.
When you want to use it you need to add these lines to your main.cpp:
#include <gtest/gtest.h>
#include <gmock/gmock.h>

and in the main function these:

testing::InitGoogleTest(&argc, argv);
RUN_ALL_TESTS();


Then you have to add these lines to you makefile (CMakeLists.txt):
under this line: set(CMAKE_CXX_STANDARD 11) add these:
add_subdirectory(lib/googletest)
include_directories(lib/googletest/googletest/include)
include_directories(lib/googletest/googlemock/include)

and under this line: set(SOURCE_FILES main.cpp tests/firstTest.cpp) add these:
add_executable(pursuit ${SOURCE_FILES} tests/firstTest.cpp)
target_link_libraries(pursuit gtest gtest_main)