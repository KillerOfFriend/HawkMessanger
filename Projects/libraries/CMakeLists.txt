cmake_minimum_required(VERSION 3.16.0 FATAL_ERROR)

project(Libraries)

#====================================================================
add_subdirectory(HawkErrors)            # Собираем библиотеку ошибок
add_subdirectory(HawkLog)               # Собираем библиотеку логирования
add_subdirectory(HawkCommon) 		# Собираем общую библиотеку
add_subdirectory(HawkServerCore) 	# Собираем ядро сервера
#add_subdirectory(HawkClientCore) 	# Собираем ядро клиента
#====================================================================
