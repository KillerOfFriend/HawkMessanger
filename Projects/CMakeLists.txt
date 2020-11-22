cmake_minimum_required(VERSION 3.16.0 FATAL_ERROR)

project(HawkMessanger VERSION 1.0.0 LANGUAGES CXX)
#====================================================================
# Задаём опции проекта
option(BUILD_HAWK_TESTS "Собрать тесты модулей системы?" ON)
option(USE_PVS_STUDIO_STATIC_ANALYSIS "Использовать статический анализатор PVS-STUDIO?" ON)
#====================================================================
# Задаём настройик компиляции
set(CMAKE_CXX_STANDARD 17)              # Задаём стандарт c++17
set(CMAKE_CXX_EXTENSIONS OFF)           # Не использовать расширения GNU
set(CMAKE_CXX_STANDARD_REQUIRED ON)     # Запретить автоматическое занижение стандарта (если компилятор не соответствует)
#====================================================================
set(PROJECT_ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR})          # Указываем путь к корневой папке проекта
set(CONFIG_PATH ${PROJECT_ROOT_PATH}/config)                # Указываем путь к файлам конфигураций
set(DIR_THIRDPARTY_PATH ${PROJECT_ROOT_PATH}/3rdParty)      # Указываем путь к папке со сторонними решениями
set(LIBRARIES_PATH ${PROJECT_ROOT_PATH}/libraries)          # Указываем путь к папке с библиотеками
#====================================================================
include(${CONFIG_PATH}/InitUtils.cmake)                     # Подключаем утилиты инициализации
#====================================================================
set(LIBRARY_OUTPUT_PATH "${BUILD_PATH}/lib")                # Указываем путь папке сборки библиотек
set(EXECUTABLE_OUTPUT_PATH "${BUILD_PATH}/bin")             # Указываем путь к папке сборки бинарей
set(3RD_PARTY_OUT_PATH "${BUILD_PATH}/3rd_party")           # Указываем путь сборки сторонних решений
set(TETSTS_OUT_PATH "${BUILD_PATH}/tests")                  # Указываем путь сборки тестов
set(LIB_TYPE SHARED)                                        # Задаём тип собираемых библиотек (SHARED\STATIC)
#====================================================================
if (BUILD_HAWK_TESTS)
    enable_testing()                                        # Включаем поддержку тестов
endif(BUILD_HAWK_TESTS)
#====================================================================
if (USE_PVS_STUDIO_STATIC_ANALYSIS)
#    set(EXCLUDE_PATH /opt/hisi-linux/x86-arm/aarch64-himix100-linux) # Игнорируем подключения компилятора при анализе
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)                   # Разрешить генерацию JSON содержащий точные вызовы компилятора для всех единиц перевода проекта в машиночитаемой форме
    include(${CONFIG_PATH}/PVS-Studio.cmake)                # Подключаем PVS_Studio
endif(USE_PVS_STUDIO_STATIC_ANALYSIS)
#====================================================================
add_subdirectory(3rdParty)                                  # Собираем сторонние зависимости
add_subdirectory(libraries)                                 # Собираем библиотеки
add_subdirectory(modules)                                   # Собираем модули
#====================================================================
