
#include <ESP8266WiFi.h>
#include <transfer.h>

#define SSID		"open"
#define WLANPASSWD	""

using namespace std;

WiFiServer server(23);
WiFiClient client;

void setup ()
{
  Serial.begin(115200);
  WiFi.begin(SSID, WLANPASSWD);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println(WiFi.localIP());

  server.begin();
}

void loop ()
{
  if (client.connected())
  {
    transferTo(client, Serial);
    transferTo(Serial, client);
  }

  if (server.hasClient())
  {
    if (client.connected())
      server.available().println("too late.");
    else
      client = server.available();
  }
}
