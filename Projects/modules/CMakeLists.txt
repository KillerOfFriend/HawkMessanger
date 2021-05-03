cmake_minimum_required(VERSION 3.16.0 FATAL_ERROR)

project(Modules)

#====================================================================
if (BUILD_HAWK_PROTOTYPES) # Если требуется собрать прототипы
    add_subdirectory(Prototypes)    # Собираем прототипы
endif(BUILD_HAWK_PROTOTYPES)

add_subdirectory(HawkConsolServer)  # Собираем сервер
#add_subdirectory(HawkConsolClient) # Собираем клиент
#====================================================================
