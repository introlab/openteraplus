find_package(Qt6Core REQUIRED)
message(STATUS "Entering package directory.")

include(InstallRequiredSystemLibraries)
get_target_property(_qt6_qmake_location Qt6::qmake IMPORTED_LOCATION)

execute_process(
    COMMAND "${_qt6_qmake_location}" -query QT_INSTALL_PREFIX
    RESULT_VARIABLE return_code
    OUTPUT_VARIABLE qt6_install_prefix
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

message(STATUS "Testing package ${_qt6_qmake_location} ${qt6_install_prefix}")

# Pre-build script, used to clean up unneeded files for example
set(CPACK_PRE_BUILD_SCRIPTS ${CMAKE_CURRENT_SOURCE_DIR}/cpack-prebuild.cmake)

if (APPLE)
    cmake_policy(SET CMP0087 NEW) # Enable generator expressions in INSTALL(CODE

    set(CPACK_GENERATOR "DragNDrop")
    set(CPACK_PACKAGE_FILE_NAME OpenTeraPlus-${CPACK_PACKAGE_VERSION})
    set(CPACK_DMG_BACKGROUND_IMAGE ${CMAKE_CURRENT_SOURCE_DIR}/../client/resources/logos/LogoOpenTeraPlus.png)
    set(CPACK_DMG_FORMAT UDZO)
    set(CPACK_MONOLITHIC_INSTALL 1)
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

#    add_custom_target(mac_deploy_qt
#        COMMAND ${qt6_install_prefix}/bin/macdeployqt "$<TARGET_FILE_DIR:OpenTeraPlus>/../.." -always-overwrite -codesign=""
#        COMMENT "Running macdeployqt ..."
#        DEPENDS OpenTeraPlus
#    )

    set(MACSIGNID "")
    set(MACQTDEPLOY_COMMAND "${qt6_install_prefix}/bin/macdeployqt $<TARGET_FILE_DIR:OpenTeraPlus>/../.. -always-overwrite -verbose=1")

    # message(STATUS "********** ${MACQTDEPLOY_COMMAND}")

    #if (TARGET install)
    #    set(APPSIGNPATH ${CMAKE_INSTALL_PREFIX})
    #else()
        set(APPSIGNPATH ${CMAKE_BINARY_DIR}/_CPack_Packages/Darwin/${CPACK_GENERATOR}/${CPACK_PACKAGE_FILE_NAME})
    #endif()

    # Put special configuration file in config/TeraClientConfig.json if desired
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/config/OpenTeraPlus.json")
        message(STATUS "Using specific configuration file for packaging.")
        install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/config/OpenTeraPlus.json DESTINATION "OpenTeraPlus.app/Contents/MacOS/config")
        # set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/config/OpenTeraPlus.json PROPERTIES MACOSX_PACKAGE_LOCATION "MacOS/config")
    endif()

    install(CODE "
        execute_process(COMMAND ${MACQTDEPLOY_COMMAND} COMMAND_ECHO STDOUT)

        execute_process(COMMAND codesign --force -s ${MACSIGNID} --deep --options=runtime --timestamp --entitlements ${CMAKE_CURRENT_SOURCE_DIR}/QtWebEngineProcess.entitlements -v ${APPSIGNPATH}/OpenTeraPlus.app COMMAND_ECHO STDOUT)
        execute_process(COMMAND codesign -dvv -v -v ${APPSIGNPATH}/OpenTeraPlus.app COMMAND_ECHO STDOUT)
        execute_process(COMMAND codesign --force --verify --verbose --sign ${MACSIGNID} --entitlements ${CMAKE_CURRENT_SOURCE_DIR}/QtWebEngineProcess.entitlements --options=runtime --timestamp -v ${APPSIGNPATH}/OpenTeraPlus.app/Contents/Frameworks/QtWebEngineCore.framework/Helpers/QtWebEngineProcess.app/Contents/MacOS/QtWebEngineProcess COMMAND_ECHO STDOUT)
        execute_process(COMMAND codesign --force --verify --verbose --sign ${MACSIGNID} --options=runtime --timestamp --entitlements ${CMAKE_CURRENT_SOURCE_DIR}/QtWebEngineProcess.entitlements -v ${APPSIGNPATH}/OpenTeraPlus.app/Contents/MacOS/OpenTeraPlus COMMAND_ECHO STDOUT)
        ")
    # execute_process(COMMAND codesign -s ${MACSIGNID} --deep -v ${CMAKE_INSTALL_PREFIX}/OpenTeraPlus.app COMMAND_ECHO STDOUT)
    include(CPack REQUIRED)

    # Deploy before packaging
    # add_custom_target(mac-package DEPENDS mac_deploy_qt)

endif(APPLE)

if (WIN32)
    # Create files for installation
    add_custom_target(windows_deploy_qt
        # Collect and copy required dependencies
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_INSTALL_PREFIX}/qtdeploy
        COMMAND ${qt6_install_prefix}/bin/windeployqt6 "$<TARGET_FILE_DIR:OpenTeraPlus>/OpenTeraPlus.exe" --release --dir "${CMAKE_INSTALL_PREFIX}/qtdeploy" --no-quick-import --no-compiler-runtime
        COMMENT "Running windeployqt6 ..."
    )

    set(CPACK_PACKAGE_NAME "OpenTeraPlus")
    set(CPACK_PACKAGE_FILE_NAME OpenTeraPlus_${CPACK_PACKAGE_VERSION}_setup)
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "OpenTeraPlus - Installateur")
    set(CPACK_PACKAGE_VENDOR "CdRV / ESTRAD / INTER / IntRoLab / Université de Sherbrooke")

    set(CPACK_PACKAGE_EXECUTABLES "bin\\\\OpenTeraPlus" "OpenTeraPlus")
    set(CPACK_CREATE_DESKTOP_LINKS "OpenTeraPlus")
    set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/LICENSE.TXT")

    set(CPACK_COMPONENTS_ALL openteraplus libprotobuf qt_dist_component openssl)

    set(CPACK_COMPONENT_OPENTERAPLUS_DISPLAY_NAME "OpenTeraPlus")
    set(CPACK_COMPONENT_OPENTERAPLUS_DEPENDS libprotobuf qt_dist_component openssl)
    set(CPACK_COMPONENT_OPENTERAPLUS_REQUIRED ON)

    set(CPACK_COMPONENT_QT_DIST_COMPONENT_DISPLAY_NAME "Qt")
    set(CPACK_COMPONENT_QT_DIST_COMPONENT_REQUIRED ON)

    set(CPACK_COMPONENT_LIBPROTOBUF_DISPLAY_NAME "Protobuf")
    set(CPACK_COMPONENT_LIBPROTOBUF_REQUIRED ON)

    set(CPACK_COMPONENT_OPENSSL_DISPLAY_NAME "OpenSSL")
    set(CPACK_COMPONENT_OPENSSL_REQUIRED ON)

    # Put special configuration file in config/TeraClientConfig.txt if desired
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/config/OpenTeraPlus.json")
        # The components that will be packaged
        set(CPACK_COMPONENTS_ALL ${CPACK_COMPONENTS_ALL} special_configuration)
        set(CPACK_COMPONENT_SPECIAL_CONFIGURATION_DISPLAY_NAME "Configuration")
        install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/config/OpenTeraPlus.json DESTINATION bin/config COMPONENT special_configuration)
    endif()

    # Virtual camera driver
    # list(APPEND CPACK_NSIS_EXTRA_INSTALL_COMMANDS "ExecWait 'cmd /c $INSTDIR\\\\bin\\\\installDriver.exe'")
    # list(APPEND CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS "ExecWait 'cmd /c $INSTDIR\\\\bin\\\\uninstallDriver.exe'")

    # add SSL librariesOpenSSL\Win_x64\bin
    # file(GLOB OPENSSL_LIBRARIES "${qt6_install_prefix}/../../Tools/OpenSSL/Win_x64/bin/*.dll")
    file(GLOB OPENSSL_LIBRARIES "${CMAKE_CURRENT_SOURCE_DIR}/openssl/64bits/*.dll")
    message(STATUS "******************* ${OPENSSL_LIBRARIES} ${qt6_install_prefix} ${CPACK_PACKAGING_INSTALL_PREFIX}")
    install(FILES ${OPENSSL_LIBRARIES} DESTINATION bin COMPONENT openssl)

    # Uses NSIS packaging system on Windows
    SET(CPACK_SOURCE_GENERATOR "NSIS")

    SET(CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES64")
    SET(CPACK_PACKAGE_INSTALL_DIRECTORY "OpenTeraPlus")
    SET(CPACK_NSIS_PACKAGE_NAME "OpenTeraPlus")
    SET(CPACK_NSIS_MODIFY_PATH OFF)
    SET(CPACK_NSIS_EXECUTABLES_DIRECTORY .)
    SET(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)

    set(CPACK_NSIS_CREATE_ICONS_EXTRA
        "CreateShortCut '$DESKTOP\\\\OpenTeraPlus.lnk' '$INSTDIR\\\\bin\\\\OpenTeraPlus.exe'"
    )
    set(CPACK_NSIS_DELETE_ICONS_EXTRA
        "Delete '$DESKTOP\\\\OpenTeraPlus.lnk'"
    )

    # Removed run after install option, as this caused issue with config file (will run as admin!)
    # SET(CPACK_NSIS_MUI_FINISHPAGE_RUN "bin\\\\OpenTeraPlus.exe")

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


   # Run VC++ redistributable
   list(APPEND CPACK_NSIS_EXTRA_INSTALL_COMMANDS "ExecWait '\\\"$INSTDIR\\\\bin\\\\vc_redist.x64.exe\\\" /install /passive /norestart'")

   # The trick is to deploy Qt to a directory and then install the directory in the bin folder
   install(DIRECTORY ${CMAKE_INSTALL_PREFIX}/qtdeploy/ DESTINATION bin COMPONENT qt_dist_component)

   # Deploy before packaging
   add_custom_target(nsis-package DEPENDS windows_deploy_qt package)

   include(CPack REQUIRED)

   # Set files to install - everything in deploy folder
   #install(
   #    # DIRECTORY $<TARGET_FILE_DIR:OpenTeraPlus>/deploy/
   #    DIRECTORY ${CMAKE_INSTALL_PREFIX}
   #    DESTINATION .
   #    COMPONENT openteraclient_installer
   #)

   # install(CODE " ")



endif (WIN32)
