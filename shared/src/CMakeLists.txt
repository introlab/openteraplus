find_package(Qt5Core REQUIRED)
find_package(Qt5Multimedia  REQUIRED)

add_definitions(-DSHAREDLIB_LIBRARY)
if (MSVC)
    add_definitions(-DOPENTERAMESSAGES_EXPORT=__declspec\(dllimport\))
    add_definitions(
      /wd4910 # Ignore export warnings (C4910) with MSVC
      /wd4661 # Ignore no suitable definition for explicit template (C4661)
      /wd4251 # Ignore needs to have dll-interface
    )
endif(MSVC)

set(sharedlib_headers
    SharedLib.h
    Logger.h
    ConfigManager.h
    Utils.h
    SimpleCrypt.h
    data/TeraData.h
    data/TeraSessionStatus.h
    data/TeraSessionEvent.h
    data/TeraSessionCategory.h
    data/TeraPreferences.h
    data/TeraSettings.h
    data/TeraAsset.h
    data/TeraTest.h
    WebAPI.h
    ParticipantWebAPI.h
    qrcodegen.hpp
)

set(sharedlib_srcs
    Logger.cpp
    ConfigManager.cpp
    Utils.cpp
    SimpleCrypt.cpp
    data/TeraData.cpp
    data/TeraSessionStatus.cpp
    data/TeraSessionEvent.cpp
    data/TeraSessionCategory.cpp
    data/TeraPreferences.cpp
    data/TeraSettings.cpp
    data/TeraAsset.cpp
    data/TeraTest.cpp
    qrcodegen.cpp
)

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

#This will generate moc_* for Qt
qt5_wrap_cpp(sharedlib_moc_srcs ${sharedlib_headers})

#compile library
add_library(opentera_shared SHARED ${sharedlib_headers} ${sharedlib_srcs} ${sharedlib_moc_srcs})
# qt5_use_modules(opentera_shared Core Network)

set_target_properties(opentera_shared PROPERTIES
        INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/bin
        INSTALL_RPATH_USE_LINK_PATH TRUE)
target_link_libraries(opentera_shared Qt5::Core Qt5::Multimedia)
if (MSVC)
target_compile_definitions(opentera_shared PUBLIC OPENTERAMESSAGES_EXPORT=__declspec\(dllimport\))
endif(MSVC)


# Install target
install(TARGETS opentera_shared DESTINATION ${CMAKE_INSTALL_BINDIR})

set(OPENTERA_SHARED_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/data CACHE INTERNAL "doc string")
set(OPENTERA_SHARED_LIBS "opentera_shared" CACHE INTERNAL "doc string")


# add_subdirectory(tests)
