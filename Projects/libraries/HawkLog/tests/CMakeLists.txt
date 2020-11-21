cmake_minimum_required(VERSION 3.16.0 FATAL_ERROR)

project(HawkLog_Tests)

#====================================================================
set(EXECUTABLE_OUTPUT_PATH ${TETSTS_OUT_PATH}) # Переопределяем путь к папке сборки тестов
#====================================================================
# Формируем список используемых библиотек
set(LINCED_LIBRARYES
    HawkLog     # Подключаем тестируемую библиотеку
    gtest       # Подключаем библиотеку Google тестов
    )
#====================================================================
# Формируем список подключаемых папок
set(INCLUDE_DIRS
    ${LIBRARIES_PATH}/HawkLog/src
    ${DIR_THIRDPARTY_PATH}/Building/googletest/googletest/include/
    )
#====================================================================
add_executable(HawkLog_CatchSegfoult_Test ${CMAKE_CURRENT_SOURCE_DIR}/CatchSegfoult/main.cpp)
target_include_directories(HawkLog_CatchSegfoult_Test PRIVATE ${INCLUDE_DIRS})
target_link_libraries(HawkLog_CatchSegfoult_Test PRIVATE ${LINCED_LIBRARYES})

add_test(NAME Test1 COMMAND HawkLog_CatchSegfoult_Test)
#====================================================================
