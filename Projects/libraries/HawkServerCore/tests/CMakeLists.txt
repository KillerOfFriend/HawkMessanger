cmake_minimum_required(VERSION 3.16.0 FATAL_ERROR)

project(HawkServerCore_Tests VERSION ${HM_VERSION} LANGUAGES CXX)

#====================================================================
set(EXECUTABLE_OUTPUT_PATH ${TETSTS_OUT_PATH}) # Переопределяем путь к папке сборки тестов
#====================================================================
# Формируем список используемых библиотек
set(TESTS_LINCED_LIBRARYES
    HawkServerCore  # Подключаем тестируемую библиотеку
    gtest           # Подключаем библиотеку Google тестов
    )
#====================================================================
# Формируем список подключаемых директорий
set(TESTS_INCLUDE_DIRS
    ${LIBRARIES_PATH}/HawkServerCore/src
    ${DIR_THIRDPARTY_PATH}/Building/googletest/googletest/include/
    ${LIBRARIES_PATH}/HawkCommon/tests/TestsCommon
    ${LIBRARIES_PATH}/HawkServerCore/tests/TestsCommon
    )
#====================================================================
set(Test1 CachedMemoryDataStorage_Test)
add_executable(${Test1} ${CMAKE_CURRENT_SOURCE_DIR}/CachedMemoryDataStorage/main.cpp)
target_include_directories(${Test1} PRIVATE ${TESTS_INCLUDE_DIRS})
target_link_libraries(${Test1} PRIVATE ${TESTS_LINCED_LIBRARYES})

add_test(NAME HawkServerCore_Test1 COMMAND CachedMemoryDataStorageTest)
#====================================================================
set(Test2 JsonDataStorage_Test)
add_executable(${Test2} ${CMAKE_CURRENT_SOURCE_DIR}/JsonDataStorage/main.cpp)
target_include_directories(${Test2} PRIVATE ${TESTS_INCLUDE_DIRS})
target_link_libraries(${Test2} PRIVATE ${TESTS_LINCED_LIBRARYES})

add_test(NAME HawkServerCore_Test2 COMMAND JsonDataStorageTest)
#====================================================================
set(Test3 CombinedDataStorage_Test)
add_executable(${Test3} ${CMAKE_CURRENT_SOURCE_DIR}/CombinedDataStorage/main.cpp)
target_include_directories(${Test3} PRIVATE ${TESTS_INCLUDE_DIRS})
target_link_libraries(${Test3} PRIVATE ${TESTS_LINCED_LIBRARYES})

add_test(NAME HawkServerCore_Test3 COMMAND CombinedDataStorage_Test)
#====================================================================
set(Test4 Builder_Test)
add_executable(${Test4} ${CMAKE_CURRENT_SOURCE_DIR}/Builder/main.cpp)
target_include_directories(${Test4} PRIVATE ${TESTS_INCLUDE_DIRS})
target_link_libraries(${Test4} PRIVATE ${TESTS_LINCED_LIBRARYES})

add_test(NAME HawkServerCore_Test4 COMMAND Builder_Test)
#====================================================================
