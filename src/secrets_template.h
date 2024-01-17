#include <pgmspace.h>
 
#define SECRET

String THINGSSPEAK_API_KEY = "****";
String THINGSSPEAK_TEMP_FIELD = "field5"; // kök(1&2), toa(3&4), sov(5&6)
String THINGSSPEAK_HUMIDITY_FIELD = "field6"; //

#define aws_iot_THINGNAME "****"
 
int8_t TIME_ZONE = 1; //STOCKHOLM(SWEDEN): +1 UTC
 
// Copy from aws iot core settings - Device data endpoint here 
const char aws_iot_MQTT_HOST[] = "**.amazonaws.com";
 
// Copy contents of AmazonRootCA1.pem here
static const char aws_iot_cacert[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----

-----END CERTIFICATE-----
)EOF";
 
 
// Copy contents from XXXXXXXX-certificate.pem.crt here ▼
static const char aws_iot_client_cert[] PROGMEM = R"KEY(
-----BEGIN CERTIFICATE-----

-----END CERTIFICATE-----
 
)KEY";
 
 
// Copy contents from  XXXXXXXX-private.pem.key here ▼
static const char aws_iot_privkey[] PROGMEM = R"KEY(
-----BEGIN RSA PRIVATE KEY-----

-----END RSA PRIVATE KEY-----
 
)KEY";