#We will only build specific plugins for now

# Base plugins
#SUBDIRS += \
#    ACapsConvert \
#    AudioDevice \
#    AudioGen \
#    Bin \
#    DesktopCapture \
#    Multiplex \
#    MultiSink \
#    MultiSrc \
#    Probe \
#    VideoCapture \
#    VirtualCamera

SET(AVKYS_PLUGIN_INCLUDES
    ${AVKYS_INCLUDES}
)

SET(AVKYS_PLUGIN_LIBRARIES
    ${AVKYS_LIBS}
)

#FFMPEG HARD CODED FOR NOW
#Useful definitions
if(APPLE)
    #Installed DLLS (from homebrew)
    FILE(GLOB FFMPEG_AV_LIBS "/usr/local/lib/libav*.dylib")
    FILE(GLOB FFMPEG_SWRESAMPLE_LIBS "/usr/local/lib/libswresample*.dylib")
    FILE(GLOB FFMPEG_SWSCALE_LIBS "/usr/local/lib/libswscale*.dylib")

    FILE(GLOB FFMPEG_DLL ${FFMPEG_AV_LIBS} ${FFMPEG_SWRESAMPLE_LIBS} ${FFMPEG_SWSCALE_LIBS})

    MESSAGE ( STATUS "FFMpeg libs = ${FFMPEG_DLL}")

    #For IN-SOURCE debug
    file(COPY ${FFMPEG_DLL} DESTINATION ${EXECUTABLE_OUTPUT_PATH})


    set(FFMPEG_INCLUDES /usr/local/include CACHE INTERNAL "doc string")
    set(FFMPEG_LIBS ${FFMPEG_DLL} CACHE INTERNAL "doc string")
endif()

if(UNIX AND NOT APPLE)
    #Installed DLLS (from packages libavcodec-dev, libavdevice-dev and dependencies)
    FILE(GLOB FFMPEG_DLL "/usr/lib/x86_64-linux-gnu/libav*.so")
    MESSAGE ( STATUS "FFMpeg libs = ${FFMPEG_DLL}")

    #For IN-SOURCE debug
    file(COPY ${FFMPEG_DLL} DESTINATION ${EXECUTABLE_OUTPUT_PATH})

    set(FFMPEG_INCLUDES /usr/local/opt/ffmpeg/include CACHE INTERNAL "doc string")
    set(FFMPEG_LIBS "-L/usr/include/x86_64-linux-gnu/libavcodec -L/usr/include/x86_64-linux-gnu/libavdevice -lavcodec -lavfilter -lavutil -lswresample -lavdevice -lavformat -lpostproc -lswscale" CACHE INTERNAL "doc string")
endif()


if(WIN32)

    #Hard coded path for now
    set(FFMPEG_PATH "C:/Libs/ffmpeg")

    FILE(GLOB FFMPEG_DLL "${FFMPEG_PATH}/bin/*.dll")
    MESSAGE ( STATUS "FFMpeg libs = ${FFMPEG_DLL}")

    #FOR SOME REASON, DLLs ARE IN THE BIN DIRECTORY...
    INSTALL(FILES ${FFMPEG_DLL} DESTINATION bin)

    #For IN-SOURCE debug
    file(COPY ${FFMPEG_DLL} DESTINATION ${EXECUTABLE_OUTPUT_PATH})

    #FOR MSVC
    set(ffmpeg_lib_names
        ${FFMPEG_PATH}/lib/avcodec.lib
        ${FFMPEG_PATH}/lib/avfilter.lib
        ${FFMPEG_PATH}/lib/avutil.lib
        ${FFMPEG_PATH}/lib/swresample.lib
        ${FFMPEG_PATH}/lib/avdevice.lib
        ${FFMPEG_PATH}/lib/avformat.lib
        ${FFMPEG_PATH}/lib/postproc.lib
        ${FFMPEG_PATH}/lib/swscale.lib
    )

    #for linking and compiling
    set(FFMPEG_INCLUDES "${FFMPEG_PATH}/include " CACHE INTERNAL "doc string")
    message ( STATUS "FFMpeg includes = ${FFMPEG_INCLUDES}")

    set(FFMPEG_LIBS "${ffmpeg_lib_names}" CACHE INTERNAL "doc string")
    message ( STATUS "FFMpeg libs = ${FFMPEG_LIBS}")

endif()

#Will output in the right directory
FILE(MAKE_DIRECTORY ${CMAKE_INSTALL_PREFIX}/bin/AvKysPlugins)
SET(LIBRARY_OUTPUT_PATH ${CMAKE_INSTALL_PREFIX}/bin/AvKysPlugins)
SET (AVKYS_LIBRARY_INSTALL_PATH ${CMAKE_INSTALL_BINDIR}/AvKysPlugins)



if(WIN32)
    add_subdirectory(MultiSrc)
    add_subdirectory(VirtualCamera)
    add_subdirectory(ACapsConvert)
    add_subdirectory(DesktopCapture)
endif(WIN32)

