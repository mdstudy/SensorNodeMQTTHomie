#include <AsyncMqttClient.h>
#include <FastLED.h>
#include <ESPAsyncWebServer.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <SPIFFS.h>
#include <time.h>
#include "UserConfig.h"
#include <Homie.h>
#include <HomieNode.h>
#include <HomieNodeFloatProperty.h>


class SimpleTempSensorNode {
    private:
        
        AsyncWebServer server;
        void setupWIFIConnect(bool connectOnly = false);
        void setupMQTT(bool connectOnly = false);
        void initHttp();
        OneWire bus;
        DallasTemperature dallasTemps;
        DeviceAddress ds18b20_1 = DS18B20_1;
        struct tm timeinfo;
        void syncLocalTime();
        void configureNtp(uint16_t daylight);
        Homie* homie;
        HomieNode* tempNode;
        bool mqttEnabled;
        float sens1Temp = 0;

        
    public:
        static SimpleTempSensorNode *self;
        //AsyncMqttClient mqttClient;
        
        SimpleTempSensorNode();
        UserConfig config;        
        
        bool inited = false;
        bool reconecting = false;
        
        HomieNodeFloatProperty* tempProp;
        void setup();
        void setupHomie();
        void loop();
        bool restartRequired;
        bool uploadFileInited;
        String uploadFileName;
        File uploadfile;
        bool uploadFIleFailed;
        void readSensor();
        
};