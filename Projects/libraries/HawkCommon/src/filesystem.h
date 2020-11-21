#ifndef FILESYSTEM_H
#define FILESYSTEM_H

// Именуем filesystem в зависимости от стандарта
//-----------------------------------------------------------------------------
#if __cplusplus >= 201703L

    #ifdef __GNUC__ // C g++ свои заморочки
        #include <features.h>
        #if __GNUC_PREREQ(8, 0) // filesystem добавлен только начиная с версии 8.0
            #include <filesystem>
            namespace filesystem = std::filesystem;
        #else
            #include <experimental/filesystem>
            namespace filesystem = std::experimental::filesystem;
        #endif
    #endif
#else
    #include <experimental/filesystem>
    namespace filesystem = std::experimental::filesystem;
#endif
//-----------------------------------------------------------------------------

#endif // FILESYSTEM_H
