foreach(argument PROGRAM SOURCE EXPECTED_OUTPUT EXPECTED_ERROR EXPECTED_EXIT_CODE)
    if(NOT DEFINED ${argument})
        message(FATAL_ERROR "Missing test argument: ${argument}")
    endif()
endforeach()

execute_process(
    COMMAND "${PROGRAM}" "${SOURCE}" ${OPTIONS}
    RESULT_VARIABLE exitCode
    OUTPUT_VARIABLE output
    ERROR_VARIABLE error
    TIMEOUT 5
)

file(READ "${EXPECTED_OUTPUT}" expectedOutput)
file(READ "${EXPECTED_ERROR}" expectedError)

if(NOT "${exitCode}" STREQUAL "${EXPECTED_EXIT_CODE}")
    message(FATAL_ERROR
        "Expected exit code ${EXPECTED_EXIT_CODE}, got ${exitCode}\n${error}"
    )
endif()

if(NOT "${output}" STREQUAL "${expectedOutput}")
    message(FATAL_ERROR
        "Unexpected output\nExpected:\n${expectedOutput}\nActual:\n${output}"
    )
endif()

if(NOT "${error}" STREQUAL "${expectedError}")
    message(FATAL_ERROR
        "Unexpected error\nExpected:\n${expectedError}\nActual:\n${error}"
    )
endif()
