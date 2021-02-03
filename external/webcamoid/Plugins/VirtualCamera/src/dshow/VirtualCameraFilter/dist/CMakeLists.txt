#This is a patch
#We will simply copy the dshow filter provided by webcamoid for now
set (dshow_filter_dll
    VirtualCameraSource.dll
    libstdc++-6.dll
    libwinpthread-1.dll
    )

#FOR SOME REASON, DLLs ARE IN THE BIN DIRECTORY...
INSTALL(FILES ${dshow_filter_dll} DESTINATION bin)

#For IN-SOURCE debug
file(COPY ${dshow_filter_dll} DESTINATION ${EXECUTABLE_OUTPUT_PATH})
