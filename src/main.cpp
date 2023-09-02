// https://github.com/meganetaaan/m5stack-avatar/blob/master/README_ja.md
//
//#include <M5AtomS3.h> // ヘッダーファイル準備（別途FastLEDライブラリをインストール）
//#include <Arduino.h>
#include <M5Unified.h>
#include <Avatar.h>
#include <FS.h>           // 書かないとコンパイルできなかったため記入、機能的には未使用
#include <faces/DogFace.h>
#include "formatString.hpp" // https://gist.github.com/GOB52/e158b689273569357b04736b78f050d6
#include "Grove_Multi_Switch.h" // 5WaySwitchライブラリ

// DRV8835
const uint8_t AIN1 = 5; // A1:PHASE
const uint8_t AIN2 = 6; // A2:ENBL
const uint8_t BIN1 = 7; // B1:PHASE
const uint8_t BIN2 = 8; // B2:ENBL

GroveMultiSwitch mswitch[1];
const char* grove_5way_tactile_keys[] = {
    "KEY A",
    "KEY B",
    "KEY C",
    "KEY D",
    "KEY E",
};
const char* grove_6pos_dip_switch_keys[] = {
    "POS 1",
    "POS 2",
    "POS 3",
    "POS 4",
    "POS 5",
    "POS 6",
};

const char** key_names;

int deviceDetect(void) {
    if (!mswitch->begin()) {
        Serial.println("***** Device probe failed *****");
        return -1;
    }

    Serial.println("***** Device probe OK *****");
    if (PID_VAL(mswitch->getDevID()) == PID_5_WAY_TACTILE_SWITCH) {
        Serial.println("Grove 5-Way Tactile Switch Inserted!");
        key_names = grove_5way_tactile_keys;
    } else if (PID_VAL(mswitch->getDevID()) == PID_6_POS_DIP_SWITCH) {
        Serial.println("Grove 6-Position DIP Switch Inserted!");
        key_names = grove_6pos_dip_switch_keys;
    }

    // enable event detection
    mswitch->setEventMode(true);

    // report device model
    Serial.print("A ");
    Serial.print(mswitch->getSwitchCount());
    Serial.print(" Button/Switch Device ");
    Serial.println(mswitch->getDevVer());
    return 0;
}


using namespace m5avatar;

Avatar avatar;
float scale = 0.0f;
int8_t position_x = 0;
int8_t position_y = 0;
uint8_t display_rotation = 3; // ディスプレイの向き(0〜3)
Face* faces[2];
const int facesSize = sizeof(faces) / sizeof(Face*);
int faceIdx = 0;

void face_change(int fno) {
  switch (fno)
  {
    case 0: avatar.setExpression(Expression::Neutral); break;
    case 1: avatar.setExpression(Expression::Happy); break;
    case 2: avatar.setExpression(Expression::Sleepy); break;
    case 3: avatar.setExpression(Expression::Doubt); break;
    case 4: avatar.setExpression(Expression::Sad); break;
    case 5: avatar.setExpression(Expression::Angry); break;
    default: avatar.setExpression(Expression::Neutral); break;  
  } 
}

void setup()
{
  auto cfg = M5.config();
  cfg.serial_baudrate = 115200;
  cfg.internal_imu  = true;  // default=true. use internal IMU.
  M5.begin(cfg);
  //M5.begin();

  Serial.println("5WaySwitch test program start");
  delay(1000);

  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);

  // pwm
  ledcSetup(0, 2000, 8); // CH,Hz,bit
  ledcSetup(1, 2000, 8); // CH,Hz,bit
  ledcSetup(2, 2000, 8); // CH,Hz,bit
  ledcSetup(3, 2000, 8); // CH,Hz,bit
  ledcAttachPin(AIN1, 0); // Pin, CH
  ledcAttachPin(AIN2, 1); // Pin, CH
  ledcAttachPin(BIN1, 2); // Pin, CH
  ledcAttachPin(BIN2, 3); // Pin, CH

  // Initial device probe
  if (deviceDetect() < 0) {
    Serial.println("Insert Grove 5-Way Tactile");
    Serial.println("or Grove 6-Position DIP Switch");
    delay(1000);
    //for (;;);
  }

  //float scale = 0.0f;
  //int8_t position_x = 0;
  //int8_t position_y = 0;
  //uint8_t display_rotation = 3; // ディスプレイの向き(0〜3)
  uint8_t first_cps = 0;
switch (M5.getBoard()) {
    case m5::board_t::board_M5AtomS3:
      first_cps = 4;
      scale = 0.5f;
      position_x = 5;
      position_y = -15;
      display_rotation = 3;
      //pin_clk  = 1;
      //pin_data = 2;
      break;

case m5::board_t::board_M5StickC:
#ifdef ARDUINO_M5Stick_C    
      M5.Power.Axp192.setLDO0(2800); // 一部これを実行しないとマイクが動かない機種がある。
#endif
      first_cps = 1;
      scale = 0.6f;
      position_x = -30;
      position_y = -15;
      //pin_clk  = 0;
      //pin_data = 34;
      break;

    case m5::board_t::board_M5StickCPlus:
#if defined(ARDUINO_M5Stick_C) || defined(ARDUINO_M5Stick_C_PLUS)     
      M5.Power.Axp192.setLDO0(2800); // 一部これを実行しないとマイクが動かない機種がある。
#endif
      first_cps = 2;
      scale = 0.7f;
      position_x = -15;
      position_y = 5;
      //pin_clk  = 0;
      //pin_data = 34;
      break;
    
    case m5::board_t::board_M5StackCore2:
      scale = 1.0f;
      position_x = 0;
      position_y = 0;
      display_rotation = 1;
      //pin_clk  = 0;
      //pin_data = 34;
      break;
      
    defalut:
      Serial.println("Invalid board.");
      break;
}

  faces[0] = avatar.getFace();
  faces[1] = new DogFace();
  M5.Lcd.setRotation(display_rotation);
  avatar.setScale(scale);
  avatar.setPosition(position_x, position_y);
  avatar.setSpeechFont(&fonts::Font4);
  avatar.init(); // 描画を開始します。
  Serial.println("avatar start");
  //Serial.printf("avatar start %d %d %d\r\n", scale, position_x, position_y);

  //M5.Imu.begin();
  delay(500);
}

int fno=1;
uint8_t new_btn = 0;
uint8_t old_btn = 0;
int dtn = 0;

void loop()
{
  M5.update();  // ボタン状態初期化

#if 1
  //5WaySwitch
  GroveMultiSwitch::ButtonEvent_t* evt;
  delay(1);
  evt = mswitch->getEvent();
  if (!evt) {
    Serial.println("5WaySwitch can not getEvent");
  }
  else {
    if (evt->event & GroveMultiSwitch::BTN_EV_HAS_EVENT) {
      Serial.print("BTN_EV_HAS_EVENT : ");
      if (!(evt->button[0] & GroveMultiSwitch::BTN_EV_RAW_STATUS)) {new_btn = 1;Serial.println("Btn0");}
      else if (!(evt->button[1] & GroveMultiSwitch::BTN_EV_RAW_STATUS)) {new_btn = 2;Serial.println("Btn1");}
      else if (!(evt->button[2] & GroveMultiSwitch::BTN_EV_RAW_STATUS)) {new_btn = 3;Serial.println("Btn2");}
      else if (!(evt->button[3] & GroveMultiSwitch::BTN_EV_RAW_STATUS)) {new_btn = 4;Serial.println("Btn3");}
      else if (!(evt->button[4] & GroveMultiSwitch::BTN_EV_RAW_STATUS)) {new_btn = 5;Serial.println("Btn4");}
      else {new_btn = 0;Serial.println("HOME");}

      if (new_btn != old_btn)
      {
        std::string s;
        old_btn = new_btn;
        int La1, La2, Lb1, Lb2 = LOW;
        switch(new_btn){ //DRV8835 MODE=0 モーターは2個逆方向
          case 0:  display_rotation=3; faceIdx = 0; La1 = La2 = Lb1 = Lb2 = LOW; break; //停止
          case 3:  display_rotation=1; faceIdx = 1; La1 = HIGH; La2 = LOW ; Lb1 = LOW ; Lb2 = HIGH; break;//前進
          case 4:  display_rotation=0; faceIdx = 1; La1 = HIGH; La2 = LOW ; Lb1 = HIGH; Lb2 = LOW ; break;//右回転
          case 1:  display_rotation=3; faceIdx = 1; La1 = LOW ; La2 = HIGH; Lb1 = HIGH; Lb2 = LOW ; break;//後退
          case 2:  display_rotation=2; faceIdx = 1; La1 = LOW ; La2 = HIGH; Lb1 = LOW ; Lb2 = HIGH; break;//左回転
          case 5:  display_rotation=3; faceIdx = 1; La1 = La2 = Lb1 = Lb2 = LOW; 
            dtn = (dtn+1)%3;
            break;
          default: display_rotation=3; faceIdx = 0; La1 = La2 = Lb1 = Lb2 = LOW; break;
        }

        //digitalWrite(AIN1, La1);
        //digitalWrite(AIN2, La2);
        //digitalWrite(BIN1, Lb1);
        //digitalWrite(BIN2, Lb2);
        
        int dt = 128+dtn*64;

        ledcWrite(0, dt*La1);// CH,Duty デューティー比0.25(64/256)でPWM制御
        ledcWrite(1, dt*La2);// CH,Duty デューティー比0.25(64/256)でPWM制御
        ledcWrite(2, dt*Lb1);// CH,Duty デューティー比0.25(64/256)でPWM制御
        ledcWrite(3, dt*Lb2);// CH,Duty デューティー比0.25(64/256)でPWM制御

        M5.Lcd.setRotation(display_rotation);
        avatar.setScale(scale);
        avatar.setPosition(position_x, position_y);
          //avatar.setScale(0.5f);
        //avatar.setPosition(5, -15);
        avatar.setFace(faces[faceIdx]);
        //avatar.init();
        //delay(100);
        if(new_btn == 5){
          s = formatString("PW:%d", int(dt/256.0*100));
          avatar.setSpeechText(s.c_str());
        }
        Serial.printf("display_rotation %d, duty %d\r\n", display_rotation, int(dt/256.0*100));
      }
    }
    else {
      //Serial.println("No event");
      //Serial.print("No event, errno = ");
      //Serial.println(mswitch->errno);
    }
  }
#endif

#if 0
  //5WaySwitch
  GroveMultiSwitch::ButtonEvent_t* evt;
  delay(1);
  evt = mswitch->getEvent();
  if (!evt) {
    // dynamic device probe
    deviceDetect();
    delay(1000);
    return;
  }

  if (!(evt->event & GroveMultiSwitch::BTN_EV_HAS_EVENT)) {
    #if 0
    Serial.print("No event, errno = ");
    Serial.println(mswitch->errno);
    #endif
    return;
  }

  for (int i = 0; i < mswitch->getSwitchCount(); i++) {
    Serial.print(key_names[i]);
    Serial.print(": RAW - ");
    Serial.print((evt->button[i] & GroveMultiSwitch::BTN_EV_RAW_STATUS) ? "HIGH " : "LOW ");
    if (PID_VAL(mswitch->getDevID()) == PID_5_WAY_TACTILE_SWITCH) {
      Serial.print((evt->button[i] & GroveMultiSwitch::BTN_EV_RAW_STATUS) ? "RELEASED " : "PRESSED ");
    } else if (PID_VAL(mswitch->getDevID()) == PID_6_POS_DIP_SWITCH) {
      Serial.print((evt->button[i] & GroveMultiSwitch::BTN_EV_RAW_STATUS) ? "OFF " : "ON ");
    }
    Serial.println("");
  }

  for (int i = 0; i < mswitch->getSwitchCount(); i++) {
    if (evt->button[i] & ~GroveMultiSwitch::BTN_EV_RAW_STATUS) {
      Serial.println("");
      Serial.print(key_names[i]);
      Serial.print(": EVENT - ");
    }
    if (evt->button[i] & GroveMultiSwitch::BTN_EV_SINGLE_CLICK) {
      Serial.print("SINGLE-CLICK ");
    }
    if (evt->button[i] & GroveMultiSwitch::BTN_EV_DOUBLE_CLICK) {
      Serial.print("DOUBLE-CLICK ");
    }
    if (evt->button[i] & GroveMultiSwitch::BTN_EV_LONG_PRESS) {
      Serial.print("LONG-PRESS ");
    }
    if (evt->button[i] & GroveMultiSwitch::BTN_EV_LEVEL_CHANGED) {
      Serial.print("LEVEL-CHANGED ");
    }
  }
  Serial.println("");

  return;
#endif

  // アバターの描画は別のスレッドで行われるので、
  // loopループの中で毎回描画をする必要はありません。
  //M5.update();  // ボタン状態初期化

#if 0
  String printStr = "";
  static String oldStr = "";
  printStr += String(",isPressed:") + String(M5.BtnA.isPressed() ? "X" : " ");
  printStr += String(",isHolding:") + String(M5.BtnA.isHolding() ? "X" : " ");
  printStr += String(",isReleased:") + String(M5.BtnA.isReleased() ? "X" : " ");
  printStr += String(",wasPressed:") + String(M5.BtnA.wasPressed() ? "X" : " ");
  printStr += String(",wasHold:") + String(M5.BtnA.wasHold() ? "X" : " ");
  printStr += String(",wasReleased:") + String(M5.BtnA.wasReleased() ? "X" : " ");
  printStr += String(",wasClicked:") + String(M5.BtnA.wasClicked() ? "X" : " ");
  if (oldStr != printStr) {
    Serial.println(printStr);
    oldStr = printStr;
  }
#endif

    // 本体ボタン処理
  if (M5.BtnA.wasPressed()) {    // ボタンが押されていれば
    face_change(fno);
    fno++;
    if (fno>5) fno=0;
    Serial.println("face change");
  }

#if 0
  //------------------- IMU test
  float ax, ay, az, gx, gy, gz, t;
  float pitch, roll, yaw;
  if (M5.Imu.isEnabled()) {
    M5.Imu.getAccel(&ax, &ay, &az);
    M5.Imu.getGyro(&gx, &gy, &gz);
    
    //Serial.printf("axyz %5.1f , %5.1f , %5.1f  ", ax, ay, az);
    //Serial.printf("gxyz %5.1f , %5.1f , %5.1f\r\n", gx, gy, gz);

    uint8_t new_dr;
    if (ax < -0.5) new_dr = 3;
    if (ax > 0.5) new_dr = 1;
    if (ay < -0.5) new_dr = 2;
    if (ay > 0.5) new_dr = 0;
    if (new_dr != display_rotation)
    {
      display_rotation = new_dr;
      M5.Lcd.setRotation(display_rotation);
      avatar.setScale(0.5f);
      avatar.setPosition(5, -15);
      avatar.init();
      Serial.printf("display_rotation %d\r\n", display_rotation);
    }
    //if (ax < -0.5) M5.Lcd.setRotation(3);
    //if (ax > 0.5) M5.Lcd.setRotation(1);
    //if (ay < -0.5) M5.Lcd.setRotation(2);
    //if (ay > 0.5) M5.Lcd.setRotation(0);
  }
#endif

  delay(100);
}