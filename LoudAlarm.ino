#include <Nefry.h>
#include <NefryDisplay.h>

// グラフは、ver1.1.5でビルドすること。
// https://github.com/Nefry-Community/arduino-esp32/issues/111
// ver1.2.0、1.2.2 NG WiFi情報がバグって表示される感じ。
// ver1.1.5 OK 図形が表示されます。

// グラフのデータ
#define GRAPH_DATA_MAX 128
int graphData[GRAPH_DATA_MAX];
int pos = 0;  // 記録位置
int cnt = 0;  // ループカウンタ

// ボリューム判定
#define LOOP_MAX 100
#define THRESHOLD_LOUD_COUNT_LOOP 30

// 判定時間
// アラーム閾値回数

#include <NefrySetting.h>
void setting(){
  Serial.println("start setting()!!!!");  
  Nefry.disableDisplayStatus();
}
static NefrySetting nefrySetting(setting);

void setup() {
  Nefry.setProgramName("LoudAlarm v0.1");
  Serial.println("start setup()");  
  //Nefry.disableDisplayStatus();
  NefryDisplay.clear();
  NefryDisplay.display();
}

void loop() {
  int loudCount = 0;
  for(int i=0; i<LOOP_MAX; i++){
    if(!digitalRead(D2)) {
      loudCount++;
    }
   Nefry.ndelay(1);
  }
  if(loudCount > THRESHOLD_LOUD_COUNT_LOOP){
    Serial.println("Loud!! " + String(loudCount));  
    Nefry.setLed(255, 0, 0);       // 赤
  }else{
    Serial.println("       " + String(loudCount));      
    Nefry.setLed(0, 255, 0);       // 緑
  }

  // ディスプレイ表示クリア
  NefryDisplay.clear();

  cnt++;
  if(0 == cnt%6){
    // グラフデータ更新
    pos++;
    if(pos>127)pos=0;
    graphData[pos] = loudCount;
  }
  
  // グラフ表示
  int drawPos = pos;
  for (int i=0; i<GRAPH_DATA_MAX; i++) {
    if(++drawPos >= GRAPH_DATA_MAX)drawPos=0;
    // drawLine(始点x, 始点y, 終点x, 終点y);
    NefryDisplay.drawLine(i, 64, i, 64-graphData[drawPos]);
  }
  // グラフがかぶるので、文字表示領域を黒にする
  NefryDisplay.setColor(BLACK);
  NefryDisplay.drawString(0, 0, String(loudCount));
  NefryDisplay.drawString(0, 2, String(loudCount));
  NefryDisplay.drawString(2, 0, String(loudCount));
  NefryDisplay.drawString(2, 2, String(loudCount));
  NefryDisplay.drawString(1, 0, String(loudCount));
  NefryDisplay.drawString(1, 2, String(loudCount));
  NefryDisplay.drawString(0, 1, String(loudCount));
  NefryDisplay.drawString(2, 1, String(loudCount));
  
  // 文字表示
  NefryDisplay.setColor(WHITE);
  NefryDisplay.drawString(1, 1, String(loudCount));
  NefryDisplay.display();

  //delay(500);
  //Nefry.ndelay(500);

}
