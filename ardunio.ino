    
  // Include libraries
  #include <dht11.h>
  #include <SoftwareSerial.h> 

  /*
    -> Since we will connect the ESP8266 module to the network to which the computer is connected, 
      we must specify the ip address assigned to the computer (you can find out using the "ipconfig" command with cmd)

    -> The port number is the channel to be listened by the Node Js HTTP server.
  */
  String ipAddress  = "<YOUR LOCAL IP ADDRESS>";   
  String portNumber = "<PORT NUMBER>";               
  
  // Network name and network password required to connect the ESP8266 module to the wifi network
  String networkName     = "<YOUR NETWORK NAME>";                   
  String networkPassword = "<YOUR NETWORK PASSWORD>";           
  
  // Outputs are specified, where the connection legs of the ESP8266 module can read and send data are connected
  int rxPin = 11;                                               
  int txPin = 10;   
  SoftwareSerial esp(rxPin, txPin);   

  // We specify the output on the ardunio to which the DHT11 sensor is connected
  dht11 DHT11;
  int dht11Pin = 2,
      dhtTemperature, 
      dhtHumidity;           
  
  void setup() {    
    Serial.begin(9600);
    esp.begin(115200);
    Serial.println("Google Cloud IoT Core MQTT");
           
    // We send an "AT" command to find out that the ESP8266 module is available                                  
    esp.println("AT");                                         
    Serial.println("'AT' command sent to ESP8266");
    while(!esp.find("OK")){                                     
      esp.println("AT");
      Serial.println("----- ESP8266 not find -----");
    }
    Serial.println("'OK' command received");    

    // We determine in which mode the ESP8266 module will work
    // AT+CWMODE=1  => STA
    // AT+CWMODE=2  => AP
    // AT+CWMODE=3  => STA + AP 
    esp.println("AT+CWMODE=1");                                 
    while(!esp.find("OK")){                                     
      esp.println("AT+CWMODE=1");
      Serial.println("===== Setting ESP8266 for STA mode =====");
    }
    Serial.println("ESP8266 set as STA mode"); 
    
    // Now that the settings are made, we send the command required for the ESP8266 module to connect to the network
    Serial.println("Connecting to the network ...");
    esp.println("AT+CWJAP=\""+networkName+"\",\""+networkPassword+"\"");    
    while(!esp.find("OK"));                                     
    Serial.println("Connected to the network");
    Serial.print("\n");
    delay(1000);
  }
  
  void loop() {
    // We connect the ESP8266 module to the HTTP server in localhost that we created with NodeJs
    esp.println("AT+CIPSTART=\"TCP\",\""+ipAddress+"\"," + portNumber);         
    if(esp.find("Error")){                                      
      Serial.println("----- AT+CIPSTART error -----");
    }
    
    // Reading temperature and humidity data from DHT11 sensor
    DHT11.read(dht11Pin);
    dhtTemperature = (int)DHT11.temperature;
    dhtHumidity    = (int)DHT11.humidity;

    Serial.println("Opening local server connection");    
    
    String url = "GET http://" + ipAddress + ":" + portNumber + "/iot";                                   
    url += "?temperature=";
    url += String(dhtTemperature);
    url += "&humidity=";
    url += String(dhtHumidity);
    url += "\r\n\r\n"; 
    
    esp.print("AT+CIPSEND=");                                   
    esp.println(url.length()+2);
    delay(2000);
    
    // The ESP8266 module transmits data to the server in localhost via HTTP protocol with GET request
    // If the operation is successful
    if(esp.find(">")){  
      Serial.println("GET REQUEST SUCCESS");                                       
      esp.print(url);                                          
      // Serial.println(url);
      Serial.print("Data transferred to local server [ temperature : ");
      Serial.print(dhtTemperature);
      Serial.print(", ");
      Serial.print("humidity : ");
      Serial.print(dhtHumidity);
      Serial.println(" ]");
      delay(1000);
    } else {
      Serial.println("GET REQUEST FAILED");
    }

    // Connection closing
    Serial.println("Closing local server connection"); 
    Serial.print("\n"); 
    esp.println("AT+CIPCLOSE");  
    
    delay(7000);                                              
  }
