
// repeater / sink / source with SSL
// pointer version, needed for BearSSL
// needs esp-tls.h

#define SSLSERVER_AXTLS   1  // esp-tls.h
#define SSLSERVER_BEARSSL 0  // esp-tls.h
#define NODELAY           false
#define SINKSOURCE        1
#define WLANSSID          "open"
#define PSK               ""
#define CLIENT_NUMBER_MAX 5

#include <ESP8266WiFi.h>
#include <transfer.h>
#include <esp-tls.h>

/////////////////////////

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

enum type_e { type_echo, type_sink, type_source };
int clients_number = 0;
CLIENT* clients [CLIENT_NUMBER_MAX];
type_e  types   [CLIENT_NUMBER_MAX];
SERVER server_echo(6969);
#if SINKSOURCE
SERVER server_sink(6970);
SERVER server_source(6971);
#endif

void setup ()
{
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  WiFi.begin(WLANSSID, PSK);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println(WiFi.localIP());

  SSLSETUPKEYS(&server_echo);
  server_echo.begin();

#if SINKSOURCE

  SSLSETUPKEYS(&server_sink);
  server_sink.begin();

  SSLSETUPKEYS(&server_source);
  server_source.begin();

#endif
}

void check_broken ()
{
  for (int i = 0; i < clients_number; )
    if (!*clients[i] || !clients[i]->connected())
    {
      delete clients[i];
      clients[i] = clients[clients_number - 1];
      types[i] = types[clients_number - 1];
      clients_number--;
    }
    else
      i++;
}

void check_new (SERVER* server, type_e type)
{
  if (server->hasClient())
  {
    if (clients_number == CLIENT_NUMBER_MAX)
      server->available().println("full");
    else
    {
      (clients[clients_number] = new CLIENT(server->available()))->setNoDelay(NODELAY);
      types[clients_number] = type;
      clients_number++;
    }
  }
}

unsigned long last = 0;

StreamSource source;
StreamSink sink;

void loop ()
{
  if (millis() > last)
  {
    Serial.println(ESP.getFreeHeap());
    last += 1000;
  }

  check_broken();

  for (int i = 0; i < clients_number; i++)
    switch (types[i])
    {
#if SINKSOURCE
      case type_source: transferTo(source, *clients[i]); break;
      case type_sink:   transferTo(*clients[i], sink); break;
#endif
      case type_echo:   transferTo(*clients[i], *clients[i]); break;
    }

  check_new(&server_echo, type_echo);
#if SINKSOURCE
  check_new(&server_sink, type_sink);
  check_new(&server_source, type_source);
#endif
}
