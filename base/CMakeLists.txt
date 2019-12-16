add_library(base OBJECT
        algorithm.h
        algorithm.cpp 
        base.h
        base.cpp
        cache.h
        cache.cpp 
        efg.h
        fogefg.h
        fogefg.cpp
        gadget.h
        gadget.cpp
        hashing.h
        hashing.cpp
        includes.h
        random.h
        random.cpp
        tree.h
        )
target_include_directories(base PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
