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
        COMMAND ${qt5_install_prefix}/bin/macdeployqt "$<TARGET_FILE_DIR:OpenTeraPlus>/../.." -always-overwrite
        COMMENT "Running macdeplotqt ..."
        DEPENDS OpenTeraPlus
    )

endif(APPLE)

if (WIN32)
    set(CPACK_PACKAGE_NAME "OpenTeraPlus")
    set(CPACK_PACKAGE_FILE_NAME OpenTeraPlus_${CPACK_PACKAGE_VERSION}_setup)
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "OpenTeraPlus - Installateur")
    set(CPACK_PACKAGE_VENDOR "CdRV / ESTRAD / IntRoLab / Université de Sherbrooke")

    set(CPACK_PACKAGE_EXECUTABLES "bin\\\\OpenTeraPlus" "OpenTeraPlus")
    set(CPACK_CREATE_DESKTOP_LINKS "OpenTeraPlus")
    set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/LICENSE.TXT")

    set(CPACK_COMPONENTS_ALL openteraplus libprotobuf qt_dist_component openssl)

    set(CPACK_COMPONENT_OPENTERAPLUS_DISPLAY_NAME "OpenTeraPlus")
    set(CPACK_COMPONENT_OPENTERAPLUS_DEPENDS libprotobuf qt_dist_component openssl)
    set(CPACK_COMPONENT_OPENTERAPLUS_REQUIRED ON)

    set(CPACK_COMPONENT_QT_DIST_COMPONENT_DISPLAY_NAME "Librairies Qt")
    set(CPACK_COMPONENT_QT_DIST_COMPONENT_REQUIRED ON)

    set(CPACK_COMPONENT_LIBPROTOBUF_DISPLAY_NAME "Librairies Protobuf")
    set(CPACK_COMPONENT_LIBPROTOBUF_REQUIRED ON)

    set(CPACK_COMPONENT_OPENSSL_DISPLAY_NAME "OpenSSL")
    set(CPACK_COMPONENT_OPENSSL_REQUIRED ON)    

    # Put special configuration file in config/TeraClientConfig.txt if desired
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/config/OpenTeraPlus.json")
        # The components that will be packaged
        set(CPACK_COMPONENTS_ALL ${CPACK_COMPONENTS_ALL} special_configuration)
        set(CPACK_COMPONENT_SPECIAL_CONFIGURATION_DISPLAY_NAME "Fichier de configuration")
        install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/config/OpenTeraPlus.json DESTINATION bin/config COMPONENT special_configuration)

    endif()

    # Create files for installation
    add_custom_target(windows_deploy_qt
        # Collect and copy required dependencies
        COMMAND ${qt5_install_prefix}/bin/windeployqt "$<TARGET_FILE_DIR:OpenTeraPlus>/OpenTeraPlus.exe" --release --dir "${CMAKE_INSTALL_PREFIX}/qtdeploy"
    )

    # add SSL librariesOpenSSL\Win_x64\bin
    file(GLOB OPENSSL_LIBRARIES "${qt5_install_prefix}/../../Tools/OpenSSL/Win_x64/bin/*.dll")
    message(STATUS "******************* ${OPENSSL_LIBRARIES} ${qt5_install_prefix} ${CPACK_PACKAGING_INSTALL_PREFIX}")
    install(FILES ${OPENSSL_LIBRARIES} DESTINATION bin COMPONENT openssl)


    # Uses NSIS packaging system on Windows
    SET(CPACK_SOURCE_GENERATOR "NSIS")


    # SET(CPACK_NSIS_INSTALL_ROOT "C:\\\\INTER")
    SET(CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES64")
    SET(CPACK_PACKAGE_INSTALL_DIRECTORY "OpenTeraPlus")
    SET(CPACK_NSIS_PACKAGE_NAME "OpenTeraPlus")
    SET(CPACK_NSIS_MODIFY_PATH OFF)
    SET(CPACK_NSIS_EXECUTABLES_DIRECTORY .)
    SET(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
    SET(CPACK_NSIS_MUI_FINISHPAGE_RUN "bin\\\\OpenTeraPlus")

    SET(CPACK_NSIS_URL_INFO_ABOUT "https://github.com/introlab/openteraplus")
    SET(CPACK_NSIS_CONTACT "simon.briere@usherbrooke.ca")


    SET(CPACK_NSIS_DEFINES "!define MUI_STARTMENUPAGE_DEFAULTFOLDER \\\"OpenTeraPlus\\\"
                                    VIProductVersion ${CPACK_PACKAGE_VERSION}.0
                                    VIAddVersionKey ProductName \\\"${CPACK_PACKAGE_NAME}\\\"
                                    VIAddVersionKey FileDescription \\\"${CPACK_PACKAGE_DESCRIPTION_SUMMARY}\\\"
                                    VIAddVersionKey Comments \\\"Installateur de ${CPACK_PACKAGE_NAME} v${CPACK_PACKAGE_VERSION}\\\"
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
   #    # DIRECTORY $<TARGET_FILE_DIR:OpenTeraPlus>/deploy/
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
