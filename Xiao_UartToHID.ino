/*
  Seeedino XIAOを使ってUART入力をHIDキー出力にする
*/

#include "HID-Project.h"

#define VERSION "0.4"
#define PIN_BTN   5
#define KEYMAP_LEN 256
//#define DEBUG_MODE

// TODO:リングバッファ及びRTSピン対応(LOWで送信可能)

bool serialMode = false; // シリアルモード(シリアル出力モードの有無)

struct Key {
  KeyboardKeycode key;
  bool shift;
};

// https://mgt.blog.ss-blog.jp/2016-01-14
// キーコード＋SHIFTキーの有無(有=1)
const Key KEYMap[KEYMAP_LEN] =
{
  {KEY_ERROR_UNDEFINED, false}, //0  0x00  NUL(null文字)
  {KEY_ERROR_UNDEFINED, false}, //1 0x01  SOH(ヘッダ開始)
  {KEY_ERROR_UNDEFINED, false}, //2 0x02  STX(テキスト開始)
  {KEY_ERROR_UNDEFINED, false}, //3 0x03  ETX(テキスト終了)
  {KEY_ERROR_UNDEFINED, false}, //4 0x04  EOT(転送終了)
  {KEY_ERROR_UNDEFINED, false}, //5 0x05  ENQ(照会)
  {KEY_ERROR_UNDEFINED, false}, //6 0x06  ACK(受信確認)
  {KEY_ERROR_UNDEFINED, false}, //7 0x07  BEL(警告)
  {KEY_BACKSPACE, false}, //8 0x08  BS(後退)
  {KEY_TAB, false}, //9 0x09  HT(水平タブ)
  {KEY_ENTER, false}, //10  0x0a  LF(改行)
  {KEY_ERROR_UNDEFINED, false}, //11  0x0b  VT(垂直タブ)
  {KEY_ERROR_UNDEFINED, false}, //12  0x0c  FF(改頁)
  {KEY_ERROR_UNDEFINED, false}, //13  0x0d  CR(復帰)
  {KEY_ERROR_UNDEFINED, false}, //14  0x0e  SO(シフトアウト)
  {KEY_ERROR_UNDEFINED, false}, //15  0x0f  SI(シフトイン)
  {KEY_ERROR_UNDEFINED, false}, //16  0x10  DLE(データリンクエスケープ)
  {KEY_ERROR_UNDEFINED, false}, //17  0x11  DC1(装置制御1)
  {KEY_ERROR_UNDEFINED, false}, //18  0x12  DC2(装置制御2)
  {KEY_ERROR_UNDEFINED, false}, //19  0x13  DC3(装置制御3)
  {KEY_ERROR_UNDEFINED, false}, //20  0x14  DC4(装置制御4)
  {KEY_ERROR_UNDEFINED, false}, //21  0x15  NAK(受信失敗)
  {KEY_ERROR_UNDEFINED, false}, //22  0x16  SYN(同期)
  {KEY_ERROR_UNDEFINED, false}, //23  0x17  ETB(転送ブロック終了)
  {KEY_ERROR_UNDEFINED, false}, //24  0x18  CAN(キャンセル)
  {KEY_ERROR_UNDEFINED, false}, //25  0x19  EM(メディア終了)
  {KEY_ERROR_UNDEFINED, false}, //26  0x1a  SUB(置換)
  {KEY_ESC, false}, //27  0x1b  ESC(エスケープ)
  {KEY_ERROR_UNDEFINED, false}, //28  0x1c  FS(フォーム区切り)
  {KEY_ERROR_UNDEFINED, false}, //29  0x1d  GS(グループ区切り)
  {KEY_ERROR_UNDEFINED, false}, //30  0x1e  RS(レコード区切り)
  {KEY_ERROR_UNDEFINED, false}, //31  0x1f  US(ユニット区切り)
  {KEY_SPACE, false}, //32  0x20  SPC(空白文字)
  {KEY_1, true}, //33  0x21  !
  {KEY_2, true}, //34  0x22  "
  {KEY_3, true}, //35  0x23  #
  {KEY_4, true}, //36  0x24  $
  {KEY_5, true}, //37  0x25  %
  {KEY_6, true}, //38  0x26  &
  {KEY_7, true}, //39  0x27  '
  {KEY_8, true}, //40  0x28  (
  {KEY_9, true}, //41  0x29  )
  {KEY_QUOTE, true}, //42  0x2a  *
  {KEY_SEMICOLON, true}, //43  0x2b  +
  {KEY_COMMA, false}, //44  0x2c  ,
  {KEY_MINUS, false}, //45  0x2d  -
  {KEY_PERIOD, false}, //46  0x2e  .
  {KEY_SLASH, false}, //47  0x2f  /
  {KEY_0, false}, //48  0x30  0
  {KEY_1, false}, //49  0x31  1
  {KEY_2, false}, //50  0x32  2
  {KEY_3, false}, //51  0x33  3
  {KEY_4, false}, //52  0x34  4
  {KEY_5, false}, //53  0x35  5
  {KEY_6, false}, //54  0x36  6
  {KEY_7, false}, //55  0x37  7
  {KEY_8, false}, //56  0x38  8
  {KEY_9, false}, //57  0x39  9
  {KEY_QUOTE, false}, //58  0x3a  :
  {KEY_SEMICOLON, false}, //59  0x3b  ;
  {KEY_COMMA, true}, //60  0x3c  <
  {KEY_MINUS, true}, //61  0x3d  =
  {KEY_PERIOD, true}, //62  0x3e  >
  {KEY_SLASH, true}, //63  0x3f  ?
  {KEY_LEFT_BRACE, false}, //64  0x40  @
  {KEY_A, true}, //65  0x41  A
  {KEY_B, true}, //66  0x42  B
  {KEY_C, true}, //67  0x43  C
  {KEY_D, true}, //68  0x44  D
  {KEY_E, true}, //69  0x45  E
  {KEY_F, true}, //70  0x46  F
  {KEY_G, true}, //71  0x47  G
  {KEY_H, true}, //72  0x48  H
  {KEY_I, true}, //73  0x49  I
  {KEY_J, true}, //74  0x4a  J
  {KEY_K, true}, //75  0x4b  K
  {KEY_L, true}, //76  0x4c  L
  {KEY_M, true}, //77  0x4d  M
  {KEY_N, true}, //78  0x4e  N
  {KEY_O, true}, //79  0x4f  O
  {KEY_P, true}, //80  0x50  P
  {KEY_Q, true}, //81  0x51  Q
  {KEY_R, true}, //82  0x52  R
  {KEY_S, true}, //83  0x53  S
  {KEY_T, true}, //84  0x54  T
  {KEY_U, true}, //85  0x55  U
  {KEY_V, true}, //86  0x56  V
  {KEY_W, true}, //87  0x57  W
  {KEY_X, true}, //88  0x58  X
  {KEY_Y, true}, //89  0x59  Y
  {KEY_Z, true}, //90  0x5a  Z
  {KEY_RIGHT_BRACE, false}, //91  0x5b [
  {HID_KEYBOARD_INTERNATIONAL3, false}, //92  0x5c  バックスラッシュ
  {KEY_BACKSLASH, false}, //93  0x5d  ]
  {KEY_EQUAL, false}, //94  0x5e  ^
  {HID_KEYBOARD_INTERNATIONAL1, true}, //95  0x5f  _
  {KEY_LEFT_BRACE, true}, //96  0x60  `
  {KEY_A, false}, //97  0x61  a
  {KEY_B, false}, //98  0x62  b
  {KEY_C, false}, //99  0x63  c
  {KEY_D, false}, //100 0x64  d
  {KEY_E, false}, //101 0x65  e
  {KEY_F, false}, //102 0x66  f
  {KEY_G, false}, //103 0x67  g
  {KEY_H, false}, //104 0x68  h
  {KEY_I, false}, //105 0x69  i
  {KEY_J, false}, //106 0x6a  j
  {KEY_K, false}, //107 0x6b  k
  {KEY_L, false}, //108 0x6c  l
  {KEY_M, false}, //109 0x6d  m
  {KEY_N, false}, //110 0x6e  n
  {KEY_O, false}, //111 0x6f  o
  {KEY_P, false}, //112 0x70  p
  {KEY_Q, false}, //113 0x71  q
  {KEY_R, false}, //114 0x72  r
  {KEY_S, false}, //115 0x73  s
  {KEY_T, false}, //116 0x74  t
  {KEY_U, false}, //117 0x75  u
  {KEY_V, false}, //118 0x76  v
  {KEY_W, false}, //119 0x77  w
  {KEY_X, false}, //120 0x78  x
  {KEY_Y, false}, //121 0x79  y
  {KEY_Z, false}, //122 0x7a  z
  {KEY_RIGHT_BRACE, true}, //123 0x7b  {
  {HID_KEYBOARD_INTERNATIONAL3, true}, //124 0x7c  |
  {KEY_BACKSLASH, true}, //125 0x7d  }
  {KEY_EQUAL, true}, //126 0x7e  ~
  {KEY_DELETE, false}, //127 0x7f  DEL(削除)
  {KEY_ERROR_UNDEFINED, false}, //128 0x80
  {KEY_ERROR_UNDEFINED, false}, //129 0x81
  {KEY_ERROR_UNDEFINED, false}, //130 0x82
  {KEY_ERROR_UNDEFINED, false}, //131 0x83
  {KEY_ERROR_UNDEFINED, false}, //132 0x84
  {KEY_ERROR_UNDEFINED, false}, //133 0x85
  {KEY_ERROR_UNDEFINED, false}, //134 0x86
  {KEY_ERROR_UNDEFINED, false}, //135 0x87
  {KEY_ERROR_UNDEFINED, false}, //136 0x88
  {KEY_ERROR_UNDEFINED, false}, //137 0x89
  {KEY_ERROR_UNDEFINED, false}, //138 0x8A
  {KEY_ERROR_UNDEFINED, false}, //139 0x8B
  {KEY_ERROR_UNDEFINED, false}, //140 0x8C
  {KEY_TILDE, false}, //141 0x8D
  {KEY_ERROR_UNDEFINED, false}, //142 0x8E
  {KEY_ERROR_UNDEFINED, false}, //143 0x8F
  {KEY_ERROR_UNDEFINED, false}, //144 0x90
  {KEY_CAPS_LOCK, false}, //145 0x91
  {KEY_F1, false}, //146 0x92
  {KEY_F2, false}, //147 0x93
  {KEY_F3, false}, //148 0x94
  {KEY_F4, false}, //149 0x95
  {KEY_F5, false}, //150 0x96
  {KEY_F6, false}, //151 0x97
  {KEY_F7, false}, //152 0x98
  {KEY_F8, false}, //153 0x99
  {KEY_F9, false}, //154 0x9A
  {KEY_F10, false}, //155 0x9B
  {KEY_F11, false}, //156 0x9C
  {KEY_F12, false}, //157 0x9D
  {KEY_PRINTSCREEN, false}, //158 0x9E
  {KEY_SCROLL_LOCK, false}, //159 0x9F
  {KEY_PAUSE, false}, //160 0xA0
  {KEY_INSERT, false}, //161 0xA1
  {KEY_HOME, false}, //162 0xA2
  {KEY_PAGE_UP, false}, //163 0xA3
  {KEY_DELETE, false}, //164 0xA4
  {KEY_END, false}, //165 0xA5
  {KEY_PAGE_DOWN, false}, //166 0xA6
  {KEY_RIGHT_ARROW, false}, //167 0xA7
  {KEY_LEFT_ARROW, false}, //168 0xA8
  {KEY_DOWN_ARROW, false}, //169 0xA9
  {KEY_UP_ARROW, false}, //170 0xAA
  {KEY_ERROR_UNDEFINED, false}, //171 0xAB
  {KEY_ERROR_UNDEFINED, false}, //172 0xAC
  {KEY_ERROR_UNDEFINED, false}, //173 0xAD
  {KEY_ERROR_UNDEFINED, false}, //174 0xAE
  {KEY_ERROR_UNDEFINED, false}, //175 0xAF
  {KEY_ERROR_UNDEFINED, false}, //176 0xB0
  {KEY_ERROR_UNDEFINED, false}, //177 0xB1
  {KEY_ERROR_UNDEFINED, false}, //178 0xB2
  {KEY_ERROR_UNDEFINED, false}, //179 0xB3
  {KEY_ERROR_UNDEFINED, false}, //180 0xB4
  {KEY_ERROR_UNDEFINED, false}, //181 0xB5
  {KEY_ERROR_UNDEFINED, false}, //182 0xB6
  {KEY_ERROR_UNDEFINED, false}, //183 0xB7
  {KEY_ERROR_UNDEFINED, false}, //184 0xB8
  {KEY_ERROR_UNDEFINED, false}, //185 0xB9
  {KEY_ERROR_UNDEFINED, false}, //186 0xBA
  {KEY_ERROR_UNDEFINED, false}, //187 0xBB
  {KEY_ERROR_UNDEFINED, false}, //188 0xBC
  {KEY_ERROR_UNDEFINED, false}, //189 0xBD
  {KEY_ERROR_UNDEFINED, false}, //190 0xBE
  {KEY_ERROR_UNDEFINED, false}, //191 0xBF
  {KEY_ERROR_UNDEFINED, false}, //192 0xC0
  {KEY_ERROR_UNDEFINED, false}, //193 0xC1
  {KEY_ERROR_UNDEFINED, false}, //194 0xC2
  {KEY_ERROR_UNDEFINED, false}, //195 0xC3
  {KEY_ERROR_UNDEFINED, false}, //196 0xC4
  {KEY_ERROR_UNDEFINED, false}, //197 0xC5
  {KEY_ERROR_UNDEFINED, false}, //198 0xC6
  {KEY_ERROR_UNDEFINED, false}, //199 0xC7
  {KEY_ERROR_UNDEFINED, false}, //200 0xC8
  {KEY_ERROR_UNDEFINED, false}, //201 0xC9
  {KEY_ERROR_UNDEFINED, false}, //202 0xCA
  {KEY_ERROR_UNDEFINED, false}, //203 0xCB
  {KEY_ERROR_UNDEFINED, false}, //204 0xCC
  {KEY_ERROR_UNDEFINED, false}, //205 0xCD
  {KEY_ERROR_UNDEFINED, false}, //206 0xCE
  {KEY_ERROR_UNDEFINED, false}, //207 0xCF
  {KEY_ERROR_UNDEFINED, false}, //208 0xD0
  {KEY_ERROR_UNDEFINED, false}, //209 0xD1
  {KEY_ERROR_UNDEFINED, false}, //210 0xD2
  {KEY_ERROR_UNDEFINED, false}, //211 0xD3
  {KEY_ERROR_UNDEFINED, false}, //212 0xD4
  {KEY_ERROR_UNDEFINED, false}, //213 0xD5
  {KEY_ERROR_UNDEFINED, false}, //214 0xD6
  {KEY_ERROR_UNDEFINED, false}, //215 0xD7
  {KEY_ERROR_UNDEFINED, false}, //216 0xD8
  {KEY_ERROR_UNDEFINED, false}, //217 0xD9
  {KEY_ERROR_UNDEFINED, false}, //218 0xDA
  {KEY_ERROR_UNDEFINED, false}, //219 0xDB
  {KEY_ERROR_UNDEFINED, false}, //220 0xDC
  {KEY_ERROR_UNDEFINED, false}, //221 0xDD
  {KEY_ERROR_UNDEFINED, false}, //222 0xDE
  {KEY_ERROR_UNDEFINED, false}, //223 0xDF
  {KEY_INTERNATIONAL2, false}, //224 0xE0
  {KEY_ERROR_UNDEFINED, false}, //225 0xE1
  {KEY_INTERNATIONAL4, false}, //226 0xE2
  {KEY_INTERNATIONAL5, false}, //227 0xE3
  {KEY_ERROR_UNDEFINED, false}, //228 0xE4
  {KEY_ERROR_UNDEFINED, false}, //229 0xE5
  {KEY_ERROR_UNDEFINED, false}, //230 0xE6
  {KEY_ERROR_UNDEFINED, false}, //231 0xE7
  {KEY_ERROR_UNDEFINED, false}, //232 0xE8
  {KEY_ERROR_UNDEFINED, false}, //233 0xE9
  {KEY_ERROR_UNDEFINED, false}, //234 0xEA
  {KEY_ERROR_UNDEFINED, false}, //235 0xEB
  {KEY_ERROR_UNDEFINED, false}, //236 0xEC
  {KEY_ERROR_UNDEFINED, false}, //237 0xED
  {KEY_ERROR_UNDEFINED, false}, //238 0xEE
  {KEY_ERROR_UNDEFINED, false}, //239 0xEF
  {KEY_ERROR_UNDEFINED, false}, //240 0xF0
  {KEY_ERROR_UNDEFINED, false}, //241 0xF1
  {KEY_ERROR_UNDEFINED, false}, //242 0xF2
  {KEY_ERROR_UNDEFINED, false}, //243 0xF3
  {KEY_ERROR_UNDEFINED, false}, //244 0xF4
  {KEY_ERROR_UNDEFINED, false}, //245 0xF5
  {KEY_ERROR_UNDEFINED, false}, //246 0xF6
  {KEY_ERROR_UNDEFINED, false}, //247 0xF7
  {KEY_LEFT_CTRL, false}, //248 0xF8
  {KEY_LEFT_SHIFT, false}, //249 0xF9
  {KEY_LEFT_ALT, false}, //250 0xFA
  {KEY_LEFT_GUI, false}, //251 0xFB
  {KEY_RIGHT_CTRL, false}, //252 0xFC
  {KEY_RIGHT_SHIFT, false}, //253 0xFD
  {KEY_RIGHT_ALT, false}, //254 0xFE
  {KEY_RIGHT_GUI, false}, //255 0xFF
};


void setup() {
  pinMode(PIN_BTN, INPUT_PULLUP);
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  Serial1.begin(115200);

  delay(1000);
  serialMode = (digitalRead(PIN_BTN) == LOW); // モードチェック

  if (serialMode) {
    digitalWrite(LED_BUILTIN, LOW);
    delay(1000);
    Serial.println("Serial Mode Ver.");
    Serial.println(VERSION);
  } else {
    digitalWrite(LED_BUILTIN, HIGH);
    Keyboard.begin();
  }
}

void taskSerialMode() {
  int len = Serial1.available();
  for (int count = 0; count < len; count++) {
    int inByte = Serial1.read();
    Serial.printf("%d %d %d\n", inByte, KEYMap[inByte].key, KEYMap[inByte].shift);
  }
}


byte isPushSimultaneouslyKey = 0;

void taskKeyMode() {
  int len = Serial1.available();
  for (int count = 0; count < len; count++) {
    int inByte = Serial1.read();
#ifdef DEBUG_MODE
    Serial.printf("%c %02X %02X %d %02X\n", (char)inByte, inByte, KEYMap[inByte].key, KEYMap[inByte].shift, isPushSimultaneouslyKey);
#endif // DEBUG_MODE
    if (inByte >= 0xF8) {
      isPushSimultaneouslyKey = isPushSimultaneouslyKey | (0x01 << (0xFF - inByte));
#ifdef DEBUG_MODE
    Serial.printf("ket %02X\n",isPushSimultaneouslyKey);
#endif 
    } else if (KEYMap[inByte].key == KEY_ERROR_UNDEFINED) {
      isPushSimultaneouslyKey = 0;
    } else {
#ifdef DEBUG_MODE
    Serial.println("--key--");
#endif 
      for (int count = 0; count < 8; count++) {
        if ((isPushSimultaneouslyKey >> (7 - count) & 0x01) != 0) {
          Keyboard.press(KEYMap[0xF8 + count].key);
#ifdef DEBUG_MODE
          Serial.printf("[%02X]->%02X\n", 0xF8 + count, KEYMap[0xF8 + count].key);
#endif // DEBUG_MODE
        }
      }
      if (KEYMap[inByte].shift) {
#ifdef DEBUG_MODE
        Serial.println("LEFT_SHIFT");
#endif // DEBUG_MODE
        Keyboard.press(KEY_LEFT_SHIFT);
      }
      Keyboard.press(KEYMap[inByte].key);
#ifdef DEBUG_MODE
          Serial.printf("[%02X]->%02X\n--key--\n", inByte, KEYMap[inByte].key);
#endif // DEBUG_MODE
      Keyboard.releaseAll();
      isPushSimultaneouslyKey = 0;
    }
  }
}

void loop() {
  if (serialMode) {
    taskSerialMode();
  } else {
    taskKeyMode();
  }
}
