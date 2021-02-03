#libAvKys taken from webcamoid (18 octobre 2017)
#GPL

#TODO UPDATE THOSE VARIABLES
set(AVKYS_LIB_DIR "LIB")
set(AVKYS_COMMONS_TARGET "PLUGINS")
add_definitions(-DQT_INSTALL_QML="./" -DLIBDIR="${AVKYS_LIB_DIR}" -DCOMMONS_TARGET="${AVKYS_COMMONS_TARGET}")

add_subdirectory(Lib)
add_subdirectory(Plugins)
add_subdirectory(Driver)
