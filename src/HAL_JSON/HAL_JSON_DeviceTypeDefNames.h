#pragma once

// Global use
#define HAL_JSON_KEYNAME_TYPE                               "type"
#define HAL_JSON_KEYNAME_UID                                "uid"
#define HAL_JSON_KEYNAME_PIN                                "pin"
#define HAL_JSON_KEYNAME_NOTE                               "note"
#define HAL_JSON_KEYNAME_ITEMS                              "items"
#define HAL_JSON_KEYNAME_REFRESHTIME_SEC                    "refreshtimesec"
#define HAL_JSON_KEYNAME_REFRESHTIME_MIN                    "refreshtimemin"



// Types
#define HAL_JSON_TYPE_DOUT                 "DOUT"
#define HAL_JSON_TYPE_DIN                  "DIN"
#define HAL_JSON_TYPE_DPOUT                "DPOUT"
#define HAL_JSON_TYPE_ADC                  "ADC"
#define HAL_JSON_TYPE_TX433                "TX433"
#define HAL_JSON_TYPE_DHT                  "DHT"
#define HAL_JSON_TYPE_PWM_ANALOG_WRITE     "PWM_AW"
#define HAL_JSON_TYPE_PWM_LEDC             "PWM_LEDC"
#define HAL_JSON_TYPE_ONE_WIRE_TEMP_GROUP  "1WTG"
#define HAL_JSON_TYPE_ONE_WIRE_TEMP_BUS    "1WTB"
#define HAL_JSON_TYPE_ONE_WIRE_TEMP_DEVICE "1WTD"
#define HAL_JSON_TYPE_REGO600              "REGO600"


// DHT models
#define HAL_JSON_TYPE_DHT_MODEL_DHT11       "DHT11"
#define HAL_JSON_TYPE_DHT_MODEL_DHT22       "DHT22"
#define HAL_JSON_TYPE_DHT_MODEL_AM2302      "AM2302"
#define HAL_JSON_TYPE_DHT_MODEL_RHT03       "RTH03"

// Single Pulse Output
#define HAL_JSON_KEYNAME_SINGLE_PULSE_OUTPUT_INACTIVE_STATE       "istate"
#define HAL_JSON_KEYNAME_SINGLE_PULSE_OUTPUT_DEFAULT_PULSE_LENGHT "plength"

// PWM output (also analog write)
#define HAL_JSON_KEYNAME_PWM_CFG_FREQUENCY                  "freq"
#define HAL_JSON_KEYNAME_PWM_CFG_RESOLUTION                 "res"
#define HAL_JSON_KEYNAME_PWM_INVOUT                         "invOut"

// 1-wire temp
#define HAL_JSON_KEYNAME_ONE_WIRE_ROMID    "romid"


// the following must be at end
#define HAL_JSON_ERR_MISSING_KEY(k) (F("Missing key: " k))
#define HAL_JSON_ERR_VALUE_TYPE(t) (F("Value type: " t))
#define HAL_JSON_ERR_STRING_EMPTY(k) (F("String is empty:" k))
#define HAL_JSON_ERR_ITEMS_EMPTY(t) (F(t " items list is empty"))
#define HAL_JSON_ERR_ITEMS_NOT_VALID(t) (F(t " do not contain any valid items"))