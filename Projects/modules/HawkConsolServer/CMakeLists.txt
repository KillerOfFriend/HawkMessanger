cmake_minimum_required(VERSION 3.16.0 FATAL_ERROR)

project(HawkConsolServer VERSION ${HM_VERSION} LANGUAGES CXX)

#====================================================================
set(CMAKE_INCLUDE_CURRENT_DIR ON) # Разрешаем работу с собственной папкой
#====================================================================
include(${CONFIG_PATH}/VersionMaker.cmake)  # Подключаем функционал формирования версии
MakeVersion()                               # Формируем класс версии
#====================================================================
# Ищим хидеры
file(GLOB_RECURSE BIN_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h
)
#====================================================================
# Ищим сорцы
file(GLOB_RECURSE BIN_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)
#====================================================================
# Ищим используемые библиотеки

#====================================================================
# Формируем список используемых библиотек
set(LINCED_LIBRARYES
    HawkLog
    HawkCommon
    HawkServerCore
    )
#====================================================================
# Формируем список подключаемых папок
set(INCLUDE_DIRS
    ${LIBRARIES_PATH}/HawkLog/src           # Подключаем библиотеку логирования
    ${LIBRARIES_PATH}/HawkCommon/src        # Подключаем общую библиотеку
    ${LIBRARIES_PATH}/HawkServerCore/src    # Подключаем библиотеку ядро сервера
    )
#====================================================================
# Создаём бинарь
add_executable(${PROJECT_NAME} ${BIN_HEADERS} ${BIN_SOURCES})
# Подключаем папки с хидерами
target_include_directories(${PROJECT_NAME} PRIVATE ${INCLUDE_DIRS})
# Линкуем библиотеки
target_link_libraries(${PROJECT_NAME} PRIVATE ${LINCED_LIBRARYES})
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
