MESSAGE ( STATUS "Buiding AkCommons...")

find_package(Qt6Core REQUIRED)
find_package(Qt6Concurrent REQUIRED)
find_package(Qt6Gui REQUIRED)
find_package(Qt6Qml REQUIRED)


add_definitions(-DQT_DEPRECATED_WARNINGS -DAKCOMMONS_LIBRARY)

include_directories(src)

set (libavkys_headers
    src/ak.h
    src/akutils.h
    src/akcaps.h
    src/akcommons.h
    src/akelement.h
    src/akfrac.h
    src/akpacket.h
    src/akplugin.h
    src/akmultimediasourceelement.h
    src/akvideocaps.h
    src/akaudiocaps.h
    src/akvideopacket.h
    src/akaudiopacket.h
)

set (libavkys_srcs
    src/ak.cpp
    src/akutils.cpp
    src/akcaps.cpp
    src/akelement.cpp
    src/akfrac.cpp
    src/akpacket.cpp
    src/akplugin.cpp
    src/akmultimediasourceelement.cpp
    src/akvideocaps.cpp
    src/akaudiocaps.cpp
    src/akvideopacket.cpp
    src/akaudiopacket.cpp
)

#This will generate moc_* for Qt
QT_WRAP_CPP(libavkys_moc_srcs ${libavkys_headers})


add_library(AvKys SHARED ${libavkys_srcs} ${libavkys_headers} ${libavkys_moc_srcs})
# qt5_use_modules(AvKys Core Concurrent Gui Qml)
target_link_libraries(AvKys Qt6::Core Qt6::Concurrent Qt6::Gui Qt6::Qml)

set(AVKYS_LIBS AvKys CACHE INTERNAL "doc string")
set(AVKYS_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/src CACHE INTERNAL "doc string")

install(TARGETS AvKys RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

