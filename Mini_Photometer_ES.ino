#include "Wire.h"
#include "LiquidCrystal_I2C.h"
#include "BH1750FVI.h"
LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x3F, 16, 2);

/*sensitivity                     - value have to be between 0.45 - 3.68, default 1.00 or use macros BH1750_SENSITIVITY_DEFAULT
accuracy                          - value have to be between 0.96 - 1.44, default 1.20 or use macros BH1750_ACCURACY_DEFAULT */
BH1750FVI myBH1750(BH1750_DEFAULT_I2CADDR, BH1750_ONE_TIME_HIGH_RES_MODE_2, BH1750_SENSITIVITY_DEFAULT, BH1750_ACCURACY_DEFAULT);

#define sensorOut PB4
#define S1 PB14
#define S0 PB13
#define S2 PB3
#define S3 PB15
#define UV PB0
#define BL PB1
int red_light_pin = PA8;
int green_light_pin = PA9;
int blue_light_pin = PA10;

byte UV_PWM = 255, BL_PWM = 255, R_PWM = 255, G_PWM = 255, B_PWM = 255;

int C_delay = 20;
int C_times = 100;
int L_delay = 100;
int L_times = 5;
byte L_on = 1;
byte C_on = 1;
byte Comp_on = 0;
byte Comp_t = 0;

int t_but0 = 0;
int t_but1 = 0;
int t_but2 = 0;
int PosicionH = 0;

byte buttons[] = {1,2,3};
const byte nrButtons = 3;
int menusize = 20;

int len(String str) {
  return str.length();
}

String menu[] = {
  "Menu",                                          //0
  "Menu>Analisis Rapido",                          //1
  "Menu>Manual",                                   //2
  "Menu>Manual>Realizar Ensayo",                   //3
  "Menu>Manual>Config Luz",                        //4
  "Menu>Manual>Config Luz>RGB",                    //5  
  "Menu>Manual>Config Luz>UV",                     //6
  "Menu>Manual>Config Luz>Blanca",                 //7
  "Menu>Manual>Config Sens.",                      //8
  "Menu>Manual>Config Sens.>Color",                //9
  "Menu>Manual>Config Sens.>Volver o no",          //10
  "Menu>Manual>Config Sens.>Color>Prendido*",      //11
  "Menu>Manual>Config Sens.>Color>Delay",          //12  
  "Menu>Manual>Config Sens.>Color>Repeticiones",   //13  
  "Menu>Manual>Config Sens.>Lux",                  //14
  "Menu>Manual>Config Sens.>Lux>Delay",            //15
  "Menu>Manual>Config Sens.>Lux>Prendido*",        //16
  "Menu>Manual>Config Sens.>Lux>Repeticiones",     //17
  "Menu>Manual>Comparativo",                       //18
  "Menu>Completo RGB",                             //19
};

int t;
byte pressedButton, currentPos,currentPosParent, possiblePos[40], possiblePosCount, possiblePosScroll = 0;
String parent = "";
byte saltearunavuelta = 0;

int N_Prom, R_Prom, G_Prom, B_Prom, L_Prom;
int UV_N_Prom, UV_R_Prom, UV_G_Prom, UV_B_Prom, UV_L_Prom;
int B_N_Prom, B_R_Prom, B_G_Prom, B_B_Prom, B_L_Prom;
int G_N_Prom, G_R_Prom, G_G_Prom, G_B_Prom, G_L_Prom;
int R_N_Prom, R_R_Prom, R_G_Prom, R_B_Prom, R_L_Prom;
int BL_N_Prom, BL_R_Prom, BL_G_Prom, BL_B_Prom, BL_L_Prom;
int T_N_Prom, T_R_Prom, T_G_Prom, T_B_Prom, T_L_Prom;
int N_Prom_B, R_Prom_B, G_Prom_B, B_Prom_B, L_Prom_B;

byte Tick[] = {
  B00000,
  B00000,
  B00001,
  B00011,
  B10110,
  B11100,
  B01000,
  B00000
};

//--------------------------------------

void RGB_color(int red_light_value, int green_light_value, int blue_light_value)
 {
  analogWrite(red_light_pin, 255-red_light_value);
  analogWrite(green_light_pin, 255-green_light_value);
  analogWrite(blue_light_pin, 255-blue_light_value);
}

//-------------------------------------

void updateMenu () {

  possiblePosCount = 0;
  while (possiblePosCount == 0) {
    for (t = 1; t < menusize;t++) {
      if (mid(menu[t],1,inStrRev(menu[t],">")-1).equals(menu[currentPos])) {
        possiblePos[possiblePosCount]  =  t;
        possiblePosCount = possiblePosCount + 1;
      }
    }
    
    //find the current parent for the current menu
    parent = mid(menu[currentPos],1,inStrRev(menu[currentPos],">")-1);
    currentPosParent = 0;
    for (t = 0; t < menusize; t++) {
       if (parent == menu[t]) {currentPosParent = t;}
    }

    // reached the end of the Menu line
    if (possiblePosCount == 0) {
      //Menu Option Items
      switch (currentPos) {

        case 11: //Toggle
          if (mid(menu[currentPos],len(menu[currentPos]),1) == "*") {
            menu[currentPos] = mid(menu[currentPos],1,len(menu[currentPos])-1);
            C_on = 0;
          } else {
            menu[currentPos] = menu[currentPos] + "*";    
            C_on = 1;
        }   
        break;

        case 16: //Toggle
          if (mid(menu[currentPos],len(menu[currentPos]),1) == "*") {
            menu[currentPos] = mid(menu[currentPos],1,len(menu[currentPos])-1);
            L_on = 0;
          } else {
            menu[currentPos] = menu[currentPos] + "*";    
            L_on = 1;
        }   
        break;

        case 18: //Toggle
          if (mid(menu[currentPos],len(menu[currentPos]),1) == "*") {
            menu[currentPos] = mid(menu[currentPos],1,len(menu[currentPos])-1);
            Comp_on = 0;
          } else {
            menu[currentPos] = menu[currentPos] + "*";    
            Comp_on = 1;
        }   
        break;
      }

      //Set Variables
      switch (currentPos) {
        case 1:
          lcd.clear();
          Serial.print("Analisis Rapido: ");
          lcd.setCursor(0,0); lcd.print("Analisis Rapido");
          lcd.setCursor(0,1); lcd.print("Analizando..."); 
              C_delay = 20;
              C_times = 100;
              L_delay = 100;
              L_times = 5;
              L_on = 1;
              C_on = 1;
              delay(100);
              analogWrite(UV, 255);
              delay(200);
              N_Prom = Color_N();
              R_Prom = Color_R();
              G_Prom = Color_G();
              B_Prom = Color_B();
              analogWrite(UV, 0);
              RGB_color(255, 255, 255);
              delay(200);
              L_Prom = Lux_P();
              Serial.print("T/UV LNRGB: "); Serial.print(L_Prom); Serial.print(","); Serial.print(N_Prom); Serial.print(","); Serial.print(R_Prom);Serial.print(","); Serial.print(G_Prom);Serial.print(","); Serial.println(B_Prom);
              RGB_color(0, 0, 0);
    
          lcd.clear();
          lcd.setCursor(0,0); lcd.print("Prom: "); lcd.print(L_Prom); lcd.print(" lx");
          lcd.setCursor(0,1); lcd.print("N:"); lcd.print(N_Prom); lcd.print(" R:"); lcd.print(R_Prom);

             for (byte i = 0; i < 2; i++)
             {
              int but0 = digitalRead(buttons[0]);
              int but1 = digitalRead(buttons[1]);
              int but2 = digitalRead(buttons[2]);           
                     if (but0 != 0) {
                       currentPos = currentPosParent; //Back
                       i = 2;
                       but0 = 0;
                       updateMenu();
                       break;
                    }
                     if (but1 != 0) {
                        lcd.clear();
                        lcd.setCursor(0,0); lcd.print("Prom: "); lcd.print(L_Prom); lcd.print(" lx");
                        lcd.setCursor(0,1); lcd.print("N:"); lcd.print(N_Prom); lcd.print(" R:"); lcd.print(R_Prom);
                    }
                     if (but2 != 0) {
                        lcd.clear();
                        lcd.setCursor(0,0); lcd.print("N:"); lcd.print(N_Prom); lcd.print(" R:"); lcd.print(R_Prom);
                        lcd.setCursor(0,1); lcd.print("G:"); lcd.print(G_Prom); lcd.print(" B:"); lcd.print(B_Prom);
                    }
    
               //Mantiene hasta levantar boton
               while (but1 != 0) {
                but1 = 0;
                if (digitalRead(buttons[1]) != 0) {but1 = 1;}
                delay(2);
               }
               while (but2 != 0) {
                but2 = 0;
                if (digitalRead(buttons[2]) != 0) {but2 = 1;}
                delay(2);
               }         
             i = 0;
             }
            break;

        case 3:
          lcd.clear();
          Serial.println("Analisis Manual");
          lcd.setCursor(0,0); lcd.print("Analisis Manual");
          delay (800);
          Comp_t = 0;

          if (Comp_on == 1) {
            lcd.clear();
            lcd.setCursor(0,0); lcd.print("Ingrese blanco");       
            lcd.setCursor(0,1); lcd.print("Presione 2do bot");
                  for (byte i = 0; i < 2; i++)
                 {
                  int but0 = digitalRead(buttons[0]);
                  int but1 = digitalRead(buttons[1]);
                  int but2 = digitalRead(buttons[2]);  
                     if (but0 != 0) {
                       currentPos = currentPosParent; //Back
                       saltearunavuelta = 1;
                       i = 2;
                       but0 = 0;
                       break;}
                     if (but1 != 0) {
                      lcd.clear();
                      lcd.setCursor(0,0); lcd.print("Analizando blanco");     
                      delay(100);
                      analogWrite(UV, UV_PWM);
                      analogWrite(BL, BL_PWM);
                      RGB_color(R_PWM, G_PWM, B_PWM);
                      delay(200);
                      N_Prom_B = Color_N();
                      R_Prom_B = Color_R();
                      G_Prom_B = Color_G();
                      B_Prom_B = Color_B();
                      L_Prom_B = Lux_P();
                      delay(100);
                      analogWrite(UV, 0);
                      analogWrite(BL, 0);
                      RGB_color(0, 0, 0);
                      Serial.println("Blanco:");
                      Serial.print("UV "); Serial.print(UV_PWM); Serial.print(","); Serial.print(" BL "); Serial.print(BL_PWM); Serial.print(","); Serial.print(" R,G,B "); Serial.print(R_PWM); Serial.print(","); Serial.print(G_PWM); Serial.print(","); Serial.println(B_PWM);
                      Serial.print("L,N,R,G,B "); Serial.print(L_Prom_B); Serial.print(","); Serial.print(N_Prom_B); Serial.print(","); Serial.print(R_Prom_B); Serial.print(","); Serial.print(G_Prom_B);Serial.print(","); Serial.println(B_Prom_B);
                      lcd.clear();
                      lcd.setCursor(0,0); lcd.print("Prom: "); lcd.print(L_Prom_B); lcd.print(" lx");
                      lcd.setCursor(0,1); lcd.print("N:"); lcd.print(N_Prom_B); lcd.print(" R:"); lcd.print(R_Prom_B);
                                         for (byte i = 0; i < 2; i++)
                                         {
                                          int but0 = digitalRead(buttons[0]);
                                          int but1 = digitalRead(buttons[1]);
                                          int but2 = digitalRead(buttons[2]);           
                                                 if (but0 != 0) {
                                                  if (Comp_t == 0) {
                                                  lcd.clear();
                                                  lcd.setCursor(0,0); lcd.print("Ingrese muestra");       
                                                  lcd.setCursor(0,1); lcd.print("Presione 2do bot");
                                                  Comp_t = 1;
                                                  }
                                                      if (Comp_t == 2) {
                                                       currentPos = currentPosParent; //Back
                                                       saltearunavuelta = 1;
                                                       i = 2;
                                                       but0 = 0;
                                                       break;
                                                      }
                                                }
                                                 if (but1 != 0) {
                                                    if (Comp_t == 0) {
                                                      lcd.clear();
                                                      lcd.setCursor(0,0); lcd.print("Prom: "); lcd.print(L_Prom_B); lcd.print(" lx");
                                                      lcd.setCursor(0,1); lcd.print("N:"); lcd.print(N_Prom_B); lcd.print(" R:"); lcd.print(R_Prom_B);
                                                      }
                                                        if (Comp_t == 1) {
                                                              lcd.clear();
                                                              lcd.setCursor(0,0); lcd.print("Analizando muestra");     
                                                              delay(100);
                                                              analogWrite(UV, UV_PWM);
                                                              analogWrite(BL, BL_PWM);
                                                              RGB_color(R_PWM, G_PWM, B_PWM);
                                                              delay(200);
                                                              N_Prom = Color_N();
                                                              R_Prom = Color_R();
                                                              G_Prom = Color_G();
                                                              B_Prom = Color_B();
                                                              L_Prom = Lux_P();
                                                              delay(100);
                                                              analogWrite(UV, 0);
                                                              analogWrite(BL, 0);
                                                              RGB_color(0, 0, 0);
                                                              Serial.println("Muestra:");
                                                              Serial.print("L,N,R,G,B "); Serial.print(L_Prom); Serial.print(","); Serial.print(N_Prom); Serial.print(","); Serial.print(R_Prom); Serial.print(","); Serial.print(G_Prom);Serial.print(","); Serial.println(B_Prom);
                                                              Serial.println("Diferencia:");
                                                              N_Prom = N_Prom_B - N_Prom;
                                                              R_Prom = R_Prom_B - R_Prom;
                                                              G_Prom = G_Prom_B - G_Prom;
                                                              B_Prom = B_Prom_B - B_Prom;
                                                              L_Prom = L_Prom_B - L_Prom;
                                                              Serial.print("L,N,R,G,B "); Serial.print(L_Prom); Serial.print(","); Serial.print(N_Prom); Serial.print(","); Serial.print(R_Prom); Serial.print(","); Serial.print(G_Prom);Serial.print(","); Serial.println(B_Prom);
                                                              lcd.clear();
                                                              lcd.setCursor(0,0); lcd.print("Dif abs: "); lcd.print(L_Prom); lcd.print(" lx");
                                                              lcd.setCursor(0,1); lcd.print("N:"); lcd.print(N_Prom); lcd.print(" R:"); lcd.print(R_Prom);
                                                              Comp_t = 2;                                                      
                                                              }
                                                                    if (Comp_t == 2) {
                                                                      lcd.clear();
                                                                      lcd.setCursor(0,0); lcd.print("Dif: "); lcd.print(L_Prom); lcd.print(" lx");
                                                                      lcd.setCursor(0,1); lcd.print("N:"); lcd.print(N_Prom); lcd.print(" R:"); lcd.print(R_Prom);                                                
                                                                      }
                                                }
                                                 if (but2 != 0) {
                                                    if (Comp_t == 0) {
                                                    lcd.clear();
                                                    lcd.setCursor(0,0); lcd.print("N:"); lcd.print(N_Prom_B); lcd.print(" R:"); lcd.print(R_Prom_B);
                                                    lcd.setCursor(0,1); lcd.print("G:"); lcd.print(G_Prom_B); lcd.print(" B:"); lcd.print(B_Prom_B);
                                                    }
                                                        if (Comp_t == 2) {
                                                        lcd.clear();
                                                        lcd.setCursor(0,0); lcd.print("N:"); lcd.print(N_Prom); lcd.print(" R:"); lcd.print(R_Prom);
                                                        lcd.setCursor(0,1); lcd.print("G:"); lcd.print(G_Prom); lcd.print(" B:"); lcd.print(B_Prom);
                                                        }
                                                }
                                           while (but0 != 0) {
                                            but0 = 0;
                                            if (digitalRead(buttons[1]) != 0) {but0 = 1;}
                                            delay(2);
                                           }
                                           while (but1 != 0) {
                                            but1 = 0;
                                            if (digitalRead(buttons[1]) != 0) {but1 = 1;}
                                            delay(2);
                                           }
                                           while (but2 != 0) {
                                            but2 = 0;
                                            if (digitalRead(buttons[2]) != 0) {but2 = 1;}
                                            delay(2);
                                           }         
                                         i = 0;
                                         }
                      
                      }
                   while (but0 != 0) {
                    but0 = 0;
                    if (digitalRead(buttons[1]) != 0) {but0 = 1;}
                    delay(2);
                   }
                   while (but1 != 0) {
                    but1 = 0;
                    if (digitalRead(buttons[1]) != 0) {but1 = 1;}
                    delay(2);
                   }
                 i = 0;
                 }
          } else {
            lcd.clear();
            lcd.setCursor(0,0); lcd.print("Ingrese muestra");       
            lcd.setCursor(0,1); lcd.print("Presione 2do bot");
                for (byte i = 0; i < 2; i++)
                 {
                  int but0 = digitalRead(buttons[0]);
                  int but1 = digitalRead(buttons[1]);
                  int but2 = digitalRead(buttons[2]);  
                     if (but0 != 0) {
                       currentPos = currentPosParent; //Back
                       saltearunavuelta = 1;
                       i = 2;
                       but0 = 0;
                       break;}
                     if (but1 != 0) {
                      lcd.clear();
                      lcd.setCursor(0,0); lcd.print("Analizando muestra");     
                      delay(100);
                      analogWrite(UV, UV_PWM);
                      analogWrite(BL, BL_PWM);
                      RGB_color(R_PWM, G_PWM, B_PWM);
                      delay(200);
                      N_Prom_B = Color_N();
                      R_Prom_B = Color_R();
                      G_Prom_B = Color_G();
                      B_Prom_B = Color_B();
                      L_Prom_B = Lux_P();
                      delay(100);
                      analogWrite(UV, 0);
                      analogWrite(BL, 0);
                      RGB_color(0, 0, 0);
                      Serial.println("Muestra:");
                      Serial.print("UV "); Serial.print(UV_PWM); Serial.print(","); Serial.print(" BL "); Serial.print(BL_PWM); Serial.print(","); Serial.print(" R,G,B "); Serial.print(R_PWM); Serial.print(","); Serial.print(G_PWM); Serial.print(","); Serial.println(B_PWM);
                      Serial.print("L,N,R,G,B "); Serial.print(L_Prom_B); Serial.print(","); Serial.print(N_Prom_B); Serial.print(","); Serial.print(R_Prom_B); Serial.print(","); Serial.print(G_Prom_B);Serial.print(","); Serial.println(B_Prom_B);
                      lcd.clear();
                      lcd.setCursor(0,0); lcd.print("Prom: "); lcd.print(L_Prom_B); lcd.print(" lx");
                      lcd.setCursor(0,1); lcd.print("N:"); lcd.print(N_Prom_B); lcd.print(" R:"); lcd.print(R_Prom_B);
                                         for (byte i = 0; i < 2; i++)
                                         {
                                          int but0 = digitalRead(buttons[0]);
                                          int but1 = digitalRead(buttons[1]);
                                          int but2 = digitalRead(buttons[2]);           
                                                 if (but0 != 0) {
                                                    currentPos = currentPosParent; //Back
                                                    saltearunavuelta = 1;
                                                    i = 2;
                                                    but0 = 0;
                                                    break;
                                                    }
                                                  if (but1 != 0) {
                                                    lcd.clear();
                                                    lcd.setCursor(0,0); lcd.print("Prom: "); lcd.print(L_Prom_B); lcd.print(" lx");
                                                    lcd.setCursor(0,1); lcd.print("N:"); lcd.print(N_Prom_B); lcd.print(" R:"); lcd.print(R_Prom_B);
                                                    }
                                                  if (but2 != 0) {
                                                    lcd.clear();
                                                    lcd.setCursor(0,0); lcd.print("N:"); lcd.print(N_Prom_B); lcd.print(" R:"); lcd.print(R_Prom_B);
                                                    lcd.setCursor(0,1); lcd.print("G:"); lcd.print(G_Prom_B); lcd.print(" B:"); lcd.print(B_Prom_B);
                                                    }
                                          while (but0 != 0) {
                                            but0 = 0;
                                            if (digitalRead(buttons[1]) != 0) {but0 = 1;}
                                            delay(2);
                                           }
                                           while (but1 != 0) {
                                            but1 = 0;
                                            if (digitalRead(buttons[1]) != 0) {but1 = 1;}
                                            delay(2);
                                           }
                                           while (but2 != 0) {
                                            but2 = 0;
                                            if (digitalRead(buttons[2]) != 0) {but2 = 1;}
                                            delay(2);
                                           }         
                                         i = 0;
                                         }
                     }                 
                   while (but0 != 0) {
                    but0 = 0;
                    if (digitalRead(buttons[1]) != 0) {but0 = 1;}
                    delay(2);
                   }
                   while (but1 != 0) {
                    but1 = 0;
                    if (digitalRead(buttons[1]) != 0) {but1 = 1;}
                    delay(2);
                   }
                 i = 0;
                 }
          }          
          break;

        case 5:
          lcd.clear();
          lcd.setCursor(0,0); lcd.print("Valor (R,G,B)");
          lcd.setCursor(0,1); lcd.print("(");lcd.print(R_PWM); lcd.write(1); lcd.print(",");lcd.print(G_PWM); lcd.print(",");lcd.print(B_PWM); lcd.print(")");
          PosicionH = 0;
             for (byte i = 0; i < 2; i++)
             {
              int but0 = digitalRead(buttons[0]);
              int but1 = digitalRead(buttons[1]);
              int but2 = digitalRead(buttons[2]);  
                    if (but0 != 0) {
                       currentPosParent = 4;
                       currentPos = 7;
                       possiblePosCount = 3;
                       possiblePosScroll = 0;
                       saltearunavuelta = 1;
                       i = 2;
                       but0 = 0;
                       break;
                    }
                     if (but1 != 0) {
                      if (PosicionH == 0) {
                        lcd.setCursor(0,1); lcd.print("                ");
                        lcd.setCursor(0,1); lcd.print("("); lcd.print(R_PWM); lcd.print(",");lcd.print(G_PWM); lcd.write(1); lcd.print(",");lcd.print(B_PWM); lcd.print(")");}
                      if (PosicionH == 1) {
                        lcd.setCursor(0,1); lcd.print("                ");
                        lcd.setCursor(0,1); lcd.print("("); lcd.print(R_PWM); lcd.print(",");lcd.print(G_PWM); lcd.print(",");lcd.print(B_PWM); lcd.write(1); lcd.print(")");}                        
                      if (PosicionH == 2) {
                        lcd.setCursor(0,1); lcd.print("                ");
                        lcd.setCursor(0,1); lcd.print("("); lcd.print(R_PWM); lcd.write(1); lcd.print(",");lcd.print(G_PWM); lcd.print(",");lcd.print(B_PWM); lcd.print(")");}                       
                      PosicionH = PosicionH + 1;
                      if (PosicionH > 2) {PosicionH = 0;}
                    }
                     if (but2 != 0) {
                      if (PosicionH == 0) {
                        R_PWM = R_PWM + 1;
                        lcd.setCursor(0,1); lcd.print("                ");
                        lcd.setCursor(0,1); lcd.print("("); lcd.print(R_PWM); lcd.write(1); lcd.print(",");lcd.print(G_PWM); lcd.print(",");lcd.print(B_PWM); lcd.print(")");}
                      if (PosicionH == 1) {
                        G_PWM = G_PWM + 1;
                        lcd.setCursor(0,1); lcd.print("                ");
                        lcd.setCursor(0,1); lcd.print("("); lcd.print(R_PWM); lcd.print(",");lcd.print(G_PWM); lcd.write(1); lcd.print(",");lcd.print(B_PWM); lcd.print(")");}
                      if (PosicionH == 2) {
                        B_PWM = B_PWM + 1;
                        lcd.setCursor(0,1); lcd.print("                ");
                        lcd.setCursor(0,1); lcd.print("("); lcd.print(R_PWM); lcd.print(",");lcd.print(G_PWM); lcd.print(",");lcd.print(B_PWM); lcd.write(1); lcd.print(")");}
                    }
    
               if (digitalRead(buttons[1]) == 0) {t_but1 =0;}
               if (digitalRead(buttons[2]) == 0) {t_but2 =0;}
               
               while (but1 != 0) {
                but1 = 0;
                if (digitalRead(buttons[1]) != 0) {but1 = 1;}
                delay(2);
               }
               while (but2 != 0) {
                but2 = 0;
                  if (digitalRead(buttons[2]) != 0) {
                  t_but2 = t_but2 + 1;
                  but2 = 1;
                      if (t_but2 > 250) {
                      delay(20);
                      but2 = 0;
                      }
                  }
               delay(2);         
              }  
             i = 0;
             }
            break;           

        case 6:
          lcd.clear();
          lcd.setCursor(0,0); lcd.print("Valor UV (0-255)");
          lcd.setCursor(0,1); lcd.print(UV_PWM);

             for (byte i = 0; i < 2; i++)
             {
              int but0 = digitalRead(buttons[0]);
              int but1 = digitalRead(buttons[1]);
              int but2 = digitalRead(buttons[2]);  
                    if (but0 != 0) {
                       currentPosParent = 4;
                       currentPos = 6;
                       possiblePosCount = 3;
                       possiblePosScroll = 1;
                       saltearunavuelta = 1;
                       i = 2;
                       but0 = 0;
                       break;
                    }
                     if (but1 != 0) {
                        UV_PWM = UV_PWM + 1;
                        lcd.setCursor(0,1); lcd.print("   ");
                        lcd.setCursor(0,1); lcd.print(UV_PWM);
                    }
                     if (but2 != 0) {
                        UV_PWM = UV_PWM - 1;
                        lcd.setCursor(0,1); lcd.print("   ");
                        lcd.setCursor(0,1); lcd.print(UV_PWM);
                    }
    
               if (digitalRead(buttons[1]) == 0) {t_but1 =0;}
               if (digitalRead(buttons[2]) == 0) {t_but2 =0;}
               
               while (but1 != 0) {
                but1 = 0;
                      if (digitalRead(buttons[1]) != 0) {
                      t_but1 = t_but1 + 1;
                      but1 = 1;
                          if (t_but1 > 250) {
                          delay(20);
                          but1 = 0;
                          }
                      }
                delay(2);
               }
               while (but2 != 0) {
                but2 = 0;
                      if (digitalRead(buttons[2]) != 0) {
                      t_but2 = t_but2 + 1;
                      but2 = 1;
                          if (t_but2 > 250) {
                          delay(20);
                          but2 = 0;
                          }
                      }
               delay(2);         
             }
             i = 0;
             }
            break;

       case 7:
          lcd.clear();
          lcd.setCursor(0,0); lcd.print("Valor BL (0-255)");
          lcd.setCursor(0,1); lcd.print(BL_PWM);

             for (byte i = 0; i < 2; i++)
             {
              int but0 = digitalRead(buttons[0]);
              int but1 = digitalRead(buttons[1]);
              int but2 = digitalRead(buttons[2]);  
                    if (but0 != 0) {
                       currentPosParent = 4;
                       currentPos = 5;
                       possiblePosCount = 3;
                       possiblePosScroll = 2;
                       saltearunavuelta = 1;
                       i = 2;
                       but0 = 0;
                       break;
                    }
                     if (but1 != 0) {
                        BL_PWM = BL_PWM + 1;
                        lcd.setCursor(0,1); lcd.print("   ");
                        lcd.setCursor(0,1); lcd.print(BL_PWM);
                    }
                     if (but2 != 0) {
                        BL_PWM = BL_PWM - 1;
                        lcd.setCursor(0,1); lcd.print("   ");
                        lcd.setCursor(0,1); lcd.print(BL_PWM);
                    }
    
               if (digitalRead(buttons[1]) == 0) {t_but1 =0;}
               if (digitalRead(buttons[2]) == 0) {t_but2 =0;}
               
               while (but1 != 0) {
                but1 = 0;
                      if (digitalRead(buttons[1]) != 0) {
                      t_but1 = t_but1 + 1;
                      but1 = 1;
                          if (t_but1 > 250) {
                          delay(20);
                          but1 = 0;
                          }
                      }
                delay(2);
               }
               while (but2 != 0) {
                but2 = 0;
                      if (digitalRead(buttons[2]) != 0) {
                      t_but2 = t_but2 + 1;
                      but2 = 1;
                          if (t_but2 > 250) {
                          delay(20);
                          but2 = 0;
                          }
                      }
               delay(2);         
             }
             i = 0;
             }
            break;    

        case 12:
          lcd.clear();
          lcd.setCursor(0,0); lcd.print("Setear delay ms");
          lcd.setCursor(0,1); lcd.print(C_delay);

             for (byte i = 0; i < 2; i++)
             {
              int but0 = digitalRead(buttons[0]);
              int but1 = digitalRead(buttons[1]);
              int but2 = digitalRead(buttons[2]);  
                    if (but0 != 0) {
                       currentPos = currentPosParent; //Back
                       saltearunavuelta = 1;
                       i = 2;
                       but0 = 0;
                       break;
                    }
                     if (but1 != 0) {
                        C_delay = C_delay + 1;
                        lcd.setCursor(0,1); lcd.print("        ");
                        lcd.setCursor(0,1); lcd.print(C_delay);
                    }
                     if (but2 != 0) {
                        C_delay = C_delay - 1;
                        lcd.setCursor(0,1); lcd.print("        ");
                        lcd.setCursor(0,1); lcd.print(C_delay);
                    }
    
               if (digitalRead(buttons[1]) == 0) {t_but1 =0;}
               if (digitalRead(buttons[2]) == 0) {t_but2 =0;}
               
               while (but1 != 0) {
                but1 = 0;
                      if (digitalRead(buttons[1]) != 0) {
                      t_but1 = t_but1 + 1;
                      but1 = 1;
                          if (t_but1 > 250) {
                          delay(20);
                          but1 = 0;
                          }
                      }
                delay(2);
               }
               while (but2 != 0) {
                but2 = 0;
                      if (digitalRead(buttons[2]) != 0) {
                      t_but2 = t_but2 + 1;
                      but2 = 1;
                          if (t_but2 > 250) {
                          delay(20);
                          but2 = 0;
                          }
                      }
               delay(2);         
             }
             i = 0;
             }
            break;

        case 13:
          lcd.clear();
          lcd.setCursor(0,0); lcd.print("Setear n");  lcd.print((char)223) ; lcd.print(" veces");
          lcd.setCursor(0,1); lcd.print(C_times);

             for (byte i = 0; i < 2; i++)
             {
              int but0 = digitalRead(buttons[0]);
              int but1 = digitalRead(buttons[1]);
              int but2 = digitalRead(buttons[2]);  
                    if (but0 != 0) {
                       currentPos = currentPosParent; //Back
                       saltearunavuelta = 1;
                       i = 2;
                       but0 = 0;
                       break;
                    }
                     if (but1 != 0) {
                        C_times = C_times + 1;
                        lcd.setCursor(0,1); lcd.print("        ");
                        lcd.setCursor(0,1); lcd.print(C_times);
                    }
                     if (but2 != 0) {
                        C_times = C_times - 1;
                        lcd.setCursor(0,1); lcd.print("        ");
                        lcd.setCursor(0,1); lcd.print(C_times);
                    }
    
               if (digitalRead(buttons[1]) == 0) {t_but1 =0;}
               if (digitalRead(buttons[2]) == 0) {t_but2 =0;}
               
               while (but1 != 0) {
                but1 = 0;
                      if (digitalRead(buttons[1]) != 0) {
                      t_but1 = t_but1 + 1;
                      but1 = 1;
                          if (t_but1 > 250) {
                          delay(20);
                          but1 = 0;
                          }
                      }
                delay(2);
               }
               while (but2 != 0) {
                but2 = 0;
                      if (digitalRead(buttons[2]) != 0) {
                      t_but2 = t_but2 + 1;
                      but2 = 1;
                          if (t_but2 > 250) {
                          delay(20);
                          but2 = 0;
                          }
                      }
               delay(2);         
             }
             i = 0;
             }
            break;

        case 15:
          lcd.clear();
          lcd.setCursor(0,0); lcd.print("Setear delay ms");
          lcd.setCursor(0,1); lcd.print(L_delay);

             for (byte i = 0; i < 2; i++)
             {
              int but0 = digitalRead(buttons[0]);
              int but1 = digitalRead(buttons[1]);
              int but2 = digitalRead(buttons[2]);  
                    if (but0 != 0) {
                       currentPos = currentPosParent; //Back
                       saltearunavuelta = 1;
                       i = 2;
                       but0 = 0;
                       break;
                    }
                     if (but1 != 0) {
                        L_delay = L_delay + 1;
                        lcd.setCursor(0,1); lcd.print("        ");
                        lcd.setCursor(0,1); lcd.print(L_delay);
                    }
                     if (but2 != 0) {
                        L_delay = L_delay - 1;
                        lcd.setCursor(0,1); lcd.print("        ");
                        lcd.setCursor(0,1); lcd.print(L_delay);
                    }
    
               if (digitalRead(buttons[1]) == 0) {t_but1 =0;}
               if (digitalRead(buttons[2]) == 0) {t_but2 =0;}
               
               while (but1 != 0) {
                but1 = 0;
                      if (digitalRead(buttons[1]) != 0) {
                      t_but1 = t_but1 + 1;
                      but1 = 1;
                          if (t_but1 > 250) {
                          delay(20);
                          but1 = 0;
                          }
                      }
                delay(2);
               }
               while (but2 != 0) {
                but2 = 0;
                      if (digitalRead(buttons[2]) != 0) {
                      t_but2 = t_but2 + 1;
                      but2 = 1;
                          if (t_but2 > 250) {
                          delay(20);
                          but2 = 0;
                          }
                      }
               delay(2);         
             }
             i = 0;
             }
            break;

        case 17:
          lcd.clear();
          lcd.setCursor(0,0); lcd.print("Setear n");  lcd.print((char)223) ; lcd.print(" veces");
          lcd.setCursor(0,1); lcd.print(L_times);

             for (byte i = 0; i < 2; i++)
             {
              int but0 = digitalRead(buttons[0]);
              int but1 = digitalRead(buttons[1]);
              int but2 = digitalRead(buttons[2]);  
                    if (but0 != 0) {
                       currentPos = currentPosParent; //Back
                       saltearunavuelta = 1;
                       i = 2;
                       but0 = 0;
                       break;
                    }
                     if (but1 != 0) {
                        L_times = L_times + 1;
                        lcd.setCursor(0,1); lcd.print("        ");
                        lcd.setCursor(0,1); lcd.print(L_times);
                    }
                     if (but2 != 0) {
                        L_times = L_times - 1;
                        lcd.setCursor(0,1); lcd.print("        ");
                        lcd.setCursor(0,1); lcd.print(L_times);
                    }
    
               if (digitalRead(buttons[1]) == 0) {t_but1 =0;}
               if (digitalRead(buttons[2]) == 0) {t_but2 =0;}
               
               while (but1 != 0) {
                but1 = 0;
                      if (digitalRead(buttons[1]) != 0) {
                      t_but1 = t_but1 + 1;
                      but1 = 1;
                          if (t_but1 > 250) {
                          delay(20);
                          but1 = 0;
                          }
                      }
                delay(2);
               }
               while (but2 != 0) {
                but2 = 0;
                      if (digitalRead(buttons[2]) != 0) {
                      t_but2 = t_but2 + 1;
                      but2 = 1;
                          if (t_but2 > 250) {
                          delay(20);
                          but2 = 0;
                          }
                      }
               delay(2);         
             }
             i = 0;
             }
            break;          
              
          case 19:
                    pressedButton =0;
                    lcd.clear();
                    Serial.println("Completo RGBUV (L,N,R,G,B)");
                    lcd.setCursor(0,0); lcd.print("Completo RGBUV");
                    lcd.setCursor(0,1); lcd.print("Analizando...");      
                      C_delay = 20;
                      C_times = 100;
                      L_delay = 100;
                      L_times = 5;
                      L_on = 1;
                      C_on = 1;
                      delay(100);
                      analogWrite(UV, 255);
                      delay(200);
                      UV_N_Prom = Color_N();
                      UV_R_Prom = Color_R();
                      UV_G_Prom = Color_G();
                      UV_B_Prom = Color_B();
                      UV_L_Prom = Lux_P();               
                      Serial.print("UV: "); Serial.print(UV_L_Prom); Serial.print(","); Serial.print(UV_N_Prom); Serial.print(","); Serial.print(UV_R_Prom);Serial.print(","); Serial.print(UV_G_Prom);Serial.print(","); Serial.println(UV_B_Prom);
                      analogWrite(UV, 0);
                      
                      RGB_color(0, 0, 50);
                      delay(200);
                      B_N_Prom = Color_N();
                      B_R_Prom = Color_R();
                      B_G_Prom = Color_G();
                      B_B_Prom = Color_B();
                      B_L_Prom = Lux_P();               
                      Serial.print("B: "); Serial.print(B_L_Prom); Serial.print(","); Serial.print(B_N_Prom); Serial.print(","); Serial.print(B_R_Prom);Serial.print(","); Serial.print(B_G_Prom);Serial.print(","); Serial.println(B_B_Prom);
                      RGB_color(0, 0, 0);

                      RGB_color(0, 50, 0);
                      delay(200);
                      G_N_Prom = Color_N();
                      G_R_Prom = Color_R();
                      G_G_Prom = Color_G();
                      G_B_Prom = Color_B();
                      G_L_Prom = Lux_P();               
                      Serial.print("G: "); Serial.print(G_L_Prom); Serial.print(","); Serial.print(G_N_Prom); Serial.print(","); Serial.print(G_R_Prom);Serial.print(","); Serial.print(G_G_Prom);Serial.print(","); Serial.println(G_B_Prom);
                      RGB_color(0, 0, 0);

                      RGB_color(50, 0, 0);
                      delay(200);
                      R_N_Prom = Color_N();
                      R_R_Prom = Color_R();
                      R_G_Prom = Color_G();
                      R_B_Prom = Color_B();
                      R_L_Prom = Lux_P();               
                      Serial.print("R: "); Serial.print(R_L_Prom); Serial.print(","); Serial.print(R_N_Prom); Serial.print(","); Serial.print(R_R_Prom);Serial.print(","); Serial.print(R_G_Prom);Serial.print(","); Serial.println(R_B_Prom);
                      RGB_color(0, 0, 0);

                      analogWrite(BL, 255);
                      delay(200);
                      BL_N_Prom = Color_N();
                      BL_R_Prom = Color_R();
                      BL_G_Prom = Color_G();
                      BL_B_Prom = Color_B();
                      BL_L_Prom = Lux_P();               
                      Serial.print("BL: "); Serial.print(BL_L_Prom); Serial.print(","); Serial.print(BL_N_Prom); Serial.print(","); Serial.print(BL_R_Prom);Serial.print(","); Serial.print(BL_G_Prom);Serial.print(","); Serial.println(BL_B_Prom);
                      analogWrite(BL, 0);

                      analogWrite(BL, 255);
                      RGB_color(255, 255, 255);
                      delay(200);
                      T_N_Prom = Color_N();
                      T_R_Prom = Color_R();
                      T_G_Prom = Color_G();
                      T_B_Prom = Color_B();
                      T_L_Prom = Lux_P();               
                      Serial.print("T: "); Serial.print(T_L_Prom); Serial.print(","); Serial.print(T_N_Prom); Serial.print(","); Serial.print(T_R_Prom);Serial.print(","); Serial.print(T_G_Prom);Serial.print(","); Serial.println(T_B_Prom);
                      analogWrite(BL, 0);
                      RGB_color(0, 0, 0);
                      
                     lcd.clear();
                     lcd.setCursor(0,0); lcd.print("UV L: "); lcd.print(UV_L_Prom); lcd.print(" lx");
                     lcd.setCursor(0,1); lcd.print("UV N:"); lcd.print(UV_N_Prom); lcd.print(" R:"); lcd.print(UV_R_Prom);

                  int L = 0;
                   for (byte i = 0; i < 2; i++)             
                   {
                   int but0 = digitalRead(buttons[0]);
                   int but1 = digitalRead(buttons[1]);
                   int but2 = digitalRead(buttons[2]);  
                         if (but0 != 0) {
                           currentPos = currentPosParent; //Back
                           i = 2;
                           updateMenu();
                           but0 = 0;
                           break;
                        }
                         if (but1 != 0) {
                                if (L == 1) {
                                  lcd.clear();
                                  lcd.setCursor(0,0); lcd.print("UV L: "); lcd.print(UV_L_Prom); lcd.print(" lx");
                                  lcd.setCursor(0,1); lcd.print("UV N:"); lcd.print(UV_N_Prom); lcd.print(" R:"); lcd.print(UV_R_Prom);
                                  L = 0;
                                }
                                if (L == 2) {
                                  lcd.clear();
                                  lcd.setCursor(0,0); lcd.print("UV G:"); lcd.print(UV_G_Prom); lcd.print(" B:"); lcd.print(UV_B_Prom);        
                                  lcd.setCursor(0,1); lcd.print("B L: "); lcd.print(B_L_Prom); lcd.print(" lx");
                                  L = 1;
                                }
                                if (L == 3) {
                                  lcd.clear();
                                  lcd.setCursor(0,0); lcd.print("B N:"); lcd.print(B_N_Prom); lcd.print(" R:"); lcd.print(B_R_Prom);
                                  lcd.setCursor(0,1); lcd.print("B G:"); lcd.print(B_G_Prom); lcd.print(" B:"); lcd.print(B_B_Prom);  
                                  L = 2;
                                }
                                if (L == 4) {
                                  lcd.clear();
                                  lcd.setCursor(0,0); lcd.print("G L: "); lcd.print(G_L_Prom); lcd.print(" lx"); 
                                  lcd.setCursor(0,1); lcd.print("G N:"); lcd.print(G_N_Prom); lcd.print(" R:"); lcd.print(G_R_Prom);
                                  L = 3;
                                }
                                if (L == 5) {
                                  lcd.clear();
                                  lcd.setCursor(0,0); lcd.print("G G:"); lcd.print(G_G_Prom); lcd.print(" B:"); lcd.print(G_B_Prom); 
                                  lcd.setCursor(0,1); lcd.print("R L: "); lcd.print(R_L_Prom); lcd.print(" lx");
                                  L = 4;
                                }  
                                if (L == 6) {
                                  lcd.clear();
                                  lcd.setCursor(0,0); lcd.print("R N:"); lcd.print(R_N_Prom); lcd.print(" R:"); lcd.print(R_R_Prom);
                                  lcd.setCursor(0,1); lcd.print("R G:"); lcd.print(R_G_Prom); lcd.print(" B:"); lcd.print(R_B_Prom);  
                                  L = 5;
                                } 
                                if (L == 7) {
                                  lcd.clear();
                                  lcd.setCursor(0,0); lcd.print("BL L: "); lcd.print(BL_L_Prom); lcd.print(" lx");
                                  lcd.setCursor(0,1); lcd.print("BL N:"); lcd.print(BL_N_Prom); lcd.print(" R:"); lcd.print(BL_R_Prom);   
                                  L = 6;
                                }      
                                if (L == 8) {
                                  lcd.clear();
                                  lcd.setCursor(0,0); lcd.print("BL G:"); lcd.print(BL_G_Prom); lcd.print(" B:"); lcd.print(BL_B_Prom);  
                                  lcd.setCursor(0,1); lcd.print("T L: "); lcd.print(T_L_Prom); lcd.print(" lx");
                                  L = 7;
                                }                    
                        }
                         if (but2 != 0) {
                                if (L == 7) {
                                  lcd.clear();
                                  lcd.setCursor(0,0); lcd.print("T N:"); lcd.print(T_N_Prom); lcd.print(" R:"); lcd.print(T_R_Prom);
                                  lcd.setCursor(0,1); lcd.print("T G:"); lcd.print(T_G_Prom); lcd.print(" B:"); lcd.print(T_B_Prom);   
                                  L = 8;
                                }
                                if (L == 6) {
                                  lcd.clear();
                                  lcd.setCursor(0,0); lcd.print("BL G:"); lcd.print(BL_G_Prom); lcd.print(" B:"); lcd.print(BL_B_Prom);  
                                  lcd.setCursor(0,1); lcd.print("T L: "); lcd.print(T_L_Prom); lcd.print(" lx");
                                  L = 7;
                                }
                                if (L == 5) {
                                  lcd.clear();
                                  lcd.setCursor(0,0); lcd.print("BL L: "); lcd.print(BL_L_Prom); lcd.print(" lx");
                                  lcd.setCursor(0,1); lcd.print("BL N:"); lcd.print(BL_N_Prom); lcd.print(" R:"); lcd.print(BL_R_Prom);   
                                  L = 6;
                                }  
                                if (L == 4) {
                                  lcd.clear();
                                  lcd.setCursor(0,0); lcd.print("R N:"); lcd.print(R_N_Prom); lcd.print(" R:"); lcd.print(R_R_Prom);
                                  lcd.setCursor(0,1); lcd.print("R G:"); lcd.print(R_G_Prom); lcd.print(" B:"); lcd.print(R_B_Prom);  
                                  L = 5;
                                }  
                                if (L == 3) {
                                  lcd.clear();
                                  lcd.setCursor(0,0); lcd.print("G G:"); lcd.print(G_G_Prom); lcd.print(" B:"); lcd.print(G_B_Prom); 
                                  lcd.setCursor(0,1); lcd.print("R L: "); lcd.print(R_L_Prom); lcd.print(" lx");
                                  L = 4;
                                } 
                                if (L == 2) {
                                  lcd.clear();
                                  lcd.setCursor(0,0); lcd.print("G L: "); lcd.print(G_L_Prom); lcd.print(" lx"); 
                                  lcd.setCursor(0,1); lcd.print("G N:"); lcd.print(G_N_Prom); lcd.print(" R:"); lcd.print(G_R_Prom);
                                  L = 3;
                                }
                                if (L == 1) {
                                  lcd.clear();
                                  lcd.setCursor(0,0); lcd.print("B N:"); lcd.print(B_N_Prom); lcd.print(" R:"); lcd.print(B_R_Prom);
                                  lcd.setCursor(0,1); lcd.print("B G:"); lcd.print(B_G_Prom); lcd.print(" B:"); lcd.print(B_B_Prom);  
                                  L = 2;
                                }
                               if (L == 0) {
                                  lcd.clear();
                                  lcd.setCursor(0,0); lcd.print("UV G:"); lcd.print(UV_G_Prom); lcd.print(" B:"); lcd.print(UV_B_Prom);        
                                  lcd.setCursor(0,1); lcd.print("B L: "); lcd.print(B_L_Prom); lcd.print(" lx");
                                  L = 1;
                                }                         
                        }
                        
                     //Mantiene hasta levantar boton
                     while (but1 != 0) {
                      but1 = 0;
                      if (digitalRead(buttons[1]) != 0) {but1 = 1;}
                      delay(2);
                     }
                     while (but2 != 0) {
                      but2 = 0;
                      if (digitalRead(buttons[2]) != 0) {but2 = 1;}
                      delay(2);
                     } 
                                      
                   i = 0;
                   }
                   break;
      }
      // go to the parent
      currentPos = currentPosParent;
    }
  }
    lcd.clear();
    lcd.setCursor(0,0); lcd.print(mid(menu[currentPos],inStrRev(menu[currentPos],">")+1,len(menu[currentPos])-inStrRev(menu[currentPos],">")));
    lcd.setCursor(0,1); lcd.print(mid(menu[possiblePos[possiblePosScroll]],inStrRev(menu[possiblePos[possiblePosScroll]],">")+1,len(menu[possiblePos[possiblePosScroll]])-inStrRev(menu[possiblePos[possiblePosScroll]],">")));
}

//-------------------------------------------

// Look for a button press
byte checkButtonPress() {
  byte bP = 0;
  byte rBp = 0;
  for (t = 0; t<nrButtons;t++) {
    if (digitalRead(buttons[t]) != 0) {bP = (t + 1);}
    delay(1);
  }
  rBp = bP;
  while (bP != 0) { // wait while the button is still down
    bP = 0;
    for (t = 0; t<nrButtons;t++) {
      if (digitalRead(buttons[t]) != 0) {bP = (t + 1);}
    }
  }
  return rBp;
}

//-------------------------------------------

void setup() {

Serial.begin(9600);

// +LCD
  lcd.init();
  lcd.backlight(); 
  lcd.createChar(1, Tick);
  lcd.clear();
// -LCD

// +LUX
  myBH1750.begin();  
// -LUX

// +LED
  pinMode(BL, OUTPUT);
  pinMode(UV, OUTPUT);
// -LED

// +COLOR
  // Setting the outputs
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  // Setting the sensorOut as an input
  pinMode(sensorOut, INPUT);  
  // Setting frequency scaling to 20%  
  digitalWrite(S0,HIGH);
  digitalWrite(S1,LOW);  
// -COLOR

// +BOTONES
  for (t=0;t<nrButtons;t++) {
    pinMode(buttons[t],INPUT_PULLUP); }
// -BOTONES

lcd.setCursor(0,0); lcd.print("Hola");
delay(1000);
updateMenu();
  
}

//-------------------------------------------

void loop() {

     if (saltearunavuelta == 0){
      pressedButton = checkButtonPress();
          if (pressedButton !=0) {
            switch (pressedButton) {
              case 1:
                currentPos = currentPosParent; //Back
                //possiblePosScroll = 0;
              break;
              case 2:
                currentPos = possiblePos[possiblePosScroll]; //Okay
              break;
              case 3:
                possiblePosScroll = (possiblePosScroll + 1) % possiblePosCount; // Scroll
              break;
            }
            updateMenu();
          }
    }
    else {
      saltearunavuelta = 0;
      checkButtonPress();
    }
}

//-------------------------------------------

String mid(String str, int start, int len) {
   int t = 0;
   String u = ""; 
   for (t = 0; t < len;t++) {
    u = u + str.charAt(t+start-1);
   }
   return u;
}

int inStrRev(String str,String chr) {
  int t = str.length()-1;
  int u = 0;
   while (t>-1) {
    if (str.charAt(t)==chr.charAt(0)) {
      u = t+1;t = -1;
    }
    t = t - 1;
   }
  return u;
}

//-------------------------------------------

int Lux_P(){
  int Li = 0;
  if (L_on == 1) {
  delay(100);
  int i = 0;
  for (i = 0; i < L_times; i++)
    { Li = Li + myBH1750.readLightLevel();
      delay(L_delay);  }
  Li = Li / L_times;
  return Li;
  } else {
      Li = 0;
      return Li;}
  }

int Color_N(){
  int Ni = 0;
  if (C_on == 1) {
  int i = 0;
  digitalWrite(S2,HIGH);
  digitalWrite(S3,LOW);
  for (i = 0; i < C_times; i++)
    {Ni = Ni + pulseIn(sensorOut, LOW);
      delay(C_delay);}
  Ni = Ni / C_times;
  return Ni;
    } else {
      Ni = 0;
      return Ni;}
  }

int Color_R(){
  int Ri = 0;
  if (C_on == 1) {
  int i = 0;
  digitalWrite(S2,LOW);
  digitalWrite(S3,LOW);
  for (i = 0; i < C_times; i++)
    {Ri = Ri + pulseIn(sensorOut, LOW);
     delay(C_delay);}
  Ri = Ri / C_times;
  return Ri;
   } else {
      Ri = 0;
      return Ri;}
  }

int Color_G(){
  int Gi = 0;
  if (C_on == 1) {
  int i = 0;
  digitalWrite(S2,HIGH);
  digitalWrite(S3,HIGH);
  for (i = 0; i < C_times; i++)
    {Gi = Gi + pulseIn(sensorOut, LOW);
      delay(C_delay);}
  Gi = Gi / C_times;
  return Gi;
     } else {
      Gi = 0;
      return Gi;}
  }

int Color_B(){
    int Bi = 0;
    if (C_on == 1) {
    int i = 0;
    digitalWrite(S2,LOW);
    digitalWrite(S3,HIGH);
    for (i = 0; i < C_times; i++)
      {Bi = Bi + pulseIn(sensorOut, LOW);
        delay(C_delay);}
    Bi = Bi / C_times;
    return Bi;
         } else {
        Bi = 0;
        return Bi;}
  }

//------------------------------------------
