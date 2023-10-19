#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

const char* ssid = "Get-2G-348BE1";
const char* password = "nruvcujmfahwp";

ESP8266WebServer server(80);

void handleRoot() {
  String html = getHTMLPage();
  server.send(200, "text/html", html);
}

void handleSendMessage() {
  String message = server.arg("message");
  String mode = server.arg("mode");
  String serialPacket = "|" + message + ";" + mode + "^";
  String html = getHTMLPage();
  server.send(200, "text/html", html);
  Serial.println(serialPacket);
}

void handleNotFound() {
  server.send(404, "text/plain", "Page Not Found");
}

void connectToWifi() {
  WiFi.begin(ssid, password);
  int retries = 0;
  while (WiFi.status() != WL_CONNECTED && (retries < 15)) {
    retries++;
    delay(1000);
    Serial.println("Connecting to WiFi...^");
  }
  if (retries > 14) {
    Serial.println("WiFi connection FAILED^");
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Connected to WiFi^");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP()+"^");
  }
  Serial.println("Setup ready^");
}

void setup() {
  Serial.begin(115200);

  connectToWifi();

  server.on("/", handleRoot);
  server.on("/sendmessage", handleSendMessage);
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started^");
}

void loop() {
  server.handleClient();
}

String getHTMLPage() {
  return R"(
    <!DOCTYPE html>
    <html>
    <head>
        <style>
            body {
                display: flex;
                flex-direction: column;
                align-items: center;
                justify-content: center;
                height: 100vh;
                margin: 0;
            }

            form {
                text-align: center;
                background-color: #f5f5f5;
                padding: 20px;
                border-radius: 10px;
                width: 350px;
		            align-items: center;
                justify-content: center;
            }

            .checkbox-group {
              display: flex;
              flex-direction: row;
              align-items: center;
	            justify-content: space-evenly;
	            margin-bottom: 20px;
            }

            .checkbox-and-label {
              display: flex;
              flex-direction: row;
              align-items: center;
              justify-content: center;	
            }

            input[type='text'] {
              width: 90%;
              height: 20%;
              padding: 10px;
              border: 1px solid #ccc;
              border-radius: 5px;
            }

            input[type='submit'] {
                background-color: #007BFF;
                color: #fff;
                border: none;
                padding: 10px 20px;
                border-radius: 5px;
                cursor: pointer;
            }
        </style>
    </head>
    <body>
        <h1>Arduino Notifier</h1>
        
        <form>
            <div class='checkbox-group'> 	
              <div class='checkbox-and-label'>
	              <label for='option1'>Low</label>
                <input type='radio' id='option1' name='option' value='option1'>
              </div>
              <div class='checkbox-and-label'>
	              <label for='option2'>Medium</label>
                <input type='radio' id='option2' name='option' value='option2'>
              </div>
              <div class='checkbox-and-label'>
		            <label for='option3'>High</label>
                <input type='radio' id='option3' name='option' value='option3'>
              </div>
            </div>
            
            <input type='text' id='text-input' name='text-input' placeholder='Enter text here'>
            
            <br><br>
            
            <input type='button' value='Submit' onclick='sendRequest()'>
        </form>

      <script>
        
        function sendRequest() {

          const checkboxGroup = document.getElementsByName('option');
          let selectedOption = '';

          for (const checkbox of checkboxGroup) {
            if (checkbox.checked) {
              selectedOption = checkbox.value;
              break;
            }
          }

          let mode = selectedOption.charAt(selectedOption.length-1);

          const text = document.getElementById('text-input').value;

          if(text=='' || selectedOption =='') return;

          const url = 'http://192.168.0.110/sendmessage' + '?message=' + text + '&mode=' + mode;
          
          fetch(url, {
                  method: 'GET',
                  headers: {'Content-Type': 'text/html'}
            })
            .then(response => {
                if (response.ok) {
                  return response.text();
                } else {
                  throw new Error('Request failed');
                }
            })
            .then(responseData => {
              console.log(responseData);
            })
            .catch(error => {
              console.error(error);
            });

            return false;
        }

      </script>	
    </body>
  </html>
  )";
}
