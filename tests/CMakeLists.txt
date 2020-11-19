cmake_minimum_required(VERSION 3.12)

project(
    fsmlite_tests
    DESCRIPTION "Test application for fsmlite"
)

list(APPEND tests
    test_basic_row
    test_mem_fn_row
    test_notrans
    test_player
    test_recursive
    test_row
    test_scoped
    test_shared
    test_traits
)

macro(add_fsmlite_test NAME)
    add_executable(${NAME} ${CMAKE_CURRENT_SOURCE_DIR}/${NAME}.cpp)
    target_link_libraries(${NAME}
        PRIVATE
            fsmlite::fsmlite
    )
    if (MSVC AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 19.14)
        target_compile_options(${NAME}
            PRIVATE
                /Zc:__cplusplus
        )
    endif()
    add_test(NAME ${PROJECT_NAME}_${NAME} COMMAND ${NAME})
endmacro(add_fsmlite_test)

foreach(test ${tests})
    add_fsmlite_test(${test})
endforeach()