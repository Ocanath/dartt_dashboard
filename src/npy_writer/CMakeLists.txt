add_library(npy_writer STATIC
    npy_writer.cpp
)

target_include_directories(npy_writer PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
