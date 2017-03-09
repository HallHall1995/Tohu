#include <Suteppa.h>

//-----------------------------------------------------------//
//　↓↓↓↓ここから一般設定↓↓↓↓
//-----------------------------------------------------------//

//初期プレス時間(ms)
const int FIRST_PRESS_TIME = 8000;

//プレス時間(ms)
const int PRESS_TIME = 1800;

//プレス後上昇時間(ms)
const int UP_PRESS_TIME = 700;

//プレスしてからカットするまでの遅延(ms)
const int CUT_DELAY = 1000;

//揚げる時間(ms)   ここはコメントアウトで、切り替えてーお
const unsigned long FRY_TIME = 15000; //とりあえず60秒
//const unsigned long FRY_TIME = 5000; //とりあえず5秒

//カッターの電源投入時間(ms)
const int CUT_TIME = 200;


//-----------------------------------------------------------//
//　↓↓↓↓ここから詳細設定↓↓↓↓
//-----------------------------------------------------------//

//スライダーの最大ステップ数(スライドが最大まで行くためのステップ数)
const int SLIDER_MAX_STEP = 2880;
//スライダーの稼働速度(大きければ遅く。小さければ早く。1500未満は禁止)
const int SLIDER_SPEED = 1500;

//-----------------------------------------------------------//
//　ピン定義です
//　↓↓↓↓ここから変更禁止↓↓↓↓
//-----------------------------------------------------------//

//押し出しのスイッチ上
#define SW_PRESSER_TOP 13
//下
#define SW_PRESSER_BOTTOM 12
//フライヤーのスイッチ上
#define SW_FRYER_TOP 16
//下
#define SW_FRYER_BOTTOM 15
//フライヤーのモーター
#define M_FRYER_A 11 
#define M_FRYER_B 10
//カッターのモーター
#define M_CUTTER_A 9
#define M_CUTTER_B 8
//押し出しのモーター
#define M_PRESSER_A 7
#define M_PRESSER_B 6
//スライド用モーター
#define SM_SLIDER_1 2
#define SM_SLIDER_2 4
#define SM_SLIDER_3 3
#define SM_SLIDER_4 5 
//スライド用スイッチ
#define SW_SLIDER 14
//ボタン
#define BUTTON 19

Suteppa slider;
boolean first_time_fry_2; //動画作成用

void setup() {
  first_time_fry_2 = true; //初回かどうかのboolean
  pinMode(M_FRYER_A, OUTPUT);
  pinMode(M_FRYER_B, OUTPUT);
  pinMode(M_CUTTER_A, OUTPUT);
  pinMode(M_CUTTER_B, OUTPUT);
  pinMode(M_PRESSER_A, OUTPUT);
  pinMode(M_PRESSER_B, OUTPUT);
  pinMode(SM_SLIDER_1, OUTPUT);
  pinMode(SM_SLIDER_2, OUTPUT);
  pinMode(SM_SLIDER_3, OUTPUT);
  pinMode(SM_SLIDER_4, OUTPUT);
  pinMode(SW_PRESSER_TOP, INPUT);
  pinMode(SW_PRESSER_BOTTOM, INPUT);
  pinMode(SW_FRYER_TOP, INPUT);
  pinMode(SW_FRYER_BOTTOM, INPUT);
  pinMode(SW_SLIDER, INPUT);
  pinMode(BUTTON, INPUT);

  Serial.begin(9600); 

  slider.init(200, step);
  slider.setSpeed(SLIDER_SPEED);
  slider.beginSmooth(100, 2000);
}

void loop() {
  wait_button();
  reset();
  wait_button();
  work();
}

//-----------------------------------------------------------//
//仕事
//-----------------------------------------------------------//
void work(){
  Serial.println("#begin work");
  //テーブルを左へ
  work_table_left();
  work_first_press();
  while(1){
    work_fry2();
    //プッシュ降ろす＆切る。
    bool goal = work_press_and_cut();
    //
    //フライを降ろす＆上げる。
    work_fry();
    //おろし切ったら終了
    if(goal) break;
    wait_button();
  }
  Serial.println("#end work");
}
void work_table_left(){
  Serial.println(" -work_table_left");
  slider.rotate(Suteppa::ABSOLUTE, SLIDER_MAX_STEP);
  digitalWrite(SM_SLIDER_1, LOW);
  digitalWrite(SM_SLIDER_2, LOW);
  digitalWrite(SM_SLIDER_3, LOW);
  digitalWrite(SM_SLIDER_4, LOW);
}
void work_first_press(){
  Serial.println(" -work_press_and_cut");
  push(1);
  delay(FIRST_PRESS_TIME);
  push(0);
  delay(1000);
}
bool work_press_and_cut(){
  bool goal = false;
  Serial.println(" -work_press_and_cut");
  unsigned long t = millis();
  while(1){
    if(digitalRead(SW_PRESSER_TOP) == HIGH){
      goal = true;
      break;
    }
    push(1);
    if(t + PRESS_TIME < millis()){
      break;
    }
  }
  if(!goal){
    push(-1);
    delay(UP_PRESS_TIME);
  }
  push(0);
  delay(CUT_DELAY);
  task_cut();
  return goal;
}
void work_fry(){
  Serial.println(" -work_fry");
  //fry(1);
  //while(digitalRead(SW_FRYER_BOTTOM) == LOW){}
  //fry(0);
  delay(FRY_TIME);
  fry(-1);
  while(digitalRead(SW_FRYER_TOP) == LOW){}
  fry(0);
}
void work_fry2(){
  Serial.println(" -work_fry2");
  if (first_time_fry_2 == false ) {
    fry(1);
    while(digitalRead(SW_FRYER_BOTTOM) == LOW){}
  }
  first_time_fry_2 = false;
  fry(0);
}
void task_cut(){
  cut(1);
  delay(CUT_TIME);
  cut(-1);
  delay(CUT_TIME);
  cut(0);
}

//-----------------------------------------------------------//
//リセット
//-----------------------------------------------------------//
void reset(){
  Serial.println("#begin reset");
  reset_presser();
  reset_slider(); 
  reset_cutter();
  reset_fryer();
  Serial.println("#end reset");
}
void reset_slider() {
  Serial.println(" -slider");
  //位置調整
  slider.rotate(Suteppa::RELATIVE, -200000, false);
  while(slider.tick() && !digitalRead(SW_SLIDER)){}
  slider.rotate(Suteppa::RELATIVE, 100);
  slider.rotate(Suteppa::RELATIVE, -200000, false);
  while(slider.tick() && !digitalRead(SW_SLIDER)){}
  slider.rotate(Suteppa::RELATIVE, 100);
  slider.setHome();
  
  digitalWrite(SM_SLIDER_1, LOW);
  digitalWrite(SM_SLIDER_2, LOW);
  digitalWrite(SM_SLIDER_3, LOW);
  digitalWrite(SM_SLIDER_4, LOW);
}
void reset_presser() {
  Serial.println(" -presser");
  push(-1);
  while(digitalRead(SW_PRESSER_BOTTOM) == LOW){}
  push(0);
}
void reset_cutter() {
  Serial.println(" -cutter");
  cut(-1);
  delay(CUT_TIME);
  cut(0);
}
void reset_fryer(){
  Serial.println(" -flyer");
  fry(1);
  while(digitalRead(SW_FRYER_BOTTOM) == LOW){}
  fry(0);
}
//-----------------------------------------------------------//
//各モーター操作用関数
//-----------------------------------------------------------//
void push(int dir){
  if(dir == 1){
    digitalWrite(M_PRESSER_A, HIGH);
    digitalWrite(M_PRESSER_B, LOW);
  }else if(dir == -1){
    digitalWrite(M_PRESSER_A, LOW);
    digitalWrite(M_PRESSER_B, HIGH);
  }else{
    digitalWrite(M_PRESSER_A, LOW);
    digitalWrite(M_PRESSER_B, LOW);
  }
}
void cut(int dir){
  if(dir == -1){
    digitalWrite(M_CUTTER_A, HIGH);
    digitalWrite(M_CUTTER_B, LOW);
  }else if(dir == 1){
    digitalWrite(M_CUTTER_A, LOW);
    digitalWrite(M_CUTTER_B, HIGH);
  }else{
    digitalWrite(M_CUTTER_A, LOW);
    digitalWrite(M_CUTTER_B, LOW);
  }
}
void fry(int dir){
  if(dir == 1){
    digitalWrite(M_FRYER_A, HIGH);
    digitalWrite(M_FRYER_B, LOW);
  }else if(dir == -1){
    digitalWrite(M_FRYER_A, LOW);
    digitalWrite(M_FRYER_B, HIGH);
  }else{
    digitalWrite(M_FRYER_A, LOW);
    digitalWrite(M_FRYER_B, LOW);
  }
}
//-----------------------------------------------------------//
//ボタン待ち
//-----------------------------------------------------------//
void wait_button(){
  while(digitalRead(BUTTON)){}
}
//-----------------------------------------------------------//
//ステッピングモーター用
//-----------------------------------------------------------//
void step(int d)
{
  static const int hs[8] = {0b1000,0b1100,0b0100,0b0110,0b0010,0b0011,0b0001,0b1001};
  static int i;
  i += d;
  if(i > 7) i = 0;
  if(i < 0) i = 7;
  byte b = hs[i];
  digitalWrite(SM_SLIDER_1, bitRead(b, 0));
  digitalWrite(SM_SLIDER_2, bitRead(b, 1));
  digitalWrite(SM_SLIDER_3, bitRead(b, 2));
  digitalWrite(SM_SLIDER_4, bitRead(b, 3));
}
