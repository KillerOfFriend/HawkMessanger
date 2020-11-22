cmake_minimum_required(VERSION 3.16.0 FATAL_ERROR)

project(HawkServerCore VERSION 1.0.0 LANGUAGES CXX)

#====================================================================
set(CMAKE_INCLUDE_CURRENT_DIR ON)           # Разрешаем работу с собственной папкой
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
    HawkCommon
    HawkLog

    # Qt6::Core подтягивается от HawkCommon
    )
#====================================================================
# Формируем список подключаемых папок
set(INCLUDE_DIRS
    ${DIR_THIRDPARTY_PATH}/HPP/json/include # Подключаем библиотеку JSON
    ${LIBRARIES_PATH}/HawkCommon/src        # Подключаем общую библиотеку
    ${LIBRARIES_PATH}/HawkLog/src           # Подключаем библиотеку логирования
    )
#====================================================================
# Собираем динамическую библиотеку
add_library(${PROJECT_NAME} ${LIB_TYPE} ${LIB_HEADERS} ${LIB_SOURCES})
# Подключаем папки с хидерами
target_include_directories(${PROJECT_NAME} PUBLIC ${INCLUDE_DIRS})
# Линкуем библиотеки
target_link_libraries(${PROJECT_NAME} PUBLIC ${LINCED_LIBRARYES})
#====================================================================
