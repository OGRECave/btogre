cmake_minimum_required(VERSION 2.8.6)

project(BtOgre)

find_package(Bullet REQUIRED)
find_package(OGRE 1.10 REQUIRED)

option(BUILD_DEMO "build demo application" TRUE)

include_directories(
    ${PROJECT_SOURCE_DIR}/include/
    ${BULLET_INCLUDE_DIRS}
    ${OGRE_INCLUDE_DIRS}
)
link_directories(${OGRE_LIBRARY_DIRS})

add_library(BtOgre BtOgre.cpp)
target_link_libraries(BtOgre ${BULLET_LIBRARIES} ${OGRE_LIBRARIES})

INSTALL(TARGETS BtOgre
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)

if(BUILD_DEMO)
    add_executable(BtOgreTest demo/main.cpp)
    target_link_libraries(BtOgreTest BtOgre)

    configure_file(demo/data/resources.cfg.in ${CMAKE_BINARY_DIR}/resources.cfg @ONLY)

    # copy essential config files next to our binary where OGRE autodiscovers them
    file(COPY ${OGRE_CONFIG_DIR}/plugins.cfg DESTINATION ${CMAKE_BINARY_DIR})
endif()


option(BUILD_DEMO2 "build demo2 application" TRUE)

include_directories(
    ${PROJECT_SOURCE_DIR}/include/
    ${PROJECT_SOURCE_DIR}/pydemo/
    ${BULLET_INCLUDE_DIRS}
    ${OGRE_INCLUDE_DIRS}
)
link_directories(${OGRE_LIBRARY_DIRS})

add_library(btOgreLayer ${PROJECT_SOURCE_DIR}/BtOgre.cpp ${PROJECT_SOURCE_DIR}/pydemo/btOgreLayer.cpp)
target_link_libraries(btOgreLayer ${BULLET_LIBRARIES} ${OGRE_LIBRARIES})

INSTALL(TARGETS btOgreLayer
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)

if(BUILD_DEMO2)
    add_executable(btOgreLayerTest pydemo/demo2/main2.cpp)
    target_link_libraries(btOgreLayerTest btOgreLayer)

    configure_file(demo/data/resources.cfg.in ${CMAKE_BINARY_DIR}/resources.cfg @ONLY)

    # copy essential config files next to our binary where OGRE autodiscovers them
    file(COPY ${OGRE_CONFIG_DIR}/plugins.cfg DESTINATION ${CMAKE_BINARY_DIR})
endif()

# Python Bindings
option(BUILD_USER_PYTHON "build user's python bindings" ON)
if(BUILD_USER_PYTHON)
    add_subdirectory(pydemo/python)
endif()
Message("Remember to set your own bullet sdk lib in your project, BulletConfig.cmake is not enough")