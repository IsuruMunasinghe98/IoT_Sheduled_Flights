#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ESP8266WebServer.h>

// Update these with values suitable for your network.
const char* ssid = "VIP_ADSL";
const char* password = "0776638279";

const char* mqtt_server = "test.mosquitto.org";

//subscribed topics by node-mcu 
const char* flight_count_topic = "flight_count";
const char* departure_date_topic = "departure_date";
const char* arrival_date_topic = "arrival_date";
const char* departure_time_topic = "departure_time";
const char* arrival_time_topic = "arrival_time";
const char* airline_name_topic = "airline_name";

//publishing topic
const char* inputPlacesTopic="arrival_departure_inputs";

//variables for process data
const int dm_flight_count=6;
String data_array[dm_flight_count][6];
String dd_array[dm_flight_count];
String ad_array[dm_flight_count];
String dt_array[dm_flight_count];
String at_array[dm_flight_count];
String airline_array[dm_flight_count];

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
  //Serial.println("called make_data_array");
}
WiFiClient espClient;
PubSubClient client(espClient);
ESP8266WebServer server(80);

//varibles for publish messsages from take_input function
#define MSG_BUFFER_SIZE  (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;
String jsonObj;
String departure,arrival;
char inputBuffer[100];

//variable to store no of flight schedules available
int flight_count=-1;
//variable to store html code of home page "/"
char temp[6000];
//received topics published from node-red
int received_count=0;
//variable to store html code of result page "/table"
String result_page_template = "";

//function to send html code of home page from server
void handleRoot() 
{//storing html webpage for temp variable
  snprintf(temp, 6000,"<html>\
  <head>\
    <link href=\"https://cdn.jsdelivr.net/npm/bootstrap@5.0.2/dist/css/bootstrap.min.css\" rel=\"stylesheet\" integrity=\"sha384-EVSTQN3/azprG1Anm3QDgpJLIm9Nao0Yz1ztcQTwFspd3yD65VohhpuuCOmLASjC\" crossorigin=\"anonymous\">\
    <script src=\"https://cdn.jsdelivr.net/npm/bootstrap@5.0.2/dist/js/bootstrap.bundle.min.js\" integrity=\"sha384-MrcW6ZMFYlzcLA8Nl+NtUVF0sA7MsXsP1UyJoMp4YLEuNSfAP+JcXn/tWtIaxVXM\" crossorigin=\"anonymous\"></script>\
    <script src=\"https://cdn.jsdelivr.net/npm/@popperjs/core@2.9.2/dist/umd/popper.min.js\" integrity=\"sha384-IQsoLXl5PILFhosVNubq5LC7Qb9DXgDA9i+tQ8Zj3iwWAwPtgFTxbJ8NT4GN1R8p\" crossorigin=\"anonymous\"></script>\
    <script src=\"https://cdn.jsdelivr.net/npm/bootstrap@5.0.2/dist/js/bootstrap.min.js\" integrity=\"sha384-cVKIPhGWiC2Al4u+LWgxfKTRIcfu0JTxR+EQDz/bgldoEyl4H0zUF0QKbrJ0EcQF\" crossorigin=\"anonymous\"></script>\
    <meta charset=\"utf-8\">\
    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\
    <title>Flight Schedule Details</title>\
  </head>\
  <style>\
    body{background-image: url('https://img.etimg.com/thumb/width-640,height-480,imgsize-20444,resizemode-1,msid-75090200/industry/transportation/airlines-/-aviation/indian-airlines-hold-on-to-6000-cr-to-avoid-hard-landing/aviation.jpg');background-repeat: no-repeat;background-attachment: fixed;background-size: 100%% 100%%;height: 100%%;width: 100%%;}\
    html{height: 100%%;}\
    .form-container {height: 100%%;display: flex;justify-content: center;align-items: center;}\
    .form-label{font-size: 26px;font-weight: 500;}\
    .form-item-div{display: grid;}\
    .form-styles{background-color: #4c4c4c7a;padding: 44px 26px;border: white;border-style: solid;}\
  </style>\
  <body>\
      <nav class=\"navbar navbar-dark bg-dark\">\
        <div class=\"container-fluid\" style=\"justify-content: center;\">\
          <h1 style=\"text-align:center;\">\
            <font color=\"white\" style=\"font-family: monospace;\">Flight Schedule Details</font>\
          </h1>\
        </div>\
      </nav>\
      <div class=\"form-container col-sm-12 col-md-12 col-lg-12\">\
        <form class=\"col-sm-8 col-md-6 col-lg-4 form-styles\" action=\"/action_page\" method=\"get\">\
          <div class=\"form-item-div\">\
            <label for =\"departure_Airport\" class=\"form-label\" style=\"font-family: monospace;\"> Departure Airport :</label>\
            <input type=\"text\" name=\"departure_Airport\" placeholder=\"-select the Departure Airport-\"id=\"departure_Airport\" list=\"airport_List1\">\
            <datalist id=\"airport_List1\">\
              <option value=\"DXB\">Dubai</option>\
              <option value=\"SIN\">Singapore Changi</option>\
              <option value=\"CDG\">Paris Charles</option>\
              <option value=\"PEK\">Beijing</option>\
              <option value=\"PVG\">Shanghai</option>\
              <option value=\"DEL\">Indira Gandhi</option>\
              <option value=\"AUH\">Abu Dhabhi</option>\
              <option value=\"CMB\">Katunayake</option>\
              <option value=\"MAA\">Chennai</option>\
              <option value=\"YXU\">London</option>\
              <option value=\"SYD\">Sydney</option>\
              <option value=\"MEL\">Melbourn</option>\
              <option value=\"VKO\">Moscow</option>\
              <option value=\"MLE\">Maldives</option>\
              <option value=\"DAC\">Bangladesh</option>\
              <option value=\"JFK\">New York</option>\
              <option value=\"YYZ\">Toronto</option>\
              <option value=\"KHI\">Karachi</option>\
              <option value=\"LHE\">Lohore</option>\
              <option value=\"LAX\">Los Angeles</option>\
              <option value=\"IAD\">Washington</option>\
            </datalist>\
          </div>\
          <br>\
          <div style=\"display: grid;\">\
            <label for =\"arrival_Airport\" class=\"form-label\" style=\"font-family: monospace;\"> Arrival Airport : </label>\
            <input type=\"text\" name=\"arrival_Airport\" placeholder=\"-select the Arrival Airport-\"id=\"arrival_Airport\" list=\"airport_List2\">\
            <datalist id=\"airport_List2\">\
              <option value=\"DXB\">Dubai</option>\
              <option value=\"SIN\">Singapore Changi</option>\
              <option value=\"CDG\">Paris Charles</option>\
              <option value=\"PEK\">Beijing</option>\
              <option value=\"PVG\">Shanghai</option>\
              <option value=\"DEL\">Indira Gandhi</option>\
              <option value=\"AUH\">Abu Dhabhi</option>\
              <option value=\"CMB\">Katunayake</option>\
              <option value=\"MAA\">Chennai</option>\
              <option value=\"YXU\">London</option>\
              <option value=\"SYD\">Sydney</option>\
              <option value=\"MEL\">Melbourn</option>\
              <option value=\"VKO\">Moscow</option>\
              <option value=\"MLE\">Maldives</option>\
              <option value=\"DAC\">Bangladesh</option>\
              <option value=\"JFK\">New York</option>\
              <option value=\"YYZ\">Toronto</option>\
              <option value=\"KHI\">Karachi</option>\
              <option value=\"LHE\">Lohore</option>\
              <option value=\"LAX\">Los Angeles</option>\
              <option value=\"IAD\">Washington</option>\
            </datalist>\
          </div>\
          <br>\
          <div style=\"text-align-last: center;\">\
            <input type=\"submit\" value=\"Get Schedules\" class=\"btn btn-dark\">\
          </div>\
        </form>\
      </div>\  
  </body>\
  </html>");
  server.send(200, "text/html", temp);
}

//taking departure and arrival airports from webpage and publishing through mqtt for node-red, 
//this function will be called if Details button is clicked and "/action_page" is loaded.
void takeInput(){
  arrival=server.arg("arrival_Airport");
  departure=server.arg("departure_Airport");
  jsonObj="{\"arrival\":\""+arrival+"\",\"departure\":\""+departure+"\"}";
  jsonObj.toCharArray(inputBuffer,jsonObj.length()+1);
  snprintf (msg, MSG_BUFFER_SIZE, inputBuffer, value);
  Serial.print("Publish message: ");
  Serial.println(msg);
  client.publish(inputPlacesTopic, msg);
  received_count=0;
  //redirecting to '/table' webpage
  server.sendHeader("Location","/table",true);
  server.send(302,"text/plane","");
}

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
//making html code of "\table" webpage from received input 
void html_table(){
  make_data_array();
  result_page_template+="<html>\
    <head>\
      <link href=\"https://cdn.jsdelivr.net/npm/bootstrap@5.0.2/dist/css/bootstrap.min.css\" rel=\"stylesheet\" integrity=\"sha384-EVSTQN3/azprG1Anm3QDgpJLIm9Nao0Yz1ztcQTwFspd3yD65VohhpuuCOmLASjC\" crossorigin=\"anonymous\">\
      <script src=\"https://cdn.jsdelivr.net/npm/bootstrap@5.0.2/dist/js/bootstrap.bundle.min.js\" integrity=\"sha384-MrcW6ZMFYlzcLA8Nl+NtUVF0sA7MsXsP1UyJoMp4YLEuNSfAP+JcXn/tWtIaxVXM\" crossorigin=\"anonymous\"></script>\
      <script src=\"https://cdn.jsdelivr.net/npm/@popperjs/core@2.9.2/dist/umd/popper.min.js\" integrity=\"sha384-IQsoLXl5PILFhosVNubq5LC7Qb9DXgDA9i+tQ8Zj3iwWAwPtgFTxbJ8NT4GN1R8p\" crossorigin=\"anonymous\"></script>\
      <script src=\"https://cdn.jsdelivr.net/npm/bootstrap@5.0.2/dist/js/bootstrap.min.js\" integrity=\"sha384-cVKIPhGWiC2Al4u+LWgxfKTRIcfu0JTxR+EQDz/bgldoEyl4H0zUF0QKbrJ0EcQF\" crossorigin=\"anonymous\"></script>\
      <meta charset=\"utf-8\">\
      <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\
    </head>\
    <style>\
      body{background-image: url('https://img.etimg.com/thumb/width-640,height-480,imgsize-20444,resizemode-1,msid-75090200/industry/transportation/airlines-/-aviation/indian-airlines-hold-on-to-6000-cr-to-avoid-hard-landing/aviation.jpg');background-repeat: no-repeat;background-attachment: fixed;background-size: 100% 100%;height: 100%;width: 100%;}\
      html{height: 100%;}\
      .table{text-align-last: center;}\
      .table-container{height: 100%;display: block;justify-content: center;align-items: center;text-align-last: center;}\
      .no_planes{height: 20%;display: flex; font-size:25px;font-weight:500;font-color:black;justify-content: center;align-items: center;text-align-last: center;}\
      .no_planes_btn{display: flex;justify-content: center;align-items: center;text-align-last: center;}\
    </style>\
    <body>";
  if (flight_count!=0){//processing the table if flight count is not zero
    result_page_template+="<nav class=\"navbar navbar-dark bg-dark\" style=\"margin: 0px 0px 100px 0px;\">\
        <div class=\"container-fluid\" style=\"justify-content: center;\">\
          <h1 style=\"text-align:center;\">\
            <font color=\"white\" style=\"font-family: monospace;\">Schedules</font>\
          </h1>\
        </div>\
      </nav>\
      <div class=\"col-sm-12 col-md-12 col-lg-12 table-container\">\
        <table class=\"table table-striped table\">\
          <thead class=\"table-dark\">\
            <tr>\
              <th style=\"width:10%\">No:</th>\
              <th style=\"width:22%\">Air Line</th>\
              <th style=\"width:20%\">Departure Date</th>\
              <th style=\"width:20%\">Departure Time</th>\
              <th style=\"width:20%\">Arrival Date</th>\
              <th style=\"width:20%\">Arrival Time</th>\
            </tr>\
          </thead>\
          <tbody  style=\"line-height: 41px;\">\
      ";
    for(int i=0;i<flight_count;i++){
      result_page_template+="<tr style=\"text-align:center\">";
      result_page_template += ("<td>"+data_array[i][0]+"</td>");
      result_page_template += ("<td>"+data_array[i][1]+"</td>");
      result_page_template += ("<td>"+data_array[i][2]+"</td>");
      result_page_template += ("<td>"+data_array[i][3]+"</td>");
      result_page_template += ("<td>"+data_array[i][4]+"</td>");
      result_page_template += ("<td>"+data_array[i][5]+"</td>");
      result_page_template +="</tr>";
      }
    
    result_page_template += "</tbody>\
        </table>\
        ";
    result_page_template += "<div>\
          <a href=\"/\"><button type=\"button\" class=\"btn btn-dark\">Go back</button></a>\
        </div>\
      </div>\
    </body>\
  </html>";
  }
  else{//sending if flight count is zero.
    result_page_template+="<div class=\"no_planes\" style=\"justify-content: center;\">\
      <label  class=\"form-label\" style=\"font-family: monospace;\"> There are no scheduled flights at this moment.</label>\
      </div>\
      ";
     result_page_template += "<div class=\"no_planes_btn\">\
        <a href=\"/\"><button type=\"button\" class=\"btn btn-dark\">Go back</button></a>\
      </div>\
    </body>\
  </html>";
  }
 
  
}

//this function will send received to processed information to webpage "/table"
void table(){

  if(result_page_template!=""){
    server.send ( 200, "text/html",result_page_template);
    Serial.println("Sending data to web-interface");
    received_count=0;
  }
}

//this function process received payloads which are published from node red and store them in arrays according to the topic 
void fill_arrays(byte *payload,String *sArray,unsigned int length){
    String part="";
    int count=0;
    for(int i=2;i<length-2;i++){//processing input to get refresh time
      if((char)payload[i]=='"'){
        //Serial.println(part);
        sArray[count]=part;
        part="";
        count+=1;
        i+=2;
      } 
      else{
        part+=(char)payload[i];
        if(i==length-3){
          sArray[count]=part;
          //Serial.println(part);
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
      Serial.println("flight_count came");
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

void setup() {
  // Initialize the BUILTIN_LED pin as an output
  pinMode(BUILTIN_LED, OUTPUT);     
  Serial.begin(115200);
  setup_wifi();

  //Starting HTTP server
  server.on("/", handleRoot);
  server.on("/action_page", takeInput);
  server.on("/table",table);
  server.begin();
  Serial.println("HTTP server started");

  //Starting MQTT server
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
 
}

void loop() {
  //reconnecting  if mqtt client is disconnected
  if (!client.connected()) {
    reconnect();
  }
  //sending data to web page if all data came from all six subscribed topics
  if(received_count==6){
    html_table();
    table();
    result_page_template="";
    flight_count=-1;
 }
  client.loop();//mqtt
  
  server.handleClient();//html
  
}
