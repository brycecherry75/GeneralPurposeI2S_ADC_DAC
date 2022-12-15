# GeneralPurposeI2S_ADC_DAC
Use a low cost and high resolution digital to analog/analog to digital converter for I2S audio to generate/measure voltages which boards with them are easy to obtain

Requires the BeyondByte library (version 1.1.1 or later which includes the required transfer functions): http://github.com/brycecherry75/BeyondByte

Requires the ShiftX library (version 1.1.0 or later which includes the required transfer functions): http://github.com/brycecherry75/ShiftX

Revisions:

1.0.0	First release

## Usage

init(format, field_bit_count, frame_bit_count, LeftRightPinToUse, ClockPinToUse, EnablePinToUse, DataInPinToUse, DataInPinIsed, DataOutPinToUse, DataOutPinUsed, SPIspeedToUse) - returns true if values are valid

Format is Audio_I2S/Audio_LeftJustified/Audio_RightJustified - just about every serial ADC/DAC for audio use will support at least one of these formats)

Bit count (both values are common to the connected ADC and DAC in the library member) defines how many bits are required by the ADC/DAC - the frame bit count must be equal to or greater than the field bit count and the SPI interface will start automatically if the frame bit count is a multiple of 8.

The SPIspeed option value (0-5) sets the sampling clock in the library member which is 1 MHz * (2 ^ SPIspeed)

Consult your ADC/DAC datasheet for proper configuration of the above three items.

Usage fields are a bool and the Data IN/OUT pins can be SPI hardware pins - if the SPIautoRestart bool in the libary member is true, the SPI interface will be disabled when writing a frame which is not a multiple of 8 and will be enabled afterwards.

transfer(*ChannelIn, *ChannelOut, ChannelSigned_0, ChannelSigned_1) - transfers *ChannelOut to the DACs and *ChannelIn from the ADCs: *Channel(In/Out) are two element int32_t arrays where Element 0 is Left and Element 1 is Right, and the return value of *ChannelOut will be the value of the field in the frame transmitted to the DAC after processing.

For reading an ADC or writing a DAC, the zero value for unsigned ADC/DAC values (with the IsSigned bool set to true) is at the lower end of the voltage scale while the zero value for signed ADC/DAC values (with the ChannelSigned bool set to false) is at the centre of the voltage scale; a serial audio ADC input/DAC output (before DC blocking) typically has a voltage offset i.e. the lowest voltage is higher than zero volts when the lowest possible negative signed number is programmed (MSB set to 1 and all other bits set to 0).

Blocking of the clock line (required when idle) can be easily implemented by connecting a 1K0 resistor between the Clock on the ADC/DAC and the microcontroller and also connecting a signal diode with its Anode on the ADC/DAC side and the Cathode on the microcontroller Select line - the LowEnable bool in the library member is false by default to cater for this arrangement.

If any of the Clock/Data IN/Data OUT signals are not on SPI hardware pins, set the NotOnSPIpins bool in the library member to true (it is false by default).

To set a channel as signed, OR SignedChannels (none are signed by default) with Channel_Signed(In/Out)_(0/1); to set a channel as unsigned, AND SignedChannels with the inverse of Channel_Signed(In/Out)_(0/1)