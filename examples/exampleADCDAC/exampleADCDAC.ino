/*

  GeneralPurposeI2S_ADC_DAC demo by Bryce Cherry

  Commands:
  INIT (I2S/LEFT_JUSTIFIED/RIGHT_JUSTIFIED) field_bits frame_bits LeftRightPin ClockPin EnablePin DataInPin DataOutPin clock_speed(0-5) (SPI_AUTO_RESTART) (LOW_ENABLE)
  WRITE left_channel_value right_channel_value (SIGNEDIN_0/SIGNEDIN_1/SIGNEDIN_01/UNSIGNEDIN SIGNEDOUT0/SIGNEDOUT1/SIGNEDOUT01/UNSIGNEDIN) - write to the DAC and read from the ADC - if signing is required, both sign options must be used or ADC read/DAC write will be unsigned)
  USE_SPI_PINS (TRUE/FALSE) - use SPI mode or software mode

*/

#include <GeneralPurposeI2S_ADC_DAC.h>

GeneralPurposeI2S_ADC_DAC ADCDAC;

const byte DefaultAudioFormat = Audio_RightJustified;
const byte DefaultFieldBits = 16;
const byte DefaultFrameBits = 32;
const byte DefaultLeftRightPin = 9;
const byte DefaultClockPin = 13; // corresponds to SPI SCK on an Arduino Uno and derivatives
const byte DefaultSelectPin = 10; // corresponds to SPI SS on an Arduino Uno and derivatives
const byte DefaultDataInPin = 12; // corresponds to SPI MISO on an Arduino Uno and derivatives
const byte DefaultDataInSupported = true;
const byte DefaultDataOutPin = 11; // corresponds to SPI MOSI on an Arduino Uno and derivatives
const bool DefaultDataOutSupported = true;
const byte DefaultSpeed = 0;
const bool DefaultSPIautoRestart = false;
const bool DefaultLowEnable = false;
const bool DefaultNotOnSPIpins = false;
const bool DefaultChannel0inSigned = false;
const bool DefaultChannel1inSigned = false;
const bool DefaultChannel0outSigned = false;
const bool DefaultChannel1outSigned = false;

const int CommandSize = 100;
const byte FieldSize = 20;
char Command[CommandSize];

// ensures that the serial port is flushed fully on request
const unsigned long SerialPortRate = 9600;
const byte SerialPortRateTolerance = 5; // percent - increase to 50 for rates above 115200 up to 4000000
const byte SerialPortBits = 10; // start (1), data (8), stop (1)
const unsigned long TimePerByte = ((((1000000ULL * SerialPortBits) / SerialPortRate) * (100 + SerialPortRateTolerance)) / 100); // calculated on serial port rate + tolerance and rounded down to the nearest uS, long caters for even the slowest serial port of 75 bps

void FlushSerialBuffer() {
  while (true) {
    if (Serial.available() > 0) {
      byte dummy = Serial.read();
      while (Serial.available() > 0) { // flush additional bytes from serial buffer if present
        dummy = Serial.read();
      }
      if (TimePerByte <= 16383) {
        delayMicroseconds(TimePerByte); // delay in case another byte may be received via the serial port
      }
      else { // deal with delayMicroseconds limitation
        unsigned long DelayTime = TimePerByte;
        DelayTime /= 1000;
        if (DelayTime > 0) {
          delay(DelayTime);
        }
        DelayTime = TimePerByte;
        DelayTime %= 1000;
        if (DelayTime > 0) {
          delayMicroseconds(DelayTime);
        }
      }
    }
    else {
      break;
    }
  }
}

void getField (char* buffer, int index) {
  int CommandPos = 0;
  int FieldPos = 0;
  int SpaceCount = 0;
  while (CommandPos < CommandSize) {
    if (Command[CommandPos] == 0x20) {
      SpaceCount++;
      CommandPos++;
    }
    if (Command[CommandPos] == 0x0D || Command[CommandPos] == 0x0A) {
      break;
    }
    if (SpaceCount == index) {
      buffer[FieldPos] = Command[CommandPos];
      FieldPos++;
    }
    CommandPos++;
  }
  for (int ch = 0; ch < strlen(buffer); ch++) { // correct case of command
    buffer[ch] = toupper(buffer[ch]);
  }
  buffer[FieldPos] = '\0';
}

void setup() {
  Serial.begin(SerialPortRate);
  ADCDAC.SPIautoRestart = DefaultSPIautoRestart;
  ADCDAC.LowEnable = DefaultLowEnable;
  ADCDAC.NotOnSPIpins = DefaultNotOnSPIpins;
  ADCDAC.SignedChannels = 0;
  if (DefaultChannel0inSigned == true) {
    ADCDAC.SignedChannels |= Channel_SignedIn_0;
  }
  if (DefaultChannel1inSigned == true) {
    ADCDAC.SignedChannels |= Channel_SignedIn_1;
  }
  if (DefaultChannel0outSigned == true) {
    ADCDAC.SignedChannels |= Channel_SignedOut_0;
  }
  if (DefaultChannel1outSigned == true) {
    ADCDAC.SignedChannels |= Channel_SignedOut_1;
  }
  if (ADCDAC.init(DefaultAudioFormat, DefaultFieldBits, DefaultFrameBits, DefaultLeftRightPin, DefaultClockPin, DefaultSelectPin, DefaultDataInPin, DefaultDataInSupported, DefaultDataOutPin, DefaultDataOutSupported, DefaultSpeed) == false) {
    Serial.println(F("Incorrect initialization parameters"));
  }
}

void loop() {
  static int ByteCount = 0;
  if (Serial.available() > 0) {
    char value = Serial.read();
    if (value != '\n' && ByteCount < CommandSize) {
      Command[ByteCount] = value;
      ByteCount++;
    }
    else {
      ByteCount = 0;
      bool ValidField = true;
      char field[FieldSize];
      getField(field, 0);
      if (strcmp(field, "INIT") == 0) {
        getField(field, 1);
        byte format;
        if (strcmp(field, "I2S") == 0) {
          format = Audio_I2S;
        }
        else if (strcmp(field, "LEFT_JUSTIFIED") == 0) {
          format = Audio_LeftJustified;
        }
        else if (strcmp(field, "RIGHT_JUSTIFIED") == 0) {
          format = Audio_RightJustified;
        }
        else {
          ValidField = false;
        }
        if (ValidField == true) {
          getField(field, 2);
          byte field_bit_count = atoi(field);
          getField(field, 3);
          byte frame_bit_count = atoi(field);
          getField(field, 4);
          byte LeftRightPinToUse = atoi(field);
          getField(field, 5);
          byte ClockPinToUse = atoi(field);
          getField(field, 6);
          byte EnablePinToUse = atoi(field);
          getField(field, 7);
          byte DataInPinToUse = atoi(field);
          getField(field, 8);
          byte DataOutPinToUse = atoi(field);
          getField(field, 9);
          byte SPIspeed = atoi(field);
          ValidField = ADCDAC.init(format, field_bit_count, frame_bit_count, LeftRightPinToUse, ClockPinToUse, EnablePinToUse, DataInPinToUse, true, DataOutPinToUse, true, SPIspeed);
          if (ValidField == true) {
            getField(field, 10);
            if (strcmp(field, "SPI_AUTO_RESTART") == 0) {
              ADCDAC.SPIautoRestart = true;
              getField(field, 11);
              if (strcmp(field, "LOW_ENABLE") == 0) {
                ADCDAC.LowEnable = true;
              }
              else {
                ADCDAC.LowEnable = false;
              }
            }
            else if (strcmp(field, "LOW_ENABLE") == 0) {
              ADCDAC.SPIautoRestart = false;
              ADCDAC.LowEnable = true;
            }
            else {
              ADCDAC.SPIautoRestart = false;
              ADCDAC.LowEnable = false;
            }
            ValidField = ADCDAC.init(format, field_bit_count, frame_bit_count, LeftRightPinToUse, ClockPinToUse, EnablePinToUse, DataInPinToUse, true, DataOutPinToUse, true, SPIspeed);
          }
        }
      }
      else if (strcmp(field, "WRITE") == 0) {
        signed long ValuesToWrite[2];
        signed long ValuesToRead[2];
        getField(field, 1);
        ValuesToWrite[0] = atol(field);
        getField(field, 2);
        ValuesToWrite[1] = atol(field);
        bool IsSigned = false;
        getField(field, 3);
        byte SignFlags = 0;
        if (strcmp(field, "SIGNEDIN_0") == 0 || strcmp(field, "SIGNEDIN_1") == 0 || strcmp(field, "SIGNEDIN_01") == 0 || strcmp(field, "UNSIGNEDIN") == 0) {
          if (strcmp(field, "SIGNEDIN_0") == 0) {
            SignFlags |= Channel_SignedIn_0;
          }
          else if (strcmp(field, "SIGNEDIN_1") == 0) {
            SignFlags |= Channel_SignedIn_1;
          }
          else if (strcmp(field, "SIGNEDIN_01") == 0) {
            SignFlags |= (Channel_SignedIn_0 | Channel_SignedIn_1);
          }
          getField(field, 4);
          if (strcmp(field, "SIGNEDOUT_0") == 0 || strcmp(field, "SIGNEDOUT_1") == 0 || strcmp(field, "SIGNEDOUT_01") == 0 || strcmp(field, "UNSIGNEDOUT") == 0) {
            if (strcmp(field, "SIGNEDOUT_0") == 0) {
              SignFlags |= Channel_SignedOut_0;
            }
            if (strcmp(field, "SIGNEDOUT_1") == 0) {
              SignFlags |= Channel_SignedOut_1;
            }
            else if (strcmp(field, "SIGNEDOUT_01") == 0) {
              SignFlags |= (Channel_SignedOut_0 | Channel_SignedOut_1);
            }
          }
          else {
            SignFlags = 0;
          }
        }
        ADCDAC.SignedChannels = SignFlags;
        ADCDAC.transfer(ValuesToRead, ValuesToWrite);
        Serial.print(F("Values read (DEC): "));
        for (int i = 0; i < 2; i++) {
          Serial.print(ValuesToRead[i]);
          Serial.print(F(" "));
        }
        Serial.println(F(""));
        Serial.print(F("Values read (HEX): "));
        for (int Channel = 0; Channel < 2; Channel++) {
          unsigned long mask = 0xF0000000;
          for (int HalfByte = 0; HalfByte < 7; HalfByte++) {
            if ((ValuesToRead[Channel] & mask) != 0) {
              break;
            }
            Serial.print(F("0"));
            mask >>= 4;
          }
          Serial.print(ValuesToRead[Channel], HEX);
          Serial.print(F(" "));
        }
        Serial.println(F(""));
        Serial.print(F("Values written (HEX): "));
        for (int Channel = 0; Channel < 2; Channel++) {
          unsigned long mask = 0xF0000000;
          for (int HalfByte = 0; HalfByte < 7; HalfByte++) {
            if ((ValuesToWrite[Channel] & mask) != 0) {
              break;
            }
            Serial.print(F("0"));
            mask >>= 4;
          }
          Serial.print(ValuesToWrite[Channel], HEX);
          Serial.print(F(" "));
        }
        Serial.println(F(""));
      }
      else if (strcmp(field, "USE_SPI_PINS") == 0) {
        getField(field, 1);
        if (strcmp(field, "TRUE") == 0) {
          ADCDAC.NotOnSPIpins = false;
        }
        else if (strcmp(field, "FALSE") == 0) {
          SPI.end();
          ADCDAC.NotOnSPIpins = true;
        }
        else {
          ValidField = false;
        }
        if (ValidField == true) {
          Serial.println(F("Now initialize with the INIT command"));
        }
      }
      else {
        ValidField = false;
      }
      FlushSerialBuffer();
      if (ValidField == true) {
        Serial.println(F("OK"));
      }
      else {
        Serial.println(F("ERROR"));
      }
    }
  }
}