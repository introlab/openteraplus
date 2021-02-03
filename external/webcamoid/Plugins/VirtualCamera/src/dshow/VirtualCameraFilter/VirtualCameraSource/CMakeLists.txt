set(virtualcam_srcs
    src/vcguidef.cpp
    src/virtualcamerasource.cpp
    src/virtualcamerasourcestream.cpp
    src/colorconv.cpp
    src/imgfilters.cpp

)

set(virtualcam_private_srcs
      src/dllmain.cpp
    )

set(virtualcam_headers
    src/vcguidef.h
    src/virtualcamerasource.h
    src/virtualcamerasourcestream.h
    src/resources.h
    src/colorconv.h
    src/imgfilters.h
)

include_directories(src ${STRMBASE_INCLUDES} ${IPC_INCLUDES})

set(virtualcam_link_libs
    strmiids
    uuid
    ole32
    oleaut32
    advapi32
    user32
    winmm
    gdi32
    gdiplus
    pthread
    ipc
    strmbase
 )

set(virtualcam_supp_files
    #Just need to add this file and will be handled by the target as source and compiled with resource compiler.
    VirtualCameraSource.rc
)

add_definitions(-D__STDC_CONSTANT_MACROS -DNO_DSHOW_STRSAFE)

add_library(VirtualCameraSourcePrivate STATIC ${virtualcam_private_srcs})


add_library(VirtualCameraSource SHARED ${virtualcam_srcs} ${virtualcam_headers} ${virtualcam_supp_files})
target_link_libraries(VirtualCameraSource PRIVATE VirtualCameraSourcePrivate ${virtualcam_link_libs}  )
SET (LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
install(TARGETS VirtualCameraSource DESTINATION bin)

