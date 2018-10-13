cmake_minimum_required(VERSION 2.8.11)
# CMP0054: Only interpret if() arguments as variables or keywords when unquoted.
cmake_policy(SET CMP0054 NEW)

project(QueueWorld)

enable_testing()

set(CMAKE_CXX_FLAGS "")

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  # using Clang
  EXECUTE_PROCESS( COMMAND ${CMAKE_CXX_COMPILER} --version OUTPUT_VARIABLE clang_full_version_string )
  string (REGEX REPLACE ".*clang version ([0-9]+\\.[0-9]+).*" "\\1" CLANG_VERSION_STRING ${clang_full_version_string})

  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Werror -Weverything -Wno-c++98-compat -Wno-unused-macros -Wno-unused-const-variable -Wno-old-style-cast -Wno-double-promotion -Wno-reserved-id-macro -Wno-undef -Wno-padded -Wno-invalid-offsetof -Wno-deprecated-dynamic-exception-spec -Wno-zero-as-null-pointer-constant -Wno-deprecated")

  if (CLANG_VERSION_STRING VERSION_LESS 6.0.0)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-extended-offsetof")
    # -Wno-extended-offsetof is apparently not present in clang 6
    # see http://www.open-std.org/jtc1/sc22/wg14/www/docs/n2031.htm
    # shown as defect report 496 here: http://www.open-std.org/jtc1/sc22/wg14/www/abq/compendium.htm
    # the warnings were removed from clang in November 2017: https://reviews.llvm.org/D40267
    # here's an example of clang 5 showing the warnings and clang 6 not: https://godbolt.org/z/3ZnxQC
  endif()

elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  # using GCC
  set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -std=c++11 -Werror -Wall -Wextra -pedantic")

elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  # using Visual Studio C++
  # /EHsc enable exceptions (needed by Catch unit testing framework, not by QueueWorld)
  # /W4 maximum warning level
  # /WX treat warnings as errors
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++11 /EHsc /W4")

endif()

add_subdirectory(tests)

