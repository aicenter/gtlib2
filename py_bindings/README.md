# Example showing python bindings:

`pybind11` is a lightweight header-only library that exposes C++ types in Python and vice versa, mainly to create Python bindings of existing C++ code. 

Documentation is here: http://pybind11.readthedocs.io/en/master/index.html

You will need to install dependencies:
    
    pip install python-config pybind11

Example files are example.cpp and draw.py; example.cpp is compiled like this:
   
    c++ -O3 -shared -std=c++17 -fPIC \
        -I`pwd` \
        `python3 -m pybind11 --includes` \
        `python-config --cflags --ldflags` \
        -o py_bindings/example.so \
        base/base.h base/base.cpp \
        base/efg.h base/efg.cpp \
        domains/pursuit.h domains/pursuit.cpp \
        py_bindings/example.cpp 
    
Then you can run the python script.

TODO: currently `example.cpp` is broken, uses old code!
