# Вычисляем разрядность текущей сборки
if(${CMAKE_SIZEOF_VOID_P} STREQUAL "4")
    set(BUILD_BITNESS "x32")
else()
    set(BUILD_BITNESS "x64")
endif()
