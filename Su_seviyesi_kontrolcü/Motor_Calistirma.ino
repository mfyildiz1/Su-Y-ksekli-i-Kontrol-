  #include <LiquidCrystal.h>
  LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
#define Buton1 22
#define Buton2 24
#define Buton3 26
#define echoPin 8
#define trigPin 7
#define power1 9
#define power2 6
#define potpin A0


int maximumRange = 50;
int minimumRange = 0;

int buton1_durumu = 0;
int buton2_durumu = 0;
int buton3_durumu = 0;

int olcum = 0, hata = 0, deger = 0, deger_i = 0;
double now = 0, dt = 0, last_time = 0, onceki = 0, kp, ki, kd, i_hata = 0, d_hata = 0, toplam_hata = 0;
float yukseklik = 0;
    
void setup() {
  Serial.begin(9600);
  lcd.begin(16, 2);
  pinMode(Buton1, INPUT);
  pinMode(Buton2, INPUT);
  pinMode(Buton3, INPUT);
  pinMode(power1,OUTPUT);
  pinMode(power2,OUTPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
}

void loop() {
  deger = analogRead(potpin);
  yukseklik = (20.00/1024.00)*deger;
  deger_i=round(yukseklik);

  lcd.setCursor(0, 0);
  lcd.print("Ref:");
  lcd.setCursor(5, 0); // Set the cursor position for the first value
  lcd.print(String(deger_i));
  olcum = mesafe(maximumRange, minimumRange);


  lcd.setCursor(0, 1);
  lcd.print("H:");
  lcd.setCursor(3, 2);
  lcd.print(String(olcum));
  lcd.setCursor(9, 2);

  
  buton1_durumu = digitalRead(Buton1);
  buton2_durumu = digitalRead(Buton2);
  buton3_durumu = digitalRead(Buton3); 

  if(buton1_durumu == 0 && buton2_durumu ==0 && buton3_durumu==0)
    {
    analogWrite(power1, 0); 
    analogWrite(power2, 0);
    lcd.print("Bos");
    } else if (buton1_durumu == 1 && buton2_durumu ==0 && buton3_durumu==0) 
      {
        // ac-kapa KONTROLCU
        Serial.println("AC-kapa");
        ac_kapa_kontrollor(deger_i);
        lcd.print("Ac-Kapa");
      }
      else if (buton1_durumu == 0 && buton2_durumu ==1 && buton3_durumu==0) 
      {
               // P KONTROLCU 
        Serial.println("\np");
        p_kontrollor(deger_i);
        lcd.print("P");
      }
      else if (buton1_durumu == 1 && buton2_durumu ==1 && buton3_durumu==0) 
      {
        // PI KONTROLCU
        Serial.println("\nPI");
        p_i_kontrollor(deger_i);
        lcd.print("PI");
      }
      else if(buton1_durumu == 0 && buton2_durumu ==0 && buton3_durumu==1)
      {
       // PD KONTROLCU
        Serial.println("\nPD");
        p_d_kontrollor(deger_i);
        lcd.print("PD");
      }
      else if(buton1_durumu == 1 && buton2_durumu ==0 && buton3_durumu==1)
      {
       // PID KONTROLCU
        Serial.println("\nPID");
        p_i_d_kontrollor(deger_i);
        lcd.print("PID");
      }
  
delay(100);
lcd.clear();
}

int mesafe(int maxrange, int minrange)
{
  long duration, distance;

  digitalWrite(trigPin,LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH);
  distance = duration / 58.2;

  if(distance >= maxrange || distance <= minrange)
  return 0;
  return 13 - distance;
}

void ac_kapa_kontrollor(int ref){
  olcum = mesafe(maximumRange, minimumRange);
  hata = ref - olcum;
  if(hata > 0){
    analogWrite(power2, 255); 
    analogWrite(power1, 0);
  }else{
    analogWrite(power2, 0); 
    analogWrite(power1, 255);
}  
}

void p_kontrollor(int ref){
  kp = 15;
  olcum = mesafe(maximumRange, minimumRange);
  hata = ref - olcum;
  if(hata > 0){
    if(80+hata*kp > 255){
        analogWrite(power2, 255);
    }else{
      analogWrite(power2, 80+hata*kp);
    }
    analogWrite(power1, 0); 
  }else if (hata < 0){
    if(80-1*hata*kp > 255){
        analogWrite(power1, 255);
    }else{
      analogWrite(power1, 80-1*hata*kp);
    }
    analogWrite(power2, 0); 
}
else{  
  analogWrite(power1, 0); 
  analogWrite(power2, 0);
}
}


void p_i_kontrollor(int ref){
  kp = 15;
  ki = 0.3;
  olcum = mesafe(maximumRange, minimumRange);
  now = millis();
  dt = (now - last_time)/1000;
  last_time = now;
  hata = ref - olcum;
  i_hata += hata*dt;
  if(hata > 0){
    toplam_hata = 80+((kp*hata) + (ki*i_hata));
    if(toplam_hata > 255){
        toplam_hata = 255;
    }
    analogWrite(power2, toplam_hata);
    analogWrite(power1, 0);
  }
  else if (hata < 0){
    toplam_hata = 80 + abs((kp*hata) + (ki*i_hata));
    if(toplam_hata > 255){
        toplam_hata = 255;
    }
    analogWrite(power1, toplam_hata);
    analogWrite(power2, 0);
    
  }
  else{  
  analogWrite(power1, 0); 
  analogWrite(power2, 0);
}
  
}


void p_d_kontrollor(int ref){
  kp =15;
  kd = 0.5;
  olcum = mesafe(maximumRange, minimumRange);
  now = millis();
  dt = (now - last_time)/1000;
  last_time = now;
  hata = ref - olcum;
  d_hata = (hata-onceki)/dt;
  onceki = hata;
  if(hata > 0){
    toplam_hata = 80 + ((kp*hata) + (kd*d_hata));
    if(toplam_hata > 255){
        toplam_hata = 255;
    }
    analogWrite(power2, toplam_hata);
    analogWrite(power1, 0);
  }
  else if (hata < 0){
    toplam_hata = 80 -1*((kp*hata) + (kd*d_hata));
    if(toplam_hata > 255){
        toplam_hata = 255;
    }
    analogWrite(power1, toplam_hata);
    analogWrite(power2, 0);
    
  }
  else{  
  analogWrite(power1, 0); 
  analogWrite(power2, 0);
}
  
}





void p_i_d_kontrollor(int ref){
  kp = 15;
  ki = 0.3;
  kd = 0.5;
  olcum = mesafe(maximumRange, minimumRange);
  now = millis();
  dt = (now - last_time)/1000;
  last_time = now;
  hata = ref - olcum;
  i_hata += hata*dt;
  d_hata = (hata-onceki)/dt;
  onceki = hata;
  if(hata > 0){
    toplam_hata = 50+((kp*hata) + (kd*d_hata) +(ki*i_hata));
    if(toplam_hata > 255){
        toplam_hata = 255;
    }
    analogWrite(power2, toplam_hata);
    analogWrite(power1, 0);
  }
  else if(hata < 0){
    toplam_hata = 50+ abs((kp*hata) + (kd*d_hata) + (ki*i_hata));
    if(toplam_hata > 255){
        toplam_hata = 255;
    }
    analogWrite(power1, toplam_hata);
    analogWrite(power2, 0);
    
  }
  else{  
  analogWrite(power1, 0); 
  analogWrite(power2, 0);
}
  
}
