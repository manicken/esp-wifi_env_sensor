#include "../src/HAL_JSON/HAL_JSON_UID_Path.h"
#include "../src/HAL_JSON/HAL_JSON_Value.h"
#include "../src/HAL_JSON/HAL_JSON_Operations.h"


/** this is only for developing on windows environment */
namespace HAL_JSON {
    class Manager {
    
    public:
        static bool read(const HALReadRequest &req);
        static bool write(const HALWriteRequest &req);
    };
}