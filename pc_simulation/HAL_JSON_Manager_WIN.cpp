
#include "./HAL_JSON_Manager_WIN.h"

/** this is only for developing on windows environment */
namespace HAL_JSON {

    bool Manager::read(const HALReadRequest &req) {
        req.out_value = 42;
        return true;
    }
    bool Manager::write(const HALWriteRequest &req) {
        return true;
    }
}