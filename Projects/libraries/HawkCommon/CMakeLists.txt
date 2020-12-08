cmake_minimum_required(VERSION 3.16.0 FATAL_ERROR)

project(HawkCommon VERSION 1.0.0 LANGUAGES CXX)

#====================================================================
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)    # Разрешаем автоматический экспорт всех символов
#====================================================================
# Ищим хидеры
file(GLOB_RECURSE LIB_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h
)
#====================================================================
# Ищим сорцы
file(GLOB_RECURSE LIB_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)
#====================================================================
# Ищим используемые библиотеки
find_package(Qt6 COMPONENTS Core REQUIRED) # Ищим библиотеки QT
#====================================================================
# Автоматизируем Qt сборку
set(CMAKE_AUTOMOC ON)       # Автоматическая компиляция MOC
#====================================================================
# Формируем список используемых библиотек
set(LINCED_LIBRARYES
    Qt6::Core
    )
#====================================================================
# Формируем список подключаемых папок
#set(INCLUDE_DIRS
#
#    )
#====================================================================
# Собираем динамическую библиотеку
add_library(${PROJECT_NAME} ${LIB_TYPE} ${LIB_HEADERS} ${LIB_SOURCES})
# Подключаем папки с хидерами
#target_include_directories(${PROJECT_NAME} PUBLIC ${INCLUDE_DIRS})
# Линкуем библиотеки
target_link_libraries(${PROJECT_NAME} PUBLIC ${LINCED_LIBRARYES})
#====================================================================
if (BUILD_HAWK_TESTS)
    add_subdirectory(tests) # Собираем тесты
endif(BUILD_HAWK_TESTS)
#====================================================================
# Статический анализ
if (USE_PVS_STUDIO_STATIC_ANALYSIS)
    pvs_studio_add_target(TARGET ${PROJECT_NAME}.analyze ALL
                          OUTPUT FORMAT errorfile
                          ANALYZE ${PROJECT_NAME}
                          MODE GA:1,2 OP
                          LOG target.err)
#                          ARGS -e ${EXCLUDE_PATH})
endif()
#====================================================================
