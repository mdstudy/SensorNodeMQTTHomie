#include <Arduino.h>
#include <ArduinoJson.h>

#define MONDAY "Monday"
#define TUESDAY "Tuesday"
#define WEDNESDAY "Wednesday"
#define THURSDAY "Thursday"
#define FRIDAY "Friday"
#define SATURDAY "Saturday"
#define SUNDAY "Sunday"


class UserConfig {
    
#define WIFI_NAME "..."
#define WIFI_PASSWORD "..."


#define NTP_SERVER "pool.ntp.org"
#define TIMEZONE_OFFSET 3600
#define DS18B20_1 {0x28, 0xFF, 0x1E, 0x59, 0x33, 0x18, 0x2, 0xC1}
#define DOWN_BOUND_TEMP -20
#define UPPER_BOUND_TEMP 100
#define SENSOR_COUNT 1
#define TEMP_PIN 12
#define UPDATE_DELAY 60

    private:
        
        void generateDefault();
    
    public:
        StaticJsonDocument<3072> json;
        UserConfig();
        void setup();
        void save();
        void load();
        void check();
        void checkMqtt();
        bool summerTime = false;

        void checkAndNewConfig(JsonObject newConfig);
        void checkMQTTConfig(JsonObject object);

        void setStringProperty(String name, String value);
        String getStringProperty(String name);

        void setStringProperty(String sub, String name, String value);
        String getStringProperty(String sub, String name);

        void setU8Property(String name, uint8_t value);
        uint8_t getU8Property(String name);

        void setU8Property(String sub, String name, uint8_t value);
        uint8_t getU8Property(String sub, String name);

        void setU16Property(String name, uint16_t value);
        uint16_t getU16Property(String name);

        void setU16Property(String sub, String name, uint16_t value);
        uint16_t getU16Property(String sub, String name);

        bool getBoolProperty(String name);
        bool getBoolProperty(String sub, String name);
};