cmake_minimum_required(VERSION 3.16.0 FATAL_ERROR)

project(HawkCommon_Tests)

#====================================================================
set(EXECUTABLE_OUTPUT_PATH ${TETSTS_OUT_PATH}) # Переопределяем путь к папке сборки тестов
#====================================================================
# Формируем список используемых библиотек
set(TESTS_LINCED_LIBRARYES
    HawkCommon      # Подключаем тестируемую библиотеку
    gtest           # Подключаем библиотеку Google тестов
    )
#====================================================================
# Формируем список подключаемых директорий
set(TESTS_INCLUDE_DIRS
    ${LIBRARIES_PATH}/HawkCommon/src
    ${DIR_THIRDPARTY_PATH}/Building/googletest/googletest/include/
    ${LIBRARIES_PATH}/HawkCommon/tests/TestsCommon
    )
#====================================================================
set(HC_Test1 HawkErrors_ErrorCodeTest)
add_executable(${HC_Test1} ${CMAKE_CURRENT_SOURCE_DIR}/error_code/main.cpp)
target_include_directories(${HC_Test1} PRIVATE ${TESTS_INCLUDE_DIRS})
target_link_libraries(${HC_Test1} PRIVATE ${TESTS_LINCED_LIBRARYES})

add_test(NAME HawkErrors_Test1 COMMAND ErrorCodeTest)
#====================================================================
