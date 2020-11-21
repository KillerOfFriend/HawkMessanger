# Вычисляем разрядность текущей сборки
if(${CMAKE_SIZEOF_VOID_P} STREQUAL "4")
    set(BUILD_BITNESS "x32")
else()
    set(BUILD_BITNESS "x64")
endif()

# Формируем суфикс сборки на основе параметров компилятора
set(COMPILER_SUFFIX ${CMAKE_CXX_COMPILER_ID}/${CMAKE_BUILD_TYPE}/${BUILD_BITNESS})
# Формируем путь к папке сборки
set(BUILD_PATH ${PROJECT_ROOT_PATH}/Build/${COMPILER_SUFFIX})