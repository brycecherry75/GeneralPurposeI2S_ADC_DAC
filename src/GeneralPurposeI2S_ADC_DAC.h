#ifndef GeneralPurposeI2S_ADC_DAC_H
#define GeneralPurposeI2S_ADC_DAC_H
#include <Arduino.h>
#include <SPI.h>
#include <stdint.h>
#include <BeyondByte.h> // obtain at http://github.com/brycecherry75/BeyondByte (version 1.1.1 or later which includes the required transfer functions)
#include <ShiftX.h> // obtain at http://github.com/brycecherry75/ShiftX (version 1.1.0 or later which includes the required transfer functions)

#define Audio_I2S 0
#define Audio_LeftJustified 1
#define Audio_RightJustified 2

// for SignedChannels
#define Channel_SignedIn_0 0x01
#define Channel_SignedIn_1 0x02
#define Channel_SignedOut_0 0x04
#define Channel_SignedOut_1 0x08

#define Audio_MaximumSPIspeed 5

class GeneralPurposeI2S_ADC_DAC {
  public:
  GeneralPurposeI2S_ADC_DAC();

  void transfer(int32_t *ChannelIn, int32_t *ChannelOut);
  bool init(uint8_t format, uint8_t field_bit_count, uint8_t frame_bit_count, uint8_t LeftRightPinToUse, uint8_t ClockPinToUse, uint8_t EnablePinToUse, uint8_t DataInPinToUse, bool DataInPinUsed, uint8_t DataOutPinToUse, bool DataOutPinUsed, uint8_t SPIspeedToUse);

  SPISettings GeneralPurposeI2S_ADC_DAC_SPI_Speed0;
  SPISettings GeneralPurposeI2S_ADC_DAC_SPI_Speed1;
  SPISettings GeneralPurposeI2S_ADC_DAC_SPI_Speed2;
  SPISettings GeneralPurposeI2S_ADC_DAC_SPI_Speed3;
  SPISettings GeneralPurposeI2S_ADC_DAC_SPI_Speed4;
  SPISettings GeneralPurposeI2S_ADC_DAC_SPI_Speed5;

  bool SPIautoRestart = false;
  bool LowEnable = false;
  bool NotOnSPIpins = false;
  uint8_t SignedChannels = 0;

  private:
  uint8_t AudioFormat;
  uint8_t FieldBits;
  uint8_t FrameBits;
  uint8_t EnablePin;
  uint8_t LeftRightPin;
  uint8_t ClockPin;
  uint8_t DataInPin = 255;
  uint8_t DataOutPin = 255;
  uint8_t SPIspeed;
};

#endif