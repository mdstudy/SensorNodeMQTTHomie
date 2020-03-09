#include <Arduino.h>
#include <WIFI.h>
#include <ArduinoJson.h>
#include <AsyncJson.h>
#include "SimpleTempSensorNode.h"
#include <esp_int_wdt.h>
#include <esp_task_wdt.h>
#include <Update.h>
#include <AsyncMQTTClientWrapper.h>


SimpleTempSensorNode *SimpleTempSensorNode::self = NULL;

SimpleTempSensorNode::SimpleTempSensorNode() : server(80),
                                                 config(),
                                                 restartRequired(false),
                                                 uploadFileInited(false),
                                                 uploadFileName(""),
                                                 uploadFIleFailed(false),
                                                 mqttEnabled(false),
                                                 bus(TEMP_PIN),
                                                 dallasTemps(&bus)

{
    SimpleTempSensorNode::self = this;
}

void SimpleTempSensorNode::setup()
{
    Serial.println("Version 1.0");
    config.setup();
}

void SimpleTempSensorNode::setupHomie() {
    if(config.getBoolProperty("MQTT", "MQTT_ENABLED")) {
        String address = config.getStringProperty("MQTT", "MQTT_ADDRESS");
        uint16_t port = config.getU16Property("MQTT", "MQTT_PORT");
        String username = config.getStringProperty("MQTT", "MQTT_USERNAME");
        String password = config.getStringProperty("MQTT", "MQTT_PASSWORD");
        String deviceid = config.getStringProperty("MQTT", "MQTT_DEVICEID");
        String devicename = config.getStringProperty("MQTT", "MQTT_DEVICENAME");
        String nodeName = config.getStringProperty("MQTT", "MQTT_NODENAME");
        String temppropname = config.getStringProperty("MQTT", "MQTT_TEMPPROPNAME");
        homie = new Homie(address, port, username, password, devicename, deviceid, 1);
        tempNode = new HomieNode(homie, "temperatureSensor", nodeName, "DS18B20", 4);
        char buffer[20];
        sprintf(buffer, "%f:%f", DOWN_BOUND_TEMP, UPPER_BOUND_TEMP);
        tempProp = new HomieNodeFloatProperty("temperatur", temppropname, buffer, "°C", false, true, NULL, NULL);
        tempNode->addProperty(tempProp);
        homie->addNode(tempNode);
        homie->init<AsyncMQTTClientWrapper>();
        mqttEnabled = true;
    }
}

void SimpleTempSensorNode::loop()
{
    if (WiFi.status() != WL_CONNECTED)
    {
        setupWIFIConnect(true);
    }
    EVERY_N_SECONDS(UPDATE_DELAY)
    {
        readSensor();
    }
    if (restartRequired)
    {
        yield();
        delay(1000);
        yield();
#if defined(ESP8266)
        ESP.restart();
#elif defined(ESP32)
        esp_task_wdt_init(1, true);
        esp_task_wdt_add(NULL);
        while (true)
            ;
#endif
    }
}

void SimpleTempSensorNode::readSensor() {
    sens1Temp = dallasTemps.getTempC(ds18b20_1);
    if(sens1Temp > DOWN_BOUND_TEMP && sens1Temp < UPPER_BOUND_TEMP && mqttEnabled) {
        tempProp->setFloatValue(sens1Temp);
    }
}

void SimpleTempSensorNode::setupWIFIConnect(bool connectOnly)
{
    Serial.println("[LWS] setting up WiFi Connect");
    WiFi.disconnect();
    server.reset();
    String wifiName = config.getStringProperty("WIFI_NAME");
    WiFi.begin(wifiName.c_str(), config.getStringProperty("WIFI_PASSWORD").c_str());
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.printf("Connecting to %s\n", wifiName.c_str());
    }
    Serial.println("Connected");
    initHttp();
    if (!inited)
    {
        configureNtp(0);
        inited = true;
        setupHomie();
    }

    //setupMQTT(connectOnly);
    //handle mqtt broker here
}

void SimpleTempSensorNode::configureNtp(uint16_t daylight)
{
    configTime(config.getU16Property("TIMEZONE_OFFSET"), daylight, config.getStringProperty("NTP_SERVER").c_str());
}

void SimpleTempSensorNode::initHttp()
{

    //curl --request POST -F file="@firmware.bin" http://192.168.2..../update
    server.on("/update", HTTP_POST, [&](AsyncWebServerRequest *request) {
        // the request handler is triggered after the upload has finished...
        // create the response, add header, and send response
        if(Update.hasError()) {
            AsyncWebServerResponse *response = request->beginResponse((Update.hasError()) ? 500 : 200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
            response->addHeader("Connection", "close");
            response->addHeader("Access-Control-Allow-Origin", "*");
            request->send(response);
        } else {
            request->redirect("/");
        }
        
        SimpleTempSensorNode::self->restartRequired = true; }, [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
        //Upload handler chunks in data
        
        if (!index)
        {

                if (!Update.begin(UPDATE_SIZE_UNKNOWN))
                { // Start with max available size
                    Update.printError(Serial);
                }

                /*#if defined(ESP8266)
                        Update.runAsync(true); // Tell the updaterClass to run in async mode
                    #endif*/
            }

            // Write chunked data to the free sketch space
            if (Update.write(data, len) != len)
            {
                Update.printError(Serial);
            }

            if (final)
            { // if the final flag is set then this is the last frame of data
                if (Update.end(true))
                { //true to set the size to the current progress
                }
            } });

    server.on("/mqtt", HTTP_GET, [](AsyncWebServerRequest *request) {
        File indexFile = SPIFFS.open("/configMqtt.html", "r");
        String content = indexFile.readString();
        
        request->send(200, "text/html", content);
        indexFile.close();
    });

    
    //curl --request POST -F file="@file" http://192.168.2..../updateFile
    server.on("/updateFile", HTTP_POST, [&](AsyncWebServerRequest *request) {
        // the request handler is triggered after the upload has finished...
        // create the response, add header, and send response
        if(SimpleTempSensorNode::self->uploadFIleFailed) {
            AsyncWebServerResponse *response = request->beginResponse((SimpleTempSensorNode::self->uploadFIleFailed) ? 500 : 200, "text/plain", (SimpleTempSensorNode::self->uploadFIleFailed) ? "FAIL" : "OK");
            response->addHeader("Connection", "close");
            response->addHeader("Access-Control-Allow-Origin", "*");
            request->send(response);
        } else {
            request->redirect("updateFile");
        }
        SimpleTempSensorNode::self->uploadFIleFailed = false; }, [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
        //Upload handler chunks in data
        if(!SimpleTempSensorNode::self->uploadFileInited || SimpleTempSensorNode::self->uploadFileName.equals("/"+filename))
        {
            
            if (!index)
            {
                SimpleTempSensorNode::self->uploadFileInited = true;
                SimpleTempSensorNode::self->uploadFileName = "/"+filename;
                if(SPIFFS.exists(SimpleTempSensorNode::self->uploadFileName)) {
                    SPIFFS.rename(SimpleTempSensorNode::self->uploadFileName, SimpleTempSensorNode::self->uploadFileName+".old");
                }
                SimpleTempSensorNode::self->uploadfile = SPIFFS.open(SimpleTempSensorNode::self->uploadFileName, "w");
            }
            // Write chunked data to the free sketch space

            if (SimpleTempSensorNode::self->uploadfile.write(data, len) != len)
            {
                SimpleTempSensorNode::self->uploadFIleFailed = true;
            }
            

            if (final)
            { // if the final flag is set then this is the last frame of data
                SimpleTempSensorNode::self->uploadfile.close();
                SimpleTempSensorNode::self->uploadFileInited = false;
                if(SimpleTempSensorNode::self->uploadFIleFailed) {
                    SPIFFS.remove(SimpleTempSensorNode::self->uploadFileName);
                    SPIFFS.rename(SimpleTempSensorNode::self->uploadFileName+".old", SimpleTempSensorNode::self->uploadFileName);
                } else {
                    SPIFFS.remove(SimpleTempSensorNode::self->uploadFileName+".old");
                }
                
            }
        } });

    server.on("/rest/mqttConfig", HTTP_GET, [](AsyncWebServerRequest *request) {
        AsyncResponseStream *response = request->beginResponseStream("application/json");
        
        serializeJson(SimpleTempSensorNode::self->config.json["MQTT"], *response);
        response->addHeader("Access-Control-Allow-Origin", "*");
        request->send(response);
    });

    server.on("/rest/mqttConfig", HTTP_OPTIONS, [](AsyncWebServerRequest *request) {
        AsyncResponseStream *response = request->beginResponseStream("text/plain");
        response->addHeader("Access-Control-Allow-Origin", "*");
        response->addHeader("Access-Control-Allow-Methods", "POST, GET, PUT, UPDATE, DELETE, OPTIONS");
        response->addHeader("Access-Control-Max-Age", "1000");
        response->addHeader("Access-Control-Allow-Headers", "*");
        response->setCode(200);
        request->send(response);
    });

    AsyncCallbackJsonWebHandler *mqttConfigHandler = new AsyncCallbackJsonWebHandler("/rest/mqttConfig", [](AsyncWebServerRequest *request, JsonVariant json) {
        Serial.println("having new mqtt config");
        JsonObject jsonObj = json.as<JsonObject>();
        SimpleTempSensorNode::self->config.checkMQTTConfig(jsonObj);
        AsyncResponseStream *response = request->beginResponseStream("application/json");
        serializeJson(SimpleTempSensorNode::self->config.json, *response);
        response->addHeader("Access-Control-Allow-Origin", "*");
        request->send(response);
        SimpleTempSensorNode::self->restartRequired = true;
    });
    server.begin();
}

void SimpleTempSensorNode::syncLocalTime()
{
    if (!getLocalTime(&timeinfo))
    {
        Serial.println("Failed to obtain time");
        return;
    }
    uint8_t offset = 7 - (timeinfo.tm_wday);
    if (offset == 7)
    {
        offset = 0;
    }
    if (config.summerTime && ((offset == 0 && timeinfo.tm_hour >= 3 && timeinfo.tm_mon == 9 && timeinfo.tm_mday + offset > 31) ||
                              (offset != 0 && timeinfo.tm_mday + offset > 31 && timeinfo.tm_mon == 9) ||
                              timeinfo.tm_mon > 9 || timeinfo.tm_mon < 2))
    {
        Serial.println("Setting wintertime");
        config.summerTime = false;
        configureNtp(0);
        getLocalTime(&timeinfo);
    }
    else if (!config.summerTime && ((offset == 0 && timeinfo.tm_hour >= 2 && timeinfo.tm_mon == 2 && timeinfo.tm_mday + offset > 31) ||
                                    (offset != 0 && timeinfo.tm_mday + offset > 31 && timeinfo.tm_mon == 2) ||
                                    (timeinfo.tm_mon > 2 && timeinfo.tm_mon < 10)))
    {
        Serial.println("Setting summertime");
        config.summerTime = true;
        configureNtp(3600);
        getLocalTime(&timeinfo);
    }
}
