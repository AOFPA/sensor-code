#include <ESP8266WiFi.h>    // WiFi
#include <PubSubClient.h>   // MQTT client

#define WIFI_STA_NAME "realme X7 Pro 5G"
#define WIFI_STA_PASS "15062543"

//MQTT protocol
#define MQTT_SERVER   "34.121.1.224"   //ip server ของคุณ
#define MQTT_PORT     1883
#define MQTT_USERNAME "aofpa"    //user mqtt ที่คุณสร้าง
#define MQTT_PASSWORD "1234"
#define MQTT_NAME     "aofMQTT"   //ตั้งชื่ออะไรก็ได้

#define TRIGGER_PIN  D1
#define ECHO_PIN     D2

WiFiClient client;
PubSubClient mqtt(client);

void setup() {
  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(BUILTIN_LED, OUTPUT);

  Serial.begin(9600);         //ปรับตามที่ตั้งค่าใน Device Manager
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_STA_NAME);

  //เซ็ตโหมดไวไฟ
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_STA_NAME, WIFI_STA_PASS);

  //พยายามเชื่อมต่อไวไฟจนกว่าจะต่อได้
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  }

  //เชื่อมต่อสำเร็จ
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  //เซ็ตค่า mqtt server
  mqtt.setServer(MQTT_SERVER, MQTT_PORT);
  mqtt.setCallback(mqtt_callback);

}

//ถ้าไวไฟหลุด จะเรียกฟังก์ชันนี้ จนกว่าจะต่อใหม่ได้
void reconnectWiFi() {
  Serial.print("Reconnecting");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_STA_NAME, WIFI_STA_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected!");
}

//ฟังก์ชันการทำงานของ MQTT
void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  //อ่านข้อความที่รับมา
  payload[length] = '\0';
  String topic_str = topic, payload_str = (char*)payload;
  Serial.println("[" + topic_str + "]: " + payload_str);

  //กำหนดเงื่อนไขที่แตกต่างกันตาม Topic
  if ( topic_str == "/device001/SetT1" ) {
    Serial.print( "sub-/device001/SetT1 : " );
    Serial.println( payload_str );
  } else if ( topic_str == "/device001/SetT2" ) {
    Serial.print( "sub-/device001/SetT2 : " );
    Serial.println( payload_str );
  }
}

long getdistance() {
  long duration, distance;
  digitalWrite(TRIGGER_PIN, LOW);  // Added this line
  delayMicroseconds(2); // Added this line
  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(10); // Added this line
  digitalWrite(TRIGGER_PIN, LOW);
  duration = pulseIn(ECHO_PIN, HIGH);
  distance = (duration / 2) / 29.1;
  Serial.print(distance);
  Serial.println(" cm");
  delay(1000);
  return distance ;
}


void loop() {

  //ถ้าการเชื่อมต่อไวไฟหาด จะพยายามต่อใหม่จนกว่าจะสำเร็จ
  if (mqtt.connected() == false) {
    Serial.print( "WiFi Status : " );
    Serial.println( WiFi.status() );
    if (WiFi.status() != WL_CONNECTED) {
      reconnectWiFi();
    }

    //พยายามเชื่อมต่อ MQTT server
    Serial.print("MQTT connection... ");
    if (mqtt.connect(MQTT_NAME, MQTT_USERNAME, MQTT_PASSWORD)) {  //for mqtt cloud
      //ต่อสำเร็จ แล้ว subscribe 2 topics
      Serial.println("connected");
      mqtt.subscribe("/device001/SetT1");
      mqtt.subscribe("/device001/SetT2");
    } else {
      //ต่อ MQTT Server ล้มเหลว รอ 5000 mili-sec แล้วพยายามต่อใหม่
      Serial.println("failed");
      delay(5000);
    }
  } else {
    //ถ้า MQTT ต่ออยู่แล้ว ก็ให้ทำงานใน callback
    mqtt.loop();
  }
  long distance = getdistance();
  char buf[16];

// assign lval something then...
  ltoa(distance,buf,10);


  
  //ทดลอง publish ทุกๆ 3 วินาที
  //Send publish to MQTT Broker
  mqtt.publish("/device001/hello", buf);
  delay(3000);

}
