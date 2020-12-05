#include <Arduino.h>
#include <WiFi.h>
#include <IPAddress.h>
#include <ESPWebServer.hpp>
#include <ESPWebServerSecure.hpp>

#include <ctime>
#include <html_template.hpp>
#include <server_private_key.hpp>

bool log_flag{false};

ESPWebServer webserverHTTP(80);
ESPWebServerSecure webserver(443);

// Server certificate
const char server_cert[] PROGMEM = R"EOF(-----BEGIN CERTIFICATE-----
MIIDQjCCAioCFADEl2YHOqmk0txGyfWbOGpvbIQWMA0GCSqGSIb3DQEBCwUAMFUx
CzAJBgNVBAYTAkFUMQ8wDQYDVQQIDAZzdHlyaWExHzAdBgNVBAoMFlVsdGltYXRl
IEF1dGhvcml0eSBJbmMxFDASBgNVBAMMC0ludGVyY29tbWllMB4XDTIwMTEzMDE5
MTkxOVoXDTMxMTExMzE5MTkxOVowZjELMAkGA1UEBhMCQVQxDzANBgNVBAgMBlN0
eXJpYTENMAsGA1UEBwwER3JhejEfMB0GA1UECgwWVWx0aW1hdGUgQXV0aG9yaXR5
IEluYzEWMBQGA1UEAwwNODQuMTE0LjExMy41MjCCASIwDQYJKoZIhvcNAQEBBQAD
ggEPADCCAQoCggEBAMInA7dLJ5ZDOn+FmdEm38flhDGzknHogO/LmP+mijglAU3B
Ez2xR/cBFcIBV+J/qPz07K1pZAHJe6pAYreNUaU1mvpW0tUVNbvsZ8fMyJId1yGQ
3f2jZItpm7ctk6/y94HNEKdcxr3tm66vhxThyxN0GG8j3vKuNsWO9qZZhrxqo6Ta
1QDUMb9bqQi6uvYaemKq0ZNX/fUNQt40n+MH98cAWqjbvwc6mkbqLYzavN575QNc
lf/aqUjvD9asUg1xjCidycoXSPwPFJZcTfh39vEI2Om/Bd0KjC1N494ipmsc+ioe
3ivSqqdAaiuCQlQ4HkAHRCnU57eBthPCmXiG4ocCAwEAATANBgkqhkiG9w0BAQsF
AAOCAQEAhW8YMIFgyRiSMyHsJqMT+yXpHId1wgsSP87CsoLkVSl0vz2iT0qxuS/A
Pt0kAo88ZqyV4Z0bdM7mYncBv+s/TNvp6Zfs5XThXS3EABGI69IyOhyyHlDLQN1j
if48aKKiHVkkaJAD3TNChwuV51dYvD152IObGM99wujwxwSxpQrM1wHcRlQ5qXbl
U8DDISWxkjkHvbOhcg9G3Zr9ZLHtz3jmSe6yH0AMs9kxqX0hP7tra+GStDVJVZ3h
dpMkW0em99r+AUr23CP/JYFcCmbVok1qWymzvC6iUK2vQOSiszUkT69tO2f3wzMM
+8GQJR5ATN7K47ra14V+k3vX3pCX6Q==
-----END CERTIFICATE-----
)EOF";

void setClock()
{
    configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");

    Serial.print("Waiting for NTP time sync: ");
    time_t now = time(nullptr);
    while (now < 8 * 3600 * 2)
    {
        delay(500);
        Serial.print(".");
        now = time(nullptr);
    }
    Serial.println("");
    tm timeinfo;
    gmtime_r(&now, &timeinfo);
    Serial.print("Current time:  ");
    Serial.print(asctime(&timeinfo));
}


struct MainProg
{
    struct EarlyInit
    {
        EarlyInit() { Serial.begin(230400); }
    } _;

    struct Relay
    {
        static const unsigned default_activation_time = 5000;
        const uint8_t& relay_pin = SCL;
        bool relay_on = false;
        unsigned long start_time;

        void reset_start_time()
        {
            start_time = millis();
        }

        void on()
        {
            if(relay_on)
            {
                Serial.println("Relay is already on: only resetting time");
                reset_start_time();
                return;
            }
            Serial.println("Relay was off, turning it on now.");
            relay_on = true;
            reset_start_time();
            digitalWrite(relay_pin, HIGH);
        }

        void off()
        {
            Serial.println("Turning relay off");
            digitalWrite(relay_pin, LOW);
            relay_on = false;
        }

        void check()
        {
            unsigned long now = millis();
            if(relay_on && now > start_time + default_activation_time)
            {
                Serial.println("Relay was on for a long time!");
                off();
            } 
        }
    };

    Relay relay;

    static void showWebpage()
    {
        webserver.send(200, "text/html", html_text);
    }

    void setup()
    {
        // disable LED blinking upon WiFi activity
        //wifi_status_led_uninstall();
        pinMode(LED_BUILTIN, OUTPUT);
        digitalWrite(LED_BUILTIN, HIGH);
        pinMode(SCL, OUTPUT);

          
        WiFi.begin("Mario ist besser als Yoshi", "666fghjkl666A");
        
        while (WiFi.status() != WL_CONNECTED) {
            delay(1000);
            Serial.println("Connecting to WiFi..");
        }

        // 10dBm corresponds to  10 mW
        // 20dBm corresponds to 100 mW
        //WiFi.setOutputPower(10);
        WiFi.setTxPower(WIFI_POWER_11dBm);
        
        Serial.println(WiFi.localIP());
        
        /*// WiFi STA setup
        IPAddress  address(192, 168, 0, 1);
        IPAddress& gateway = address;
        IPAddress  subnet(255, 255, 255, 0);

        WiFiManager wifiManager;
        wifiManager.setAPStaticIPConfig(address, gateway, subnet);
        wifiManager.autoConnect("Intercommie");*/


        setClock(); // Required for X.509 validation

        webserver.setServerKeyAndCert(reinterpret_cast<const uint8_t*>(server_private_key), 1676, reinterpret_cast<const uint8_t*>(server_cert), 1193);
        // webserver.getServer().setRSACert(new X509List(server_cert), new PrivateKey(server_private_key));

        webserver.on("/", [](){webserver.send(200, "text/html", html_text);});
        webserver.on("/relay=on", [&](){/*webserver.send(200, "text/plain", "success");*/ relay.on();});
        webserver.on("/relay=off", [&](){/*webserver.send(200, "text/plain", "success");*/ relay.off();});


        webserver.begin();


        // HTTP Server forwarding to https only
        webserverHTTP.on("/", [](){
                webserverHTTP.sendHeader("Location", String("https://84.114.113.52"), true);
                webserverHTTP.send(301, "text/plain", "");
            });
        webserverHTTP.begin();
        Serial.println("Server started");
    }


    void loop()
    {
        relay.check();
        webserverHTTP.handleClient();
        webserver.handleClient();
    }

private:
} R;

void setup() { R.setup(); }

void loop() { R.loop(); }
