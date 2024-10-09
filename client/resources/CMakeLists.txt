set(TERACLIENT_RES_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "doc string")

set(translations
    translations/openteraplus_en.ts
    translations/openteraplus_fr.ts
)

add_custom_target(translation_files SOURCES ${translations})
