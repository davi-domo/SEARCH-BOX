/*
███████╗███╗   ███╗██╗  ██╗ ██████╗ ███████╗██╗   ██╗
██╔════╝████╗ ████║██║  ██║██╔═══██╗██╔════╝╚██╗ ██╔╝
███████╗██╔████╔██║███████║██║   ██║███████╗ ╚████╔╝
╚════██║██║╚██╔╝██║██╔══██║██║   ██║╚════██║  ╚██╔╝
███████║██║ ╚═╝ ██║██║  ██║╚██████╔╝███████║   ██║
╚══════╝╚═╝     ╚═╝╚═╝  ╚═╝ ╚═════╝ ╚══════╝   ╚═╝
Projet  : SEARCH BOX casier connecté
crée le : 15/01/2024
par     : David AUVRÉ alias SMHOSY

*** remerciement ***
*/

#include <Arduino.h>

// defifinition du mode de debug
#define DEBUG true
#define Serial \
  if (DEBUG)   \
  Serial

// bibliotheque pour le wifi et serveur web
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>

// bibliotheque pour sqlite3
#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <LittleFS.h>
#include <FS.h>
#include <SPI.h>

// bibliotheque pour les bandes led
#include <FastLED.h>

// definition des bande led
#define nb_bande 3
#define nb_led_bande 133
#define nb_led nb_led_bande *nb_bande

CRGBArray<nb_led> leds;

#define bande_1_pin 42
#define bande_2_pin 41
#define bande_3_pin 1
#define clock_pin 13
static uint8_t hue = 0;
static uint16_t led = 0;
bool fade = true;

uint8_t cycle = 0;
const uint8_t nb_tiroir = 12;
const uint8_t nb_led_tiroir = 24;
uint16_t tiroir[nb_tiroir][nb_led_tiroir]; // 12 tiroir avec 26 led
const uint16_t max_led = nb_tiroir * nb_led_tiroir;
uint16_t scenario_led[max_led];
const uint8_t first_led_tiroir[4] = {4, 38, 71, 105};
const uint8_t delay_locate = 10; // delay en seconde
// chargement du fichier de confuration
#include <config.h>

// chargement du fichier de fonction
AsyncWebServer server(80);

sqlite3 *db1;

sqlite3_stmt *res;
int rec_count = 0;
const char *tail;

int rc;

#define FORMAT_LITTLEFS_IF_FAILED true

int id_table = 0;
int id_del = -1;
int req_loc_box = -1;
uint8_t etat_loc_box = 0;

bool add_liste = false;
String add_categorie;
String add_nom;
String add_detail;
int add_stock;
int add_stock_alert;
int add_tiroir;
String add_img;

int id_update = -1;
String udp_categorie = "";
String udp_nom = "";
String udp_detail = "";
int udp_stock = 0;
int udp_stock_alert = 0;
int udp_tiroir = 0;
String udp_img = "";

// chargement du fichier de fonction
#include <fonction.h>

int seq_start = 0;
void setup()
{
  Serial.begin(115200);
  // initialisation des variables d'animation led
  int nb_led_scenario = 0;
  int ligne_tiroir = 0;
  for (int i = 0; i < nb_tiroir; i++)
  {
    for (int j = 0; j < nb_led_tiroir; j++)
    {
      int led_tiroir = first_led_tiroir[i % 4] + ((nb_led_bande * ligne_tiroir) + j);
      tiroir[i][j] = led_tiroir;
      scenario_led[nb_led_scenario] = led_tiroir;
      nb_led_scenario++;
    }
    if (i % 4 == 3)
    {
      ligne_tiroir++;
    }
  }

  // initialisation des bandes de leds
  FastLED.addLeds<WS2812B, bande_1_pin>(leds, 0, nb_led_bande);
  FastLED.addLeds<WS2812B, bande_2_pin>(leds, nb_led_bande, nb_led_bande);
  FastLED.addLeds<WS2812B, bande_3_pin>(leds, 2 * nb_led_bande, nb_led_bande);
  leds[tiroir[0][seq_start]] = CHSV(0, 255, 255);
  seq_start++;
  FastLED.show();
  delay(500);

  // Initialisation de l'espace de fichier LittleFS
  Serial.print(F("- Initialisation de LittleFS => "));
  if (!LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED, "/littlefs"))
  {
    Serial.println(F("[ERROR]\n"));

    leds[tiroir[0][seq_start]] = CHSV(95, 255, 255);
    seq_start++;
    FastLED.show();
    return;
  }
  Serial.println(F("[OK]\n"));

  leds[tiroir[0][seq_start]] = CHSV(0, 255, 255);
  seq_start++;
  FastLED.show();
  /*************************************************************/

  // connection au wifi
  Serial.print(F("- Initialisation du WIFI => "));
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    Serial.println(F("[ERROR]\n"));
    leds[tiroir[0][seq_start]] = CHSV(95, 255, 255);
    seq_start++;
    FastLED.show();
    return;
  }
  Serial.println(F("[OK]"));
  leds[tiroir[0][seq_start]] = CHSV(0, 255, 255);
  seq_start++;
  FastLED.show();
  Serial.print(F("IP de SEARCH BOX : "));
  Serial.println(WiFi.localIP());

  if (!MDNS.begin(hostname))
  {
    Serial.println(F("DNS => [ERROR]"));
    leds[tiroir[0][seq_start]] = CHSV(95, 255, 255);
    seq_start++;
    FastLED.show();
    return;
  }
  else
  {

    Serial.println(F("DNS => [OK]"));
    leds[tiroir[0][seq_start]] = CHSV(0, 255, 255);
    seq_start++;
    FastLED.show();
  }

  sqlite3_initialize();
  sqlite3 *db1;

  extract_id();

  /***   requete search   ***/
  server.on("/search", HTTP_GET, [](AsyncWebServerRequest *request)
            {
            String param_name [10];
      String param_value[10];
  //on stock le nombre de parametre  
    uint8_t nb_params = request->params();
    //on verifie la presence de parametre
    if(nb_params){  
      // préparation des variables
       param_name[nb_params];
       param_value[nb_params];
      Serial.println(F("Présence de parametre GET"));
      for(int i=0;i<nb_params;i++){
			  AsyncWebParameter* p = request->getParam(i);
			  param_name[i] = p->name();
			  param_value[i] = p->value();
        Serial.print(param_name[i]);
        Serial.print(F(" : "));
        Serial.println(param_value[i]);
      }

    }
        request->send(200, "application/json",search(param_value[0]) ); });
  /*************************************************************************/

  /***   Requete liste   ***/
  server.on("/list", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(200, "application/json", list()); });
  /*************************************************************************/

  /***   Requete liste   ***/
  server.on("/close_led", HTTP_GET, [](AsyncWebServerRequest *request)
            {   
       req_loc_box = -1;
       request->send(200, "application/json","[{\"result\": 1 }]" ); });
  /*************************************************************************/

  /***   Requete DETAIL   ***/
  server.on("/detail", HTTP_GET, [](AsyncWebServerRequest *request)
            {
  String param_name [10] ;
  String param_value[10] ;
  //on stock le nombre de parametre  
    uint8_t nb_params = request->params();
    //on verifie la presence de parametre
    if(nb_params){  
      // préparation des variables
       param_name[nb_params];
       param_value[nb_params];
      Serial.println(F("Présence de parametre GET"));
      for(int i=0;i<nb_params;i++){
			  AsyncWebParameter* p = request->getParam(i);
			  param_name[i] = p->name();
			  param_value[i] = p->value();
        Serial.print(param_name[i]);
        Serial.print(F(" : "));
        Serial.println(param_value[i]);
      }

    }
        request->send(200, "application/json",detail(param_value[0]) ); });
  /*************************************************************************/

  /***   Requete Locate_box   ***/
  server.on("/locate_box", HTTP_GET, [](AsyncWebServerRequest *request)
            {
      String param_name [10] ;
      String param_value[10] ;
      //on stock le nombre de parametre  
      uint8_t nb_params = request->params();
      //on verifie la presence de parametre
      if(nb_params){  
        // préparation des variables
        param_name[nb_params];
        param_value[nb_params];
        Serial.println(F("Présence de parametre GET"));
        for(int i=0;i<nb_params;i++){
			    AsyncWebParameter* p = request->getParam(i);
			    param_name[i] = p->name();
			    param_value[i] = p->value();
          Serial.print(param_name[i]);
          Serial.print(F(" : "));
          Serial.println(param_value[i]);
        }
      }
    request->send(200, "application/json",locate_box(param_value[0]) ); });
  /*************************************************************************/

  /***   Requete delete article   ***/
  server.on("/delete", HTTP_GET, [](AsyncWebServerRequest *request)
            {
      String param_name [10] ;
      String param_value[10] ;
      //on stock le nombre de parametre  
      uint8_t nb_params = request->params();
      //on verifie la presence de parametre
      if(nb_params){  
        // préparation des variables
        param_name[nb_params];
        param_value[nb_params];
        Serial.println(F("Présence de parametre GET"));
        for(int i=0;i<nb_params;i++){
			    AsyncWebParameter* p = request->getParam(i);
			    param_name[i] = p->name();
			    param_value[i] = p->value();
          Serial.print(param_name[i]);
          Serial.print(F(" : "));
          Serial.println(param_value[i]);
        }
       id_del = param_value[0].toInt();
      }
    request->send(200, "application/json","[{\"result\": 1 }]" ); });
  /*************************************************************************/

  /***   Requete ajout article   ***/
  server.on("/add", HTTP_GET, [](AsyncWebServerRequest *request)
            {
      String param_name [10] ;
      String param_value[10] ;
      //on stock le nombre de parametre  
      uint8_t nb_params = request->params();
      //on verifie la presence de parametre
      if(nb_params){  
        // préparation des variables
        param_name[nb_params];
        param_value[nb_params];
        Serial.println(F("Présence de parametre GET"));
        for(int i=0;i<nb_params;i++){
			    AsyncWebParameter* p = request->getParam(i);
			    param_name[i] = p->name();
			    param_value[i] = p->value();
          Serial.print(param_name[i]);
          Serial.print(F(" : "));
          Serial.println(param_value[i]);
        }
       if(param_name[0] == "categorie"){
        add_categorie = param_value[0];
       }
       if(param_name[1] == "nom"){
        add_nom = param_value[1];
       }
       if(param_name[2] == "detail"){
        add_detail = param_value[2];
       }
       if(param_name[3] == "stock"){
        add_stock = param_value[3].toInt();
       }
       if(param_name[4] == "stock_alert"){
        add_stock_alert = param_value[4].toInt();
       }
       if(param_name[5] == "tiroir"){
        add_tiroir = param_value[5].toInt();
       }
       if(param_name[6] == "img"){
        add_img = param_value[6];
       }
       add_liste = true;
      }
    request->send(200, "application/json","[{\"result\": 1 }]" ); });
  /*************************************************************************/

  /***   Requete ajout article   ***/
  server.on("/update", HTTP_GET, [](AsyncWebServerRequest *request)
            {
      String param_name [10] ;
      String param_value[10] ;
      //on stock le nombre de parametre  
      uint8_t nb_params = request->params();
      //on verifie la presence de parametre
      if(nb_params){  
        // préparation des variables
        param_name[nb_params];
        param_value[nb_params];
        Serial.println(F("Présence de parametre GET"));
        for(int i=0;i<nb_params;i++){
			    AsyncWebParameter* p = request->getParam(i);
			    param_name[i] = p->name();
			    param_value[i] = p->value();
          Serial.print(param_name[i]);
          Serial.print(F(" : "));
          Serial.println(param_value[i]);
        }
       if(param_name[0] == "categorie"){
        udp_categorie = param_value[0];
       }
       if(param_name[1] == "nom"){
        udp_nom = param_value[1];
       }
       if(param_name[2] == "detail"){
        udp_detail = param_value[2];
       }
       if(param_name[3] == "stock"){
        udp_stock = param_value[3].toInt();
       }
       if(param_name[4] == "stock_alert"){
        udp_stock_alert = param_value[4].toInt();
       }
       if(param_name[5] == "tiroir"){
        udp_tiroir = param_value[5].toInt();
       }
       if(param_name[6] == "img"){
        udp_img = param_value[6];
       }
       if(param_name[7] == "id"){
        id_update = param_value[7].toInt();
       }
      }
    request->send(200, "application/json","[{\"result\": 1 }]" ); });
  /*************************************************************************/

  /***   Requete ajout article   ***/
  server.on("/update_loc", HTTP_GET, [](AsyncWebServerRequest *request)
            {
      String param_name [10] ;
      String param_value[10] ;
      //on stock le nombre de parametre  
      uint8_t nb_params = request->params();
      //on verifie la presence de parametre
      if(nb_params){  
        // préparation des variables
        param_name[nb_params];
        param_value[nb_params];
        Serial.println(F("Présence de parametre GET"));
        for(int i=0;i<nb_params;i++){
			    AsyncWebParameter* p = request->getParam(i);
			    param_name[i] = p->name();
			    param_value[i] = p->value();
          Serial.print(param_name[i]);
          Serial.print(F(" : "));
          Serial.println(param_value[i]);
        }
       if(param_name[0] == "stock"){
        udp_stock = param_value[0].toInt();
       }
       if(param_name[1] == "stock_alert"){
        udp_stock_alert = param_value[1].toInt();
       }
       if(param_name[2] == "tiroir"){
        udp_tiroir = param_value[2].toInt();
       }
       if(param_name[3] == "id"){
        id_update = param_value[3].toInt();
       }
      }
    request->send(200, "application/json","[{\"result\": 1 }]" ); });
  /*************************************************************************/

  server.on("/", HTTP_ANY, [](AsyncWebServerRequest *request)
            {
    request->send(LittleFS, "/index.html");
    Serial.println("\n*** nouveau client Connecté ***\n"); });

  // On renvoie toute les requetes web vers le SPIFFS
  server.serveStatic("/", LittleFS, "/");

  server.begin();
  delay(100);

  FastLED.clear();
  FastLED.show();
}

void loop()
{
  if (id_del >= 0)
  {
    delete_id(id_del);
    id_del = -1;
  }
  if (add_liste)
  {
    insert_data(add_categorie, add_nom, add_detail, add_stock, add_stock_alert, add_tiroir, add_img);
    add_liste = false;
  }

  if (id_update >= 0)
  {
    update_data(id_update, udp_categorie, udp_nom, udp_detail, udp_stock, udp_stock_alert, udp_tiroir, udp_img);
    id_update = -1;
  }

  action_tiroir();

  delay(10);
}
