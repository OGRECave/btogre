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