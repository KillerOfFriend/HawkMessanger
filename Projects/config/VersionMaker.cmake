function(MakeVersion)
    set(VERSION_FILE_NAME "${PROJECT_NAME}Version")

    configure_file(${CONFIG_PATH}/IncTemplates/TemplateVersion.h ${CMAKE_CURRENT_SOURCE_DIR}/src/${VERSION_FILE_NAME}.h)
    configure_file(${CONFIG_PATH}/IncTemplates/TemplateVersion.cpp ${CMAKE_CURRENT_SOURCE_DIR}/src/${VERSION_FILE_NAME}.cpp)
endfunction()
