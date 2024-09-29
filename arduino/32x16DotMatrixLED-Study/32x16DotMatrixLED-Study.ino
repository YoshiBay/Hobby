/*
 32x16DotMatrixLED-Study
*/
// 1画面分(32bitX16line)の書き込みデータをProgram(flash)領域に格納する
// [0]左画面(D31-D16, [1]右画面(D15-D0)
const PROGMEM unsigned int font_tbl_r[2][16] =
{
  {0x0000, 0x2340, 0x2844, 0x3bfc, 0xa844, 0xa844, 0xa044, 0xa7fe,
     0xa040, 0xa040, 0x20a0, 0x20a0, 0x2110, 0x2208, 0x2406, 0x0000},
  {0x0000, 0x6044, 0x37fc, 0x0040, 0x07fc, 0x0444, 0xe444, 0x27fc,
     0x2140, 0x2260, 0x2450, 0x2848, 0x2044, 0x5000, 0xbffe, 0x0000}
};
const PROGMEM unsigned int font_tbl_g[2][16] =
{
  {0x0000, 0x2340, 0x2844, 0x3bfc, 0xa844, 0xa844, 0xa044, 0xa7fe,
     0xa040, 0xa040, 0x20a0, 0x20a0, 0x2110, 0x2208, 0x2406, 0x0000},
  {0x0000, 0x6044, 0x37fc, 0x0040, 0x07fc, 0x0444, 0xe444, 0x27fc,
     0x2140, 0x2260, 0x2450, 0x2848, 0x2044, 0x5000, 0xbffe, 0x0000}
};

// 表示用バッファ
unsigned int disp_buf_r[2][16];
unsigned int disp_buf_g[2][16];

// for debug
char debug_buf[100];

// I/O port configuration
#define _AD0 7
#define _AD1 6
#define _AD2 5
#define _AD3 4
#define _A_BB 3
#define _SEin 2
#define _ALE 12
#define _WE 10
#define _CLK 9
#define _DR 11
#define _DG 8

#define RAMA 0
#define RAMB 1
void select_ram(int iram){
  if ( iram == RAMA ) digitalWrite(_A_BB, HIGH);
  else                digitalWrite(_A_BB, LOW);
}

#define NUM_PANEL 1 // パネルの枚数(1枚)

void addr_set(int addr){
  if ( false ) { // debug
    Serial.println("addr_set");
  }
  if ( (addr & 0x8)  != 0 ) digitalWrite(_AD3, HIGH);
  else                    digitalWrite(_AD3, LOW);
  if ( (addr & 0x4)  != 0 ) digitalWrite(_AD2, HIGH);
  else                    digitalWrite(_AD2, LOW);
  if ( (addr & 0x2)  != 0 ) digitalWrite(_AD1, HIGH);
  else                    digitalWrite(_AD1, LOW);
  if ( (addr & 0x1)  != 0 ) digitalWrite(_AD0, HIGH);
  else                    digitalWrite(_AD0, LOW);
  if ( false ) { // debug
    word temp = digitalRead(_AD3);
    temp = ((temp << 1 ) & 0xE) | digitalRead(_AD2);
    temp = ((temp << 1 ) & 0xE) | digitalRead(_AD1);
    temp = ((temp << 1 ) & 0xE) | digitalRead(_AD0);
    sprintf(debug_buf,"%x, %d, %x", addr, temp);
    Serial.println(debug_buf);
  }
}

// データを１行分書き込む
void send_data(int iaddr_y, unsigned long *idata_r, unsigned long *idata_g){
  // iaddr_y: 書き込み行アドレス(Line番号)
  // *idata_r, *idata_g : データの配列先先頭要素アドレス
  unsigned long m, n;

  for ( int i = 0; i < NUM_PANEL ; i++ ){
    m = *idata_r;
    n = *idata_g;
    if ( true ) {  // debug
      sprintf(debug_buf,"%08lx, %08lx, %02d", m, n, iaddr_y);
      Serial.println(debug_buf);
    }
    // 32bit のデータを下位(addr=0)から順に送る addr=0が画面の左端
    for ( int j = 0; j < 32; j++ ){
      digitalWrite(_CLK, LOW);
      if ( (m & 0x80000000) !=0 ){
         digitalWrite(_DR, HIGH);
      } else {
         digitalWrite(_DR, LOW);
      }
      if ( (n & 0x80000000) !=0 ){
         digitalWrite(_DG, HIGH);
      } else {
         digitalWrite(_DG, LOW);
      }
      delay(10);
      m = m << 1;
      n = n << 1;
      digitalWrite(_CLK, HIGH);
      if (true) { // debug
        Serial.print(digitalRead(_DR));
      }
      delay(10);
    }
    if (true) { // debug
      Serial.println();
    }
    idata_r++; // 次の配列を指し示す
    idata_g++; // 次の配列を指し示す
  }
  addr_set(iaddr_y);
  digitalWrite(_ALE,HIGH);
  delay(10);
  digitalWrite(_WE, HIGH);
  delay(10);
  digitalWrite(_WE, LOW);
  delay(10);
  digitalWrite(_ALE, LOW);
  delay(10);
}
  
void setup() {
  Serial.begin(115200);
  
  // I/O configuration
  pinMode(_AD0, OUTPUT);
  pinMode(_AD1, OUTPUT);
  pinMode(_AD2, OUTPUT);
  pinMode(_AD3, OUTPUT);
  pinMode(_A_BB, OUTPUT);
  pinMode(_SEin, OUTPUT);
  pinMode(_ALE, OUTPUT);
  pinMode(_WE, OUTPUT);
  pinMode(_CLK, OUTPUT);
  pinMode(_DR, OUTPUT);
  pinMode(_DG, OUTPUT);

  // initialize
  digitalWrite(_AD0, LOW);
  digitalWrite(_AD1, LOW);
  digitalWrite(_AD2, LOW);
  digitalWrite(_AD3, LOW);
  digitalWrite(_A_BB, LOW);
  digitalWrite(_ALE, LOW);
  digitalWrite(_WE, LOW);
  digitalWrite(_CLK, LOW);
  digitalWrite(_DR, LOW);
  digitalWrite(_DG, LOW);

  // RAM選択
  digitalWrite(_SEin, LOW); // AutoMode

  // パネルのpowerOnReset待ち
  delay(100);

}

void sendFrame(){
  unsigned long m[NUM_PANEL], n[NUM_PANEL];
  for ( int i =0; i < 16; i++ ) {
    m[0]=disp_buf_r[1][i];
    m[0]=(m[0]<<16) + disp_buf_r[0][i];
    n[0]=disp_buf_g[1][i];
    n[0]=(m[0]<<16) + disp_buf_g[0][i];
    send_data(i, &m[0], &n[0]);
  }
}
void loop() {
  // Program領域にある１画面のdataをRAM上に転送する
  for ( int i = 0; i < 16; i++ ) {
    disp_buf_r[1][i]=pgm_read_word(&font_tbl_r[0][i]);
    disp_buf_g[1][i]=pgm_read_word(&font_tbl_g[0][i]);
    disp_buf_r[0][i]=pgm_read_word(&font_tbl_r[1][i]);
    disp_buf_g[0][i]=pgm_read_word(&font_tbl_g[1][i]);
  }
  // debug
  for ( int i = 0; i < 16; i++ ) {
    disp_buf_r[1][i]=0x0;
    disp_buf_g[1][i]=0;
    disp_buf_r[0][i]=0x0;
    disp_buf_g[0][i]=0;
  }
  disp_buf_r[1][0]=0xffff;
  disp_buf_r[0][0]=0xffff;
  sendFrame();
}
