cmake_minimum_required(VERSION 3.16.0 FATAL_ERROR)

project(HawkCommon_Tests VERSION ${HM_VERSION} LANGUAGES CXX)

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
set(HC_Test1 HawkCommon_UserInfoTest)
add_executable(${HC_Test1} ${CMAKE_CURRENT_SOURCE_DIR}/UserInfo/main.cpp)
target_include_directories(${HC_Test1} PRIVATE ${TESTS_INCLUDE_DIRS})
target_link_libraries(${HC_Test1} PRIVATE ${TESTS_LINCED_LIBRARYES})

add_test(NAME HawkServerCore_Test1 COMMAND UserInfoTest)
#====================================================================
set(HC_Test2 HawkCommon_GroupInfoTest)
add_executable(${HC_Test2} ${CMAKE_CURRENT_SOURCE_DIR}/GroupInfo/main.cpp)
target_include_directories(${HC_Test2} PRIVATE ${TESTS_INCLUDE_DIRS})
target_link_libraries(${HC_Test2} PRIVATE ${TESTS_LINCED_LIBRARYES})

add_test(NAME HawkServerCore_Test2 COMMAND GroupInfoTest)
#====================================================================
set(HC_Test3 HawkCommon_GroupMessageTest)
add_executable(${HC_Test3} ${CMAKE_CURRENT_SOURCE_DIR}/GroupMessage/main.cpp)
target_include_directories(${HC_Test3} PRIVATE ${TESTS_INCLUDE_DIRS})
target_link_libraries(${HC_Test3} PRIVATE ${TESTS_LINCED_LIBRARYES})

add_test(NAME HawkServerCore_Test3 COMMAND GroupMessageTest)
#====================================================================
set(HC_Test4 HawkCommon_UserListTest)
add_executable(${HC_Test4} ${CMAKE_CURRENT_SOURCE_DIR}/UserList/main.cpp)
target_include_directories(${HC_Test4} PRIVATE ${TESTS_INCLUDE_DIRS})
target_link_libraries(${HC_Test4} PRIVATE ${TESTS_LINCED_LIBRARYES})

add_test(NAME HawkServerCore_Test4 COMMAND UserListTest)
#====================================================================
set(HC_Test5 HawkCommon_GroupListTest)
add_executable(${HC_Test5} ${CMAKE_CURRENT_SOURCE_DIR}/GroupList/main.cpp)
target_include_directories(${HC_Test5} PRIVATE ${TESTS_INCLUDE_DIRS})
target_link_libraries(${HC_Test5} PRIVATE ${TESTS_LINCED_LIBRARYES})

add_test(NAME HawkServerCore_Test5 COMMAND GroupListTest)
#====================================================================
