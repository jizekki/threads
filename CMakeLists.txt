cmake_minimum_required(VERSION 3.3)
project(threads C)

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
if (STACK_PROTECT)
add_definitions(-DSTACK_PROTECT=1)
endif()

include_directories(include)

set(INSTALL_DIR ${CMAKE_SOURCE_DIR}/install)
SET(CMAKE_INSTALL_RPATH "${INSTALL_DIR}/lib") # Define the link to the lib after installing
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE) # Avoid removing link to the lib
set(executables
    01-main
    02-switch
    03-equity
    11-join
    12-join-main
    21-create-many
    22-create-many-recursive
    23-create-many-once
    31-switch-many
    32-switch-many-join
    33-switch-many-cascade
    51-fibonacci
    52-sum-table
    53-merge-sort
    61-mutex
    62-mutex
    71-preemption
    #81-deadlock
    91-infinite-recursive
    93-access-non-allocated
    )
set(TST_BUILD_PATH ${CMAKE_BINARY_DIR}/tst)

add_subdirectory(src)
add_subdirectory(tst)
