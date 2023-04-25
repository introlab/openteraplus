find_package(Qt6Core REQUIRED)
find_package(Qt6Widgets REQUIRED)

if(NOT OPENTERA_WEBASSEMBLY)
    find_package(Qt6LinguistTools REQUIRED)
endif()

find_package(Qt6Xml REQUIRED)
find_package(Qt6Network REQUIRED)

set(PTZDrivers_headers
    ICameraDriver.h
    CameraUtilities.h
    Vivotek8111.h
    CamImageSettingsDialog.h
)

set(PTZDrivers_srcs
    #Empty for now
    ICameraDriver.cpp
    CameraUtilities.cpp
    Vivotek8111.cpp
    CamImageSettingsDialog.cpp
)

SET(PTZDrivers_uis
    CamImageSettingsDialog.ui
)

SET(PTZDrivers_qrcs

)

include_directories(
    ${OPENTERA_SHARED_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

#Generate .h files from the .ui files
QT6_WRAP_UI(moc_uis ${PTZDrivers_uis})


# generate rules for building source files from the resources
QT6_ADD_RESOURCES(ptz_drivers_qrc ${PTZDrivers_qrcs})

#This will generate moc_* for Qt
qt6_wrap_cpp(ptz_drivers_moc_srcs ${PTZDrivers_headers})


add_library(PTZDrivers STATIC ${PTZDrivers_headers} ${PTZDrivers_srcs} ${ptz_drivers_qrc} ${ptz_drivers_moc_srcs} ${moc_uis})
target_link_libraries(PTZDrivers Qt6::Core Qt6::Widgets Qt6::Xml Qt6::Network)

set(PTZ_DRIVERS_LIBS PTZDrivers CACHE INTERNAL "doc string")
set(PTZ_DRIVERS_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "doc string")

# Install
# install(TARGETS PTZDrivers
#    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
#    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
#    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

