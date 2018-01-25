

pybind11 - pybind11 is a lightweight header-only library that exposes C++ types in Python and vice versa,
mainly to create Python bindings of existing C++ code. Documentation is here: http://pybind11.readthedocs.io/en/master/index.html
Example files are example.cpp and draw.py; example.cpp is compiled like this: c++ -O3 -shared -std=c++11 -fPIC -I <path>/pybind11/include `python-config --cflags --ldflags`  utils/example.cpp base.cpp efg.cpp pursuit.cpp -o utils/example.so
Then you can run the python script.

