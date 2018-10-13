if (NOT WIN32)
	set(THREADS_PREFER_PTHREAD_FLAG ON)
endif()
find_package(Threads REQUIRED)

include_directories(${QueueWorld_SOURCE_DIR}/include)

file(GLOB_RECURSE test_sources
	*.cpp *.h
	../include/*.h
	../src/*.cpp
	)

add_executable(QueueWorld_test QwTestMain.cpp ${test_sources})

if (("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang"))

  # -Wno-exit-time-destructors is because Catch TEST macros introduce exit-time dtors.
  set_target_properties(QueueWorld_test PROPERTIES COMPILE_FLAGS "-Wno-exit-time-destructors" )

endif()

target_compile_definitions(QueueWorld_test PUBLIC CATCH_CONFIG_NO_VARIADIC_MACROS=1) # should compile with C++03 and Catch doesn't detect language version
target_link_libraries(QueueWorld_test Threads::Threads)

add_test(NAME QueueWorld_test COMMAND QueueWorld_test)
