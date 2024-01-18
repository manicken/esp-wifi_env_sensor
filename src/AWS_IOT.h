
#ifndef AWS_IOT_H
#define AWS_IOT_H

#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include "NTP.h"
#include "secrets/db_kitchen/secrets.h"
//#include "secrets/db_toilet/secrets.h"
//#include "secrets/db_bedroom/secrets.h"


namespace AWS_IOT {

    #define DEBUG_UART Serial1
    std::string AWS_IOT_PUBLISH_TOPIC_str = std::string(AWS_IOT_THINGNAME) + "/pub";   // need to create it here, otherwise it gets out of scope
    std::string AWS_IOT_SUBSCRIBE_TOPIC_str = std::string(AWS_IOT_THINGNAME) + "/sub"; // need to create it here, otherwise it gets out of scope

    const char* AWS_IOT_PUBLISH_TOPIC = AWS_IOT_PUBLISH_TOPIC_str.c_str();
    const char* AWS_IOT_SUBSCRIBE_TOPIC = AWS_IOT_SUBSCRIBE_TOPIC_str.c_str();
    // AWS stuff
    WiFiClientSecure wifiClientSecure;

    BearSSL::X509List ca_cert(AWS_IOT_CA_CERTIFICATE); // from #include "secrets/*/secrets.h"
    BearSSL::X509List client_cert(AWS_IOT_CLIENT_CERTIFICATE);
    BearSSL::PrivateKey private_key(AWS_IOT_PRIVATE_KEY);

    PubSubClient client(wifiClientSecure);

    void messageReceived(char *topic, byte *payload, unsigned int length)
    {
        DEBUG_UART.print("Received [");
        DEBUG_UART.print(topic);
        DEBUG_UART.print("]: ");
        for (int i = 0; i < length; i++)
        {
            DEBUG_UART.print((char)payload[i]);
        }
        DEBUG_UART.println();
    }

    void setup_and_connect(void)
    {
        DEBUG_UART.print("sub topic: ");
        DEBUG_UART.println(AWS_IOT_SUBSCRIBE_TOPIC);
        DEBUG_UART.print("pub topic: ");
        DEBUG_UART.println(AWS_IOT_PUBLISH_TOPIC);

        NTP::NTPConnect();

        wifiClientSecure.setTrustAnchors(&ca_cert);
        wifiClientSecure.setClientRSACert(&client_cert, &private_key);
        
        client.setServer(AWS_IOT_MQTT_HOST, 8883);
        client.setCallback(messageReceived);
        
        
        DEBUG_UART.println("Connecting to AWS IOT");
        
        while (!client.connect(AWS_IOT_THINGNAME))
        {
            DEBUG_UART.print(".");
            delay(1000);
        }
        
        if (!client.connected()) {
            DEBUG_UART.println("AWS IoT Timeout!");
            return;
        }
        // Subscribe to a topic
        
        client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);
        
        DEBUG_UART.println("AWS IoT Connected!");
    }

    void publishMessage(float h, float t)
    {
        struct tm* timeinfo;
        NTP::now = time(nullptr);
        //gmtime_r(&NTP::now, &timeinfo);
        timeinfo = localtime(&NTP::now);

        StaticJsonDocument<200> doc;
        doc["time"] = asctime(timeinfo);// millis();
        doc["humidity"] = h;
        doc["temperature"] = t;
        char jsonBuffer[512];
        serializeJson(doc, jsonBuffer); // print to client
        
        client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
    }
}

#endif