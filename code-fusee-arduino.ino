#include <SD.h>
File myFile;

#include <SPI.h>
#include <Wire.h>
#include "SSD1306Ascii.h"
#include "SSD1306AsciiAvrI2c.h"
#include <SFE_BMP180.h>
SFE_BMP180 pressure;
#include <Servo.h>
Servo servoMoteur;

#define I2C_ADDRESS 0x3C
#define RST_PIN -1

SSD1306AsciiAvrI2c oled;




int boutton = 0;
int monte = 0;

void setup() {
  Serial.begin(9600);
  //oled begin
  Wire.beginTransmission(0x3C);
#if RST_PIN >= 0
  oled.begin(&Adafruit128x32, I2C_ADDRESS, RST_PIN);
#else // RST_PIN >= 0
  oled.begin(&Adafruit128x32, I2C_ADDRESS);
#endif // RST_PIN >= 0
  // Call oled.setI2cClock(frequency) to change from the default frequency.

  oled.setFont(Adafruit5x7);

  uint32_t m = micros();

  oled.clear();


  // BMP180 pression begin

  oled.println("    Initialisation");
  oled.println("     ");
  oled.println("      du BMP180");

  delay(3000);

  oled.clear();

  Wire.endTransmission();

  Wire.beginTransmission(0x77);
  if (pressure.begin()) {

    Wire.endTransmission();

    Wire.beginTransmission(0x3C);

    oled.println("    initialisation");
    oled.println("    ");
    oled.set2X();
    oled.println("  REUSSIE");
    Wire.endTransmission();
    delay(3000);

    oled.clear();
  }

  Wire.endTransmission();

  //SD begin

  Wire.beginTransmission(0x3C);
  oled.set1X();
  oled.println("    Initialisation");
  oled.println("     ");
  oled.println("    de la carte SD");

  delay(3000);

  oled.clear();

  Wire.endTransmission();

  if (!SD.begin(4)) { // si l'arduino n as pas trouvé la carte sd
    Wire.beginTransmission(0x3C);
    oled.println("      Erreur de");
    oled.println("l'initialisation de");
    oled.println("     la carte SD");
    Wire.endTransmission();
    return;
  }
  Wire.beginTransmission(0x3C);
  oled.println("    initialisation");
  oled.println("    ");
  oled.set2X();
  oled.println("  REUSSIE");
  Wire.endTransmission();
  delay(3000);

  oled.clear();

  myFile = SD.open("file.txt", FILE_WRITE); // test d'ecriture dans le fichier
  myFile.println("-----");
  myFile.close();



  //bouton begin
  pinMode(5, INPUT_PULLUP);//initilasilastion des bouttons
  pinMode(9, INPUT_PULLUP);

  //servo begin
  servoMoteur.attach(6);
  servoMoteur.write(0);
}


void loop() {

  double Altitude, pressionFinale, pressionBase, pressionTest, pressionMaintenant, pressionAvant;

  Wire.beginTransmission(0x3C);
  oled.set1X();

  Wire.endTransmission();
  if (digitalRead(5) == LOW) {

    Wire.beginTransmission(0x77);
    pressionBase = getPressure(); //prise de la pression de base 
    pressionTest = pressionBase - 0.06;
    Wire.endTransmission();
    myFile = SD.open("file.txt", FILE_WRITE);
    myFile.println("La pression de base :");// ecriture de  la pression de base dans la carte sd
    myFile.println(pressionBase);
    myFile.println("  ");
    myFile.println("Pressions enregistrées durant le vol :");
    myFile.close();
    boutton = boutton + 20;
  }

  if (boutton > 2) {
    oled.println("Lecture du BMP");
    oled.println("                ");
    oled.println("                ");

    pressionAvant = getPressure();
    delay(250);
    pressionMaintenant = getPressure(); // prise de  la ression  avec un délait
    delay(250);


    if (pressionMaintenant < pressionAvant - 0.1) { // test pour voir si il y as une grosse differance de pression entre les deux variables
      oled.clear();
      oled.println("on monte");
      for (int i = 0; i <= 50; i++) { // ecriture de toutes les pressions dans la carte SD
        myFile = SD.open("file.txt", FILE_WRITE);
        myFile.println(getPressure());
        myFile.close();
        delay(50);
      }
      delay(50);
      oled.clear();
      monte = monte + 20;
    }

    if (pressionMaintenant > pressionAvant + 0.1) {// test pour voir si il y as une grosse differance de pression entre les deux variables
      oled.clear();
      oled.println("on descend");
      servoMoteur.write(90); // larguage du parachute avec l'ouverture du servo
      pressionFinale = getPressure();
      myFile = SD.open("file.txt", FILE_WRITE);
      myFile.println("  ");
      myFile.println("La pression à l'apogée :");
      myFile.println(pressionFinale);
      myFile.close();
      delay(500);
      oled.clear();


    }

  }



  if (digitalRead(9) == LOW) {
    Altitude = pressure.altitude(pressionFinale, pressionBase);
    boutton = boutton - boutton;
    oled.clear();
    myFile = SD.open("file.txt", FILE_WRITE);
    myFile.println("Altitude relative: ");
    myFile.println("    ");
    if (Altitude >= 0.0) myFile.print(" ");
    myFile.println(Altitude, 1);
    myFile.println(" mètres");
    myFile.close();

    oled.println("Altitude relative: ");
    if (Altitude >= 0.0) oled.print(" ");
    oled.print(Altitude, 1);
    oled.print(" mètres");
    delay(6000);
    oled.clear();
  }



  Wire.endTransmission();

}



double getPressure() {
  Wire.beginTransmission(0x77);
  char status;
  double T, P, p0, a;

  // Vous devez d'abord obtenir une mesure de température pour effectuer une lecture de pression.

  // Lancer une mesure de température :
  // Si la requête aboutit, le nombre de ms à attendre est renvoyé.
  // Si la requête échoue, 0 est renvoyé.

  status = pressure.startTemperature();
  if (status != 0)
  {
    // Attendez la fin de la mesure :

    delay(status);

    // Récupération de la mesure de température terminée :
    // Notez que la mesure est stockée dans la variable T.
    // Utilisez '& T' pour fournir l'adresse de T à la fonction.
    // La fonction renvoie 1 en cas de succès, 0 en cas d'échec.

    status = pressure.getTemperature(T);
    if (status != 0)
    {
      // Lancer une mesure de pression :
      // Le paramètre est le paramètre de suréchantillonnage, de 0 à 3 (résolution la plus élevée, attente la plus longue).
      // Si la requête aboutit, le nombre de ms à attendre est renvoyé.
      // Si la requête échoue, 0 est renvoyé.

      status = pressure.startPressure(3);
      if (status != 0)
      {
        // Attendez la fin de la mesure :
        delay(status);

        // Récupération de la mesure de pression terminée :
        // Notez que la mesure est stockée dans la variable P.
        // Utilisez '& P' pour fournir l'adresse de P.
        // Notez également que la fonction nécessite la mesure de température précédente (T).
        // (Si la température est stable, vous pouvez effectuer une mesure de température pour plusieurs mesures de pression.)
        // La fonction renvoie 1 en cas de succès, 0 en cas d'échec.

        status = pressure.getPressure(P, T);
        if (status != 0)
        {
          return (P);
        }
        else Serial.println("error retrieving pressure measurement\n");
      }
      else Serial.println("error starting pressure measurement\n");
    }
    else Serial.println("error retrieving temperature measurement\n");
  }
  else Serial.println("error starting temperature measurement\n");
  Wire.endTransmission();
}
