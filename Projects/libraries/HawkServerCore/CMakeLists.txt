cmake_minimum_required(VERSION 3.16.0 FATAL_ERROR)

project(HawkServerCore VERSION ${HM_VERSION} LANGUAGES CXX)

#====================================================================
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)    # Разрешаем автоматический экспорт всех символов
#====================================================================
include(${CONFIG_PATH}/VersionMaker.cmake)  # Подключаем функционал формирования версии
MakeVersion()                               # Формируем класс версии
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
# Формируем список используемых библиотек
set(LINCED_LIBRARYES
    HawkLog                     # Подключаем библиотеку логирования
    HawkCommon                  # Подключаем общую библиотеку
    HawkNet                     # Подключаем сетевую библиотеку
    )
#====================================================================
# Формируем список подключаемых папок
set(INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/src         # Подключаем собственный корневой каталог
    ${DIR_THIRDPARTY_PATH}/HPP/json/include # Подключаем библиотеку JSON

    ${LIBRARIES_PATH}/HawkLog/src           # Подключаем библиотеку логирования
    ${LIBRARIES_PATH}/HawkCommon/src        # Подключаем общую библиотеку
    ${LIBRARIES_PATH}/HawkNet/src           # Подключаем сетевую библиотеку
    )
#====================================================================
# Собираем динамическую библиотеку
add_library(${PROJECT_NAME} ${LIB_TYPE} ${LIB_HEADERS} ${LIB_SOURCES})
# Подключаем папки с хидерами
target_include_directories(${PROJECT_NAME} PUBLIC ${INCLUDE_DIRS})
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
