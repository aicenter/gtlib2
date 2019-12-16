add_library(utils OBJECT
        args.hpp
        benchmark.h
        combinatorics.h
        combinatorics.cpp
        export.cpp export.h
        export.h
        export.cpp
        logging.h
        range.h
        utils.h
        nn_transform_data.h
        nn_transform_data.cpp
        )
target_include_directories(utils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

set(TEST_FILES ${TEST_FILES}
        utils/exportTest.cpp
        utils/rangeTest.cpp
        PARENT_SCOPE)
