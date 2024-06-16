add_library(threads SHARED thread.c)

set_target_properties(threads PROPERTIES PUBLIC_HEADER "${CMAKE_SOURCE_DIR}/include/queue.h;${CMAKE_SOURCE_DIR}/include/thread.h")

install(TARGETS threads
	LIBRARY DESTINATION ${INSTALL_DIR}/lib
	PUBLIC_HEADER DESTINATION ${INSTALL_DIR}/include)

add_subdirectory(graphs)
