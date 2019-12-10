#include <wifiboypro.h>
#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>

// const int motor1 = 0;
// const int motor2 = 5;
// const int motor3 = 18;
// const int motor4 = 19;
const int forward = 36;
const int back = 39;
const int right = 32;
const int left = 33;
uint8_t state = 0;

esp_now_peer_info_t slave;
#define CHANNEL 3
#define PRINTSCANRESULTS 0
#define DELETEBEFOREPAIR 0

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

// Scan for slaves in AP mode
void ScanForSlave()
{
	int8_t scanResults = WiFi.scanNetworks();
	// reset on each scan
	bool slaveFound = 0;
	memset(&slave, 0, sizeof(slave));

	Serial.println("");
	if (scanResults == 0)
	{
		Serial.println("No WiFi devices in AP Mode found");
	}
	else
	{
		Serial.print("Found ");
		Serial.print(scanResults);
		Serial.println(" devices ");
		for (int i = 0; i < scanResults; ++i)
		{
			// Print SSID and RSSI for each device found
			String SSID = WiFi.SSID(i);
			int32_t RSSI = WiFi.RSSI(i);
			String BSSIDstr = WiFi.BSSIDstr(i);

			if (PRINTSCANRESULTS)
			{
				Serial.print(i + 1);
				Serial.print(": ");
				Serial.print(SSID);
				Serial.print(" (");
				Serial.print(RSSI);
				Serial.print(")");
				Serial.println("");
			}
			// Check if the current device starts with `Slave`
			if (SSID.indexOf("Slave") == 0)
			{
				// SSID of interest
				Serial.println("Found a Slave.");
				Serial.print(i + 1);
				Serial.print(": ");
				Serial.print(SSID);
				Serial.print(" [");
				Serial.print(BSSIDstr);
				Serial.print("]");
				Serial.print(" (");
				Serial.print(RSSI);
				Serial.print(")");
				Serial.println("");
				// Get BSSID => Mac Address of the Slave
				int mac[6];
				if (6 == sscanf(BSSIDstr.c_str(), "%x:%x:%x:%x:%x:%x", &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]))
				{
					for (int ii = 0; ii < 6; ++ii)
					{
						slave.peer_addr[ii] = (uint8_t)mac[ii];
					}
				}

				slave.channel = CHANNEL; // pick a channel
				slave.encrypt = 0;		 // no encryption

				slaveFound = 1;
				// we are planning to have only one slave in this example;
				// Hence, break after we find one, to be a bit efficient
				break;
			}
		}
	}

	if (slaveFound)
	{
		Serial.println("Slave Found, processing..");
	}
	else
	{
		Serial.println("Slave Not Found, trying again.");
	}

	// clean up ram
	WiFi.scanDelete();
}

// Check if the slave is already paired with the master.
// If not, pair the slave with master
bool manageSlave()
{
	if (slave.channel == CHANNEL)
	{
		if (DELETEBEFOREPAIR)
		{
			deletePeer();
		}

		Serial.print("Slave Status: ");
		// check if the peer exists
		bool exists = esp_now_is_peer_exist(slave.peer_addr);
		if (exists)
		{
			// Slave already paired.
			Serial.println("Already Paired");
			return true;
		}
		else
		{
			// Slave not paired, attempt pair
			esp_err_t addStatus = esp_now_add_peer(&slave);
			if (addStatus == ESP_OK)
			{
				// Pair success
				Serial.println("Pair success");
				return true;
			}
			else if (addStatus == ESP_ERR_ESPNOW_NOT_INIT)
			{
				// How did we get so far!!
				Serial.println("ESPNOW Not Init");
				return false;
			}
			else if (addStatus == ESP_ERR_ESPNOW_ARG)
			{
				Serial.println("Invalid Argument");
				return false;
			}
			else if (addStatus == ESP_ERR_ESPNOW_FULL)
			{
				Serial.println("Peer list full");
				return false;
			}
			else if (addStatus == ESP_ERR_ESPNOW_NO_MEM)
			{
				Serial.println("Out of memory");
				return false;
			}
			else if (addStatus == ESP_ERR_ESPNOW_EXIST)
			{
				Serial.println("Peer Exists");
				return true;
			}
			else
			{
				Serial.println("Not sure what happened");
				return false;
			}
		}
	}
	else
	{
		// No slave found to process
		Serial.println("No Slave found to process");
		return false;
	}
}

void deletePeer()
{
	esp_err_t delStatus = esp_now_del_peer(slave.peer_addr);
	Serial.print("Slave Delete Status: ");
	if (delStatus == ESP_OK)
	{
		// Delete success
		Serial.println("Success");
	}
	else if (delStatus == ESP_ERR_ESPNOW_NOT_INIT)
	{
		// How did we get so far!!
		Serial.println("ESPNOW Not Init");
	}
	else if (delStatus == ESP_ERR_ESPNOW_ARG)
	{
		Serial.println("Invalid Argument");
	}
	else if (delStatus == ESP_ERR_ESPNOW_NOT_FOUND)
	{
		Serial.println("Peer not found.");
	}
	else
	{
		Serial.println("Not sure what happened");
	}
}

// send data
void sendData(uint8_t data)
{
	data++;
	const uint8_t *peer_addr = slave.peer_addr;
	Serial.print("Sending: ");
	Serial.println(data);
	esp_err_t result = esp_now_send(peer_addr, &data, sizeof(data));
	Serial.print("Send Status: ");
	if (result == ESP_OK)
	{
		Serial.println("Success");
	}
	else if (result == ESP_ERR_ESPNOW_NOT_INIT)
	{
		// How did we get so far!!
		Serial.println("ESPNOW not Init.");
	}
	else if (result == ESP_ERR_ESPNOW_ARG)
	{
		Serial.println("Invalid Argument");
	}
	else if (result == ESP_ERR_ESPNOW_INTERNAL)
	{
		Serial.println("Internal Error");
	}
	else if (result == ESP_ERR_ESPNOW_NO_MEM)
	{
		Serial.println("ESP_ERR_ESPNOW_NO_MEM");
	}
	else if (result == ESP_ERR_ESPNOW_NOT_FOUND)
	{
		Serial.println("Peer not found.");
	}
	else
	{
		Serial.println("Not sure what happened");
	}
}

// callback when data is sent from Master to Slave
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
	char macStr[18];
	snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
			 mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
	Serial.print("Last Packet Sent to: ");
	Serial.println(macStr);
	Serial.print("Last Packet Send Status: ");
	Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void setup()
{
	// pinMode(motor1, OUTPUT);
	// pinMode(motor2, OUTPUT);
	// pinMode(motor3, OUTPUT);
	// pinMode(motor4, OUTPUT);
	pinMode(forward, INPUT);
	pinMode(back, INPUT);
	pinMode(left, INPUT);
	pinMode(right, INPUT);
	Serial.begin(115200);

	WiFi.mode(WIFI_STA);
	Serial.println("ESPNow/Basic/Master Example");

	Serial.print("STA MAC: ");
	Serial.println(WiFi.macAddress());

	InitESPNow();

	esp_now_register_send_cb(OnDataSent);
}

// void m_Left()
// {
// 	digitalWrite(motor1, LOW);
// 	digitalWrite(motor2, HIGH);
// }

// void m_Right()
// {
// 	digitalWrite(motor1, HIGH);
// 	digitalWrite(motor2, LOW);
// }

// void m_Forward()
// {
// 	digitalWrite(motor3, LOW);
// 	digitalWrite(motor4, HIGH);
// }

// void m_Back()
// {
// 	digitalWrite(motor3, HIGH);
// 	digitalWrite(motor4, LOW);
// }

// void m_Stop()
// {
// 	digitalWrite(motor1, LOW);
// 	digitalWrite(motor2, LOW);
// 	digitalWrite(motor3, LOW);
// 	digitalWrite(motor4, LOW);
// }

void Movement()
{
	switch (state)
	{
	case 0:
		sendData(9);
		break;

	case 1: //forward
		sendData(1);
		break;

	case 2: //back
		sendData(2);
		break;

	case 3: //right
		if (digitalRead(forward) == HIGH && digitalRead(back) == HIGH)
			sendData(3);
		if (digitalRead(forward) == LOW)
			sendData(5);
		if (digitalRead(back) == LOW)
			sendData(6);
		break;

	case 4: //left
		if (digitalRead(forward) == HIGH && digitalRead(back) == HIGH)
			sendData(4);
		if (digitalRead(forward) == LOW)
			sendData(7);
		if (digitalRead(back) == LOW)
			sendData(8);
		break;
	}
}
void loop()
{
	Movement();
	ScanForSlave();
	// If Slave is found, it would be populate in `slave` variable
	// We will check if `slave` is defined and then we proceed further
	if (slave.channel == CHANNEL)
	{ // check if slave channel is defined
		// `slave` is defined
		// Add slave as peer if it has not been added already
		bool isPaired = manageSlave();
		if (isPaired)
		{
		}
		else
		{
			// slave pair failed
			Serial.println("Slave pair failed!");
		}
	}
	else
	{
		// No slave found to process
	}
	if (digitalRead(forward) == LOW && digitalRead(left) == HIGH && digitalRead(right) == HIGH)
	{
		state = 1;
	}
	if (digitalRead(forward) == HIGH && digitalRead(left) == HIGH && digitalRead(right) == HIGH && digitalRead(back) == HIGH)
	{
		state = 0;
	}
	if (digitalRead(back) == LOW && digitalRead(left) == HIGH && digitalRead(right) == HIGH)
	{
		state = 2;
	}
	if (digitalRead(right) == LOW)
	{
		state = 3;
	}
	if (digitalRead(left) == LOW)
	{
		state = 4;
	}
}