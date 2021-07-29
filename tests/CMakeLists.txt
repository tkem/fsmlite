macro(fsmlite_add_test TESTNAME)
    add_executable(${TESTNAME} ${CMAKE_CURRENT_SOURCE_DIR}/${TESTNAME}.cpp)
    target_link_libraries(${TESTNAME} PRIVATE fsmlite)
    if (MSVC AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 19.14)
        target_compile_options(${TESTNAME}
            PRIVATE
                /Zc:__cplusplus
        )
    endif()
    add_test(NAME ${PROJECT_NAME}_${TESTNAME} COMMAND ${TESTNAME})
endmacro(fsmlite_add_test)

fsmlite_add_test(test_basic_row)
fsmlite_add_test(test_mem_fn_row)
fsmlite_add_test(test_notrans)
fsmlite_add_test(test_player)
fsmlite_add_test(test_recursive)
fsmlite_add_test(test_row)
fsmlite_add_test(test_scoped)
fsmlite_add_test(test_shared)
fsmlite_add_test(test_traits)
