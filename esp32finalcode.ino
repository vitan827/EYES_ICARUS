#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>

AsyncWebServer server(80);
HardwareSerial UNOSerial(2);

bool matrixState[9] = {0};

float temps[9] = {
  0,0,0,
  0,0,0,
  0,0,0
};

const char* html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta charset="utf-8">
<title>ESP32 Matrix Dashboard</title>

<style>

body{
  font-family:Arial;
  max-width:700px;
  margin:auto;
  padding:20px;
}

.grid{
  display:grid;
  grid-template-columns:repeat(3,1fr);
  gap:12px;
}

.cell{

  border-radius:12px;
  padding:20px;
  text-align:center;
  cursor:pointer;
  background:#ddd;
  font-size:18px;
}

.on{
  background:#4CAF50;
  color:white;
}

.temp{
  font-size:14px;
  margin-top:6px;
}

button{
  padding:10px;
  margin-top:10px;
}

</style>
</head>
<body>

<h2>ESP32 MATRIX 3x3</h2>

<div class="grid" id="grid"></div>

<hr>

<form method="POST"
      action="/upload"
      enctype="multipart/form-data">

<input type="file" name="file">

<input type="submit" value="Upload">

</form>

<br>

<button onclick="runFile()">
RUN FILE
</button>

<script>

let states=[
0,0,0,
0,0,0,
0,0,0
];

let temps=[
0,0,0,
0,0,0,
0,0,0
];

function draw(){

 let grid=
 document.getElementById("grid");

 grid.innerHTML="";

 for(let i=0;i<9;i++){

   let div=
   document.createElement("div");

   div.className=
   "cell "+
   (states[i]?"on":"");

   div.innerHTML=
   "CELL "+(i+1)+
   "<div class='temp'>"+
   temps[i]+" °C"+
   "</div>";

   div.onclick=
   ()=>toggle(i);

   grid.appendChild(div);
 }
}

function toggle(index){

 fetch("/toggle?cell="+index)

 .then(r=>r.text())

 .then(()=>{

   states[index]=
   !states[index];

   draw();
 });
}

function runFile(){

 fetch("/run?file=matrix.txt");
}

function refreshTemps(){

 fetch("/temps")

 .then(r=>r.json())

 .then(data=>{

   temps=data;

   draw();
 });
}

draw();

setInterval(
 refreshTemps,
 1000
);

</script>

</body>
</html>
)rawliteral";

void sendMatrixToUNO(){

  String s="";

  for(int i=0;i<9;i++){

    s += matrixState[i] ? "1":"0";
  }

  UNOSerial.println(s);

  Serial.println("SEND:");
  Serial.println(s);
}

void setup(){

  Serial.begin(115200);

  UNOSerial.begin(
    115200,
    SERIAL_8N1,
    16,
    17
  );

  LittleFS.begin(true);

  WiFi.softAP(
    "ESP32_MATRIX"
  );

  Serial.println(
    WiFi.softAPIP()
  );

  server.on("/",
    HTTP_GET,
    [](AsyncWebServerRequest *req){

      req->send(
        200,
        "text/html",
        html
      );
    });

  server.on("/toggle",
    HTTP_GET,
    [](AsyncWebServerRequest *req){

      if(
        req->hasParam("cell")
      ){

        int c=
        req->getParam(
          "cell"
        )->value().toInt();

        if(
          c>=0 &&
          c<9
        ){

          matrixState[c]=
          !matrixState[c];

          sendMatrixToUNO();
        }
      }

      req->send(
        200,
        "text/plain",
        "OK"
      );
    });

  server.on("/temps",
    HTTP_GET,
    [](AsyncWebServerRequest *req){

      String json="[";

      for(int i=0;i<9;i++){

        json +=
        String(
          temps[i],1
        );

        if(i<8)
          json += ",";
      }

      json += "]";

      req->send(
        200,
        "application/json",
        json
      );
    });

  server.on(
    "/upload",
    HTTP_POST,

    [](AsyncWebServerRequest *req){

      req->send(
        200,
        "text/plain",
        "Uploaded"
      );

    },

    [](AsyncWebServerRequest *req,
       String filename,
       size_t index,
       uint8_t *data,
       size_t len,
       bool final){

      static File file;

      if(!index){

        file=
        LittleFS.open(
          "/" + filename,
          FILE_WRITE
        );
      }

      if(file){

        file.write(
          data,
          len
        );
      }

      if(final && file){

        file.close();
      }
    }
  );

  server.on("/run",
    HTTP_GET,
    [](AsyncWebServerRequest *req){

      File file=
      LittleFS.open(
        "/matrix.txt",
        "r"
      );

      if(!file){

        req->send(
          404,
          "text/plain",
          "No file"
        );

        return;
      }

      while(file.available()){

        String line=
        file.readStringUntil(
          '\n'
        );

        line.trim();

        if(
          line.length()>=9
        ){

          UNOSerial.println(
            line
          );

          delay(100);
        }
      }

      file.close();

      req->send(
        200,
        "text/plain",
        "RUN"
      );
    });

  server.begin();
}

void loop(){

  if(
    UNOSerial.available()
  ){

    String line=
    UNOSerial.readStringUntil(
      '\n'
    );

    line.trim();

    if(
      line.startsWith("T:")
    ){

      line.remove(0,2);

      for(
        int i=0;
        i<9;
        i++
      ){

        int comma=
        line.indexOf(',');

        if(comma==-1){

          temps[i]=
          line.toFloat();

          break;
        }

        temps[i]=
        line.substring(
          0,
          comma
        ).toFloat();

        line=
        line.substring(
          comma+1
        );
      }
    }
  }
}