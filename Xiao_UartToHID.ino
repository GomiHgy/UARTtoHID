/*
  Seeedino XIAOを使ってUART入力をHIDキー出力にする
*/

#include "HID-Project.h"

#define VERSION "0.3"
#define PIN_BTN   5
#define KEYMAP_LEN 256
//#define DEBUG_MODE

#define KEY_SHIFT   0x0100

// TODO:リングバッファ及びRTSピン対応(LOWで送信可能)

bool serialMode = false; // シリアルモード(シリアル出力モードの有無)

struct Key {
  KeyboardKeycode key;
  bool shift;
};

// https://mgt.blog.ss-blog.jp/2016-01-14
// キーコード＋SHIFTキーの有無(有=1)
static const uint16_t KEYMap[KEYMAP_LEN] =
{
  KEY_ERROR_UNDEFINED, //0  0x00  NUL(null文字)
  KEY_ERROR_UNDEFINED, //1 0x01  SOH(ヘッダ開始)
  KEY_ERROR_UNDEFINED, //2 0x02  STX(テキスト開始)
  KEY_ERROR_UNDEFINED, //3 0x03  ETX(テキスト終了)
  KEY_ERROR_UNDEFINED, //4 0x04  EOT(転送終了)
  KEY_ERROR_UNDEFINED, //5 0x05  ENQ(照会)
  KEY_ERROR_UNDEFINED, //6 0x06  ACK(受信確認)
  KEY_ERROR_UNDEFINED, //7 0x07  BEL(警告)
  KEY_BACKSPACE, //8 0x08  BS(後退)
  KEY_TAB, //9 0x09  HT(水平タブ)
  KEY_ENTER, //10  0x0a  LF(改行)
  KEY_ERROR_UNDEFINED, //11  0x0b  VT(垂直タブ)
  KEY_ERROR_UNDEFINED, //12  0x0c  FF(改頁)
  KEY_ERROR_UNDEFINED, //13  0x0d  CR(復帰)
  KEY_ERROR_UNDEFINED, //14  0x0e  SO(シフトアウト)
  KEY_ERROR_UNDEFINED, //15  0x0f  SI(シフトイン)
  KEY_ERROR_UNDEFINED, //16  0x10  DLE(データリンクエスケープ)
  KEY_ERROR_UNDEFINED, //17  0x11  DC1(装置制御1)
  KEY_ERROR_UNDEFINED, //18  0x12  DC2(装置制御2)
  KEY_ERROR_UNDEFINED, //19  0x13  DC3(装置制御3)
  KEY_ERROR_UNDEFINED, //20  0x14  DC4(装置制御4)
  KEY_ERROR_UNDEFINED, //21  0x15  NAK(受信失敗)
  KEY_ERROR_UNDEFINED, //22  0x16  SYN(同期)
  KEY_ERROR_UNDEFINED, //23  0x17  ETB(転送ブロック終了)
  KEY_ERROR_UNDEFINED, //24  0x18  CAN(キャンセル)
  KEY_ERROR_UNDEFINED, //25  0x19  EM(メディア終了)
  KEY_ERROR_UNDEFINED, //26  0x1a  SUB(置換)
  KEY_ESC, //27  0x1b  ESC(エスケープ)
  KEY_ERROR_UNDEFINED, //28  0x1c  FS(フォーム区切り)
  KEY_ERROR_UNDEFINED, //29  0x1d  GS(グループ区切り)
  KEY_ERROR_UNDEFINED, //30  0x1e  RS(レコード区切り)
  KEY_ERROR_UNDEFINED, //31  0x1f  US(ユニット区切り)
  KEY_SPACE, //32  0x20  SPC(空白文字)
  KEY_1|KEY_SHIFT, //33  0x21  !
  KEY_2|KEY_SHIFT, //34  0x22  "
  KEY_3|KEY_SHIFT, //35  0x23  #
  KEY_4|KEY_SHIFT, //36  0x24  $
  KEY_5|KEY_SHIFT, //37  0x25  %
  KEY_6|KEY_SHIFT, //38  0x26  &
  KEY_7|KEY_SHIFT, //39  0x27  '
  KEY_8|KEY_SHIFT, //40  0x28  (
  KEY_9|KEY_SHIFT, //41  0x29  )
  KEY_QUOTE|KEY_SHIFT, //42  0x2a  *
  KEY_SEMICOLON|KEY_SHIFT, //43  0x2b  +
  KEY_COMMA, //44  0x2c  ,
  KEY_MINUS, //45  0x2d  -
  KEY_PERIOD, //46  0x2e  .
  KEY_SLASH, //47  0x2f  /
  KEY_0, //48  0x30  0
  KEY_1, //49  0x31  1
  KEY_2, //50  0x32  2
  KEY_3, //51  0x33  3
  KEY_4, //52  0x34  4
  KEY_5, //53  0x35  5
  KEY_6, //54  0x36  6
  KEY_7, //55  0x37  7
  KEY_8, //56  0x38  8
  KEY_9, //57  0x39  9
  KEY_QUOTE, //58  0x3a  :
  KEY_SEMICOLON, //59  0x3b  ;
  KEY_COMMA|KEY_SHIFT, //60  0x3c  <
  KEY_MINUS|KEY_SHIFT, //61  0x3d  =
  KEY_PERIOD|KEY_SHIFT, //62  0x3e  >
  KEY_SLASH|KEY_SHIFT, //63  0x3f  ?
  KEY_LEFT_BRACE, //64  0x40  @
  KEY_A|KEY_SHIFT, //65  0x41  A
  KEY_B|KEY_SHIFT, //66  0x42  B
  KEY_C|KEY_SHIFT, //67  0x43  C
  KEY_D|KEY_SHIFT, //68  0x44  D
  KEY_E|KEY_SHIFT, //69  0x45  E
  KEY_F|KEY_SHIFT, //70  0x46  F
  KEY_G|KEY_SHIFT, //71  0x47  G
  KEY_H|KEY_SHIFT, //72  0x48  H
  KEY_I|KEY_SHIFT, //73  0x49  I
  KEY_J|KEY_SHIFT, //74  0x4a  J
  KEY_K|KEY_SHIFT, //75  0x4b  K
  KEY_L|KEY_SHIFT, //76  0x4c  L
  KEY_M|KEY_SHIFT, //77  0x4d  M
  KEY_N|KEY_SHIFT, //78  0x4e  N
  KEY_O|KEY_SHIFT, //79  0x4f  O
  KEY_P|KEY_SHIFT, //80  0x50  P
  KEY_Q|KEY_SHIFT, //81  0x51  Q
  KEY_R|KEY_SHIFT, //82  0x52  R
  KEY_S|KEY_SHIFT, //83  0x53  S
  KEY_T|KEY_SHIFT, //84  0x54  T
  KEY_U|KEY_SHIFT, //85  0x55  U
  KEY_V|KEY_SHIFT, //86  0x56  V
  KEY_W|KEY_SHIFT, //87  0x57  W
  KEY_X|KEY_SHIFT, //88  0x58  X
  KEY_Y|KEY_SHIFT, //89  0x59  Y
  KEY_Z|KEY_SHIFT, //90  0x5a  Z
  KEY_RIGHT_BRACE, //91  0x5b [
  HID_KEYBOARD_INTERNATIONAL3, //92  0x5c  バックスラッシュ
  KEY_BACKSLASH, //93  0x5d  ]
  KEY_EQUAL, //94  0x5e  ^
  HID_KEYBOARD_INTERNATIONAL1|KEY_SHIFT, //95  0x5f  _
  KEY_LEFT_BRACE|KEY_SHIFT, //96  0x60  `
  KEY_A, //97  0x61  a
  KEY_B, //98  0x62  b
  KEY_C, //99  0x63  c
  KEY_D, //100 0x64  d
  KEY_E, //101 0x65  e
  KEY_F, //102 0x66  f
  KEY_G, //103 0x67  g
  KEY_H, //104 0x68  h
  KEY_I, //105 0x69  i
  KEY_J, //106 0x6a  j
  KEY_K, //107 0x6b  k
  KEY_L, //108 0x6c  l
  KEY_M, //109 0x6d  m
  KEY_N, //110 0x6e  n
  KEY_O, //111 0x6f  o
  KEY_P, //112 0x70  p
  KEY_Q, //113 0x71  q
  KEY_R, //114 0x72  r
  KEY_S, //115 0x73  s
  KEY_T, //116 0x74  t
  KEY_U, //117 0x75  u
  KEY_V, //118 0x76  v
  KEY_W, //119 0x77  w
  KEY_X, //120 0x78  x
  KEY_Y, //121 0x79  y
  KEY_Z, //122 0x7a  z
  KEY_RIGHT_BRACE|KEY_SHIFT, //123 0x7b  {
  HID_KEYBOARD_INTERNATIONAL3|KEY_SHIFT, //124 0x7c  |
  KEY_BACKSLASH|KEY_SHIFT, //125 0x7d  }
  KEY_EQUAL|KEY_SHIFT, //126 0x7e  ~
  KEY_DELETE, //127 0x7f  DEL(削除)
  KEY_ERROR_UNDEFINED, //128 0x80
  KEY_ERROR_UNDEFINED, //129 0x81
  KEY_ERROR_UNDEFINED, //130 0x82
  KEY_ERROR_UNDEFINED, //131 0x83
  KEY_ERROR_UNDEFINED, //132 0x84
  KEY_ERROR_UNDEFINED, //133 0x85
  KEY_ERROR_UNDEFINED, //134 0x86
  KEY_ERROR_UNDEFINED, //135 0x87
  KEY_ERROR_UNDEFINED, //136 0x88
  KEY_ERROR_UNDEFINED, //137 0x89
  KEY_ERROR_UNDEFINED, //138 0x8A
  KEY_ERROR_UNDEFINED, //139 0x8B
  KEY_ERROR_UNDEFINED, //140 0x8C
  KEY_TILDE, //141 0x8D
  KEY_ERROR_UNDEFINED, //142 0x8E
  KEY_ERROR_UNDEFINED, //143 0x8F
  KEY_ERROR_UNDEFINED, //144 0x90
  KEY_CAPS_LOCK, //145 0x91
  KEY_F1, //146 0x92
  KEY_F2, //147 0x93
  KEY_F3, //148 0x94
  KEY_F4, //149 0x95
  KEY_F5, //150 0x96
  KEY_F6, //151 0x97
  KEY_F7, //152 0x98
  KEY_F8, //153 0x99
  KEY_F9, //154 0x9A
  KEY_F10, //155 0x9B
  KEY_F11, //156 0x9C
  KEY_F12, //157 0x9D
  KEY_PRINTSCREEN, //158 0x9E
  KEY_SCROLL_LOCK, //159 0x9F
  KEY_PAUSE, //160 0xA0
  KEY_INSERT, //161 0xA1
  KEY_HOME, //162 0xA2
  KEY_PAGE_UP, //163 0xA3
  KEY_DELETE, //164 0xA4
  KEY_END, //165 0xA5
  KEY_PAGE_DOWN, //166 0xA6
  KEY_RIGHT_ARROW, //167 0xA7
  KEY_LEFT_ARROW, //168 0xA8
  KEY_DOWN_ARROW, //169 0xA9
  KEY_UP_ARROW, //170 0xAA
  KEY_ERROR_UNDEFINED, //171 0xAB
  KEY_ERROR_UNDEFINED, //172 0xAC
  KEY_ERROR_UNDEFINED, //173 0xAD
  KEY_ERROR_UNDEFINED, //174 0xAE
  KEY_ERROR_UNDEFINED, //175 0xAF
  KEY_ERROR_UNDEFINED, //176 0xB0
  KEY_ERROR_UNDEFINED, //177 0xB1
  KEY_ERROR_UNDEFINED, //178 0xB2
  KEY_ERROR_UNDEFINED, //179 0xB3
  KEY_ERROR_UNDEFINED, //180 0xB4
  KEY_ERROR_UNDEFINED, //181 0xB5
  KEY_ERROR_UNDEFINED, //182 0xB6
  KEY_ERROR_UNDEFINED, //183 0xB7
  KEY_ERROR_UNDEFINED, //184 0xB8
  KEY_ERROR_UNDEFINED, //185 0xB9
  KEY_ERROR_UNDEFINED, //186 0xBA
  KEY_ERROR_UNDEFINED, //187 0xBB
  KEY_ERROR_UNDEFINED, //188 0xBC
  KEY_ERROR_UNDEFINED, //189 0xBD
  KEY_ERROR_UNDEFINED, //190 0xBE
  KEY_ERROR_UNDEFINED, //191 0xBF
  KEY_ERROR_UNDEFINED, //192 0xC0
  KEY_ERROR_UNDEFINED, //193 0xC1
  KEY_ERROR_UNDEFINED, //194 0xC2
  KEY_ERROR_UNDEFINED, //195 0xC3
  KEY_ERROR_UNDEFINED, //196 0xC4
  KEY_ERROR_UNDEFINED, //197 0xC5
  KEY_ERROR_UNDEFINED, //198 0xC6
  KEY_ERROR_UNDEFINED, //199 0xC7
  KEY_ERROR_UNDEFINED, //200 0xC8
  KEY_ERROR_UNDEFINED, //201 0xC9
  KEY_ERROR_UNDEFINED, //202 0xCA
  KEY_ERROR_UNDEFINED, //203 0xCB
  KEY_ERROR_UNDEFINED, //204 0xCC
  KEY_ERROR_UNDEFINED, //205 0xCD
  KEY_ERROR_UNDEFINED, //206 0xCE
  KEY_ERROR_UNDEFINED, //207 0xCF
  KEY_ERROR_UNDEFINED, //208 0xD0
  KEY_ERROR_UNDEFINED, //209 0xD1
  KEY_ERROR_UNDEFINED, //210 0xD2
  KEY_ERROR_UNDEFINED, //211 0xD3
  KEY_ERROR_UNDEFINED, //212 0xD4
  KEY_ERROR_UNDEFINED, //213 0xD5
  KEY_ERROR_UNDEFINED, //214 0xD6
  KEY_ERROR_UNDEFINED, //215 0xD7
  KEY_ERROR_UNDEFINED, //216 0xD8
  KEY_ERROR_UNDEFINED, //217 0xD9
  KEY_ERROR_UNDEFINED, //218 0xDA
  KEY_ERROR_UNDEFINED, //219 0xDB
  KEY_ERROR_UNDEFINED, //220 0xDC
  KEY_ERROR_UNDEFINED, //221 0xDD
  KEY_ERROR_UNDEFINED, //222 0xDE
  KEY_ERROR_UNDEFINED, //223 0xDF
  KEY_INTERNATIONAL2, //224 0xE0
  KEY_ERROR_UNDEFINED, //225 0xE1
  KEY_INTERNATIONAL4, //226 0xE2
  KEY_INTERNATIONAL5, //227 0xE3
  KEY_ERROR_UNDEFINED, //228 0xE4
  KEY_ERROR_UNDEFINED, //229 0xE5
  KEY_ERROR_UNDEFINED, //230 0xE6
  KEY_ERROR_UNDEFINED, //231 0xE7
  KEY_ERROR_UNDEFINED, //232 0xE8
  KEY_ERROR_UNDEFINED, //233 0xE9
  KEY_ERROR_UNDEFINED, //234 0xEA
  KEY_ERROR_UNDEFINED, //235 0xEB
  KEY_ERROR_UNDEFINED, //236 0xEC
  KEY_ERROR_UNDEFINED, //237 0xED
  KEY_ERROR_UNDEFINED, //238 0xEE
  KEY_ERROR_UNDEFINED, //239 0xEF
  KEY_ERROR_UNDEFINED, //240 0xF0
  KEY_ERROR_UNDEFINED, //241 0xF1
  KEY_ERROR_UNDEFINED, //242 0xF2
  KEY_ERROR_UNDEFINED, //243 0xF3
  KEY_ERROR_UNDEFINED, //244 0xF4
  KEY_ERROR_UNDEFINED, //245 0xF5
  KEY_ERROR_UNDEFINED, //246 0xF6
  KEY_ERROR_UNDEFINED, //247 0xF7
  KEY_LEFT_CTRL, //248 0xF8
  KEY_LEFT_SHIFT, //249 0xF9
  KEY_LEFT_ALT, //250 0xFA
  KEY_LEFT_GUI, //251 0xFBaL5
  KEY_RIGHT_CTRL, //252 0xFC
  KEY_RIGHT_SHIFT, //253 0xFD
  KEY_RIGHT_ALT, //254 0xFE
  KEY_RIGHT_GUI, //255 0xFF
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
    Serial.printf("%d %d %d\n", inByte, KEYMap[inByte] & 0xFF, (KEYMap[inByte] & KEY_SHIFT) != 0);
  }
}


byte isPushSimultaneouslyKey = 0;

void taskKeyMode() {
  int len = Serial1.available();
  for (int count = 0; count < len; count++) {
    int inByte = Serial1.read();
#ifdef DEBUG_MODE
    Serial.printf("%c %02X %02X %d %02X\n", (char)inByte, inByte, KEYMap[inByte] & 0xFF, (KEYMap[inByte] & KEY_SHIFT) != 0, isPushSimultaneouslyKey);
#endif // DEBUG_MODE
    if (inByte >= 0xF8) {
      isPushSimultaneouslyKey = isPushSimultaneouslyKey | (0x01 << (0xFF - inByte));
#ifdef DEBUG_MODE
    Serial.printf("ket %02X\n",isPushSimultaneouslyKey);
#endif 
    } else if ((KeyboardKeycode)(KEYMap[inByte] & 0x00FF) == KEY_ERROR_UNDEFINED) {
      isPushSimultaneouslyKey = 0;
    } else {
#ifdef DEBUG_MODE
    Serial.println("--key--");
#endif 
      for (int count = 0; count < 8; count++) {
        if ((isPushSimultaneouslyKey >> (7 - count) & 0x01) != 0) {
          Keyboard.press((KeyboardKeycode)(KEYMap[0xF8 + count] & 0x00FF));
#ifdef DEBUG_MODE
          Serial.printf("[%02X]->%02X\n", 0xF8 + count, KEYMap[0xF8 + count] & 0xFF);
#endif // DEBUG_MODE
        }
      }
      if ((KEYMap[inByte] & KEY_SHIFT) != 0) {
#ifdef DEBUG_MODE
        Serial.println("LEFT_SHIFT");
#endif // DEBUG_MODE
        Keyboard.press(KEY_LEFT_SHIFT);
      }
      Keyboard.press((KeyboardKeycode)(KEYMap[inByte] & 0x00FF));
#ifdef DEBUG_MODE
          Serial.printf("[%02X]->%02X\n--key--\n", inByte, KEYMap[inByte] & 0xFF);
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
