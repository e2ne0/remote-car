#include <wifiboypro.h>
#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
// #include "wb-sprite.h"

const int motor1 = 0;
const int motor2 = 5;
const int motor3 = 18;
const int motor4 = 19;
const int forward = 36;
const int back = 39;
const int right = 32;
const int left = 33;
int state = 0;
#define CHANNEL 1

// Init ESP Now with fallback
void InitESPNow()
{
    WiFi.disconnect();
    if (esp_now_init() == ESP_OK)
    {
        Serial.println("ESPNow Init Success");
    }
    else
    {
        Serial.println("ESPNow Init Failed");
        // Retry InitESPNow, add a counte and then restart?
        // InitESPNow();
        // or Simply Restart
        ESP.restart();
    }
}

// config AP SSID
void configDeviceAP()
{
    const char *SSID = "Slave_1";
    bool result = WiFi.softAP(SSID, "Slave_1_Password", CHANNEL, 0);
    if (!result)
    {
        Serial.println("AP Config failed.");
    }
    else
    {
        Serial.println("AP Config Success. Broadcasting with AP: " + String(SSID));
    }
}

void m_Left()
{
    digitalWrite(motor1, LOW);
    digitalWrite(motor2, HIGH);
    delay(100);
}

void m_Right()
{
    digitalWrite(motor1, HIGH);
    digitalWrite(motor2, LOW);
    delay(100);
}

void m_Forward()
{
    digitalWrite(motor3, LOW);
    digitalWrite(motor4, HIGH);
}

void m_Back()
{
    digitalWrite(motor3, HIGH);
    digitalWrite(motor4, LOW);
}

void m_Stop()
{
    digitalWrite(motor1, LOW);
    digitalWrite(motor2, LOW);
    digitalWrite(motor3, LOW);
    digitalWrite(motor4, LOW);
}

void OnDataRecv(const uint8_t *mac_addr, const uint8_t *data, int data_len)
{
    char macStr[18];
    snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
             mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
    Serial.print("Last Packet Recv from: ");
    Serial.println(macStr);
    Serial.print("Last Packet Recv Data: ");
    state = *data - 1;
    Serial.println(state);
    Serial.println("");
}

void setup()
{
    // wbpro_init();
    // wbpro_initBuf8();
    Serial.begin(115200);
    pinMode(motor1, OUTPUT);
    pinMode(motor2, OUTPUT);
    pinMode(motor3, OUTPUT);
    pinMode(motor4, OUTPUT);
    pinMode(forward, INPUT);
    pinMode(back, INPUT);
    pinMode(left, INPUT);
    pinMode(right, INPUT);
    // for(int i=0; i<256; i++)
        // wbpro_setPal8(i, wbpro_color565(sprite_pal[i][0],sprite_pal[i][1],sprite_pal[i][2]));
    Serial.println("ESPNow/Basic/Slave Example");
    //Set device in AP mode to begin with
    WiFi.mode(WIFI_AP);
    // configure device AP mode
    configDeviceAP();
    // This is the mac address of the Slave in AP Mode
    Serial.print("AP MAC: ");
    Serial.println(WiFi.softAPmacAddress());
    // Init ESPNow with a fallback logic
    InitESPNow();
    // Once ESPNow is successfully Init, we will register for recv CB to
    // get recv packer info.
    esp_now_register_recv_cb(OnDataRecv);
}

void Movement()
{
    switch (state)
    {
    case 1: //forward
        m_Forward();
        break;

    case 2: //back
        m_Back();
        break;

    case 3: //right
        m_Right();
        break;

    case 4: //left
        m_Left();
        break;

    case 5: //right forward
        m_Right();
        m_Forward();
        break;
    
    case 6: //right back
        m_Right();
        m_Back();
        break;
    
    case 7: //left forward
        m_Left();
        m_Forward();
        break;

    case 8: //left back
        m_Left();
        m_Forward();
        break;

    case 9:
        m_Stop();
        break;
    }
}

void loop()
{
    // wbpro_clearBuf8();
    Movement();
    // wbpro_blitBuf8(state * 8 + 120, 1 * 8, 240, 100, 100, 8, 8, (uint8_t *)sprites);
    // if (digitalRead(forward) == LOW && digitalRead(left) == HIGH && digitalRead(right) == HIGH)
    // {
    //     state = 1;
    // }
    // if (digitalRead(forward) == HIGH && digitalRead(left) == HIGH && digitalRead(right) == HIGH && digitalRead(back) == HIGH)
    // {
    //     state = 0;
    // }
    // if (digitalRead(back) == LOW && digitalRead(left) == HIGH && digitalRead(right) == HIGH)
    // {
    //     state = 2;
    // }
    // if (digitalRead(right) == LOW)
    // {
    //     state = 3;
    // }
    // if (digitalRead(left) == LOW)
    // {
    //     state = 4;
    // }
    // wbpro_blit8();
}