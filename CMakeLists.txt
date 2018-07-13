cmake_minimum_required(VERSION 3.5)

project(IBR_COIN)
include(FindSGXSDK.cmake REQUIRED)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(SODIUMPP_STATIC ON)
set(CMAKE_PREFIX_PATH /usr/local/Cellar/qt/5.11.0/lib/cmake)
#find_package(Qt5Sql REQUIRED)
#find_package(Qt5Quick REQUIRED)
#find_package(sqlite3 REQUIRED)

find_package(Qt5Core REQUIRED)
find_package(Qt5Network REQUIRED)
find_package(Qt5Widgets REQUIRED)

if(APPLE)
    set(CMAKE_MACOSX_RPATH ON)
endif()

if(SODIUMPP_MACPORTS)
    include_directories("/opt/local/include")
    link_directories("/opt/local/lib")
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/sodiumpp/include)

if(SODIUMPP_STATIC)
	add_library(sodiumpp STATIC sodiumpp/sodiumpp.cpp sodiumpp/z85/z85.c sodiumpp/z85/z85_impl.cpp sodiumpp/base64.cpp)
	set_property(TARGET sodiumpp PROPERTY
			IMPORTED_LOCATION 
			libsodium.a)
else()
	add_library(sodiumpp SHARED sodiumpp/sodiumpp.cpp sodiumpp/z85/z85.c sodiumpp/z85/z85_impl.cpp sodiumpp/base64.cpp)
	
    target_link_libraries(sodiumpp  sodium)
    find_library(SODIUMLIB sodium)
endif()

set(SOURCES
    Chain/blockchain.cpp
    Chain/merkletree.cpp
    Chain/transactions.cpp
    Chain/block.cpp
    Database/database.cpp
    Network/server.cpp
    Network/client.cpp
    Network/peermanager.cpp
    Network/connection.cpp
    Interface/console.cpp
    Interface/consolehandler.cpp
    libs/sha1.cpp
    libs/sqlite3.c
    sodiumpp/crypt.cpp
    helperfunctions.cpp
    Interface/gui.cpp
    Interface/gui.ui
)

set(HONEST
    main.cpp
)
#set(FAKE tests/fakemain.cpp tests/fakeminer.cpp)
#link_directories(libs)
add_executable(${PROJECT_NAME} ${HONEST} ${SOURCES})

#add_executable(FAKEEXEC ${FAKE} ${SOURCES})

set(TB_C_FLAGS "-Wall -g")
set(TB_CXX_FLAGS "-std=c++11 -g")

set(CMAKE_C_FLAGS "${TB_C_FLAGS}")
set(CMAKE_CXX_FLAGS "${TB_C_FLAGS} ${TB_CXX_FLAGS}")

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2 -g")

target_link_libraries(${PROJECT_NAME} ${LIBAPP} sodiumpp )
target_include_directories(${PROJECT_NAME} PUBLIC ${SGXSDK_INCLUDE_DIRS})
install(DIRECTORY sodiumpp/include/sodiumpp DESTINATION include)
install_targets(/lib sodiumpp)

target_link_libraries(${PROJECT_NAME} Qt5::Network Qt5::Core Qt5::Widgets pthread dl -lstdc++ -lm "-L/ibr/y-home/y0080610/Downloads/18-ibr_ds_0/codesharing/Blockchain" sodium)
#target_link_libraries(FAKEEXEC pthread dl)
