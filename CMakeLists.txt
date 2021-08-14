cmake_minimum_required(VERSION 3.0)
project(indi-bresserexos2 VERSION 0.900)

LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/")
LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules/")
LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../cmake_modules/")
include(GNUInstallDirs)

set(THREADS_PREFER_PTHREAD_FLAG ON)

set(CMAKE_DEBUG_POSTFIX d)

if (APPLE)
	set(CMAKE_CXX_STANDARD 14)
else ()
	set(CMAKE_CXX_STANDARD 11)
endif ()

set(CMAKE_CXX_STANDARD_REQUIRED True)

if(NOT CMAKE_INSTALL_PREFIX)
	set(CMAKE_INSTALL_PREFIX "/usr")
endif ()

if (NOT XML_INSTALL_DIR)
	set(XML_INSTALL_DIR "/usr/share/indi")
endif ()

if(INDI_VERSION VERSION_LESS "1.9.0")
	set(INDI_LEGACY_ENABLED "1")
else ()
	set(INDI_LEGACY_ENABLED "0")
endif ()

configure_file(config.h.cmake config.h)
configure_file(indi_bresserexos2.xml.cmake indi_bresserexos2.xml)

find_package(Threads REQUIRED)
find_package(INDI REQUIRED)

include_directories(${PROJECT_BINARY_DIR})

option(USE_CERR_LOGGING "log error messages using std::cerr for debugging" ON)

add_executable(indi_bresserexos2 BresserExosIIGoToDriver.cpp IndiSerialWrapper.cpp SerialCommand.cpp)
target_link_libraries(indi_bresserexos2 ${INDI_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} Threads::Threads)
target_include_directories(indi_bresserexos2 PUBLIC
						  "${PROJECT_BINARY_DIR}"
						  )
			  
include(GNUInstallDirs)
install(TARGETS indi_bresserexos2 DESTINATION ${CMAKE_INSTALL_PREFIX})
install(FILES ${PROJECT_BINARY_DIR}/indi_bresserexos2.xml DESTINATION ${XML_INSTALL_DIR})
