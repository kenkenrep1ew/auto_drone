#include <SoftwareSerial.h>
#include <ctype.h>

SoftwareSerial gps(2,3);
float g_lat, g_lon;
int led=13;
char c;

void setup () {
  int i;
  pinMode(led, OUTPUT);
  Serial.begin(9600);
  gps.begin(9600);
  gps.flush();
  
}

void loop () {
  int i;
  static char s[256];
  static int pos = 0;
  float dist_lat = 34.7334333, dist_lon = 135.7344833;//Here is the center of NAIST Ground
  
  if (gps.available()) {
      s[pos] = gps.read();
      if (s[pos] == '\n') {
        s[pos - 1] = '\0'; // Escape '\r'
        if(analyze_data(s)){
          if  (dist_lat < g_lat){
              hover();
          }else{
              move_forward();
          }
        }
        pos = 0;
      } else {
        pos++;
      }
  }
}

// analyze GPS output.
int analyze_data(char *s) {
  char *type;
  char *time;
  char *latitude, *longitude;
  char *tmp;
  
  //parse GPS data.
  type = strtok(s, ",");
  if(strcmp(s, "$GPRMC") != 0) {
    return 0;
  }

  time = strtok(NULL, ",");
  tmp  = strtok(NULL, ",");
  latitude = strtok(NULL, ",");
  tmp = strtok(NULL, ",");
  longitude = strtok(NULL, ",");

  g_lat = stod(latitude);
  g_lon = stod(longitude);
  return 1;
}

// Convert sexagesimal to decimal
float stod(char *s) {
  float f;
  float deg, min;
  
  f = atof(s);

  deg = (int)f / 100;
  min = f - deg * 100;
  
  return deg + min / 60;
}

//Send a character to AR.Drone. This is converted to AT*COMMAND by AR.Drone (auto_drone) .
void move_left(){
Serial.print('a');
delay(200);
}
void move_right(){
Serial.print('d');
delay(200);
}
void move_forward(){
Serial.print('w');
delay(200);
}
void move_backward(){
Serial.print('s');
delay(200);
}
void land(){
Serial.print('j');
delay(200);
}
void takeoff(){
Serial.print('k');
delay(200);
}
void turn_left(){
Serial.print('q');
delay(200);
}
void turn_right(){
Serial.print('e');
delay(200);
}
void move_up(){
Serial.print('1');
delay(200);
}
void hover(){
Serial.print('2');
delay(200);
}
void move_down(){
Serial.print('3');
delay(200);
}
void reset(){
Serial.print('t');
delay(200);
}
void trim(){
Serial.print('y');
delay(200);
}
void halt(){
Serial.print('h');
delay(200);
}
