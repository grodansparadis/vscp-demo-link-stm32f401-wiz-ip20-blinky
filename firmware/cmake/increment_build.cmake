# increment_build.cmake
# Run as a CMake script (-P) to increment the build number and regenerate build_number.h.
# Required variable: BUILD_OUTDIR  (passed with -DBUILD_OUTDIR=...)

set(BUILD_NUMBER_FILE "${BUILD_OUTDIR}/build_number.txt")

if(EXISTS "${BUILD_NUMBER_FILE}")
    file(READ "${BUILD_NUMBER_FILE}" BUILD_NUMBER)
    string(STRIP "${BUILD_NUMBER}" BUILD_NUMBER)
    math(EXPR BUILD_NUMBER "${BUILD_NUMBER} + 1")
else()
    set(BUILD_NUMBER 1)
endif()

file(WRITE "${BUILD_NUMBER_FILE}" "${BUILD_NUMBER}")

set(HEADER_CONTENT "")
string(APPEND HEADER_CONTENT "#ifndef BUILD_NUMBER_H\n")
string(APPEND HEADER_CONTENT "#define BUILD_NUMBER_H\n")
string(APPEND HEADER_CONTENT "\n")
string(APPEND HEADER_CONTENT "/* Auto-generated - do not edit. Incremented by cmake/increment_build.cmake */\n")
string(APPEND HEADER_CONTENT "#define THIS_FIRMWARE_BUILD_VERSION (${BUILD_NUMBER}u)\n")
string(APPEND HEADER_CONTENT "\n")
string(APPEND HEADER_CONTENT "#endif /* BUILD_NUMBER_H */\n")
file(WRITE "${BUILD_OUTDIR}/build_number.h" "${HEADER_CONTENT}")

message(STATUS "Build number: ${BUILD_NUMBER}")
