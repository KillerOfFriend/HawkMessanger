#include "@VERSION_FILE_NAME@.h"

using namespace @PROJECT_NAME@;

//-----------------------------------------------------------------------------
std::string getVersion()    { return "@PROJECT_VERSION@"; }
//-----------------------------------------------------------------------------
std::uint8_t getVerMajor()  { return @PROJECT_VERSION_MAJOR@; }
//-----------------------------------------------------------------------------
std::uint8_t getVerMinor()  { return @PROJECT_VERSION_MINOR@; }
//-----------------------------------------------------------------------------
std::uint8_t getVerPatch()  { return @PROJECT_VERSION_PATCH@; }
//-----------------------------------------------------------------------------
std::uint8_t getVerTweak()  { return @PROJECT_VERSION_TWEAK@; }
//-----------------------------------------------------------------------------

