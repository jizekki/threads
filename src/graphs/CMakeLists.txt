# Find Python and Virtualenv
set(PYTHON_VERSION 3.6)

find_package(PythonInterp ${PYTHON_VERSION} REQUIRED)
find_program(VIRTUALENV virtualenv)

if (NOT VIRTUALENV)
	set(venv_enable false)
	set(pip_require "")
	message("Could not find `virtualenv` in PATH")
	set(PYTHON_CALL python3)
else ()
	set(venv_enable true)
	set(pip_require "pip-requirements")
	set(VIRTUALENV ${VIRTUALENV} -p ${PYTHON_EXECUTABLE})

	# Generate the virtualenv and ensure it's up to date.
	add_custom_command(
			OUTPUT venv
			COMMAND ${VIRTUALENV} venv
	)

	add_custom_command(OUTPUT venv.stamp
	                   DEPENDS venv requirements.txt
	                   COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/requirements.txt requirements.txt
	                   COMMAND ./venv/bin/pip install -r requirements.txt --upgrade
	                   )

	add_custom_target(pip-requirements
	                  DEPENDS venv.stamp
	                  SOURCES requirements.txt
	                  )

	set(PYTHON_CALL ${CMAKE_CURRENT_BINARY_DIR}/venv/bin/python)
endif ()

# Graphs creation
set(GRAPH_DIR "${CMAKE_BINARY_DIR}/graphs/") # set path from build dir


add_custom_command(OUTPUT graph_dir
                   COMMAND echo "Using ${GRAPH_DIR} to store graphs"
                   COMMAND ${CMAKE_COMMAND} -E make_directory ${GRAPH_DIR}
                   )

add_custom_target(graph
                  DEPENDS graph_dir)

foreach (file ${executables})
	set(path ${TST_BUILD_PATH}/${file})
	add_custom_target(${file}_graph
	                  DEPENDS ${file} ${file}-pthread ${pip_require} graph_dir
	                  COMMAND ${PYTHON_CALL} ${CMAKE_CURRENT_SOURCE_DIR}/graph.py ${TST_BUILD_PATH}/${file}
	                  ${GRAPH_DIR}/${file}.png
	                  )
	add_dependencies(graph ${file}_graph)
endforeach ()