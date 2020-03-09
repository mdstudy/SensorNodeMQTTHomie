
#include <FS.h>
#include <SPIFFS.h>
#include "UserConfig.h"

UserConfig::UserConfig()
{
    SPIFFS.begin(true);
}

void UserConfig::setup() {
    if(!SPIFFS.exists("/config.json")) {
        Serial.println("Config is not available, generating default");
        generateDefault();
        save();

    } else {
        Serial.println("Config is available.");
        load();
    }
}

void UserConfig::save() {
    File configFile = SPIFFS.open("/config.json", "w");
    serializeJson(json, configFile);
    configFile.close();
}

void UserConfig::load() {
    File configFile = SPIFFS.open("/config.json", "r");
    String input = configFile.readString();
    deserializeJson(json, input);
    check();
    serializeJsonPretty(json, Serial);
    configFile.close();
}

void UserConfig::check() {
    if(json.getMember("NTP_SERVER").isNull()) {
        json["NTP_SERVER"] = NTP_SERVER;
        json["TIMEZONE_OFFSET"] = TIMEZONE_OFFSET;
    }

    if(json.getMember("WIFI_NAME").isNull()) {
        json["WIFI_NAME"] = WIFI_NAME;
        json["WIFI_PASSWORD"] = WIFI_PASSWORD;
    }
    checkMqtt();
}

void UserConfig::checkMqtt() {
    if(json.getMember("MQTT").isNull()) {
        json.createNestedObject("MQTT");
        json["MQTT"]["MQTT_ENABLED"] = false;
        json["MQTT"]["MQTT_ADDRESS"] = "";
        json["MQTT"]["MQTT_PORT"] = 1883;
        json["MQTT"]["MQTT_USERNAME"] = "";
        json["MQTT"]["MQTT_PASSWORD"] = "";
        json["MQTT"]["MQTT_DEVICEID"] = "";
        json["MQTT"]["MQTT_DEVICENAME"] = "";
        json["MQTT"]["MQTT_NODENAME"] = "";
        json["MQTT"]["MQTT_TEMPPROPNAME"] = "";
    }
    save();

}

void UserConfig::generateDefault() {
    json["WIFI_NAME"] = WIFI_NAME;
    json["WIFI_PASSWORD"] = WIFI_PASSWORD;
    check();
}

void UserConfig::setStringProperty(String name, String value) {
    json[name] = value;
}

String UserConfig::getStringProperty(String name) {
    return json.getMember(name).as<String>();
}

void UserConfig::setStringProperty(String sub, String name, String value) {
    json[sub][name] = value;
}

String UserConfig::getStringProperty(String sub, String name) {
    return json.getMember(sub).getMember(name).as<String>();
}

void UserConfig::setU8Property(String name, uint8_t value) {
    json[name] = value;
}

uint8_t UserConfig::getU8Property(String name) {
    return json.getMember(name).as<uint8_t>();
}

void UserConfig::setU8Property(String sub, String name, uint8_t value) {
    json[sub][name] = value;
}

uint8_t UserConfig::getU8Property(String sub, String name) {
    return json.getMember(sub).getMember(name).as<uint8_t>();
}

void UserConfig::setU16Property(String name, uint16_t value) {
    json[name] = value;
}

uint16_t UserConfig::getU16Property(String name) {
    return json.getMember(name).as<uint16_t>();
}

void UserConfig::setU16Property(String sub, String name, uint16_t value) {
    json[sub][name] = value;
}

uint16_t UserConfig::getU16Property(String sub, String name) {
    return json.getMember(sub).getMember(name).as<uint16_t>();
}

bool UserConfig::getBoolProperty(String name) {
    return json.getMember(name).as<bool>();
}


bool UserConfig::getBoolProperty(String sub, String name) {
    return json.getMember(sub).getMember(name).as<bool>();
}

void UserConfig::checkMQTTConfig(JsonObject object) {
    bool mqttCanEnabled = true;
    
    if(!object.getMember("MQTT_ENABLED").isNull() && object["MQTT_ENABLED"].is<bool>()) {
        json["MQTT"]["MQTT_ENABLED"] = object["MQTT_ENABLED"].as<bool>();
    }

    if(!object.getMember("MQTT_ADDRESS").isNull() && object["MQTT_ADDRESS"].is<String>()) {
        if(object["MQTT_ADDRESS"].as<String>().length() > 0) {
            json["MQTT"]["MQTT_ADDRESS"] = object["MQTT_ADDRESS"].as<String>();
        } else {
            mqttCanEnabled = false;
        }
    }
    if(!object.getMember("MQTT_PORT").isNull() && object["MQTT_PORT"].is<uint16_t>()) {
        json["MQTT"]["MQTT_PORT"] = object["MQTT_PORT"].as<uint16_t>();
    }
    if(!object.getMember("MQTT_USERNAME").isNull() && object["MQTT_USERNAME"].is<String>()) {
            json["MQTT"]["MQTT_USERNAME"] = object["MQTT_USERNAME"].as<String>();
    }
    if(!object.getMember("MQTT_PASSWORD").isNull() && object["MQTT_PASSWORD"].is<String>()) {
        
            json["MQTT"]["MQTT_PASSWORD"] = object["MQTT_PASSWORD"].as<String>();
    }

    if(!object.getMember("MQTT_DEVICEID").isNull() && object["MQTT_DEVICEID"].is<String>()) {
        if(object["MQTT_DEVICEID"].as<String>().length() > 0) {
            json["MQTT"]["MQTT_DEVICEID"] = object["MQTT_DEVICEID"].as<String>();
        } else {
            mqttCanEnabled = false;
        }
    }

    if(!object.getMember("MQTT_DEVICENAME").isNull() && object["MQTT_DEVICENAME"].is<String>()) {
        if(object["MQTT_DEVICENAME"].as<String>().length() > 0) {
            json["MQTT"]["MQTT_DEVICENAME"] = object["MQTT_DEVICENAME"].as<String>();
        } else {
            mqttCanEnabled = false;
        }
    }
    if(!object.getMember("MQTT_NODENAME").isNull() && object["MQTT_NODENAME"].is<String>()) {
        if(object["MQTT_NODENAME"].as<String>().length() > 0) {
            json["MQTT"]["MQTT_NODENAME"] = object["MQTT_NODENAME"].as<String>();
        } else {
            mqttCanEnabled = false;
        }
    }
    if(!object.getMember("MQTT_TEMPPROPNAME").isNull() && object["MQTT_TEMPPROPNAME"].is<String>()) {
        if(object["MQTT_TEMPPROPNAME"].as<String>().length() > 0) {
            json["MQTT"]["MQTT_TEMPPROPNAME"] = object["MQTT_TEMPPROPNAME"].as<String>();
        } else {
            mqttCanEnabled = false;
        }
    }
    if(!object.getMember("MQTT_ENABLED").isNull() && object["MQTT_ENABLED"].is<bool>()) {
        if(mqttCanEnabled) {
            json["MQTT"]["MQTT_ENABLED"] = object["MQTT_ENABLED"].as<bool>();
        } else {
            json["MQTT"]["MQTT_ENABLED"] = false;
        }
    }
    save();
}