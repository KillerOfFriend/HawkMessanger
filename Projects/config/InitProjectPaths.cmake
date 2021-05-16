include(${CONFIG_PATH}/InitUtils.cmake)                     # Подключаем утилиты инициализации (для BUILD_BITNESS)

#====================================================================
# Формируем глобальные пути проекта
#====================================================================
set(DIR_THIRDPARTY_PATH     ${PROJECT_ROOT_PATH}/3rdParty)                          # Указываем путь к папке со сторонними решениями
set(LIBRARIES_PATH          ${PROJECT_ROOT_PATH}/libraries)                         # Указываем путь к папке с библиотеками

#====================================================================
# Формируем пути сборки
#====================================================================
set(COMPILER_SUFFIX ${CMAKE_CXX_COMPILER_ID}/${CMAKE_BUILD_TYPE}/${BUILD_BITNESS})  # Формируем суфикс сборки на основе параметров компилятора
set(BUILD_PATH ${PROJECT_ROOT_PATH}/Build/${COMPILER_SUFFIX})                       # Формируем путь к папке сборки

set(LIBRARY_OUTPUT_PATH     "${BUILD_PATH}/lib")                                    # Указываем путь папке сборки библиотек
set(EXECUTABLE_OUTPUT_PATH  "${BUILD_PATH}/bin")                                    # Указываем путь к папке сборки бинарей
set(3RD_PARTY_OUT_PATH      "${BUILD_PATH}/3rd_party")                              # Указываем путь сборки сторонних решений
set(TETSTS_OUT_PATH         "${BUILD_PATH}/tests")                                  # Указываем путь сборки тестов

