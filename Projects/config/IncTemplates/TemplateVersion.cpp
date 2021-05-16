#include "@VERSION_FILE_NAME@.h"

//-----------------------------------------------------------------------------
std::string @PROJECT_NAME@::getVersion()    { return "@PROJECT_VERSION@"; }
//-----------------------------------------------------------------------------
std::uint8_t @PROJECT_NAME@::getVerMajor()  { return @PROJECT_VERSION_MAJOR@; }
//-----------------------------------------------------------------------------
std::uint8_t @PROJECT_NAME@::getVerMinor()  { return @PROJECT_VERSION_MINOR@; }
//-----------------------------------------------------------------------------
std::uint8_t @PROJECT_NAME@::getVerPatch()  { return @PROJECT_VERSION_PATCH@; }
//-----------------------------------------------------------------------------
std::uint8_t @PROJECT_NAME@::getVerTweak()  { return @PROJECT_VERSION_TWEAK@; }
//-----------------------------------------------------------------------------
std::string @PROJECT_NAME@::getModuleName() { return "@PROJECT_NAME@"; }
//-----------------------------------------------------------------------------

