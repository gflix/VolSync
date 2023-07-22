# add the root of this source tree to the include path
include_directories(
    ${CMAKE_SOURCE_DIR}/src ${CMAKE_BINARY_DIR}/src
)

set(
    LIBRARY_SOURCE_FILES
    src/controllers/Client.cpp
    src/controllers/CommandLineParser.cpp
    src/controllers/Server.cpp
    src/models/CommandLineArguments.cpp
)

add_executable(
    vol-sync
    src/VolSync.cpp
)

add_library(
    vol-sync-internal
    ${LIBRARY_SOURCE_FILES}
)

# extra compiler options
target_compile_options(
    vol-sync
    PRIVATE
    -Wall -Werror
)

target_link_libraries(
    vol-sync
    vol-sync-internal
)

# install binaries to the target
install(
    TARGETS
    vol-sync
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
