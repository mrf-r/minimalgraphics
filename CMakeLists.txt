
add_library(minimalgraphics OBJECT)

target_sources(minimalgraphics
    PRIVATE
        mgl.c
        5monotxt.c
        5x7mod.c
    PUBLIC
    FILE_SET HEADERS
    FILES
        mgl.h
)

target_compile_options(minimalgraphics
    PRIVATE
        -Wall -Wextra -Wpedantic
        -std=c11 -Wdouble-promotion
)

target_link_libraries(minimalgraphics PUBLIC minimalgraphics_config)
