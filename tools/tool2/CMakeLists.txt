project(tool2)

add_executable(
    tool2
    main.cpp
    src/GaussianHistogram.cpp
)

target_include_directories(tool2 PRIVATE
    include
    ${INIREADER_INCLUDE_DIR}
)

target_link_libraries(tool2 PRIVATE
    BeamClusterTreeReader
    ${ROOT_LIBRARIES}
)

set_target_properties(tool2 PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Copy config files to build/configs/tool2
set(CONFIG_OUTPUT_DIR ${CMAKE_BINARY_DIR}/configs/tool2)
file(MAKE_DIRECTORY ${CONFIG_OUTPUT_DIR})

file(GLOB CONFIG_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.ini"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.mac"
)

foreach(config_file ${CONFIG_FILES})
    configure_file(
        ${config_file}
        ${CONFIG_OUTPUT_DIR}/
        COPYONLY
    )
endforeach()
