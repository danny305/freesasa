

set(CMAKE_VERBOSE_MAKEFILE ON)


set(THREADS_PREFER_PTHREAD_FLAG ON)

find_package(Threads REQUIRED)

find_package(LibXml2 REQUIRED)



message(STATUS "Found libXml2: ${LibXml2_FOUND}")
message(STATUS "Found JSON-C: ${JSONC_FOUND}")


file(GLOB LIB_HEADERS *.h)
file(GLOB LIB_SRC *.c)

list(FILTER LIB_SRC EXCLUDE REGEX ".*main\.c$")
list(FILTER LIB_SRC EXCLUDE REGEX ".*example\.c$")

#TODO Add json-c functionality
# find_package(jsonc REQUIRED)
list(FILTER LIB_SRC EXCLUDE REGEX ".*json\.c$")

add_library(
    freesasa 
    STATIC
    ${LIB_HEADERS}
    ${LIB_SRC}
)

set_target_properties(
    freesasa
    PROPERTIES
    C_STANDARD 99
    CXX_STANDARD 11
    ARCHIVE_OUTPUT_DIRECTORY ${LIB_DIR}
)

target_link_libraries(
    freesasa
    PRIVATE
    ${LIBXML2_LIBRARIES}
    Threads::Threads
)

target_compile_definitions(
    freesasa 
    PUBLIC 
    "HAVE_CONFIG_H"
    "HAVE_STRING"
)


add_executable(main config.h freesasa.h main.c)

target_compile_definitions(main PUBLIC "HAVE_CONFIG_H")

set_target_properties(main
    PROPERTIES
    C_STANDARD 99
    RUNTIME_OUTPUT_DIRECTORY ${BIN_DIR}
    OUTPUT_NAME freesasa
)


target_link_libraries(main freesasa)


add_executable(example freesasa.h example.c)

# target_compile_definitions(main PUBLIC "HAVE_CONFIG_H")

set_target_properties(example
    PROPERTIES
    C_STANDARD 99
    RUNTIME_OUTPUT_DIRECTORY ${BIN_DIR} 
)

target_link_libraries(example freesasa)
