#include <DNSServer.h>
#ifdef ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#endif
#include "ESPAsyncWebServer.h"

DNSServer dnsServer;
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

const IPAddress apIP(192, 168, 2, 1);
const IPAddress gateway(255, 255, 255, 0);

int sensor_pins[] = {35, 32, 34, 39, 33};
const int num_sensors = sizeof(sensor_pins) / sizeof(sensor_pins[0]);

// HTML Page Content
const char *pageContent = R""""(<html>
    <head>
        <style>
            body {
                margin: 0;
            }

            canvas {
                width: 80vmin;
                height: 80vmin;
                position: absolute;
                top: calc((100vh - 80vmin) / 2);
                left: calc((100vw - 80vmin) / 2);
                background: black;
                box-shadow: black 0px 0px 5vmin -0.5vmin;
            }
        </style>
    </head>
    <body>
        <canvas></canvas>
        <script>
            let canv = document.body.querySelector('canvas');
            let ctx = canv.getContext('2d');

            canv.width = 2000
            canv.height = 2000;

            function clamp(value, low, high) {
                return Math.max(Math.min(value, high), low);
            }

            class Grid {
                constructor(size) {
                    this.size = size;
                    this.array = Array.from({length: size}, () => Array.from({length: size}, () => 0.0));
                }

                get(x, y) {
                    return this.array[y][x];
                }

                set(x, y, value) {
                    this.array[y][x] = clamp(value, 0.0, 1.0);
                }

                display() {
                    this.array.forEach(line => {
                        console.log(line.map(x => x.toFixed(1)).join(' '));
                    });
                }

                inBounds(pt) {
                    return pt[0] >= 0 && pt[0] < this.size && pt[1] >= 0 && pt[1] < this.size;
                }

                getNeighbors(x, y) {
                    const offsets = [[-1, 0], [1, 0], [0, -1], [0, 1]];
                    let points = offsets.map(off => [x + off[0], y + off[1]]);
                    return points.filter(pt => this.inBounds(pt));
                }

                getNeighborSum(x, y) {
                    let neighbors = this.getNeighbors(x, y);
                    let values = neighbors.map(pt => this.get(pt[0], pt[1]))
                    return values.reduce((part, x) => part + x, 0);
                }

                setBlock(x, y, w, h, value) {
                    for(let i = 0; i < w; i++) {
                        for(let j = 0; j < h; j++) {
                            if(this.inBounds([x + i, y + j])) this.set(x + i, y + j, value);
                        }
                    }
                }

                draw(canv, ctx) {
                    const sizeX = canv.width / this.size;
                    const sizeY = canv.height / this.size;

                    for(let y = 0; y < this.size; y++) {
                        for(let x = 0; x < this.size; x++) {
                            let blue = 0.5 + (this.get(x, y) / 2);
                            let green = 0.1 + (this.get(x, y) / 4);
                            ctx.fillStyle = `rgb(0, ${green * 255}, ${blue * 255})`;
                            ctx.fillRect(x * sizeX, y * sizeY, sizeX, sizeY);
                        }
                    }
                }                
            }

            function propagate(left, right) {
                let size = left.size;
                for(let y = 0; y < size; y++) {
                    for(let x = 0; x < size; x++) {
                        let sum = left.getNeighborSum(x, y);
                        right.set(x, y, (sum / 2 - right.get(x, y)) * 0.98);
                    }
                }
            }

            function weightedAvgPoint(values, weights) {
                let total = weights.reduce((part, x) => part + x, 0);
                
                let avgX = 0;
                let avgY = 0;
                for(let i = 0; i < values.length; i++) {
                    let ratio = (weights[i] / total);
                    avgX += values[i][0] * ratio;
                    avgY += values[i][1] * ratio;
                }

                return [avgX, avgY];
            }

            function triangulate(data) {
                let sum = data.reduce((part, x) => part + x, 0);
                if(sum < 20) return false;

                const points = [
                    [-1, -1], [1, -1],
                            [0, 0],
                    [-1, 1], [1, 1]
                ];

                return weightedAvgPoint(points, data);
            }

            function dataMap(grid, weightedPoint) {
                let size = grid.size;
                return weightedPoint.map(val => Math.round(((val + 1) / 2) * size));
            }

            

            let one = new Grid(100);
            let two = new Grid(100);
            let swap = false;

            function loop(oldTime) {
                let newTime = Date.now();
                let delta = (newTime - oldTime) / 1000;

                two.draw(canv, ctx);

                if(swap) propagate(two, one);
                else propagate(one, two);
                swap = !swap;

                window.requestAnimationFrame(() => loop(newTime));
            }

            loop(Date.now());

            function spawn(data) {
                let weighted = triangulate(data);
                if(weighted) {
                    let point = dataMap(one, weighted);
                    (swap ? two : one).setBlock(point[0] - 1, point[1] - 1, 3, 3, 1.0);
                }
            }

            const ws = new WebSocket(`ws://192.168.2.1/ws`);
            ws.onmessage = function(e) {
                let data = e.data.split(' ').filter(x => x.length > 0).map(x => parseInt(x));
                spawn(data);
            };
        </script>
    </body>
</html>)"""";

class CaptiveRequestHandler : public AsyncWebHandler {
public:
  CaptiveRequestHandler() {}
  virtual ~CaptiveRequestHandler() {}

  bool canHandle(AsyncWebServerRequest *request){
    return true;
  }

  void handleRequest(AsyncWebServerRequest *request) {
    AsyncResponseStream *response = request->beginResponseStream("text/html");
    response->print(pageContent);
    request->send(response);
  }
};

void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len){
  if(type == WS_EVT_CONNECT) Serial.println("Websocket client connection received");
  else if(type == WS_EVT_DISCONNECT) Serial.println("Websocket client disconnected");
}

void setup(){
  for(int i = 0; i < num_sensors; i++) pinMode(sensor_pins[i], INPUT);
  
  WiFi.softAP("Pond");
  WiFi.softAPConfig(apIP, apIP, gateway);
  Serial.begin(115200);
  dnsServer.start(53, "*", WiFi.softAPIP());
  ws.onEvent(onWsEvent);
  server.addHandler(&ws);
  server.addHandler(new CaptiveRequestHandler()).setFilter(ON_AP_FILTER);
  server.begin();
}

long oldTime = 0;
long totalTime = 0;

String out;
void loop(){
  dnsServer.processNextRequest();

  long newTime = millis();
  long delta = newTime - oldTime;
  if(delta > 20) {
    out = "";
    for(int i = 0; i < num_sensors; i++) {
      out += String(map(analogRead(sensor_pins[i]), 0, 4096, 0, 100));
      out += ' ';
    }
    ws.textAll(out);
    
    oldTime = newTime;
  }
}
