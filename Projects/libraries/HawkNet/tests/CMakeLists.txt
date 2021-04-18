cmake_minimum_required(VERSION 3.16.0 FATAL_ERROR)

project(HawkNet_Tests VERSION ${HM_VERSION} LANGUAGES CXX)

#====================================================================
set(EXECUTABLE_OUTPUT_PATH ${TETSTS_OUT_PATH}) # Переопределяем путь к папке сборки тестов
#====================================================================
# Формируем список используемых библиотек
set(TESTS_LINCED_LIBRARYES
    HawkNet         # Подключаем тестируемую библиотеку
    gtest           # Подключаем библиотеку Google тестов
    )
#====================================================================
# Формируем список подключаемых директорий
set(TESTS_INCLUDE_DIRS
    ${LIBRARIES_PATH}/HawkErrors/src
    ${DIR_THIRDPARTY_PATH}/Building/googletest/googletest/include/
    )
#====================================================================
set(NT_Test1 HawkNet_QtSimpleNetTest)
add_executable(${NT_Test1} ${CMAKE_CURRENT_SOURCE_DIR}/QtSimpleNet/main.cpp)
target_include_directories(${NT_Test1} PRIVATE ${TESTS_INCLUDE_DIRS})
target_link_libraries(${NT_Test1} PRIVATE ${TESTS_LINCED_LIBRARYES})

add_test(NAME ${PROJECT_NAME}1 COMMAND QtSimpleNet)
#====================================================================

