#include <WiFi.h>
#include "DHT.h"
#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>
/*******************************************************************************************/
/*In InfluDB, a bucket is named location where the time series data is stored.
 All buckets have a retention period—it defines the duration of time that a bucket retains data. 
 Points with timestamps older than the retention period are dropped.*/
#define DEVICE "ESP32"
// Your WiFi credentials
#define WIFI_SSID "Vodafone555"
#define WIFI_PASSWORD "Vodafone@1999"
/********************************************************************************************/
                        /* ESP32 DHT11 Code */
#define DHTPIN 2     // Digital pin connected to the DHT sensor
//#define DHTTYPE DHT11 
#define DHTTYPE DHT11   // DHT 11 
DHT dht(DHTPIN, DHTTYPE);    //Create DHT instance
float temperature=0.0 , humidity=0.0;  
/********************************************************************************************/
#define INFLUXDB_URL "https://us-east-1-1.aws.cloud2.influxdata.com"
#define INFLUXDB_TOKEN "JXSXB3lPQWm_RQvc9cgzUoi6En6L0lWpPMsEbOerg4BNNj_vo3xi5twj9Tpu66mWbBxjLP0PFnORD61RNVawWw=="
#define INFLUXDB_ORG "30d1c5a8055d5d68"
#define INFLUXDB_BUCKET "ESP"

// Time zone info
#define TZ_INFO "EET-2EEST,M3.5.5/00,M10.5.5/00"


// Declare InfluxDB client instance with preconfigured InfluxCloud certificate
InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN, InfluxDbCloud2CACert);

// Declare Data point
/*measurement: the part of InfluxDB’s structure that describes the data stored in the associated fields.*/
Point sensor("DHT11_Reading");
  
/***********************************Wifi_Init************************************/
void Wifi_Init() 
{
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();
}
/***********************************----------------------***************************************/
void setup() 
{
  Serial.begin(9600); // Initialise serial communication for Serial Monitor
  // Setup wifi Module
  Wifi_Init();
  // Setup DHT11 Sensor
  dht.begin(); 
  /*********************************InfluxDB Connection*******************************************/
  // Add tags
  sensor.addTag("device", DEVICE);
  // Accurate time is necessary for certificate validation and writing in batches
  // We use the NTP servers in your area as provided by: https://www.pool.ntp.org/zone/
  // Syncing progress and the time will be printed to Serial.
  timeSync(TZ_INFO, "pool.ntp.org", "time.nis.gov");
  // Check server connection
  if (client.validateConnection()) {
    Serial.print("Connected to InfluxDB: ");
    Serial.println(client.getServerUrl());
  } else {
    Serial.print("InfluxDB connection failed: ");
    Serial.println(client.getLastErrorMessage());
  }
  /************************************************************************************************/
}

  
void loop() 
{
  /************************************DHT11**************************************************/
  temperature =dht.readTemperature();  // Read temperature in Celsius
  humidity = dht.readHumidity();     // Read Humidity

  Serial.print(F("Humidity: "));   
  Serial.print(humidity,2);
  Serial.print(F("%  Temperature: "));  
  Serial.print( temperature,2);
  Serial.print(F("°C  /  "));
  Serial.print(dht.readTemperature(true), 2);
  Serial.println(F("°F"));
  Serial.println();
  /***********************************InfluxDB**************************************************/
  // Clear fields for reusing the point. Tags will remain the same as set above.
  sensor.clearFields();
  // Store measured values into point
  sensor.addField("temperature", temperature);             // Store Temperature value into point
  sensor.addField("humidity", humidity);               // Store Humidity value into point
  // Print what are we exactly writing
  Serial.print("Writing: ");
  Serial.println(sensor.toLineProtocol());

  // Write point
  if (!client.writePoint(sensor)) {
    Serial.print("InfluxDB write failed: ");
    Serial.println(client.getLastErrorMessage());
  }
  /***************************************************************************************/
  Serial.println("Waiting 3 second");
  delay(3000);
   
}