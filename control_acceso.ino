/*
 * See: https://github.com/miguelbalboa/rfid/tree/master/examples/rfid_write_personal_data
 *
 * Uses MIFARE RFID card using RFID-RC522 reader
 * Uses MFRC522 - Library
 * -----------------------------------------------------------------------------------------
 *             MFRC522      Arduino       Arduino   Arduino    Arduino          Arduino
 *             Reader/PCD   Uno/101       Mega      Nano v3    Leonardo/Micro   Pro Micro
 * Signal      Pin          Pin           Pin       Pin        Pin              Pin
 * -----------------------------------------------------------------------------------------
 * RST/Reset   RST          9             5         D9         RESET/ICSP-5     RST
 * SPI SS      SDA(SS)      10            53        D10        10               10
 * SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16
 * SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14
 * SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15
 * 
* --------------------------
 * Conexiones I2C Arduino Uno
 * --------------------------
 * SDA to A4
 * SCL to A5
 * VCC to 5volt
 * GND to GND
 * 
 */

#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
  
//Crear el objeto lcd  dirección  0x27 y 16 columnas x 2 filas
LiquidCrystal_I2C lcd(0x27,16,2);  

// Lector RC522
#define RST_PIN         9           
#define SS_PIN          10          
MFRC522 mfrc522(SS_PIN, RST_PIN); 

// Piezo Buzzer 
const int buzzerPin = 7;

/* Registra el UID aqui! */
// Primer UID NEW_UID que se completara con datos de tarjeta a leer
#define NEW_UID {0x00, 0x00, 0x00, 0x00}
// UID Registrado para dar acceso
#define COM_UID {0x90, 0xB2, 0x1A, 0x83}

/*
|--------------
| VOIDs ANEXOS
|--------------
*/

// Sonido de acceso denegado
void sonidoError(){
    // Encender buzzer
    digitalWrite(buzzerPin,HIGH);
    // Esperar para producir sonido
    delay(100);
    // Epagar buzzer
    digitalWrite(buzzerPin,LOW);
    // Esperar para producir silencio
    delay(100);
    digitalWrite(buzzerPin,HIGH);
    delay(100);
    digitalWrite(buzzerPin,LOW);
    delay(100);
    digitalWrite(buzzerPin,HIGH);
    delay(100);
    digitalWrite(buzzerPin,LOW);
}

// Sonido de acceso concedido
void sonidoCorrecto(){
    // Producir sonido
    digitalWrite(buzzerPin,HIGH);
    delay(300);
    digitalWrite(buzzerPin,LOW);
}

// Dejar pantalla LCD en blanco luego de mostrar mensaje
// Con mensaje "Control Acceso"
void limpiarLCD(){
    // Limpiar LCD
    lcd.clear();
    // Primero cuadrado primera linea de la pantalla
    lcd.setCursor(0,0);
    // Mostrar mensaje Control Acceso
    lcd.print("Control Acceso");
}

long int concatHex(byte uid[]){
    long unsigned int uid_num = uid[0];
    uid_num = uid_num*256 + uid[1];
    uid_num = uid_num*256 + uid[2];
    uid_num = uid_num*256 + uid[3];
    return uid_num;
}

/*
|--------------
| VOID SETUP
|--------------
*/
// Iniciacion de componentes y variables (al encender arduino)
void setup() {
    /* Monitor Serial */
    Serial.begin(9600);

    /* Piezo Buzzer */
    pinMode(buzzerPin, OUTPUT);
    
    /* RFID RC522 */
    SPI.begin();                          
    mfrc522.PCD_Init();
    // Mostrar mensaje en Serial monitor                   
    Serial.println(F("Lector Datos MIFARE PICC:"));
        
    // Inicializar el LCD I2C
    lcd.init();
    //Encender la luz de fondo.
    lcd.backlight();
    lcd.print("Control Acceso");
}


/*
|--------------
| VOID LOOP
|--------------
*/
void loop() {
  
    // Cambiar cursor primer caracter segunda linea
    lcd.setCursor(0,1);
    
    /* Preparar Clave Sector, por defecto en fabrica FFFFFFFFFFFF */
    MFRC522::MIFARE_Key key;

    // LLave defecto FFFFFFFFFFFF
    for (byte i = 0; i < 6; i++) {
        key.keyByte[i] = 0xFF;
    }
    
    // Primer UID (Se completara con datos de tarjeta a leer)
    byte uid[] = NEW_UID;
    // Segundo UID (Registrada al inico para dar acceso - llave correcta)
    byte com_uid[] = COM_UID;
    
    // Si se presenta una nueva tarjeta
    if ( ! mfrc522.PICC_IsNewCardPresent() || ! mfrc522.PICC_ReadCardSerial() ) {
        delay(50);
        return;
    }

    // Mostrar UID tarjeta que se presento
    Serial.print(F("Card UID:"));
    // Recorrer el largo del uid para asignarlo en variable uid[]
    for (byte i = 0; i < mfrc522.uid.size; i++) {
        // Imprimir en el serial monitor el UID de la tarjeta leida
        Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
        Serial.print(mfrc522.uid.uidByte[i], HEX);
        // Almacenar UID de tarjeta leida en variable uid[]
        uid[i] = mfrc522.uid.uidByte[i];
    }

    // Salto de linea
    Serial.println();

    // Compactar valor uid bytes en entero tarjeta leida
    long unsigned int leido = concatHex(uid);

    // Compactar valor uid bytes en entero tarjeta registrada
    long unsigned int registrado = concatHex(com_uid);
    
    // Salto de linea
    Serial.println(leido, HEX);
    Serial.println(registrado, HEX);
    
    // Comparar UID registrado (com_uid[]) con UID leido
    if(registrado == leido){
        // Sonido de acceso correcto
        sonidoCorrecto();
        // Mostrar mensaje de acceso concedido
        lcd.print("Correcta!");
    }else{
        // Sonido de error
        sonidoError();
        // Mensaje de error
        lcd.print("No encontrada!");
    }

    // Esperar a que mensaje sea ledido por usuario
    delay(3000);
    // Limpiar LCD
    limpiarLCD();
    // Pausar el programa
    delay(2000);
}
