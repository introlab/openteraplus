MESSAGE ( STATUS "Buiding AkKysDriver...")

find_package(Qt5Core REQUIRED)
find_package(Qt5Concurrent REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Qml REQUIRED)

include_directories(${AVKYS_INCLUDES}
                    ./)

add_executable(installDriver install.cpp)
# qt5_use_modules(installDriver Core Concurrent Gui Qml)
target_link_libraries(installDriver Qt5::Core Qt5::Concurrent Qt5::Gui Qt5::Qml)
target_link_libraries(installDriver ${AVKYS_LIBS})


add_executable(uninstallDriver uninstall.cpp)
# qt5_use_modules(uninstallDriver Core Concurrent Gui Qml)
target_link_libraries(uninstallDriver Qt5::Core Qt5::Concurrent Qt5::Gui Qt5::Qml)
target_link_libraries(uninstallDriver ${AVKYS_LIBS})


install(TARGETS installDriver uninstallDriver DESTINATION bin)




