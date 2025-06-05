#include <VS1053.h>
#include <WiFi.h>
#include <WiFiManager.h>

// Broches utilisées
#define VS1053_CS     32
#define VS1053_DCS    33
#define VS1053_DREQ   15
#define SCI_SPATIAL   0x0B  // Registre pour spatialisation

// Réseau WiFi
const char *ssid = "iPhone de Maimouna";
const char *password = "Mounasse22";

#define BUFFSIZE 64
uint8_t mp3buff[BUFFSIZE];

#define NOMBRECHAINES 10
int chaine = 0;

char host[40];
char path[40];
int httpPort;

int volume = 85;
int bassAmp = 5, trebleAmp = 5;
int bassFreq = 100, trebleFreq = 10;
int spatialMode = 0;

VS1053 player(VS1053_CS, VS1053_DCS, VS1053_DREQ);
WiFiClient client;

// Fonction d'application de la tonalité
void appliquerTonalite(int bassFreq, int bassAmp, int trebleFreq, int trebleAmp) {
  uint8_t tone[4];
  tone[0] = (uint8_t)(bassFreq);
  tone[1] = (uint8_t)(bassAmp);
  tone[2] = (uint8_t)(trebleFreq);
  tone[3] = (uint8_t)(trebleAmp < 0 ? 256 + trebleAmp : trebleAmp);
  player.setTone(tone);
}

// Spatialisation
void setSpatialization(uint8_t mode) {
  switch (mode) {
    case 0:
      player.writeRegister(SCI_SPATIAL, 0x0000);
      Serial.println("Spatialisation: Off");
      break;
    case 1:
      player.writeRegister(SCI_SPATIAL, 0x2020);
      Serial.println("Spatialisation: Mode 1");
      break;
    case 2:
      player.writeRegister(SCI_SPATIAL, 0x4040);
      Serial.println("Spatialisation: Mode 2");
      break;
    case 3:
      player.writeRegister(SCI_SPATIAL, 0x6060);
      Serial.println("Spatialisation: Mode 3");
      break;
  }
}

// Connexion à une station radio
void connexionChaine() {
  switch (chaine) {
    case 0:
      strcpy(host, "stream03.ustream.ca");
      strcpy(path, "/cism128.mp3");
      httpPort = 8000;
      break;
    case 1:
      strcpy(host, "chisou-02.cdn.eurozet.pl");
      strcpy(path, "/;");
      httpPort = 8112;
      break;
    case 2:
      strcpy(host, "streamer01.sti.usherbrooke.ca");
      strcpy(path, "/cfak.mp3");
      httpPort = 8000;
      break;
    case 3:
      strcpy(host, "ecoutez.chyz.ca");
      strcpy(path, "/mp3");
      httpPort = 8000;
      break;
    case 4:
      strcpy(host, "ais-edge83-jbmedia-nyc04.cdnstream.com");
      strcpy(path, "/hot108");
      httpPort = 80;
      break;
    case 5:
      strcpy(host, "urbanhitrapfr.ice.infomaniak.ch");
      strcpy(path, "/urbanhitrapfr-128.mp3");
      httpPort = 80;
      break;
    case 6:
      strcpy(host, "icecast.radiofrance.fr");
      strcpy(path, "/mouvrapfr-midfi.mp3");
      httpPort = 80;
      break;
    case 7:
      strcpy(host, "icecast.skyrock.net");
      strcpy(path, "/skyrock.mp3");
      httpPort = 80;
      break;
  }

  Serial.print("Connection à ");
  Serial.println(host);

  if (!client.connect(host, httpPort)) {
    Serial.println("Échec de la connexion");
    return;
  }

  Serial.print("Demande du stream : ");
  Serial.println(path);

  client.print(String("GET ") + path + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");
}

void setup() {
  Serial.begin(115200);
  Serial.println("\n\nRadio WiFi");

  Serial.println("Contrôles clavier :");
  Serial.println("\t n : Changer de chaîne");
  Serial.println("\t + / - ou y / b : Volume +/-");
  Serial.println("\t g / f : Basses +/-");
  Serial.println("\t j / h : Aigus +/-");
  Serial.println("\t d : Tonalité par défaut");
  Serial.println("\t s : Spatialisation");

WiFiManager wifiManager; 
 if (!wifiManager.autoConnect("RadioESP32")) {
   Serial.println("Échec de connexion WiFi");
    delay(3000);
    ESP.restart();
  }

  Serial.println("\nWiFi connecté !");
  Serial.print("Adresse IP : ");
  Serial.println(WiFi.localIP());

  SPI.begin();
  player.begin();
  player.switchToMp3Mode();
  player.setVolume(volume);
  appliquerTonalite(bassFreq, bassAmp, trebleFreq, trebleAmp);
  setSpatialization(spatialMode);
  connexionChaine();
}

void loop() {
  if (Serial.available()) {
    char c = Serial.read();

    if (c == 'n') {
      client.stop();
      chaine = (chaine + 1) % NOMBRECHAINES;
      connexionChaine();
    }

    if (c == '+' || c == 'y') {
      if (volume < 100) {
        volume++;
        player.setVolume(volume);
        Serial.print("Volume : ");
        Serial.println(volume);
      }
    }

    if (c == '-' || c == 'b') {
      if (volume > 0) {
        volume--;
        player.setVolume(volume);
        Serial.print("Volume : ");
        Serial.println(volume);
      }
    }

    if (c == 'g') {
      if (bassAmp < 15) bassAmp++;
      appliquerTonalite(bassFreq, bassAmp, trebleFreq, trebleAmp);
      Serial.print("Basses : ");
      Serial.println(bassAmp);
    }

    if (c == 'f') {
      if (bassAmp > 0) bassAmp--;
      appliquerTonalite(bassFreq, bassAmp, trebleFreq, trebleAmp);
      Serial.print("Basses : ");
      Serial.println(bassAmp);
    }

    if (c == 'j') {
      if (trebleAmp < 7) trebleAmp++;
      appliquerTonalite(bassFreq, bassAmp, trebleFreq, trebleAmp);
      Serial.print("Aigus : ");
      Serial.println(trebleAmp);
    }

    if (c == 'h') {
      if (trebleAmp > -8) trebleAmp--;
      appliquerTonalite(bassFreq, bassAmp, trebleFreq, trebleAmp);
      Serial.print("Aigus : ");
      Serial.println(trebleAmp);
    }

    if (c == 'd') {
      bassAmp = 5;
      trebleAmp = 5;
      appliquerTonalite(bassFreq, bassAmp, trebleFreq, trebleAmp);
      Serial.println("Tonalité réinitialisée.");
    }

    if (c == 's') {
      spatialMode = (spatialMode + 1) % 4;
      setSpatialization(spatialMode);
    }
  }

  if (client.available() > 0) {
    uint8_t bytesread = client.read(mp3buff, BUFFSIZE);
    if (bytesread) {
      player.playChunk(mp3buff, bytesread);
    }
  }
}
