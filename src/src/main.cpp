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
MIIDPjCCAiYCFADEl2YHOqmk0txGyfWbOGpvbIQXMA0GCSqGSIb3DQEBCwUAMFUx
CzAJBgNVBAYTAkFUMQ8wDQYDVQQIDAZzdHlyaWExHzAdBgNVBAoMFlVsdGltYXRl
IEF1dGhvcml0eSBJbmMxFDASBgNVBAMMC0ludGVyY29tbWllMB4XDTIwMTIwNjIy
NTM0MVoXDTMxMTExOTIyNTM0MVowYjELMAkGA1UEBhMCQVQxDzANBgNVBAgMBlN0
eXJpYTENMAsGA1UEBwwER3JhejEfMB0GA1UECgwWVWx0aW1hdGUgQXV0aG9yaXR5
IEluYzESMBAGA1UEAwwJMTAuMC4wLjQ0MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8A
MIIBCgKCAQEAuwFR3h4gwliex7kCtuRuOE8XfzkO7PnT4woO5FfYuPOWloLG+Sz+
uofnE1a7u+vLE/JLDWh7AwTW1EBiNomtV/iRSN/HgkFX3djE7rUhT0m3pS7rX6cn
NgdUcUAyEPvvsy9EUiOXu1jDa1conVKObktWLCWiF0lHRDqDh7sz2zPME2PRZPm9
Xb1eywoq93knUO9Rvbbeqhg7ST1iYHz+MlRerBCQozt7YDjjPyeGpmxegYzaGULl
uwesMaq/kQD3vBG0DpWHHbIHzLMLlTFvUImuGMKGInZhTgt7J5pDle7f9c6aYMW3
HEkxl9+Iy380emDK1/d6icQl8TXouFdjiwIDAQABMA0GCSqGSIb3DQEBCwUAA4IB
AQAqEA8g8+6VYDyqpQ2pb4BLoqc1bDtnbwDF83KTydbQc93NwCr9RkfIdGjX7eaR
aubtlKSmJs7L0dvWxL/8F332edeadNRxdMbaPx/E/E7Wf1JfYdY/rlYZbptKNlsz
cp9C9t/h7sG3k661N+bCnCF77nUGWeMt9SQX978+XX9JRm/NzTGGNs3dvowlNFfB
Dwva2T0060+kEQGYll8tAoX86U4VkZTL0h1bAlr5Awz/g5/IVbbvv7SUcxqguUF2
/oEqP7SX2smyfY991Cew0V7hyFaZ8aPVLGIcfaTkaOiNbNwDBYtd/DT42pipLT3h
7LIPVa+BV5TqhjjCaQ/Pb8Gw
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

          
        //WiFi.begin("Mario ist besser als Yoshi", "666fghjkl666A");
        WiFi.begin("ZS-M_Home", "666fghjkl666");
        
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
        webserver.on("/relay=on", HTTP_POST,  [&](){webserver.send(200, "text/plain", "relay.on"); relay.on();});
        webserver.on("/relay=off", HTTP_POST,  [&](){webserver.send(200, "text/plain", "relay.off"); relay.off();});

        webserver.begin();


        // HTTP Server forwarding to https only
        webserverHTTP.on("/", [](){
                // webserverHTTP.sendHeader("Location", String("https://84.114.113.52"), true);
                webserverHTTP.sendHeader("Location", String("https://10.0.0.44"), true);
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
