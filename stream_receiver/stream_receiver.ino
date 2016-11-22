#include <ESP8266WiFi.h>
#include "os_type.h"
#include "ets_sys.h"

const char* ssid = "Tinkerer's Lab";
const char* password = "tinker@tl";
// Create an instance of the server
// specify the port to listen on as an argument
WiFiServer server(80);

unsigned char buffer[1500];
int buffer_start = 0, buffer_end = 0;

ETSTimer sampler;

void playSound(void* args){
    if(buffer_start != buffer_end){
        analogWrite(2, buffer[buffer_start]);
        buffer_start = (buffer_start+1)%1500;
    }
}

void setup() {
  Serial.begin(115200);
  delay(10);

  // prepare GPIO2
  pinMode(2, OUTPUT);
  digitalWrite(2, 0);
  
  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  
  // Start the server
  server.begin();
  Serial.println("Server started");

  // Print the IP address
  Serial.println(WiFi.localIP());

  ets_timer_setfn(&sampler, playSound, NULL);

  ets_timer_arm_new(&sampler, 90, 1, 0);
}

void loop() {
  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    return;
  }
  
  // Wait until the client sends some data
  Serial.println("new client");
  while(!client.available()){
    delay(1);
  }

  char sound;
  String request = client.readStringUntil('\r');

  Serial.println(request);

  if(request.indexOf("Play song ") != 0){
      client.write("Invalid request\n");
      client.stop();
  } else {
      unsigned long num_rec = 0;
      unsigned long num_want = request.substring(10,request.length()).toInt();

      Serial.println(num_want);

      while(num_rec < num_want){
        unsigned int buf_size = 1500 - buffer_end + buffer_start, rec = 0;
        if(buf_size < 0 ) buf_size += 1500;
        if(buf_size > 1500) buf_size -= 1500;
        
        if(buf_size > num_want-num_rec) buf_size = num_want-num_rec;

        String req_size = String(buf_size);

        if(buf_size >= 1024 || buf_size == num_want-num_rec){
            Serial.println("Requesting: " + req_size + " " + String(num_rec));
            client.println("Send " + req_size);
            /* client.flush(); */

            while(rec < buf_size){
                while(!client.available()) delay(1);

                sound = client.read();
                /* Serial.print((int)sound);Serial.print(','); */
                buffer[buffer_end] = sound;
                buffer_end = (buffer_end+1)%1500;

                rec++;
            }
        }
        /* analogWrite(2,(((int)req)+128)*4); */

        num_rec += buf_size;
      }

      buffer_start = 0;
      buffer_end = 0;

      client.write("Done!\n");
      client.stop();
      Serial.println("Done!");
  }
 }

