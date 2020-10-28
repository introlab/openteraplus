find_package(Qt5Core REQUIRED)
find_package(Qt5LinguistTools REQUIRED)
find_package(Qt5Xml REQUIRED)
find_package(Qt5Network REQUIRED)

set(PTZDrivers_headers
    ICameraDriver.h
    CameraUtilities.h
    Vivotek8111.h
)

set(PTZDrivers_srcs
    #Empty for now
    #ICameraDriver.cpp
    CameraUtilities.cpp
    Vivotek8111.cpp
)

SET(PTZDrivers_qrcs

)

include_directories(
    ${OPENTERA_SHARED_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)



# generate rules for building source files from the resources
QT5_ADD_RESOURCES(ptz_drivers_qrc ${PTZDrivers_qrcs})

#This will generate moc_* for Qt
qt5_wrap_cpp(ptz_drivers_moc_srcs ${PTZDrivers_headers})


add_library(PTZDrivers STATIC ${PTZDrivers_headers} ${PTZDrivers_srcs} ${ptz_drivers_qrc} ${ptz_drivers_moc_srcs})
target_link_libraries(PTZDrivers Qt5::Core Qt5::Xml Qt5::Network)

set(PTZ_DRIVERS_LIBS PTZDrivers CACHE INTERNAL "doc string")
set(PTZ_DRIVERS_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "doc string")

# Install
install(TARGETS PTZDrivers
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

