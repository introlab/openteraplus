find_package(Qt5Core REQUIRED)
message(STATUS "Entering package directory.")



include(InstallRequiredSystemLibraries)
get_target_property(_qt5_qmake_location Qt5::qmake IMPORTED_LOCATION)

execute_process(
    COMMAND "${_qt5_qmake_location}" -query QT_INSTALL_PREFIX
    RESULT_VARIABLE return_code
    OUTPUT_VARIABLE qt5_install_prefix
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

message(STATUS "Testing package ${_qt5_qmake_location} ${qt5_install_prefix}")

if (APPLE)
    add_custom_target(package
        COMMAND ${qt5_install_prefix}/bin/macdeployqt "$<TARGET_FILE_DIR:TeraClient>/../.." -always-overwrite
        COMMENT "Running macdeplotqt ..."
        DEPENDS TeraClient
    )

endif(APPLE)

if (WIN32)
    set(CPACK_PACKAGE_NAME "OpenTeraClient")
    set(CPACK_PACKAGE_FILE_NAME OpenTeraPlus_${CPACK_PACKAGE_VERSION}_setup)
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "OpenTera - Client - Installateur")
    set(CPACK_PACKAGE_VENDOR "Regroupement INTER")


    set(CPACK_PACKAGE_EXECUTABLES "TeraClient" "OpenTeraClient")
    set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/LICENSE.TXT")

    # Put special configuration file in config/TeraClientConfig.txt if desired
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/config/TeraClientConfig.txt")
        # The components that will be packaged
        set(CPACK_COMPONENTS_ALL openteraplus libprotobuf qt_dist_component special_configuration)

        #set(CPACK_COMPONENTS_ALL ${CPACK_COMPONENTS_ALL}special_configuration)
        install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/config/TeraClientConfig.txt DESTINATION bin/config/client COMPONENT special_configuration)
    else()
        # The components that will be packaged
        set(CPACK_COMPONENTS_ALL openteraplus libprotobuf qt_dist_component)
    endif()

    # Create files for installation
    add_custom_target(windows_deploy_qt
        # Collect and copy required dependencies
        # COMMAND ${CMAKE_COMMAND} -E remove_directory "${CMAKE_BINARY_DIR}/deploy"
        COMMAND ${qt5_install_prefix}/bin/windeployqt "$<TARGET_FILE_DIR:TeraClient>/TeraClient.exe" --release --dir "${CMAKE_INSTALL_PREFIX}/qtdeploy"
        # Copy executables to deploy directory
        # COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE_DIR:TeraClient>/TeraClient.exe" "$<TARGET_FILE_DIR:TeraClient>/deploy/TeraClient.exe"
        # COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE_DIR:opentera_shared>/opentera_shared.dll" "$<TARGET_FILE_DIR:TeraClient>/deploy/opentera_shared.dll"
        # Copy default configs to deploy directory
        # COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/defaults" "$<TARGET_FILE_DIR:TeraClient>/deploy"
    )

    # add SSL librariesOpenSSL\Win_x64\bin
    file(GLOB OPENSSL_LIBRARIES "${qt5_install_prefix}/../../Tools/OpenSSL/Win_x64/bin/*.dll")
    message(STATUS "******************* ${OPENSSL_LIBRARIES} ${qt5_install_prefix} ${CPACK_PACKAGING_INSTALL_PREFIX}")
    install(FILES ${OPENSSL_LIBRARIES} DESTINATION bin)


    # Uses NSIS packaging system on Windows
    SET(CPACK_SOURCE_GENERATOR "NSIS")


    SET(CPACK_NSIS_INSTALL_ROOT "C:\\\\INTER")
    SET(CPACK_PACKAGE_INSTALL_DIRECTORY "OpenTeraClient")
    SET(CPACK_NSIS_PACKAGE_NAME "OpenTera - Client v${CPACK_PACKAGE_VERSION}")
    SET(CPACK_NSIS_MODIFY_PATH OFF)
    SET(CPACK_NSIS_EXECUTABLES_DIRECTORY .)
    SET(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)

    SET(CPACK_NSIS_URL_INFO_ABOUT "https://github.com/introlab/openteraplus")
    SET(CPACK_NSIS_CONTACT "simon.briere@usherbrooke.ca")


    SET(CPACK_NSIS_DEFINES "!define MUI_STARTMENUPAGE_DEFAULTFOLDER \\\"OpenTeraClient\\\"
                                    VIProductVersion ${CPACK_PACKAGE_VERSION}.0
                                    VIAddVersionKey ProductName \\\"${CPACK_PACKAGE_NAME}\\\"
                                    VIAddVersionKey FileDescription \\\"${CPACK_PACKAGE_DESCRIPTION_SUMMARY}\\\"
                                    VIAddVersionKey Comments \\\"Installateur de ${CPACK_PACKAGE_NAME}\\\"
                                    VIAddVersionKey CompanyName \\\"${CPACK_PACKAGE_VENDOR}\\\"
                                    VIAddVersionKey LegalCopyright \\\"${CPACK_PACKAGE_VENDOR}\\\"
                                    VIAddVersionKey InternalName \\\"${CPACK_PACKAGE_NAME} Installer\\\"
                                    VIAddVersionKey LegalTrademarks \\\"\\\"
                                    VIAddVersionKey OriginalFilename \\\"${CPACK_PACKAGE_FILE_NAME}.exe\\\"
                                    VIAddVersionKey FileVersion ${CPACK_PACKAGE_VERSION}
                                    VIAddVersionKey ProductVersion ${CPACK_PACKAGE_VERSION}")


   include(CPack REQUIRED)


   # Run VC++ redistributable
   list(APPEND CPACK_NSIS_EXTRA_INSTALL_COMMANDS "ExecWait 'cmd /c $INSTDIR\\\\VC_redist.x64.exe /install /passive /norestart'")

   # Set files to install - everything in deploy folder
   #install(
   #    # DIRECTORY $<TARGET_FILE_DIR:TeraClient>/deploy/
   #    DIRECTORY ${CMAKE_INSTALL_PREFIX}
   #    DESTINATION .
   #    COMPONENT openteraclient_installer
   #)

   # install(CODE " ")

   # The trick is to deploy Qt to a directory and then install the directory in the bin folder
   install(DIRECTORY ${CMAKE_INSTALL_PREFIX}/qtdeploy/ DESTINATION bin COMPONENT qt_dist_component)

   # Deploy before packaging
   add_custom_target(nsis-package DEPENDS windows_deploy_qt package)


endif (WIN32)
