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
set(HC_Test1 HawkCommon_UserList)
add_executable(${HC_Test1} ${CMAKE_CURRENT_SOURCE_DIR}/UserList/main.cpp)
target_include_directories(${HC_Test1} PRIVATE ${TESTS_INCLUDE_DIRS})
target_link_libraries(${HC_Test1} PRIVATE ${TESTS_LINCED_LIBRARYES})

add_test(NAME HawkServerCore_Test1 COMMAND UserListTest)
#====================================================================
set(HC_Test2 HawkCommon_UserTest)
add_executable(${HC_Test2} ${CMAKE_CURRENT_SOURCE_DIR}/User/main.cpp)
target_include_directories(${HC_Test2} PRIVATE ${TESTS_INCLUDE_DIRS})
target_link_libraries(${HC_Test2} PRIVATE ${TESTS_LINCED_LIBRARYES})

add_test(NAME HawkServerCore_Test2 COMMAND UserTest)
#====================================================================
set(HC_Test3 HawkCommon_GroupTest)
add_executable(${HC_Test3} ${CMAKE_CURRENT_SOURCE_DIR}/Group/main.cpp)
target_include_directories(${HC_Test3} PRIVATE ${TESTS_INCLUDE_DIRS})
target_link_libraries(${HC_Test3} PRIVATE ${TESTS_LINCED_LIBRARYES})

add_test(NAME HawkServerCore_Test3 COMMAND GroupTest)
#====================================================================
set(HC_Test4 HawkCommon_GroupMessageTest)
add_executable(${HC_Test4} ${CMAKE_CURRENT_SOURCE_DIR}/GroupMessage/main.cpp)
target_include_directories(${HC_Test4} PRIVATE ${TESTS_INCLUDE_DIRS})
target_link_libraries(${HC_Test4} PRIVATE ${TESTS_LINCED_LIBRARYES})

add_test(NAME HawkServerCore_Test43 COMMAND GroupMessageTest)
#====================================================================
