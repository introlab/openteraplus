MESSAGE ( STATUS "Buiding AkKysDriver...")

find_package(Qt6Core REQUIRED)
find_package(Qt6Concurrent REQUIRED)
find_package(Qt6Gui REQUIRED)
find_package(Qt6Qml REQUIRED)

include_directories(${AVKYS_INCLUDES}
                    ./)

add_executable(installDriver install.cpp)
# qt5_use_modules(installDriver Core Concurrent Gui Qml)
target_link_libraries(installDriver Qt6::Core Qt6::Concurrent Qt6::Gui Qt6::Qml)
target_link_libraries(installDriver ${AVKYS_LIBS})


add_executable(uninstallDriver uninstall.cpp)
# qt5_use_modules(uninstallDriver Core Concurrent Gui Qml)
target_link_libraries(uninstallDriver Qt6::Core Qt6::Concurrent Qt6::Gui Qt6::Qml)
target_link_libraries(uninstallDriver ${AVKYS_LIBS})


install(TARGETS installDriver uninstallDriver DESTINATION bin)




