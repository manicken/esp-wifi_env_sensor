#pragma once

#define HAL_JSON_VALIDATE_JSON_STRICT

#if defined(HAL_JSON_VALIDATE_JSON_STRICT)
#define HAL_JSON_VALIDATE_IN_LOOP_FAIL_OPERATION return false
#else
#define HAL_JSON_VALIDATE_IN_LOOP_FAIL_OPERATION continue
#endif

#define HAL_JSON_USE_EFFICIENT_FIND

// Global use
#define HAL_JSON_KEYNAME_DISABLED                           "disabled"
#define HAL_JSON_KEYNAME_TYPE                               "type"
#define HAL_JSON_KEYNAME_UID                                "uid"
#define HAL_JSON_KEYNAME_PIN                                "pin"
#define HAL_JSON_KEYNAME_NOTE                               "note"
#define HAL_JSON_KEYNAME_ITEMS                              "items"
#define HAL_JSON_KEYNAME_REFRESHTIME_SEC                    "refreshtimesec"
#define HAL_JSON_KEYNAME_REFRESHTIME_MIN                    "refreshtimemin"

// Single Pulse Output
#define HAL_JSON_KEYNAME_SINGLE_PULSE_OUTPUT_INACTIVE_STATE       "istate"
#define HAL_JSON_KEYNAME_SINGLE_PULSE_OUTPUT_DEFAULT_PULSE_LENGHT "plength"

// PWM output (also analog write)
#define HAL_JSON_KEYNAME_PWM_CFG_FREQUENCY                  "freq"
#define HAL_JSON_KEYNAME_PWM_CFG_RESOLUTION                 "res"
#define HAL_JSON_KEYNAME_PWM_INVOUT                         "invOut"


// the following must be at end
#define HAL_JSON_ERR_MISSING_STRING_VALUE_KEY (F("Missing string value key: "))
#define HAL_JSON_ERR_VALUE_TYPE_NOT_STRING (F("Value type not string: "))
#define HAL_JSON_ERR_STRING_EMPTY (F("String is empty:"))

#define HAL_JSON_ERR_MISSING_KEY(k) (F("Missing key: " k)),jsonObj
#define HAL_JSON_ERR_VALUE_TYPE(t) (F("Value type: " t)),jsonObj
#define HAL_JSON_ERR_ITEMS_EMPTY(t) (F(t " items list is empty")),jsonObj
#define HAL_JSON_ERR_ITEMS_NOT_VALID(t) (F(t " do not contain any valid items")),jsonObj

#define HAL_JSON_DEBUG(constStr, dynStr) Serial.print(constStr); Serial.println(dynStr);
//#define HAL_JSON_DEBUG(constStr, dynStr) GlobalLogger.Info(constStr, dynStr);

