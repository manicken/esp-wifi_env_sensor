

#include "HAL_JSON_RULE_Engine_IfConditionItem.h"

namespace HAL_JSON {
    namespace Rules {

        IfConditionItem::IfConditionItem() {

        }
        IfConditionItem::~IfConditionItem() {
            
        }

        bool IfConditionItem::NotEquals_Operation_Handler(const HALValue& lhs, const HALValue& rhs) {
            return lhs != rhs;
        }
        bool IfConditionItem::Equals_Operation_Handler(const HALValue& lhs, const HALValue& rhs) {
            return lhs == rhs;
        }
        bool IfConditionItem::LessThan_Operation_Handler(const HALValue& lhs, const HALValue& rhs) {
            return lhs < rhs;
        }
        bool IfConditionItem::LargerThan_Operation_Handler(const HALValue& lhs, const HALValue& rhs) {
            return lhs > rhs;
        }
        bool IfConditionItem::LessThanOrEquals_Operation_Handler(const HALValue& lhs, const HALValue& rhs) {
            return lhs <= rhs;
        }
        bool IfConditionItem::LargerThanOrEquals_Operation_Handler(const HALValue& lhs, const HALValue& rhs) {
            return lhs >= rhs;
        }

    }
}