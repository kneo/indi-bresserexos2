cmake_minimum_required(VERSION 3.13)
project(indi-bresserexos2 VERSION 0.820)

set(CMAKE_DEBUG_POSTFIX d)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(XML_INSTALL_DIR /usr/share/indi)
set(CMAKE_INSTALL_PREFIX /usr/bin)
configure_file(Config.hpp.in Config.hpp)
configure_file(indi_bresserexos2.xml.in indi_bresserexos2.xml)

find_package(Threads REQUIRED)
#find_package(INDI REQUIRED)

include_directories(${PROJECT_BINARY_DIR})

option(USE_CERR_LOGGING "log error messages using std::cerr for debugging" ON)

add_subdirectory(SerialDeviceControl)

#add_executable(BresserExosIIGoToDriverForIndiTest Main.cpp TestSerialImplementation.cpp TestDataReceivedCallback.cpp)
#set_target_properties(BresserExosIIGoToDriverForIndiTest PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})
#target_link_libraries(BresserExosIIGoToDriverForIndiTest PUBLIC SerialDeviceControl ${CMAKE_THREAD_LIBS_INIT})

#target_include_directories(BresserExosIIGoToDriverForIndiTest PUBLIC
#						  "${PROJECT_BINARY_DIR}"
#						  "${PROJECT_SOURCE_DIR}/SerialDeviceControl"			  
#						  )

add_executable(indi_bresserexos2 BresserExosIIGoToDriver.cpp IndiSerialWrapper.cpp)
target_link_libraries(indi_bresserexos2 indidriver SerialDeviceControl ${CMAKE_THREAD_LIBS_INIT})
target_include_directories(indi_bresserexos2 PUBLIC
						  "${PROJECT_BINARY_DIR}"
						  "${PROJECT_SOURCE_DIR}/SerialDeviceControl"
						  
						  )
						  
include(GNUInstallDirs)
install(TARGETS indi_bresserexos2 DESTINATION ${CMAKE_INSTALL_PREFIX})
install(FILES ${PROJECT_BINARY_DIR}/indi_bresserexos2.xml DESTINATION ${XML_INSTALL_DIR})
