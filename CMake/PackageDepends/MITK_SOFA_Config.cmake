find_package(SOFA REQUIRED CONFIG)
list(APPEND ALL_INCLUDE_DIRECTORIES ${SOFA_INCLUDE_DIRS})
list(APPEND ALL_LIBRARIES ${SOFA_LIBRARIES})
link_directories(${SOFA_LIBRARY_DIRS})
