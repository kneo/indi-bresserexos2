cmake_minimum_required(VERSION 3.13)

set(CMAKE_DEBUG_POSTFIX d)

project(BresserExosIIGoToDriverForIndi VERSION 0.815)

option(USE_CERR_LOGGING "log error messages using std::cerr for debugging" ON)

configure_file(Config.hpp.in Config.hpp)
include_directories(${PROJECT_BINARY_DIR})

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_subdirectory(SerialDeviceControl)

find_package(Threads)

add_executable(BresserExosIIGoToDriverForIndi Main.cpp TestSerialImplementation.cpp TestDataReceivedCallback.cpp)
set_target_properties(BresserExosIIGoToDriverForIndi PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})

target_link_libraries(BresserExosIIGoToDriverForIndi PUBLIC SerialDeviceControl ${CMAKE_THREAD_LIBS_INIT})

target_include_directories(BresserExosIIGoToDriverForIndi PUBLIC
						  "${PROJECT_BINARY_DIR}"
						  "${PROJECT_SOURCE_DIR}/SerialDeviceControl"
						  )
