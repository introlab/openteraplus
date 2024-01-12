set(SUBMMODULE_NAME "qtscreen")
message(STATUS "Building ${PLUGIN_NAME} : ${SUBMMODULE_NAME} submodule.")



SET(LIBRARY_OUTPUT_PATH ${LIBRARY_OUTPUT_PATH}/submodules/${PLUGIN_NAME})

find_package(Qt6Core REQUIRED)
find_package(Qt6Concurrent REQUIRED)
find_package(Qt6Gui REQUIRED)
find_package(Qt6Qml REQUIRED)
find_package(Qt6Widgets REQUIRED)

include_directories(
    ${AVKYS_PLUGIN_INCLUDES}
    ./
    src
)

set (submodule_headers
    src/plugin.h
    src/qtscreendev.h
    ../screendev.h
)

set (submodule_srcs
    src/plugin.cpp
    src/qtscreendev.cpp
    ../screendev.cpp
)

set (submodule_qrc

)


#This will generate moc_* for Qt
QT_WRAP_CPP(submodule_moc_srcs ${submodule_headers})

QT6_ADD_RESOURCES(submodule_qrc_srcs ${summodule_qrc})

add_library(${PLUGIN_NAME}_${SUBMMODULE_NAME} SHARED ${submodule_srcs} ${submodule_headers} ${submodule_moc_srcs} ${submodule_qrc_srcs})
set_target_properties(${PLUGIN_NAME}_${SUBMMODULE_NAME} PROPERTIES OUTPUT_NAME "qtscreen")

# qt5_use_modules(${PLUGIN_NAME}_${SUBMMODULE_NAME} Core Concurrent Widgets Gui Qml)
target_link_libraries(${PLUGIN_NAME}_${SUBMMODULE_NAME} Qt6::Core Qt6::Concurrent Qt6::Widgets Qt6::Gui Qt6::Qml)

target_link_libraries(${PLUGIN_NAME}_${SUBMMODULE_NAME} ${AVKYS_PLUGIN_LIBRARIES})
install(TARGETS ${PLUGIN_NAME}_${SUBMMODULE_NAME} RUNTIME DESTINATION ${AVKYS_LIBRARY_INSTALL_PATH}/submodules/${PLUGIN_NAME})

