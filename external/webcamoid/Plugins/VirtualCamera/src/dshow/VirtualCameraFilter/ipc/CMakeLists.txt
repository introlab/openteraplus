set (ipc_headers
    #IPC Should be moved to static lib ?
    src/filtercommons.h
    src/mutex.h
    src/waitcondition.h
    src/ipcbridge.h
)

set (ipc_sources
    #IPC Should be moved to static lib ?
    src/mutex.cpp
    src/waitcondition.cpp
    src/ipcbridge.cpp
)

add_library(ipc STATIC ${ipc_sources} ${ipc_headers})
set(IPC_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/src CACHE INTERNAL "doc string")
