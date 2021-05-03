
#====================================================================
set(EXECUTABLE_OUTPUT_PATH "${EXECUTABLE_OUTPUT_PATH}/prototypes")            # Указываем путь к папке сборки прототипов
#====================================================================
set(NET_IMPLEMENTATION_QT_SIMPLE 0) # Реализация сети на простом Qt Socket
set(NET_IMPLEMENTATION_QT_SSL 1)    # Реализация сети на Qt Socket SSL
#====================================================================
# Иммитируем перечисление всех реалихаций
add_definitions(-DNET_IMPLEMENTATION_QT_SIMPLE=${NET_IMPLEMENTATION_QT_SIMPLE})
add_definitions(-DNET_IMPLEMENTATION_QT_SSL=${NET_IMPLEMENTATION_QT_SSL})
#====================================================================
