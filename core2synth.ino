#include <M5Core2.h>
#include <driver/i2s.h>
#include <Trill.h>
#include "VAEngine.h"
#include "WiFi.h"
#include "SD.h"
#include "Waveforms.h"
#define CONFIG_I2S_BCK_PIN 12
#define CONFIG_I2S_LRCK_PIN 0
#define CONFIG_I2S_DATA_PIN 2
#define CONFIG_I2S_DATA_IN_PIN 34

#define SPAKER_I2S_NUMBER I2S_NUM_0

#define MODE_MIC 0
#define MODE_SPK 1

#define WTLEN 128
#define WFCOUNT 128
#define SAMPLE_RATE 8000
float waveforms[WTLEN*WFCOUNT];
const int audio_buf_len = 512;
unsigned char audio_buf[audio_buf_len];
unsigned char audio_buf2[audio_buf_len];
VAEngine<2,WFCOUNT, WTLEN> *engine;
static QueueHandle_t i2sstateQueue = nullptr;
int baseNote=55;
int divisor = 80;
volatile bool written = false;

typedef struct
{
    uint8_t state;
    void* audioPtr;
    uint32_t audioSize;
}i2sQueueMsg_t;
int maxx=320;
int maxy=240;
Trill trillSquare;
Trill trillBar;
boolean touchActiveSquare = false;
boolean touchActiveBar = false;
int trill_x1, trill_x2,trill_y1,trill_y2;

TFT_eSprite Disbuff = TFT_eSprite(&M5.Lcd);
bool InitI2SSpakerOrMic(int mode)
{
    esp_err_t err = ESP_OK;

    i2s_driver_uninstall(SPAKER_I2S_NUMBER);
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER),
        .sample_rate = SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT, // is fixed at 12bit, stereo, MSB
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = I2S_COMM_FORMAT_I2S_MSB,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 4,
        .dma_buf_len = audio_buf_len,
    };
    if (mode == MODE_MIC)
    {
        i2s_config.mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_PDM);
    }
    else
    {
        i2s_config.mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX);
        i2s_config.use_apll = false;
        i2s_config.tx_desc_auto_clear = true;
    }

    //Serial.println("Init i2s_driver_install");

    err += i2s_driver_install(SPAKER_I2S_NUMBER, &i2s_config, 0, NULL);
    i2s_pin_config_t tx_pin_config;

    tx_pin_config.bck_io_num = CONFIG_I2S_BCK_PIN;
    tx_pin_config.ws_io_num = CONFIG_I2S_LRCK_PIN;
    tx_pin_config.data_out_num = CONFIG_I2S_DATA_PIN;
    tx_pin_config.data_in_num = CONFIG_I2S_DATA_IN_PIN;

    //Serial.println("Init i2s_set_pin");
    err += i2s_set_pin(SPAKER_I2S_NUMBER, &tx_pin_config);
    //Serial.println("Init i2s_set_clk");
    err += i2s_set_clk(SPAKER_I2S_NUMBER, SAMPLE_RATE, I2S_BITS_PER_SAMPLE_16BIT, I2S_CHANNEL_STEREO);

    return true;
}


void i2s_task(void *arg) {
  
    InitI2SSpakerOrMic(MODE_SPK);
    engine = new VAEngine<2,WFCOUNT, WTLEN>(waveforms);
    engine->init(SAMPLE_RATE);
    engine->setADSR(10,10,127,10);
    while(1)
    {
      
      size_t bytes_written = 0;
      engine->fillAudioBuffer(audio_buf,audio_buf_len);
      //memcpy(audio_buf2,audio_buf,audio_buf_len);
      i2s_write(SPAKER_I2S_NUMBER, audio_buf, audio_buf_len, &bytes_written, portMAX_DELAY);
      vTaskDelay(1);
    
    }
}

void createWaveforms()
{
  initWaveforms(waveforms,WTLEN,WFCOUNT,true,0.5f,0.6f);
}

bool sent = false;
void setup() {
  // put your setup code here, to run once:
  M5.begin(true, true, true, false);
  Disbuff.createSprite(320, 240);
  
    M5.Axp.SetBusPowerMode(0);
    M5.Axp.SetCHGCurrent(AXP192::kCHG_190mA);

    M5.Axp.SetSpkEnable(true);
    //i2sstateQueue = xQueueCreate(5, sizeof(i2sQueueMsg_t));
    createWaveforms();
    xTaskCreatePinnedToCore(i2s_task, "i2s_task", 16384, NULL, 3, NULL, 0);
    M5.Axp.SetLed(0);
    int ret = trillSquare.setup(Trill::TRILL_SQUARE);
    ret = trillBar.setup(Trill::TRILL_BAR);

}
bool firsttime = true;
int currnote = -1;
int currwave = 0;
int c = 0;
void loop() {
  // put your main code here, to run repeatedly:
  trillSquare.read();
    trillBar.read();
    if(trillSquare.getNumTouches() > 0) {
    for(int i = 0; i < trillSquare.getNumTouches(); i++) {
        trill_y1=trillSquare.touchLocation(i);
    }
    for(int i = 0; i < trillSquare.getNumHorizontalTouches(); i++) {
        trill_x1=trillSquare.touchHorizontalLocation(i);
    }
    if(!touchActiveSquare)
    {
      currnote  =((trill_x1/divisor)+baseNote)%127;
      currwave = trill_y1%WFCOUNT;
      engine->handleNoteOn(1, currnote, 60127);
      int wave1 = currwave;
      if( wave1<0)
        wave1 = 0;
      if(wave1>WFCOUNT-1)
      {
        wave1 = WFCOUNT-1;
      }
      if(wave1<0)
      {
        wave1=0;
      }
      //engine->handleSelectWaveform(1,0,currnote,wave1);
      //engine->handleSelectWaveform(1,1,currnote,127-wave1);
      
    }
    else
    {
      int newnote = ((trill_x1/divisor)+baseNote)%127;
      currwave = trill_y1%WFCOUNT;
      int wave1 = currwave;
      if( wave1<0)
        wave1 = 0;
      if(wave1>WFCOUNT-1)
      {
        wave1 = WFCOUNT-1;
      }
      if(wave1<0)
      {
        wave1=0;
      }
      //engine->handleSelectWaveform(1,0,currnote,wave1);
      //engine->handleSelectWaveform(1,1,currnote,127-wave1);
      engine->handleNoteOff(1,currnote,0);
      engine->handleNoteOn(1, newnote, 127);
      //engine->handleNoteTransition(1,currnote, newnote,127);
      currnote = newnote;
      
    }
    touchActiveSquare = true;
  }
  else if(touchActiveSquare) {
    if(currnote>0)
    {
      engine->handleNoteOff(1,currnote,0);
    }
    touchActiveSquare = false;
  }
    if(trillBar.getNumTouches() > 0) {

    for(int i = 0; i < trillBar.getNumTouches(); i++) {
        
        trill_x2 = trillBar.touchLocation(i);
        trill_y2 = 100;
    }
    touchActiveBar = true;
  }
  else if(touchActiveBar) {
    // Print a single line when touch goes off
    touchActiveBar = false;
  }
    
    uint16_t color16 = Disbuff.color565(0xff,0x9c,0);
    Disbuff.fillRect(0,0,320,240,0);
    int x1 = (trill_x1/1800.0)*maxx;
    int y1 = (trill_y1/1800.0)*maxy;
    int x2 = (trill_x2/3200.0)*maxx;
    int y2 = maxy/2;
    if(touchActiveSquare)
    {
      Disbuff.fillRect(x1,maxy-y1,10,10,color16);
    }
    if(touchActiveBar)
    {
      Disbuff.fillRect(maxx-x2,y2,10,10,color16);
    }
    Disbuff.pushSprite(0,0);


}
