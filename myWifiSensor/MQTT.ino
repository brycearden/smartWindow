/*
 Basic MQTT example 
 
  - connects to an MQTT server
  - publishes "hello world" to the topic "outTopic"
  - subscribes to the topic "inTopic"
*/

#include <SPI.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <aJson.h>

// function definitions
char *  parseJson(char *jsonString) ;

// your network name also called SSID
char ssid[] = "2226";
// your network password
char password[] = "apt2226wifi";
// MQTTServer to use
char server[] = "iot.eclipse.org";

void callback(char* topic, byte* payload, unsigned int length) {
    payload[length] = '\0';
    outputVal = atoi((char *) payload);
    Serial.print("outputVal: "); Serial.println(outputVal);
}

WiFiClient wifiClient;
PubSubClient client(server, 1883, callback, wifiClient);

/*
 * Connects to the internet using the provided ssid and password
 * Creates a new Event that sends ack to JSONify.ino to begin creating
 * JSON Strings to output to server
 */
void setup()
{
  Serial.begin(115200);
  wifiReady.begin();      // create a new Event
  delay(100);
  
  // Start Ethernet with the build in MAC Address
  // attempt to connect to Wifi network:
  Serial.print("Attempting to connect to Network named: ");
  // print the network name (SSID);
  Serial.println(ssid); 
  // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
  WiFi.begin(ssid, password);
  while ( WiFi.status() != WL_CONNECTED) {
    // print dots while we wait to connect
    Serial.print(".");
    delay(300);
  }
  
  Serial.println("\nYou're connected to the network");
  Serial.println("Waiting for an ip address");
  
  while (WiFi.localIP() == INADDR_NONE) {
    // print dots while we wait for an ip addresss
    Serial.print(".");
    delay(300);
  }

  Serial.println("\nIP Address obtained");
  // We are connected and have an IP address.
  // Print the WiFi status.
  printWifiStatus();
  wifiReady.send();
}

/*
 * Waits for a JSONpayload to be created
 * Subscribes to MQTT broker to retrieve new I2C output value
 * Publishes JSON payload with most recent light reading for fuzzylogic processing
 */
void loop()
{
  myEvent.waitFor();      // wait for the next JSON event to be triggered
  // Reconnect if the connection was lost
  if (!client.connected()) {
    Serial.println("Disconnected. Reconnecting....");

    if(!client.connect("smartWindowClient")) {
      Serial.println("Connection failed");
    } else {
      Serial.println("Connection success");
      if(client.subscribe("outputFuzzyVal")) {      // pull I2C output val from node-red server 
        Serial.println("Subscription successfull");
      }
    }
  }
  
  if(client.publish("mspSensorVal", jsonPayload)) {    // publish the json message to MQTT broker
    Serial.print("MQTT published! ("); Serial.print(light); Serial.println(")");
    free(jsonPayload);  // this libary uses malloc, so we must free after json event is sent
  } else {
    Serial.println("Publish failed");
  }
 
  // Check if any message were received
  // on the topic we subsrcived to
  client.poll();
  delay(100);
}

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

/*
 * Parse the JSON String we receive from the MQTT server
 * returns the integer value from the light sensor
 * this code is not used at the moment because we changed the output of the message from the server side
 * however, if we ever have to parse a Json message on the msp side, this will be useful
 */
char * parseJson(char *jsonString) 
{
    if(jsonString == NULL) { return NULL; }
    char* value;
    
    aJsonObject* root = aJson.parse(jsonString);
    // value = aJson.print(root)+'\0'; // Convert JSON object to char array
    // Serial.println(value);
    
    if (root != NULL) {    // myWifiSensor
       // Serial.println("Parsed successfully 1 " );
       aJsonObject* light = aJson.getObjectItem(root, "light");
       // value = aJson.print(light)+'\0'; // Convert JSON object to char array
       // Serial.println(value);
       if(light != NULL) {
           value = light->valuestring;
               // Serial.println(value);
               // return atoi(value);
       }
    }  
    
    if(value) {
      return value;
    }
    else{
      return NULL;
    }  
}

