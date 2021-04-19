
add_subdirectory(ipc)
add_subdirectory(BaseClasses)

#Not yet working...
#add_subdirectory(VirtualCameraSource)
if(WIN32)
    add_subdirectory(dist)
endif(WIN32)
