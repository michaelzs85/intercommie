#include <Arduino.h>

//#include <DNSServer.h>
//#include <ESP8266WebServer.h>
#include <IPAddress.h>
#include <WiFiManager.h>

WiFiServer server(80);
//WiFiServerSecure server(80);


struct
{
    struct EarlyInit
    {
        EarlyInit() { Serial.begin(230400); }
    } _;

    void setup()
    {
        // disable LED blinking upon WiFi activity
        wifi_status_led_uninstall();
        pinMode(LED_BUILTIN, OUTPUT);
        digitalWrite(LED_BUILTIN, HIGH);

        // WiFi STA setup
        IPAddress  address(192, 168, 0, 1);
        IPAddress& gateway = address;
        IPAddress  subnet(255, 255, 255, 0);

        WiFiManager wifiManager;
        wifiManager.setAPStaticIPConfig(address, gateway, subnet);
        wifiManager.autoConnect("Intercommie");

        Serial.println(F("setup: wifi connected"));

        // 10dBm corresponds to  10 mW
        // 20dBm corresponds to 100 mW
        WiFi.setOutputPower(10);

        pinMode(D1, OUTPUT);

        server.begin();
        Serial.println("Server started");


    }

    void loop()
    {
        WiFiClient client = server.available();
        if(!client)
            return;

        Serial.println("new client");
        while(!client.available())
            delay(1);
        
        String request = client.readStringUntil('\r');
        Serial.println(request);
        client.flush();

        int value = LOW;
        if (request.indexOf("/D1=ON") != -1) {
            digitalWrite(D1, HIGH);
            value = HIGH;
        } 
        if (request.indexOf("/D1=OFF") != -1){
            digitalWrite(D1, LOW);
            value = LOW;
        }

        client.println("HTTP/1.1 200 OK");
        client.println("Content-Type: text/html");
        client.println(""); //  do not forget this one
        client.println("<!DOCTYPE HTML>");
        client.println("<html>");
        
        client.print("D1 pin is now: ");
        
        if(value == HIGH) {
            client.print("On");  
        } else {
            client.print("Off");
        }
        client.println("<br><br>");
        client.println("Click <a href=\"/D1=ON\">here</a> turn the door relays ON<br>");
        client.println("Click <a href=\"/D1=OFF\">here</a> turn the door relays OFF<br>");
        client.println("</html>");
        
        delay(1);
        Serial.println("Client disconnected");
        Serial.println("");
    }

private:
} R;

void setup() { R.setup(); }

void loop() { R.loop(); }
