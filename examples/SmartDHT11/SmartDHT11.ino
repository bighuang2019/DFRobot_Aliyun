#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "DFRobot_Aliyun.h"
#include "DFRobot_DHT11.h"

#define DHT11_PIN  D2

/*配置WIFI名和密码*/
const char * WIFI_SSID     = "WIFI_SSID";
const char * WIFI_PASSWORD = "WIFI_PASSWORD";

/*配置设备证书信息*/
String ProductKey = "you Product Key";
String ClientId = "12345";
String DeviceName = "you Device Name";
String DeviceSecret = "you Device Secret";

/*配置域名和端口号*/
String ALIYUN_SERVER = "iot-as-mqtt.cn-shanghai.aliyuncs.com";
uint16_t PORT = 1883;

/*需要操作的产品标识符(温度和湿度两个标识符)*/
String TempIdentifier = "you Temp Identifier";
String HumiIdentifier = "you Humi Identifier";

/*需要上报和订阅的两个TOPIC*/
const char * subTopic = "you sub Topic";//****set
const char * pubTopic = "you pub Topic";//******post

DFRobot_Aliyun myAliyun;
WiFiClient espClient;
PubSubClient client(espClient);
DFRobot_DHT11 DHT;

void connectWiFi(){
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID,WIFI_PASSWORD);
  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi connected");
  Serial.print("IP Adderss: ");
  Serial.println(WiFi.localIP());
}

void callback(char * topic, byte * payload, unsigned int len){
  Serial.print("Recevice [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < len; i++){
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void ConnectAliyun(){
  while(!client.connected()){
    Serial.print("Attempting MQTT connection...");
    /*根据自动计算的用户名和密码连接到Alinyun的设备，不需要更改*/
    if(client.connect(myAliyun.client_id,myAliyun.username,myAliyun.password)){
      Serial.println("connected");
      client.subscribe(subTopic);
    }else{
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}
void setup(){
  Serial.begin(115200);
  
  /*连接WIFI*/
  connectWiFi();
  
  /*初始化Alinyun的配置，可自动计算用户名和密码*/
  myAliyun.init(ALIYUN_SERVER,ProductKey,ClientId,DeviceName,DeviceSecret);
  
  client.setServer(myAliyun.mqtt_server,PORT);
  
  /*设置回调函数，当收到订阅信息时会执行回调函数*/
  client.setCallback(callback);
  
  /*连接到Aliyun*/
  ConnectAliyun();
}

uint8_t tempTime = 0;
void loop(){
  if(!client.connected()){
    ConnectAliyun();
  }
  /*一分钟上报两次温湿度信息*/
  if(tempTime > 60){
    tempTime = 0;
    DHT.read(DHT11_PIN);
    Serial.print("DHT.temperature=");
    Serial.println(DHT.temperature);
    Serial.print("DHT.humidity");
    Serial.println(DHT.humidity);
    client.publish(pubTopic,("{\"id\":"+ClientId+",\"params\":{\""+TempIdentifier+"\":"+DHT.temperature+",\""+HumiIdentifier+"\":"+DHT.humidity+"},\"method\":\"thing.event.property.post\"}").c_str());
  }else{
    tempTime++;
    delay(500);
  }
  client.loop();
}