
#ifndef AWS_IOT_H
#define AWS_IOT_H

#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include "NTP.h"
#include "LittleFS_ext.h"

// the following are not used when having config and files on internal filesystem
//#include "secrets/db_kitchen/secrets.h"
//#include "secrets/db_toilet/secrets.h"
//#include "secrets/db_bedroom/secrets.h"


namespace AWS_IOT {

    #define DEBUG_UART Serial1
    #define AWS_IOT_FILES_DIR        "/aws_iot/"
    #define AWS_IOT_CONFIG_JSON_FILE "/aws_iot/cfg.json"
    #define AWS_IOT_JSON_FIELD_MQTT_HOST        "mqtt_host"
    #define AWS_IOT_JSON_FIELD_THINGNAME        "thingName"
    #define AWS_IOT_JSON_FIELD_CA_CERT_FILE     "ca_cert_file"
    #define AWS_IOT_JSON_FIELD_DEVICE_CERT_FILE "device_cert_file"
    #define AWS_IOT_JSON_FIELD_PRIVATE_KEY_FILE "private_key_file"

    #define AWS_IOT_FILE_DEFAULT_CA_CERT        "RootCA1.pem"
    #define AWS_IOT_FILE_DEFAULT_DEVICE_CERT    "device.crt"
    #define AWS_IOT_FILE_DEFAULT_PRIVATE_KEY    "private.key"
  
    std::string mqtt_host = "";
    std::string thingName = "";
    std::string publish_topic = "/pub";
    std::string subscribe_topic = "/sub";

    std::string file_path_ca_cert = "";
    std::string file_path_device_cert = "";
    std::string file_path_private_key = "";

    WiFiClientSecure wifiClientSecure;
    BearSSL::X509List *ca_cert;
    BearSSL::X509List *device_cert;
    BearSSL::PrivateKey *private_key;

    PubSubClient pubSubClient(wifiClientSecure);
    
    StaticJsonDocument<256> jsonDoc;

    char jsonBuffer[320];

    bool canConnect = false;
    
    bool setup_readFiles()
    {
        if(LittleFS.exists(AWS_IOT_CONFIG_JSON_FILE))
        {
            LittleFS_ext::load_from_file(AWS_IOT_CONFIG_JSON_FILE, jsonBuffer);
            deserializeJson(jsonDoc, jsonBuffer);
        }
        else
        {
            //jsonDoc.clear(); // this will try to use defaults,
            // note if the json is not found then the mqqt host cannot be resolved
            // as it should not be public visible
            // therefore 
            DEBUG_UART.println("AWS IOT error: config json file" AWS_IOT_CONFIG_JSON_FILE " not found");
            canConnect = false;
            return false;
        }

        if (jsonDoc.containsKey(AWS_IOT_JSON_FIELD_MQTT_HOST))
            mqtt_host = (std::string)jsonDoc[AWS_IOT_JSON_FIELD_MQTT_HOST];
        else {
            DEBUG_UART.println("AWS IOT error: " AWS_IOT_JSON_FIELD_MQTT_HOST " field in json missing");
            canConnect = false;
            return false; // cannot work without mqtt host name
        }

        if (jsonDoc.containsKey(AWS_IOT_JSON_FIELD_THINGNAME))
            thingName = (std::string)jsonDoc[AWS_IOT_JSON_FIELD_THINGNAME];
        else {
            DEBUG_UART.println("AWS IOT error: " AWS_IOT_JSON_FIELD_THINGNAME " field in json missing");
            canConnect = false;
            return false; // cannot work without the thingname
        }

        publish_topic = std::string(thingName) + "/pub";
        subscribe_topic = std::string(thingName) + "/sub";
        
        if (jsonDoc.containsKey(AWS_IOT_JSON_FIELD_CA_CERT_FILE))
            file_path_ca_cert = AWS_IOT_FILES_DIR + (std::string)jsonDoc[AWS_IOT_JSON_FIELD_CA_CERT_FILE];
        else
            file_path_ca_cert = AWS_IOT_FILES_DIR AWS_IOT_FILE_DEFAULT_CA_CERT;
        
        if (jsonDoc.containsKey(AWS_IOT_JSON_FIELD_DEVICE_CERT_FILE))
            file_path_device_cert = AWS_IOT_FILES_DIR + (std::string)jsonDoc[AWS_IOT_JSON_FIELD_DEVICE_CERT_FILE];
        else
            file_path_device_cert = AWS_IOT_FILES_DIR AWS_IOT_FILE_DEFAULT_CA_CERT;

        if (jsonDoc.containsKey(AWS_IOT_JSON_FIELD_PRIVATE_KEY_FILE))
            file_path_private_key = AWS_IOT_FILES_DIR + (std::string)jsonDoc[AWS_IOT_JSON_FIELD_PRIVATE_KEY_FILE];
        else
            file_path_private_key = AWS_IOT_FILES_DIR AWS_IOT_FILE_DEFAULT_PRIVATE_KEY;

        if (!LittleFS.exists(file_path_ca_cert.c_str())){DEBUG_UART.printf("AWS_IOT error: cannot find ca_cert file: %s\n",file_path_ca_cert.c_str()); return false; }
        if (!LittleFS.exists(file_path_device_cert.c_str())){DEBUG_UART.printf("AWS_IOT error: cannot find client_cert file: %s\n",file_path_device_cert.c_str()); return false; }
        if (!LittleFS.exists(file_path_private_key.c_str())){DEBUG_UART.printf("AWS_IOT error: cannot find private_key file: %s\n",file_path_private_key.c_str()); return false; }
    
        File fs = LittleFS.open(file_path_ca_cert.c_str(), "r");
        ca_cert = new BearSSL::X509List(fs, fs.available());
        fs.close();
        fs = LittleFS.open(file_path_device_cert.c_str(), "r");
        device_cert = new BearSSL::X509List(fs, fs.available());
        fs.close();
        fs = LittleFS.open(file_path_private_key.c_str(), "r");
        private_key = new BearSSL::PrivateKey(fs, fs.available());
        fs.close();
        canConnect = true;
        return true;
    }

    void setup_and_connect(std::function<void(char*, uint8_t*, unsigned int)> messageReceivedCallback)
    {
        if (canConnect == false) return;
        
        NTP::NTPConnect();
        
        
        DEBUG_UART.print("sub topic: ");
        DEBUG_UART.println(subscribe_topic.c_str());
        DEBUG_UART.print("pub topic: ");
        DEBUG_UART.println(publish_topic.c_str());

        wifiClientSecure.setTrustAnchors(ca_cert);
        wifiClientSecure.setClientRSACert(device_cert, private_key);
        
        pubSubClient.setServer(mqtt_host.c_str(), 8883);
        pubSubClient.setCallback(messageReceivedCallback);
        
        
        DEBUG_UART.printf("Connecting to AWS IOT using %s\n", thingName.c_str());
        
        while (!pubSubClient.connect(thingName.c_str()))
        {
            DEBUG_UART.print(".");
            delay(1000);
        }
        
        if (!pubSubClient.connected()) {
            DEBUG_UART.println("AWS IoT Timeout!");
            return;
        }
        // Subscribe to a topic
        
        pubSubClient.subscribe(subscribe_topic.c_str());
        
        DEBUG_UART.println("AWS IoT Connected!");
    }

    void publishMessage(float h, float t)
    {
        struct tm* timeinfo;
        NTP::now = time(nullptr);
        //gmtime_r(&NTP::now, &timeinfo);
        timeinfo = localtime(&NTP::now);

        jsonDoc.clear();
        jsonDoc["time"] = asctime(timeinfo);// millis();
        jsonDoc["humidity"] = h;
        jsonDoc["temperature"] = t;
        
        serializeJson(jsonDoc, jsonBuffer); // print to client
        
        pubSubClient.publish(publish_topic.c_str(), jsonBuffer);
    }
}

#endif