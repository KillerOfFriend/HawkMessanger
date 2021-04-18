cmake_minimum_required(VERSION 3.16.0 FATAL_ERROR)

project(HawkCommon_Tests VERSION ${HM_VERSION} LANGUAGES CXX)

#====================================================================
set(EXECUTABLE_OUTPUT_PATH ${TETSTS_OUT_PATH}) # Переопределяем путь к папке сборки тестов
#====================================================================
# Формируем список используемых библиотек
set(TESTS_LINCED_LIBRARYES
    HawkErrors      # Подключаем тестируемую библиотеку
    gtest           # Подключаем библиотеку Google тестов
    )
#====================================================================
# Формируем список подключаемых директорий
set(TESTS_INCLUDE_DIRS
    ${LIBRARIES_PATH}/HawkErrors/src
    ${DIR_THIRDPARTY_PATH}/Building/googletest/googletest/include/
    )
#====================================================================
set(ER_Test1 HawkErrors_ErrorCodeTest)
add_executable(${ER_Test1} ${CMAKE_CURRENT_SOURCE_DIR}/error_code/main.cpp)
target_include_directories(${ER_Test1} PRIVATE ${TESTS_INCLUDE_DIRS})
target_link_libraries(${ER_Test1} PRIVATE ${TESTS_LINCED_LIBRARYES})

add_test(NAME HawkErrors_Test1 COMMAND ErrorCodeTest)
#====================================================================
set(ER_Test2 HawkErrors_SystemErrorEx)
add_executable(${ER_Test2} ${CMAKE_CURRENT_SOURCE_DIR}/SystemErrorEx/main.cpp)
target_include_directories(${ER_Test2} PRIVATE ${TESTS_INCLUDE_DIRS})
target_link_libraries(${ER_Test2} PRIVATE ${TESTS_LINCED_LIBRARYES})

add_test(NAME HawkErrors_Test2 COMMAND SystemErrorEx)
#====================================================================
set(ER_Test3 HawkErrors_DataStorageError)
add_executable(${ER_Test3} ${CMAKE_CURRENT_SOURCE_DIR}/DataStorageError/main.cpp)
target_include_directories(${ER_Test3} PRIVATE ${TESTS_INCLUDE_DIRS})
target_link_libraries(${ER_Test3} PRIVATE ${TESTS_LINCED_LIBRARYES})

add_test(NAME HawkErrors_Test3 COMMAND DataStorageError)
#====================================================================
