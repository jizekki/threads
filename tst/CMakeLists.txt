set(example-src example.c)

enable_testing()

add_custom_target(pthreads)

# Allow using `make check` to run the tests
add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure)

add_custom_target(valgrind)

foreach (file ${executables})
	add_executable(${file} ${file}.c)
	target_link_libraries(${file} threads)
	add_executable(${file}-pthread ${file}.c)

	add_test(NAME ${file}_test
	         COMMAND ${file} 10 20 WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
	add_dependencies(check ${file})

	add_custom_target(${file}_valgrind
	                  DEPENDS ${file}
	                  COMMAND valgrind -v -q --leak-check=full --show-reachable=yes --track-origins=yes ./${file} 10 20)
	add_dependencies(valgrind ${file}_valgrind)

	# Allow using `make pthreads` to compile all the tests using pthread
	add_dependencies(pthreads ${file}-pthread)
	target_compile_options(${file}-pthread PRIVATE -DUSE_PTHREAD)
	target_link_libraries(${file}-pthread pthread)
endforeach ()
install(TARGETS ${executables}
        DESTINATION ${INSTALL_DIR}/bin)
