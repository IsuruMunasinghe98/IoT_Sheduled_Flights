#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <FirebaseArduino.h>

// firbase database authentiacation values
#define FIREBASE_HOST "iotgroup3-3bd3b-default-rtdb.asia-southeast1.firebasedatabase.app"
#define FIREBASE_AUTH "ycsm5IwR68UMXC2zz3ZHmDum8FTKDCOSy4NSFaOj"

//varibles for publish messsages from take_input function
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE  (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;
String jsonObj;
String departure;
String arrival;
char inputBuffer[100];
//variable to store no of flight schedules available
int flight_count=-1;
//no. of received topics published from node-red
int received_count=0;
//if there is a change in data
bool change=false;

// Update these with values suitable for your network.
const char* ssid = "xxxxxxx";
const char* password = "xxxxxxxxxx";

//variables to hold received data from data base
String start_point="";
String end_point="";
//mqtt broker
const char* mqtt_server = "test.mosquitto.org";

//subscribed topics by node-mcu 
const char* flight_count_topic = "flight_count";
const char* departure_date_topic = "departure_date";
const char* arrival_date_topic = "arrival_date";
const char* departure_time_topic = "departure_time";
const char* arrival_time_topic = "arrival_time";
const char* airline_name_topic = "airline_name";

//subscribed topics by node-mcu 
const char* inputPlacesTopic="arrival_departure_inputs";


//variables for process data
const int dm_flight_count=5;
String data_array[dm_flight_count][6];
String dd_array[dm_flight_count];
String ad_array[dm_flight_count];
String dt_array[dm_flight_count];
String at_array[dm_flight_count];
String airline_array[dm_flight_count];
String stations[2];
int count_time=0;
int n = 0;

WiFiClient espClient;
PubSubClient client(espClient);

//function to setting up wi-fi
void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

//function to create data array from inputs of subscribed topics
void make_data_array(){
  for(int i=0;i<dm_flight_count;i++){
    data_array[i][0]=String(i+1);
    data_array[i][1]=airline_array[i];
    data_array[i][2]=dd_array[i];
    data_array[i][3]=dt_array[i];
    data_array[i][4]=ad_array[i];
    data_array[i][5]=at_array[i];
  }
}


//this function process received payloads which are published from node red and store them in arrays according to the topic 
void fill_arrays(byte *payload,String *sArray,unsigned int length){
    String part="";
    int count=0;
    for(int i=2;i<length-2;i++){//processing input to get refresh time
      if((char)payload[i]=='"'){
        sArray[count]=part;
        part="";
        count+=1;
        i+=2;
      } 
      else{
        part+=(char)payload[i];
        if(i==length-3){
          sArray[count]=part;
          
        }
      }
    }
     
}

//receiving messages from subsribed topics
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.println();
  Serial.println(topic);
  Serial.println();
 
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  
  String stringTopic="";
  for(int i=0;i<strlen(topic);i++){
    stringTopic+=topic[i];
  }
  String bufferString="";
  if(stringTopic=="flight_count"){//if a message from flight_count topic arrived node mcu 
      for(int i=0;i<1;i++){//processing input to get refresh time
        bufferString+=(char)payload[i];
        
      }
      flight_count=bufferString.toInt();//converting the string to integar and assigning int to the variable
   }
   else if(stringTopic=="airline_name"){ 
     fill_arrays(payload,airline_array,length);
   }
   else if(stringTopic=="departure_date"){ 
     fill_arrays(payload,dd_array,length);
   }
   else if(stringTopic=="departure_time"){ 
     fill_arrays(payload,dt_array,length);
   }
   else if(stringTopic=="arrival_date"){ 
     fill_arrays(payload,ad_array,length);
   }
   else if(stringTopic=="arrival_time"){ 
     fill_arrays(payload,at_array,length);
   }
  received_count+=1;
 
  

}


void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      //client.publish(outTopic, "hello world");
      // ... and resubscribe
      
      client.subscribe(departure_date_topic);
      client.subscribe(arrival_date_topic);
      client.subscribe(departure_time_topic);
      client.subscribe(arrival_time_topic);
      client.subscribe(airline_name_topic);
      client.subscribe(flight_count_topic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
//function to get data from the database
void getStations(){
   
 
  end_point=Firebase.getString("end_point");
  delay(500);
  start_point=Firebase.getString("start_point");
  
  if (start_point==departure and end_point==arrival){
    if(count_time>100){
      if(start_point!="" and end_point!=""){
        
        change=true;
        count_time=0;
        stations[0]=start_point;
        stations[1]=end_point;
        departure=start_point;
        arrival=end_point;
        jsonObj="{\"arrival\":"+arrival+",\"departure\":"+departure+"}";
        jsonObj.toCharArray(inputBuffer,jsonObj.length()+1);
        snprintf (msg, MSG_BUFFER_SIZE, inputBuffer, value);
        Serial.print("Publish message: ");
        Serial.println(msg);
        client.publish(inputPlacesTopic, msg);
      }
      else{
        
      }
    }
    else{

      count_time+=1;
    }
    
    
  }
  else{
    
      if(start_point!="" and end_point!=""){
     
        change=true;
        count_time=0;
        stations[0]=start_point;
        stations[1]=end_point;
        departure=start_point;
        arrival=end_point;
        jsonObj="{\"arrival\":"+arrival+",\"departure\":"+departure+"}";
        jsonObj.toCharArray(inputBuffer,jsonObj.length()+1);
        snprintf (msg, MSG_BUFFER_SIZE, inputBuffer, value);
        Serial.print("Publish message: ");
        Serial.println(msg);
        client.publish(inputPlacesTopic, msg);
      }
      else{
 
      }
   
  }
  

  
}
//function to send data from node mcu to firbase
void sendData(String Name,String Schedule){
   // update value
  Firebase.setString(Name,Schedule);
  // handle error
  if (Firebase.failed()) {
      Serial.print("setting /number failed:");
      Serial.println(Firebase.error());  
      return;
  }
  delay(200);  
}
//uploading to details of scheduled flights to database
void upload_to_db(){
  make_data_array();
  String chunk;
  if(flight_count==0){
    chunk="\"There are no Planes\"";
    sendData("schedule0",chunk);
  }else{
    sendData("schedule0","");
    for(int i=0;i<flight_count;i++){
      chunk="\"0"+data_array[i][0]+" "+data_array[i][2]+" "+data_array[i][3]+" "+data_array[i][5]+" "+data_array[i][1]+" .\"";
    //schedule[i]=chunk;
      Serial.println(chunk);
      sendData("schedule"+String(i+1),chunk);
      chunk="";
    }
  }
}
void setup() {
  // Initialize the BUILTIN_LED pin as an output
  pinMode(BUILTIN_LED, OUTPUT);     
  Serial.begin(115200);
  setup_wifi();
  //Starting MQTT server
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  //setting up firebase
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
}



void loop() {
  //reconnecting  if mqtt client is disconnected
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  getStations();
  //sending data to web page if all data came from all six subscribed topics
  if(received_count==6){
    if(change){
      Serial.println("Uploading data to database");
      upload_to_db();
      change=false;
      received_count=0;
      flight_count=-1;
    }
    
 }
  
  

//  }
}
