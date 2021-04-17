#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>

// This should be in little endian format
static const u1_t PROGMEM APPEUI[8] = {  };
void os_getArtEui (u1_t* buf) {
  memcpy_P(buf, APPEUI, 8);
}

// This should also be in little endian format
static const u1_t PROGMEM DEVEUI[8] = {  };
void os_getDevEui (u1_t* buf) {
  memcpy_P(buf, DEVEUI, 8);
}

// This key should be in big endian format
static const u1_t PROGMEM APPKEY[16] = {  };
void os_getDevKey (u1_t* buf) {
  memcpy_P(buf, APPKEY, 16);
}

static uint8_t mydata[] = "Hello world";
static osjob_t sendjob;

// Schedule TX every this many seconds (might become longer due to duty
// cycle limitations).
const unsigned TX_INTERVAL = 60;

// Pin mapping
const lmic_pinmap lmic_pins = {
  .nss = 10,
  .rxtx = LMIC_UNUSED_PIN, // Not connected on RFM92/RFM95
  .rst = 9,  // Needed on RFM92/RFM95
  .dio = {2, 6, 7},
};

void onEvent (ev_t ev) {
  Serial.print(os_getTime());
  Serial.print(": ");
  switch (ev) {
    case EV_SCAN_TIMEOUT:
      Serial.println(F("EV_SCAN_TIMEOUT"));
      break;
    case EV_BEACON_FOUND:
      Serial.println(F("EV_BEACON_FOUND"));
      break;
    case EV_BEACON_MISSED:
      Serial.println(F("EV_BEACON_MISSED"));
      break;
    case EV_BEACON_TRACKED:
      Serial.println(F("EV_BEACON_TRACKED"));
      break;
    case EV_JOINING:
      Serial.println(F("EV_JOINING"));
      break;
    case EV_JOINED:
      Serial.println(F("EV_JOINED"));
      LMIC_setLinkCheckMode(0);
      break;
    case EV_RFU1:
      Serial.println(F("EV_RFU1"));
      break;
    case EV_JOIN_FAILED:
      Serial.println(F("EV_JOIN_FAILED"));
      break;
    case EV_REJOIN_FAILED:
      Serial.println(F("EV_REJOIN_FAILED"));
      break;
      break;
    case EV_TXCOMPLETE:
      Serial.println(F("EV_TXCOMPLETE (includes waiting for RX windows)"));
      if (LMIC.txrxFlags & TXRX_ACK)
        Serial.println(F("Received ack"));
      if (LMIC.dataLen) {
        Serial.println(F("Received "));
        Serial.println(LMIC.dataLen);
        Serial.println(F(" bytes of payload"));
      }
      // Schedule next transmission
      os_setTimedCallback(&sendjob, os_getTime() + sec2osticks(TX_INTERVAL), do_send);
      break;
    case EV_LOST_TSYNC:
      Serial.println(F("EV_LOST_TSYNC"));
      break;
    case EV_RESET:
      Serial.println(F("EV_RESET"));
      break;
    case EV_RXCOMPLETE:
      // data received in ping slot
      Serial.println(F("EV_RXCOMPLETE"));
      break;
    case EV_LINK_DEAD:
      Serial.println(F("EV_LINK_DEAD"));
      break;
    case EV_LINK_ALIVE:
      Serial.println(F("EV_LINK_ALIVE"));
      break;
    default:
      Serial.println(F("Unknown event"));
      break;
  }
}

void do_send(osjob_t* j) {
  // Check if there is not a current TX/RX job running
  if (LMIC.opmode & OP_TXRXPEND) {
    Serial.println(F("OP_TXRXPEND, not sending"));
  } else {
    // Prepare upstream data transmission at the next possible time.
    LMIC_setTxData2(1, mydata, sizeof(mydata) - 1, 0);
    Serial.println(F("Packet queued"));
    Serial.println(LMIC.freq);
  }
  // Next TX is scheduled after TX_COMPLETE event.
}

void setup() {
  Serial.begin(115200);
  Serial.println(F("Starting"));
  // LMIC init
  os_init();
  // Reset the MAC state. Session and pending data transfers will be discarded.
  LMIC_reset();
  LMIC_setClockError(MAX_CLOCK_ERROR * 1 / 100);
  LMIC_disableChannel(1);
  LMIC_disableChannel(2);
  printotaainformation();

  // Start job (sending automatically starts OTAA too)
  do_send(&sendjob);
}
void printotaainformation(void)
{
  unsigned char i;
  unsigned char chartemp;
  unsigned char messagelength;

  Serial.println(F("OTAA mode to join network"));
  Serial.print("DevEui: ");
  for (i = 0; i <= 7; i++)
  {
    chartemp = pgm_read_word_near(DEVEUI + 7 - i);
    convertandprint((chartemp >> 4) & 0xf);
    convertandprint(chartemp & 0xf);
  }
  Serial.println("");
  Serial.print("AppEui: ");
  for (i = 0; i <= 7; i++)
  {
    chartemp = pgm_read_word_near(APPEUI + 7 - i);
    convertandprint((chartemp >> 4) & 0xf);
    convertandprint(chartemp & 0xf);
  }

  Serial.println("");
  Serial.print("AppKey: ");
  for (i = 0; i <= 15; i++)
  {
    chartemp = pgm_read_word_near(APPKEY + i);
    convertandprint((chartemp >> 4) & 0xf);
    convertandprint(chartemp & 0xf);
  }
  Serial.println("");

  Serial.println("In this SW will send following information to network(uplink)");
  Serial.print((char*)mydata);
  Serial.println("");
  Serial.println("");
}

void convertandprint(unsigned char value)
{
  switch (value)
  {
    case 0  : Serial.print("0"); break;
    case 1  : Serial.print("1"); break;
    case 2  : Serial.print("2"); break;
    case 3  : Serial.print("3"); break;
    case 4  : Serial.print("4"); break;
    case 5  : Serial.print("5"); break;
    case 6  : Serial.print("6"); break;
    case 7  : Serial.print("7"); break;
    case 8  : Serial.print("8"); break;
    case 9  : Serial.print("9"); break;
    case 10  : Serial.print("A"); break;
    case 11  : Serial.print("B"); break;
    case 12  : Serial.print("C"); break;
    case 13  : Serial.print("D"); break;
    case 14  : Serial.print("E"); break;
    case 15 :  Serial.print("F"); break;
    default :
      Serial.print("?");   break;
  }
}

void loop() {
  os_runloop_once();
}
