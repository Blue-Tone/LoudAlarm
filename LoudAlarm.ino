#include <Nefry.h>
#include <NefryDisplay.h>

// グラフは、ver1.1.5でビルドすること。
// https://github.com/Nefry-Community/arduino-esp32/issues/111
// ver1.2.0、1.2.2 NG WiFi情報がバグって表示される感じ。
// ver1.1.5 OK 図形が表示されます。

#define PWM_PIN A0
#define SENSOR_PIN D2
#define ALARM_PIN A2

// グラフのデータ
#define GRAPH_DATA_MAX 128
int graphData[GRAPH_DATA_MAX];
int pos = 0;  // 記録位置
int cnt = 0;  // ループカウンタ

// ボリューム判定
#define LOOP_MAX 100
#define THRESHOLD_LOUD_COUNT_LOOP 30

enum i_conf {
  I_JUDGE_TIMES,    // 判定時間
  I_LOUD_COUNT_MAX, // 音量超過回数閾値
  I_AVERAGE_MAX,    // 平均閾値
  I_ALARM_TIME,     // アラーム鳴動時間
  I_CONF_MAX  
};

int conf[I_CONF_MAX]; // パラメータ保存用配列
unsigned long alarmTime;   // アラーム発生時間

#include <NefrySetting.h>
void setting(){
  Serial.println("start setting()!!!!");  
  Nefry.disableDisplayStatus();
  
}
static NefrySetting nefrySetting(setting);

void setup() {
  Nefry.setProgramName("LoudAlarm v0.1");
  Serial.println("start setup()");  
  NefryDisplay.clear();
  NefryDisplay.display();

  // パラメータ
  Nefry.setConfHtmlStr("判定回数",I_JUDGE_TIMES);
  Nefry.setConfHtmlStr("超過回数",I_LOUD_COUNT_MAX);
  Nefry.setConfHtmlStr("平均閾値",I_AVERAGE_MAX);
  Nefry.setConfHtmlStr("鳴動時間",I_ALARM_TIME);
  
  // パラメータ読込み
  for(int i = 0; i < I_CONF_MAX; i++) {
    // Nefry.setConfStr("1", i); // 初期化処理。0や数字の時にコメントを外して初期化する。その後コメントに戻す。
    conf[i] = Nefry.getConfStr(i).toInt();
    Serial.println("conf[" +  String(i) + "]=" + String(conf[i]));  
  }
  ledcAttachPin(PWM_PIN, 0);
  ledcSetup(0, 12800, 8);

  pinMode(ALARM_PIN, OUTPUT);
}

void loop() {
  // LOOP_MAX回計測し、ONになった回数を音量とする。
  int loudness = 0;
  for(int i=0; i<LOOP_MAX; i++){
    bool isLoud = digitalRead(D2);
    if(!isLoud) {
      loudness++;
    }
   Nefry.ndelay(1);
  }
  
  Serial.println("loudness=" + String(loudness));
  if(loudness > THRESHOLD_LOUD_COUNT_LOOP){
    Serial.println("Loud!! " + String(loudness));  
    Nefry.setLed(255, 0, 0);       // 赤
  }else{
    Serial.println("       " + String(loudness));      
    Nefry.setLed(0, 255, 0);       // 緑
  }

  // ディスプレイ表示クリア
  NefryDisplay.clear();

  cnt++;
//  if(0 == cnt%6){// 間引き
    // グラフデータ更新
    pos++;
    if(pos>127)pos=0;
    graphData[pos] = loudness;
//  }
  
  // グラフ表示
  int drawPos = pos;
  long sum = 0; // 合計
  int loudCount = 0;  // 閾値超えのカウンタ
  for (int i=0; i<GRAPH_DATA_MAX; i++) {
    if(++drawPos >= GRAPH_DATA_MAX)drawPos=0;
    // drawLine(始点x, 始点y, 終点x, 終点y);
    NefryDisplay.drawLine(i, 64, i, 64-graphData[drawPos]);
    if(conf[I_JUDGE_TIMES] > GRAPH_DATA_MAX - i){
      sum += graphData[drawPos];
      if(THRESHOLD_LOUD_COUNT_LOOP < graphData[drawPos]) loudCount++;
    }
  }
  int average = sum / conf[I_JUDGE_TIMES]; // 平均

  // 文字表示
  drawString(1, 1, String(loudness));  // 現在値
  drawString(40, 1, String(loudCount));// 閾値超え
  drawString(80, 1, String(average));  // 平均


  // 閾値判定
  if(conf[I_LOUD_COUNT_MAX] < loudCount){
    Serial.println("loudCount over!!! " + String(loudCount));
    drawString(60, 1, "##");// 閾値超え回数超過
    alarmTime  = millis();
  }
  if(conf[I_AVERAGE_MAX]    < average  ){
    Serial.println("average   over!!! " + String(average));
    drawString(100, 1, "##");  // 平均超え
    alarmTime  = millis();
  }

  NefryDisplay.display();
  //dacWrite(PWM_PIN, loudness);
  ledcWrite(0, loudness);
  //analogWrite(PWM_PIN, loudness);
  //delay(500);
  //Nefry.ndelay(500);

  // 時間内ならアラーム
  if((millis() - alarmTime) < conf[I_ALARM_TIME]){
    Serial.println("ON!!!");
    digitalWrite(ALARM_PIN, HIGH);
  }else{
    Serial.println("OFF!!!");
    digitalWrite(ALARM_PIN, LOW);
  }
  
}

void drawString(int x, int y, String str){
  // グラフがかぶるので、文字表示領域を黒にする
  NefryDisplay.setColor(BLACK);
  NefryDisplay.drawString(x,   y,   str);
  NefryDisplay.drawString(x,   y+2, str);
  NefryDisplay.drawString(x+2, y,   str);
  NefryDisplay.drawString(x+2, y+2, str);
  NefryDisplay.drawString(x+1, y,   str);
  NefryDisplay.drawString(x+1, y+2, str);
  NefryDisplay.drawString(x,   y+1, str);
  NefryDisplay.drawString(x+2, y+1, str);

  NefryDisplay.setColor(WHITE);
  NefryDisplay.drawString(x, y, str);

}

