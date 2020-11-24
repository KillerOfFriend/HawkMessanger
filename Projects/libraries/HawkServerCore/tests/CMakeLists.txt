cmake_minimum_required(VERSION 3.16.0 FATAL_ERROR)

project(HawkServerCore_Tests)

#====================================================================
set(EXECUTABLE_OUTPUT_PATH ${TETSTS_OUT_PATH}) # Переопределяем путь к папке сборки тестов
#====================================================================
# Формируем список используемых библиотек
set(TESTS_LINCED_LIBRARYES
    HawkServerCore  # Подключаем тестируемую библиотеку
    gtest           # Подключаем библиотеку Google тестов
    )
#====================================================================
# Формируем список подключаемых папок
set(TESTS_INCLUDE_DIRS
    ${LIBRARIES_PATH}/HawkServerCore/src
    ${DIR_THIRDPARTY_PATH}/Building/googletest/googletest/include/
    )
#====================================================================
set(Test1 JsonDataStorage_Test)
add_executable(${Test1} ${CMAKE_CURRENT_SOURCE_DIR}/JsonDataStorage/main.cpp)
target_include_directories(${Test1} PRIVATE ${TESTS_INCLUDE_DIRS})
target_link_libraries(${Test1} PRIVATE ${TESTS_LINCED_LIBRARYES})

add_test(NAME HawkServerCore_Test1 COMMAND JsonDataStorage)
#====================================================================
