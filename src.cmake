# add the root of this source tree to the include path
include_directories(
    ${CMAKE_SOURCE_DIR}/src ${CMAKE_BINARY_DIR}/src
)

add_compile_options(
    -DPACKAGE_NAME="${CMAKE_PROJECT_NAME}"
    -DPACKAGE_VERSION="${PROJECT_VERSION}"
)

set(
    LIBRARY_SOURCE_FILES
    src/controllers/Client.cpp
    src/controllers/CommandLineParser.cpp
    src/controllers/GenericParty.cpp
    src/controllers/Server.cpp
    src/models/CommandLineArguments.cpp
    src/models/GenericPayloadWithSize.cpp
    src/models/MessageHeader.cpp
    src/models/MessageType.cpp
    src/models/RequestSetChunkIndex.cpp
    src/models/RequestSetChunkSize.cpp
    src/models/ResponseGetChunkHash.cpp
    src/models/ResponseVersion.cpp
    src/models/ResponseVolumeInformation.cpp
    src/utils/ByteArray.cpp
    src/utils/Chunk.cpp
    src/utils/Md5.cpp
    src/utils/Volume.cpp
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
    PkgConfig::CRYPTO
)

# install binaries to the target
install(
    TARGETS
    vol-sync
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
