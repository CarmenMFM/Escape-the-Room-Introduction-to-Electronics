#include "BluetoothSerial.h"
#include "string.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

bool Comprobar_Contrasena(byte valor[5]);
BluetoothSerial SerialBT; 

/*Añadir contraseña en la siguiente línea*/
char contrasena[] = "Escriba aqui la contraseña";
int i = 0;
byte asci[10];

void setup() {
  Serial.begin(115200);
  SerialBT.begin("ESP32test"); //Bluetooth device name
  Serial.println("The device started, now you can pair it with bluetooth!");
}

void loop() {
 float candado = (24*24)/4 ;
 
  for(i = 0; i< strlen(contrasena); i++)
  {
      asci[i] = byte(contrasena[i]);
  }
  if(Comprobar_Contrasena(asci))
  {
    SerialBT.println(candado);

  }
  delay(3000);
}

bool Comprobar_Contrasena(byte valor[])
{
  if(valor[0] == 69 && valor [1] == 84 && valor[2] == 82 && valor[3] == 50 && valor[4] == 48 && valor[5] == 50 && valor[6] == 50)
  {
       return true;
  }
  else
  {
    Serial.println("Contraseña incorrecta");
    return false;
  }

}
