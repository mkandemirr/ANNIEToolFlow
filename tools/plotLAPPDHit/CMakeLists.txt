# tools/plotBeamCluster/CMakeLists.txt
project(plotLAPPDHit)

add_executable(plotLAPPDHit main.cpp)
target_link_libraries(plotLAPPDHit PRIVATE ANNIEDataStructure ${ROOT_LIBRARIES})


set_target_properties(plotLAPPDHit PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

