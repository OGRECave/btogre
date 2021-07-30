find_package(SWIG 3.0.8)
find_package(PythonInterp)
find_package(PythonLibs)

if(NOT PYTHONLIBS_FOUND OR NOT SWIG_FOUND)
    return()
endif()

include(${SWIG_USE_FILE})
include_directories(${PYTHON_INCLUDE_PATH} ${OGRE_INCLUDE_DIRS})

set(CMAKE_SWIG_FLAGS -w401,314 -builtin -Dfinal)

set_source_files_properties(${CMAKE_SOURCE_DIR}/include/BtOgre.i PROPERTIES CPLUSPLUS ON)

if(MSVC)
	add_definitions(/wd4101 /wd4102)
else()
    add_definitions(-Wno-cast-qual -Wno-shadow -Wno-maybe-uninitialized -Wno-unused-label -Wno-unused-but-set-variable -Wno-missing-declarations)
endif()

swig_add_module(BtOgre python ${CMAKE_SOURCE_DIR}/include/BtOgre.i)
swig_link_libraries(BtOgre BtOgre OgreMain ${PYTHON_LIBRARIES})
set_target_properties(${SWIG_MODULE_BtOgre_REAL_NAME} PROPERTIES DEBUG_POSTFIX "")

# install
set(PYTHON_SITE_PACKAGES lib/python${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}/dist-packages/)
install(TARGETS ${SWIG_MODULE_BtOgre_REAL_NAME} LIBRARY DESTINATION ${PYTHON_SITE_PACKAGES})
install(FILES ${PROJECT_BINARY_DIR}/python/BtOgre.py DESTINATION ${PYTHON_SITE_PACKAGES})
