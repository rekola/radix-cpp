cmake_minimum_required(VERSION 3.10)

# set the project name
project(test VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Set default build mode
if(NOT CMAKE_BUILD_TYPE)
  message(WARNING "CMAKE_BUILD_TYPE not set; setting to Release")
  set(CMAKE_BUILD_TYPE "Release")
endif()

IF (MSVC)

else()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native -O3 -Wall -Wsuggest-override -Werror=return-local-addr -Werror=multichar -Werror=enum-compare -Werror=return-type -Werror=conversion-null -Werror=parentheses -Werror=address -Werror=trigraphs -Werror=pointer-arith -Werror=write-strings -Werror=pessimizing-move -Wuninitialized -Wno-unknown-pragmas -Werror=switch -Werror=format -Werror=non-virtual-dtor -Werror=cast-qual -Wconversion -Wsign-conversion -Wsign-promo -I include")
	# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg")
	# set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pg")
endif()

# add the executable
add_executable(test test.cpp)

target_include_directories(test PRIVATE include)
