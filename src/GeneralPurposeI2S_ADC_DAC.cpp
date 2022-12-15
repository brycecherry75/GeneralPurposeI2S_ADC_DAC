#include "GeneralPurposeI2S_ADC_DAC.h"

GeneralPurposeI2S_ADC_DAC::GeneralPurposeI2S_ADC_DAC() {
  SPISettings GeneralPurposeI2S_ADC_DAC_SPI_Speed0(1000000UL, MSBFIRST, SPI_MODE0); // caters for the slowest of all audio serial ADC/DAC ICs being 32 kHz/16 bit stereo
  SPISettings GeneralPurposeI2S_ADC_DAC_SPI_Speed1(2000000UL, MSBFIRST, SPI_MODE0);
  SPISettings GeneralPurposeI2S_ADC_DAC_SPI_Speed2(4000000UL, MSBFIRST, SPI_MODE0);
  SPISettings GeneralPurposeI2S_ADC_DAC_SPI_Speed3(8000000UL, MSBFIRST, SPI_MODE0);
  SPISettings GeneralPurposeI2S_ADC_DAC_SPI_Speed4(16000000UL, MSBFIRST, SPI_MODE0);
  SPISettings GeneralPurposeI2S_ADC_DAC_SPI_Speed5(32000000UL, MSBFIRST, SPI_MODE0);
}

void GeneralPurposeI2S_ADC_DAC::transfer(int32_t *ChannelIn, int32_t *ChannelOut) {
  if (LowEnable == false) {
    digitalWrite(EnablePin, HIGH);
  }
  else {
    digitalWrite(EnablePin, LOW);
  }
  for (int Channels = 0; Channels < 2; Channels++) {
    uint32_t mask = 1;
    mask <<= (FieldBits - 1); // 16 bit field - now 0x8000
    if ((Channels == 0 && (SignedChannels & Channel_SignedOut_0) != 0) || (Channels == 1 && (SignedChannels & Channel_SignedOut_1) != 0)) { // signed channel - input value is (0xFFFF8001/-32767)/(0xFFFF0002/+32766)
      if ((ChannelOut[Channels] & 0x80000000) != 0) { // is a negative number
        ChannelOut[Channels] ^= 0xFFFFFFFF; // convert to a positive - now (0x7FFE/32766)
        ChannelOut[Channels]++; // now (0x7FFF/32767)
        if (ChannelOut[Channels] <= mask) { // input value is (0x7FFF/32767)
          ChannelOut[Channels] = (mask - ChannelOut[Channels]); // now (0x0001/1)
          ChannelOut[Channels] += mask; // final value is (0x8001/-32767)
        }
      }
    }
    else { // unsigned channel
      if (ChannelOut[Channels] >= mask) { // input value is (0xFFFE/65534)/(0x7FFE/32766)
        ChannelOut[Channels] -= mask; // final result is (0x7FFE/32766)
      }
      else {
        ChannelOut[Channels] += mask; // final result is (0xFFFE/-2)
      }
    }
    if (AudioFormat != Audio_I2S) {
      if (Channels == 0) { // Left channel (0) selected HIGH for Left/Right justified formats
        digitalWrite(LeftRightPin, HIGH);
      }
      else {
        digitalWrite(LeftRightPin, LOW);
      }
    }
    else {
      if (Channels == 0) { // Left channel (0) selected LOW for I2S
        digitalWrite(LeftRightPin, LOW);
      }
      else {
        digitalWrite(LeftRightPin, HIGH);
      }
    }
    if (AudioFormat == Audio_I2S) {
      ChannelOut[Channels] <<= (FrameBits - FieldBits - 1);
    }
    else if (AudioFormat == Audio_LeftJustified) {
      ChannelOut[Channels] <<= (FrameBits - FieldBits);
    }
    // no shift required for Right Justified
    if ((FrameBits % 8) == 0 && NotOnSPIpins == false) {
      switch (SPIspeed) {
        case 0:
          SPI.beginTransaction(GeneralPurposeI2S_ADC_DAC_SPI_Speed0);
          break;
        case 1:
          SPI.beginTransaction(GeneralPurposeI2S_ADC_DAC_SPI_Speed1);
          break;
        case 2:
          SPI.beginTransaction(GeneralPurposeI2S_ADC_DAC_SPI_Speed2);
          break;
        case 3:
          SPI.beginTransaction(GeneralPurposeI2S_ADC_DAC_SPI_Speed3);
          break;
        case 4:
          SPI.beginTransaction(GeneralPurposeI2S_ADC_DAC_SPI_Speed4);
          break;
        case 5:
          SPI.beginTransaction(GeneralPurposeI2S_ADC_DAC_SPI_Speed5);
          break;
      }
      // clock activity will write a new value to the DAC
      ChannelIn[Channels] = BeyondByte.transferDword_SPI(ChannelOut[Channels], (FrameBits / 8), MSBFIRST);
      SPI.endTransaction();
    }
    else {
      if (SPIautoRestart == true && NotOnSPIpins == false) {
        SPI.end();
      }
      ChannelIn[Channels] = ShiftX.transfer_Dword(DataInPin, DataOutPin, ClockPin, MSBFIRST, FrameBits, 1, ChannelOut[Channels], RISING);
      if (SPIautoRestart == true && NotOnSPIpins == false) {
        SPI.begin();
      }
    }
    if (AudioFormat == Audio_I2S) {
      ChannelIn[Channels] >>= (FrameBits - FieldBits - 1);
    }
    else if (AudioFormat == Audio_LeftJustified) {
      ChannelIn[Channels] >>= (FrameBits - FieldBits);
    }
    // mask off unwanted bits
    mask <<= 1; // now 0x10000;
    mask--; // now 0xFFFF
    ChannelIn[Channels] &= mask;
    // restore the mask back to its original state for the next operation
    mask >>= 1; // now 0x7FFF - avoid overflow to 0 before increment
    mask++; // now 0x8000
    if ((Channels == 0 && (SignedChannels & Channel_SignedIn_0) != 0) || (Channels == 1 && (SignedChannels & Channel_SignedIn_1) != 0)) {
      // input value is (0x8001/-32767)
      if ((ChannelIn[Channels] & mask) != 0) { // is a negative number
        mask <<= 1; // now 0x10000
        mask--; // now 0xFFFF
        mask ^= 0xFFFFFFFF; // now 0xFFFF0000
        ChannelIn[Channels] ^= mask; // final result is (0xFFFF7FFE/+32770)
      }
    }
    else {
      if (ChannelIn[Channels] >= mask) { // input value is (0x0000FFFE/+65534)
        ChannelIn[Channels] -= mask; // final result is (0x7FFE/+32767)
      }
      else { // input value is (0x00007FFE/+32766)
        ChannelIn[Channels] += mask; // final result is (0xFFFE/-2)
      }
    }
  }
  if (LowEnable == false) {
    digitalWrite(EnablePin, LOW);
  }
  else {
    digitalWrite(EnablePin, HIGH);
  }
}

bool GeneralPurposeI2S_ADC_DAC::init(uint8_t format, uint8_t field_bit_count, uint8_t frame_bit_count, uint8_t LeftRightPinToUse, uint8_t ClockPinToUse, uint8_t EnablePinToUse, uint8_t DataInPinToUse, bool DataInPinUsed, uint8_t DataOutPinToUse, bool DataOutPinUsed, uint8_t SPIspeedToUse) {
  if ((format == Audio_I2S || format == Audio_LeftJustified || format == Audio_RightJustified) && SPIspeedToUse <= Audio_MaximumSPIspeed && field_bit_count <= frame_bit_count && frame_bit_count > 0 && frame_bit_count <= 32) {
    if (NotOnSPIpins == false && (frame_bit_count % 8) == 0) {
      SPI.begin();
    }
    AudioFormat = format;
    FieldBits = field_bit_count;
    FrameBits = frame_bit_count;
    EnablePin = EnablePinToUse;
    LeftRightPin = LeftRightPinToUse;
    ClockPin = ClockPinToUse;
    pinMode(EnablePin, OUTPUT);
    digitalWrite(LeftRightPin, LOW);
    pinMode(LeftRightPin, OUTPUT);
    digitalWrite(ClockPin, LOW);
    pinMode(ClockPin, OUTPUT);
    if (DataInPinUsed == true) {
      DataInPin = DataInPinToUse;
      pinMode(DataInPin, INPUT);
    }
    if (DataOutPinUsed == true) {
      DataOutPin = DataOutPinToUse;
      pinMode(DataOutPin, OUTPUT);
    }
    SPIspeed = SPIspeedToUse;
    // zero the DACs
    int32_t dummyWrite[2] = {0, 0};
    int32_t dummyRead[2];
    transfer(dummyRead, dummyWrite); // will synchronize data transfer
    transfer(dummyRead, dummyWrite); // will write to DACs
    return true;
  }
  else {
    return false;
  }
}