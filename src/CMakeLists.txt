SET(CMAKE_LEGACY_CYGWIN_WIN32 0)
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
SET(CMAKE_CXX_STANDARD 14)
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
ADD_EXECUTABLE(
    atat
    atat.cpp
    main.cpp
)
ADD_EXECUTABLE(
    atat_test
    atat.cpp
    atat_test.cpp
    main_test.cpp
)
TARGET_LINK_LIBRARIES(
    atat_test
    CppUTest
    CppUTestExt
)
