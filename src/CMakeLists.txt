set(CMAKE_VERBOSE_MAKEFILE ON)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

find_package(LibXml2 REQUIRED)
message(STATUS "Found libXml2: ${LibXml2_FOUND}")

find_package(PkgConfig)
pkg_check_modules(JSONC REQUIRED json-c)


file(GLOB LIB_HEADERS *.h)
file(GLOB LIB_SRC *.c)

# TODO when I add json-c functionality include json.c
if (${JSONC_FOUND})
    message(STATUS "Found JSON-C: ${JSONC_FOUND}")
    list(FILTER LIB_SRC EXCLUDE REGEX ".*example\.c$")
else()
    list(FILTER LIB_SRC EXCLUDE REGEX ".*(example|json)\.c$")
endif()



add_library(
    freesasa 
    STATIC
    ${LIB_HEADERS}
    ${LIB_SRC}
)

target_include_directories(
    freesasa
    PRIVATE
        ${JSONC_INCLUDE_DIRS}
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
        ${JSONC_LDFLAGS}        
        Threads::Threads
)

target_compile_definitions(
    freesasa 
    PUBLIC 
        "HAVE_CONFIG_H"
        "HAVE_STRING"
)



add_executable(main config.h freesasa.h cif.hh  cif.cc main.cc)

set_target_properties(main
    PROPERTIES
        CXX_STANDARD 14
        RUNTIME_OUTPUT_DIRECTORY ${BIN_DIR}
        OUTPUT_NAME freesasa
)

target_include_directories(main 
    PRIVATE 
    ${PROJECT_SOURCE_DIR}/third-party/gemmi/include

)

target_link_libraries(main freesasa)



add_executable(example freesasa.h example.c)

set_target_properties(example
    PROPERTIES
        C_STANDARD 99
        RUNTIME_OUTPUT_DIRECTORY ${BIN_DIR} 
)

target_link_libraries(example freesasa)
