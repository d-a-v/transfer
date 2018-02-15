
#include <ESP8266WiFi.h>
#include <list>
#include <transfer.h>

#define SSID		"open"
#define WLANPASSWD	""

using namespace std;

class StreamSink
{
  public:

    int	availableForWrite ()
    {
      return 1 << ((sizeof(int) << 3) - 2);
    }

    size_t write (const uint8_t* buffer, size_t length)
    {
      (void)buffer;
      return length;
    }
};

class StreamSource
{
  public:

    int	available ()
    {
      return 1 << ((sizeof(int) << 3) - 2);
    };

    size_t readBytes (char* buffer, size_t length)
    {
      (void)buffer;
      return length;
    };
};

WiFiServer server_echo(6969);
WiFiServer server_sink(6970);
WiFiServer server_source(6971);
StreamSource source;
StreamSink sink;
list<WiFiClient> clients_echo, clients_sink, clients_source;

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
}

void esp_echo_check_broken (list<WiFiClient>& clients)
{
  for (auto cli = clients.begin(); cli != clients.end(); )
    if (!*cli || !cli->connected())
      clients.erase(cli++);
    else
      cli++;
}

void esp_echo_check_new (WiFiServer& server, list<WiFiClient>& clients)
{
  if (server.hasClient())
  {
    clients.push_back(server.available());
    clients.rbegin()->setNoDelay(false);
  }
}

void loop ()
{
  esp_echo_check_broken(clients_echo);
  esp_echo_check_broken(clients_sink);
  esp_echo_check_broken(clients_source);

  for (auto cli : clients_echo)
    transferTo(cli, cli);

  for (auto cli : clients_sink)
    transferTo(cli, sink);

  for (auto cli : clients_source)
    transferTo(source, cli);

  esp_echo_check_new(server_echo, clients_echo);
  esp_echo_check_new(server_sink, clients_sink);
  esp_echo_check_new(server_source, clients_source);
}
