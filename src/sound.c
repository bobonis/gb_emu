/*
Name Addr 7654 3210 Function
-----------------------------------------------------------------
       Square 1
NR10 FF10 -PPP NSSS Sweep period, negate, shift
NR11 FF11 DDLL LLLL Duty, Length load (64-L)
NR12 FF12 VVVV APPP Starting volume, Envelope add mode, period
NR13 FF13 FFFF FFFF Frequency LSB
NR14 FF14 TL-- -FFF Trigger, Length enable, Frequency MSB

       Square 2
     FF15 ---- ---- Not used
NR21 FF16 DDLL LLLL Duty, Length load (64-L)
NR22 FF17 VVVV APPP Starting volume, Envelope add mode, period
NR23 FF18 FFFF FFFF Frequency LSB
NR24 FF19 TL-- -FFF Trigger, Length enable, Frequency MSB

       Wave
NR30 FF1A E--- ---- DAC power
NR31 FF1B LLLL LLLL Length load (256-L)
NR32 FF1C -VV- ---- Volume code (00=0%, 01=100%, 10=50%, 11=25%)
NR33 FF1D FFFF FFFF Frequency LSB
NR34 FF1E TL-- -FFF Trigger, Length enable, Frequency MSB

       Noise
     FF1F ---- ---- Not used
NR41 FF20 --LL LLLL Length load (64-L)
NR42 FF21 VVVV APPP Starting volume, Envelope add mode, period
NR43 FF22 SSSS WDDD Clock shift, Width mode of LFSR, Divisor code
NR44 FF23 TL-- ---- Trigger, Length enable

       Control/Status
NR50 FF24 ALLL BRRR Vin L enable, Left vol, Vin R enable, Right vol
NR51 FF25 NW21 NW21 Left enables, Right enables
NR52 FF26 P--- NW21 Power control/status, Channel length statuses

       Not used
     FF27 ---- ----
     .... ---- ----
     FF2F ---- ----

       Wave Table
     FF30 0000 1111 Samples 0 and 1
     ....
     FF3F 0000 1111 Samples 30 and 31
     
*/

#include "sound.h"
#include "memory.h"
#include "definitions.h"
#include <stdio.h>




typedef struct {

    unsigned int enable;
    
    /* Right */
    unsigned char S01_volume;
    unsigned char S01_Vin_output;
    
    /* Left */
    unsigned char S02_volume;
    unsigned char S02_Vin_output;
    
    
    unsigned int fivebitcounter;
    unsigned int unifiedlengthclock;
    unsigned int framesequencerclock;
    unsigned int framesequencerstep;
    
    struct { /* Tone & Sweep */
        
        unsigned int dac;
        unsigned int trigger;
        unsigned int enable;
        unsigned short frequency;
        unsigned char length_cnt;
        unsigned int length_cnt_skip;
        unsigned char wave_duty;
        
        struct {
            unsigned char volume;
            unsigned char increasing;
            unsigned char period;
            } envelope;
            
        struct {
            unsigned char period;
            unsigned char increasing;
            unsigned char shift;
            unsigned int enable;
            unsigned short shadowregister;
            } sweep;
        } Chn1;

    struct { /* Tone */

        unsigned int dac;
        unsigned int trigger;
        unsigned int enable;
        unsigned short frequency;
        unsigned char length_cnt;
        unsigned int length_cnt_skip;
        unsigned char wave_duty;
        
        struct {
            unsigned char volume;
            unsigned char increasing;
            unsigned char period;
            } envelope;
        } Chn2;

    struct { /* Wave Output */

        unsigned short wave_ram[16];
        unsigned int dac;
        unsigned int trigger;
        unsigned int enable;
        unsigned short frequency;
        unsigned short length_cnt;
        unsigned int length_cnt_skip;
        unsigned char volume;
        } Chn3;

    struct { //Noise
    
        unsigned int dac;
        unsigned int trigger;
        unsigned int enable;
        unsigned char length_cnt;
        unsigned int length_cnt_skip;
        unsigned char noise;
        
        struct {
            unsigned char volume;
            unsigned char increasing;
            unsigned char period;
            } envelope;
        } Chn4;

} _GB_SOUND_HARDWARE_;

static _GB_SOUND_HARDWARE_ soundstate;


/*
Step   Length Ctr  Vol Env     Sweep
---------------------------------------
0      Clock       -           -
1      -           -           -
2      Clock       -           Clock
3      -           -           -
4      Clock       -           -
5      -           -           -
6      Clock       -           Clock
7      -           Clock       -
---------------------------------------
Rate   256 Hz      64 Hz       128 Hz
*/
void soundTick(void)
{
    soundstate.framesequencerclock -= 4;
    
    if (soundstate.framesequencerclock == 0) {
        soundstate.framesequencerclock = 8192;
        switch (soundstate.framesequencerstep) {
            case 0 :
                soundTickLenghthCounter();
                break;
            case 1 :
            
                break;
            case 2 :
                soundTickLenghthCounter();
                soundTickSweepCounter();
                break;
            case 3 :
            
                break;
            case 4 :
                soundTickLenghthCounter();
                break;
            case 5 :
            
                break;
            case 6 :
                soundTickLenghthCounter();
                soundTickSweepCounter();
                break;
            case 7 :
            
                break;
            default :
                break;
        }
        
        if (soundstate.framesequencerstep < 7){
            soundstate.framesequencerstep += 1;
        } else {
            soundstate.framesequencerstep = 0;
        }
        
    }
    
    soundTickProgrammableCounter();
    //soundTickLenghthCounter();
    //soundTickSweepCounter();

}


void soundTickProgrammableCounter(void)
{
   // soundstate.fivebitcounter -= 4;
    
    //if (soundstate.fivebitcounter != 0)
       // return;
    
    /* Channel 1 */
    

    /* Channel 2 */
    
    
    /* Channel 3 */
    
    //soundstate.fivebitcounter = 32;
}



void soundTickLenghthCounter(void)
{

    /* It is a single clock, running at 256hz, probably divided directly from the GB
       processor's clock (4.194304Mhz). It is used to clock the length counters of
       each channel, when the length counter enable bit (see above) for that
       particular channel is on. It is common to all channels, so when it clocks for
       one channel it clocks all of them. */

    //soundstate.unifiedlengthclock -=4;
    
    //if (soundstate.unifiedlengthclock == 0) {
        //soundstate.unifiedlengthclock = 16384;

        if (soundstate.Chn1.enable | soundstate.Chn2.enable | soundstate.Chn3.enable | soundstate.Chn4.enable |
            soundstate.Chn1.trigger  | soundstate.Chn2.trigger  | soundstate.Chn3.trigger  | soundstate.Chn4.trigger)
        printf("[SND] CH1 %d-%d %2d, CH2 %d-%d %2d, CH3 %d-%d %2d, CH4 %d-%d %2d, FF26=%x\n",
        soundstate.Chn1.trigger,soundstate.Chn1.enable,soundstate.Chn1.length_cnt,
        soundstate.Chn2.trigger,soundstate.Chn2.enable,soundstate.Chn2.length_cnt,
        soundstate.Chn3.trigger,soundstate.Chn3.enable,soundstate.Chn3.length_cnt,
        soundstate.Chn4.trigger,soundstate.Chn4.enable,soundstate.Chn4.length_cnt, memory[NR52] ); 
        
        /* The clocking of the counter is enabled or disabled depending on the status of
           the 'length counter enable register' (AKA the 'consecutive' bit) in register
           set 5, a 1 enabling the counter, a 0 disabling it. */
           
        /*************
        * Channel 1  *
        *************/    
        if ((soundstate.Chn1.enable == 1) && (soundstate.Chn1.length_cnt > 0)) {
                soundstate.Chn1.length_cnt -= 1;

                if (soundstate.Chn1.length_cnt == 0) { /* Length becoming 0 should clear status */
                    printf("[SND] Channel 1 Disabled\n");
                    soundstate.Chn1.trigger = FALSE;
                    setBit(NR52,0,FALSE);
                }
        }
        /*************
        * Channel 2  *
        *************/
        if ((soundstate.Chn2.enable == 1) && (soundstate.Chn2.length_cnt > 0)) {
                soundstate.Chn2.length_cnt -= 1;

                if (soundstate.Chn2.length_cnt == 0) { /* Length becoming 0 should clear status */
                    printf("[SND] Channel 2 Disabled\n");
                    soundstate.Chn2.trigger = FALSE;
                    setBit(NR52,1,FALSE);
                }
        } 
        /*************
        * Channel 3  *
        *************/
        if ((soundstate.Chn3.enable == 1) && (soundstate.Chn3.length_cnt > 0)) {
                soundstate.Chn3.length_cnt -= 1;

                if (soundstate.Chn3.length_cnt == 0) { /* Length becoming 0 should clear status */
                    printf("[SND] Channel 2 Disabled\n");
                    soundstate.Chn3.trigger = FALSE;
                    setBit(NR52,2,FALSE);
                }
        } 
        /*************
        * Channel 4  *
        *************/
        if ((soundstate.Chn4.enable == 1) && (soundstate.Chn4.length_cnt > 0)) {
                soundstate.Chn4.length_cnt -= 1;

                if (soundstate.Chn4.length_cnt == 0) { /* Length becoming 0 should clear status */
                    printf("[SND] Channel 2 Disabled\n");
                    soundstate.Chn2.trigger = FALSE;
                    setBit(NR52,3,FALSE);
                }
        } 
    //}
}

void soundTickSweepCounter(void) {
    /* When the sweep's internal enabled flag is set and the sweep period is not zero, 
       a new frequency is calculated and the overflow check is performed 
     */
     unsigned short temp_frequency;
    if ((soundstate.Chn1.sweep.enable) && (soundstate.Chn1.sweep.period)){
        temp_frequency = soundstate.Chn1.sweep.shadowregister + (soundstate.Chn1.sweep.shadowregister >> soundstate.Chn1.sweep.shift);
        if ((temp_frequency <= 2047) && (soundstate.Chn1.sweep.shift)) {
            soundstate.Chn1.sweep.shadowregister = temp_frequency;
            memory[NR13] = temp_frequency & 0x00FF;
            memory[NR14] &= 0xF8;
            memory[NR14] |= ((temp_frequency >> 8) & 0x0007);
            
            soundstate.Chn1.sweep.shadowregister += soundstate.Chn1.sweep.shadowregister >> soundstate.Chn1.sweep.shift;
            /* if this is greater than 2047, square 1 is disabled */
            if (soundstate.Chn1.sweep.shadowregister > 2047) {
                soundstate.Chn1.trigger = FALSE;
                setBit(NR52,0,FALSE);
            }
        }
    }
}

void soundWriteRegister(unsigned short address,unsigned char value)
{
    
    printf("[SND] Wrote %x at %x, DAC=%d%d%d%d, clock=%4d\n",value,address,soundstate.Chn1.dac,soundstate.Chn2.dac,soundstate.Chn3.dac,soundstate.Chn4.dac,soundstate.unifiedlengthclock);
    
    if ((address >= 0xFF30) && (address <= 0xFF3F)) {
        memory[address] = value;
    }
    
    if (soundstate.enable == FALSE) {
        if (address == NR52) {
            if ((value & (1<<7)) == 0) { /* Turn audio processing unit off */
                soundTurnOff();
                /* Writing a 0 to bit position 7 in register $FF26(NR52) disables the power to the audio processing unit */
                soundstate.enable = FALSE;
                /* Unused bits 4,5,6 are 1, all other bits are 0 */
                memory[NR52] = 0x70; 
            } else { /* Turn audio processing unit on */
                soundTurnOn();
                soundstate.enable = TRUE;
                memory[NR52] = 0x80; 
            }
            
        }
        return;
    }
    
    switch (address) {
        /*************
        * Channel 1  *
        *************/
        case NR10 : /* 0xFF10 -PPP NSSS Sweep period, negate, shift */
            memory[NR10] = value;
            soundstate.Chn1.sweep.period = (value & 0x70) >> 4;
            soundstate.Chn1.sweep.increasing = (value & 0x08) >> 3;
            soundstate.Chn1.sweep.shift = value & 0x07;
            break;
        case NR11 : /* 0xFF11 DDLL LLLL Duty, Length load (64-L) */
            memory[NR11] = value;
            /* The 5-bit length value is subtracted from 64 and the result being written to the counter */
            printf("[SND] Lenght before: %d, after: %d\n",soundstate.Chn1.length_cnt,64 - (value & 0x3F));
            soundstate.Chn1.length_cnt = 64 - (value & 0x3F);
            soundstate.Chn1.wave_duty = (value >> 6) & 0x03; 
            break;
        case NR12 : /* 0xFF12 VVVV APPP Starting volume, Envelope add mode, period */
            memory[NR12] = value;
            soundstate.Chn1.envelope.volume = (value >> 4) & 0x0F;
            soundstate.Chn1.envelope.increasing = (value & 0x08) >> 3;
            soundstate.Chn1.envelope.period = value & 0x07;
            if (!(value & 0xF8)) { /* Disabling DAC should disable channel immediately */
                soundstate.Chn1.dac = FALSE;
                soundstate.Chn1.trigger = FALSE;
                //soundstate.Chn1.length_cnt = 0;
                setBit(NR52,0,FALSE);
            } else { /* Enabling DAC shouldn't re-enable channel */
                soundstate.Chn1.dac = TRUE;
            }
            break;
        case NR13 : /* 0xFF13 FFFF FFFF Frequency LSB */
            memory[NR13] = value;
            soundstate.Chn1.frequency = ((memory[NR14] & 0x07) << 8 ) | value;
            break;
        case NR14 : /* 0xFF14 TL-- -FFF Trigger, Length enable, Frequency MSB */
            memory[NR14] = value;
            soundstate.Chn1.frequency = ((value & 0x07) << 8 ) | memory[NR13];

            if (( (value & 0x80) >> 7) ){ /* trigger channel */
                soundstate.Chn1.trigger = TRUE;
                setBit(NR52,0,TRUE);
            }

            /* Frequency Sweep */
            
            if (soundstate.Chn1.trigger) {
                
                /* Square 1's frequency is copied to the shadow register */
                soundstate.Chn1.sweep.shadowregister = soundstate.Chn1.frequency;
                
                /* The sweep timer is reloaded */

                
                /* The internal enabled flag is set if either the sweep period or shift are non-zero, cleared otherwise */
                if ((soundstate.Chn1.sweep.period) || (soundstate.Chn1.sweep.shift)) {
                    soundstate.Chn1.sweep.enable = TRUE;
                } else {
                    soundstate.Chn1.sweep.enable = FALSE;
                }
                                
                /* If the sweep shift is non-zero, frequency calculation and the overflow check are performed immediately */
                if (soundstate.Chn1.sweep.shift) {
                    /* Frequency calculation consists of taking the value in the frequency shadow register, shifting it right by sweep shift, optionally negating the value, and summing this with the frequency shadow register to produce a new frequency */
                    soundstate.Chn1.sweep.shadowregister += soundstate.Chn1.sweep.shadowregister >> soundstate.Chn1.sweep.shift;
                    /* if this is greater than 2047, square 1 is disabled */
                    if (soundstate.Chn1.sweep.shadowregister > 2047) {
                        soundstate.Chn1.trigger = FALSE;
                        setBit(NR52,0,FALSE);
                    } 
                }
            }



            if ( ( (value & 0x40) >> 6)  ) {
                /* when the frame sequencer's next step is one that doesn't clock the length counter
                   if the length counter was PREVIOUSLY disabled and now enabled */
                if ((soundstate.framesequencerstep % 2) && (soundstate.Chn1.enable == FALSE)) {
                    /* if the length counter is not zero, it is decremented */
                    if (soundstate.Chn1.length_cnt) {
                        soundstate.Chn1.length_cnt -= 1;
                        /* If this decrement makes it zero and trigger is clear, the channel is disabled */
                        if (soundstate.Chn1.length_cnt == 0){
                            soundstate.Chn1.trigger = FALSE;
                            setBit(NR52,0,FALSE);
                        }
                    }
                }
                soundstate.Chn1.enable = TRUE;
            } else {
                soundstate.Chn1.enable = FALSE;
            }

            /* If a channel is triggered when the frame sequencer's next step is one that doesn't clock 
               the length counter and the length counter is now enabled and length is being set to 64 
               (256 for wave channel) because it was previously zero, it is set to 63 instead */
            if (((value & 0x80) >> 7) && (soundstate.Chn1.length_cnt == 0)) {
                if ((soundstate.Chn1.enable) && (soundstate.framesequencerstep % 2)){
                    soundstate.Chn1.length_cnt = 63;
                } else {
                    soundstate.Chn1.length_cnt = 64;
                }
            }

            /* if the channel's DAC is off, after the above actions occur the channel will be immediately disabled again */ 
            if (soundstate.Chn1.dac == OFF) {
                soundstate.Chn1.trigger = FALSE;
                setBit(NR52,0,FALSE);
            }

            printf("[SND] Lenght after: %d\n",soundstate.Chn1.length_cnt);

            
            break;
        /*************
        * Channel 2  *
        *************/
         case NR20 : /* 0xFF15 ---- ---- Not used */
            memory[NR20] = 0xFF;
            break; 
         case NR21 : /* 0xFF16 DDLL LLLL Duty, Length load (64-L) */
            memory[NR21] = value;
            /* The 5-bit length value is subtracted from 64 and the result being written to the counter */
            soundstate.Chn2.length_cnt = 64 - (value & 0x3F);
            soundstate.Chn2.wave_duty = (value >> 6) & 0x03; 
            break;
        case NR22 : /* 0xFF17 VVVV APPP Starting volume, Envelope add mode, period */
            memory[NR22] = value;
            soundstate.Chn2.envelope.volume = (value >> 4) & 0x0F;
            soundstate.Chn2.envelope.increasing = (value & 0x08) >> 3;
            soundstate.Chn2.envelope.period = value & 0x07;
            if (!(value & 0xF8)) { /* Disabling DAC should disable channel immediately */
                soundstate.Chn2.dac = FALSE;
                soundstate.Chn2.trigger = FALSE;
                setBit(NR52,1,FALSE);
            } else { /* Enabling DAC shouldn't re-enable channel */
                soundstate.Chn2.dac = TRUE;
            }
            break;
        case NR23 : /* 0xFF18 FFFF FFFF Frequency LSB */
            memory[NR23] = value;
            soundstate.Chn2.frequency = ((memory[NR24] & 0x07) << 8 ) | value;
            break;
        case NR24 : /* 0xFF19 TL-- -FFF Trigger, Length enable, Frequency MSB */
            memory[NR24] = value;
            soundstate.Chn2.frequency = ((value & 0x07) << 8 ) | memory[NR23];
            
            if (( (value & 0x80) >> 7) ){ /* trigger channel */
                soundstate.Chn2.trigger = TRUE;
                setBit(NR52,1,TRUE);
            }


            if ( ( (value & 0x40) >> 6)  ) {
                /* when the frame sequencer's next step is one that doesn't clock the length counter
                   if the length counter was PREVIOUSLY disabled and now enabled */
                if ((soundstate.framesequencerstep % 2) && (soundstate.Chn2.enable == FALSE)) {
                    /* if the length counter is not zero, it is decremented */
                    if (soundstate.Chn2.length_cnt) {
                        soundstate.Chn2.length_cnt -= 1;
                        /* If this decrement makes it zero and trigger is clear, the channel is disabled */
                        if (soundstate.Chn2.length_cnt == 0){
                            soundstate.Chn2.trigger = FALSE;
                            setBit(NR52,1,FALSE);
                        }
                    }
                }
                soundstate.Chn2.enable = TRUE;
            } else {
                soundstate.Chn2.enable = FALSE;
            }

            /* If a channel is triggered when the frame sequencer's next step is one that doesn't clock 
               the length counter and the length counter is now enabled and length is being set to 64 
               (256 for wave channel) because it was previously zero, it is set to 63 instead */
            if (((value & 0x80) >> 7) && (soundstate.Chn2.length_cnt == 0)) {
                if ((soundstate.Chn2.enable) && (soundstate.framesequencerstep % 2)){
                    soundstate.Chn2.length_cnt = 63;
                } else {
                    soundstate.Chn2.length_cnt = 64;
                }
            }

            /* if the channel's DAC is off, after the above actions occur the channel will be immediately disabled again */ 
            if (soundstate.Chn2.dac == OFF) {
                soundstate.Chn2.trigger = FALSE;
                setBit(NR52,1,FALSE);
            }

            printf("[SND] Lenght after: %d\n",soundstate.Chn2.length_cnt);

            
            break;
        /*************
        * Channel 3  *
        *************/  
        case NR30 : /* 0xFF1A E--- ---- DAC power */
            memory[NR30] = value;
            soundstate.Chn3.dac = (value & 0x80) >> 7;
            break; 
        case NR31 : /* 0xFF1B LLLL LLLL Length load (256-L) */
            memory[NR31] = value;
            soundstate.Chn3.length_cnt = 256 - value;
            break;
        case NR32 : /* 0xFF1C -VV- ---- Volume code (00=0%, 01=100%, 10=50%, 11=25%) */
            memory[NR32] = value;
            soundstate.Chn3.volume = (value & 0x60) >> 5;
            if (!(value & 0xF8)) { /* Disabling DAC should disable channel immediately */
                soundstate.Chn3.dac = FALSE;
                soundstate.Chn3.trigger = FALSE;
                setBit(NR52,2,FALSE);
            } else { /* Enabling DAC shouldn't re-enable channel */
                soundstate.Chn3.dac = TRUE;
            }
            break;
        case NR33 : /* 0xFF1D FFFF FFFF Frequency LSB */
            memory[NR33] = value;
            soundstate.Chn3.frequency = ((memory[NR34] & 0x07) << 8 ) | value;
            break;
        case NR34 : /* 0xFF1E TL-- -FFF Trigger, Length enable, Frequency MSB */
            memory[NR34] = value;
            soundstate.Chn3.frequency = ((value & 0x07) << 8 ) | memory[NR33];
            
            if (( (value & 0x80) >> 7) ){ /* trigger channel */
                soundstate.Chn3.trigger = TRUE;
                setBit(NR52,2,TRUE);
            }


            if ( ( (value & 0x40) >> 6)  ) {
                /* when the frame sequencer's next step is one that doesn't clock the length counter
                   if the length counter was PREVIOUSLY disabled and now enabled */
                if ((soundstate.framesequencerstep % 2) && (soundstate.Chn3.enable == FALSE)) {
                    /* if the length counter is not zero, it is decremented */
                    if (soundstate.Chn3.length_cnt) {
                        soundstate.Chn3.length_cnt -= 1;
                        /* If this decrement makes it zero and trigger is clear, the channel is disabled */
                        if (soundstate.Chn3.length_cnt == 0){
                            soundstate.Chn3.trigger = FALSE;
                            setBit(NR52,2,FALSE);
                        }
                    }
                }
                soundstate.Chn3.enable = TRUE;
            } else {
                soundstate.Chn3.enable = FALSE;
            }

            /* If a channel is triggered when the frame sequencer's next step is one that doesn't clock 
               the length counter and the length counter is now enabled and length is being set to 64 
               (256 for wave channel) because it was previously zero, it is set to 63 instead */
            if (((value & 0x80) >> 7) && (soundstate.Chn3.length_cnt == 0)) {
                if ((soundstate.Chn3.enable) && (soundstate.framesequencerstep % 2)){
                    soundstate.Chn3.length_cnt = 255;
                } else {
                    soundstate.Chn3.length_cnt = 256;
                }
            }

            /* if the channel's DAC is off, after the above actions occur the channel will be immediately disabled again */ 
            if (soundstate.Chn3.dac == OFF) {
                soundstate.Chn3.trigger = FALSE;
                setBit(NR52,2,FALSE);
            }

            printf("[SND] Lenght after: %d\n",soundstate.Chn3.length_cnt);

          
            break;
        /*************
        * Channel 4  *
        *************/
        case NR40 : /* 0xFF1F ---- ---- Not used */
            memory[NR40] = 0xFF;
            break; 
        case NR41 : /* 0xFF20 --LL LLLL Length load (64-L) */
            memory[NR41] = value;
            soundstate.Chn4.length_cnt = 64 - (value & 0x3F);
            break;
        case NR42 : /* 0xFF21 VVVV APPP Starting volume, Envelope add mode, period */
            memory[NR42] = value;
            soundstate.Chn4.envelope.volume = (value >> 4) & 0x0F;
            soundstate.Chn4.envelope.increasing = (value & 0x08) >> 3;
            soundstate.Chn4.envelope.period = value & 0x07;
            if (!(value & 0xF8)) { /* Disabling DAC should disable channel immediately */
                soundstate.Chn4.dac = FALSE;
                soundstate.Chn4.trigger = FALSE;
                setBit(NR52,3,FALSE);
            } else { /* Enabling DAC shouldn't re-enable channel */
                soundstate.Chn4.dac = TRUE;
            }
            break;    
        case NR43 : /* 0xFF22 SSSS WDDD Clock shift, Width mode of LFSR, Divisor code */
            memory[NR43] = value;
            soundstate.Chn4.noise = value;
            break;
        case NR44 : /* 0xFF23 TL-- ---- Trigger, Length enable */
            memory[NR44] = value;

            if (( (value & 0x80) >> 7) ){ /* trigger channel */
                soundstate.Chn4.trigger = TRUE;
                setBit(NR52,3,TRUE);
            }


            if ( ( (value & 0x40) >> 6)  ) {
                /* when the frame sequencer's next step is one that doesn't clock the length counter
                   if the length counter was PREVIOUSLY disabled and now enabled */
                if ((soundstate.framesequencerstep % 2) && (soundstate.Chn4.enable == FALSE)) {
                    /* if the length counter is not zero, it is decremented */
                    if (soundstate.Chn4.length_cnt) {
                        soundstate.Chn4.length_cnt -= 1;
                        /* If this decrement makes it zero and trigger is clear, the channel is disabled */
                        if (soundstate.Chn4.length_cnt == 0){
                            soundstate.Chn4.trigger = FALSE;
                            setBit(NR52,3,FALSE);
                        }
                    }
                }
                soundstate.Chn4.enable = TRUE;
            } else {
                soundstate.Chn4.enable = FALSE;
            }

            /* If a channel is triggered when the frame sequencer's next step is one that doesn't clock 
               the length counter and the length counter is now enabled and length is being set to 64 
               (256 for wave channel) because it was previously zero, it is set to 63 instead */
            if (((value & 0x80) >> 7) && (soundstate.Chn4.length_cnt == 0)) {
                if ((soundstate.Chn4.enable) && (soundstate.framesequencerstep % 2)){
                    soundstate.Chn4.length_cnt = 63;
                } else {
                    soundstate.Chn4.length_cnt = 64;
                }
            }

            /* if the channel's DAC is off, after the above actions occur the channel will be immediately disabled again */ 
            if (soundstate.Chn4.dac == OFF) {
                soundstate.Chn4.trigger = FALSE;
                setBit(NR52,3,FALSE);
            }

            printf("[SND] Lenght after: %d\n",soundstate.Chn4.length_cnt);
            
            break;
        /*************
        * Control    *
        *************/            
        case NR50 : /* 0xNR50 FF24 ALLL BRRR Vin L enable, Left vol, Vin R enable, Right vol */
            memory[NR50] = value;
            soundstate.S01_volume = value & 0x07;
            soundstate.S01_Vin_output = (value & 0x08) >> 3;
            soundstate.S02_volume = (value & 0x70) >> 4;
            soundstate.S02_Vin_output = (value & 0x80) >> 7;
            break;    
        case NR51: /* 0xFF25 NW21 NW21 Left enables, Right enables */    
            memory[NR51] = value;
            break;
        case NR52 : /* 0xFF26 P--- NW21 Power control/status, Channel length statuses */
            if ((value & (1<<7)) == 0) { /* Turn audio processing unit off */
                soundTurnOff();
                /* Writing a 0 to bit position 7 in register $FF26(NR52) disables the power to the audio processing unit */
                soundstate.enable = FALSE;
                /* Unused bits 4,5,6 are 1, all other bits are 0 */
                memory[NR52] = 0x70; 
            } else { /* Turn audio processing unit on */
                soundTurnOn();
                soundstate.enable = TRUE;
                memory[NR52] = 0x80; 
            }
            break;
        default :
            break;
    }
    
    
}



unsigned char soundReadRegister(unsigned short address)
{
    unsigned char data = 0x00;
    
    switch (address) {
        /*************
        * Channel 1  *
        *************/ 
        case NR10 :
            data = memory[NR10] | 0x80;                 /* BIT 7 Not Used               */
            break;
        case NR11 :
            data = memory[NR11] | 0x3F;                 /* Only Bits 7-6 can be read    */
            break;
        case NR12 :
            data = memory[NR12];
            break;
        case NR13 :
            data |= 0xFF;                               /* Cant be read                 */
            break;
        case NR14 :
            data = memory[NR14] | 0xBF;                 /* Only Bit 6 can be read       */
            break;
        /*************
        * Channel 2  *
        *************/
        case NR20 :
            data = memory[NR20];                        /* Not Used                     */
            break;
        case NR21 :
            data = memory[NR21] | 0x3F;                 /* Only bits 7-6 can be read    */
            break;
        case NR22 :
            data = memory[NR22];
            break;
        case NR23 :
            data |= 0xFF;                               /* Cant be read                 */
            break;
        case NR24 :
            data = memory[NR24] | 0xBF;                 /* Only Bit 6 can be read       */
            break;
        /*************
        * Channel 3  *
        *************/
        case NR30 :
            data = memory[NR30] | 0x7F;                 /* BIT 6,5,4,3,2,1,0 Not Used   */
            break;
        case NR31 :
            data |= 0xFF;                               /* Cant be read                 */
            break;
        case NR32 :
            data = memory[NR32] | 0x9F;                 /* BIT 7,4,3,2,1,0 Not Used     */
            break;
        case NR33 :
            data |= 0xFF;                               /* Cant be read                 */
            break;
        case NR34 :
            data = memory[NR34] | 0xBF;                 /* Only Bit 6 can be read       */
            break;
        /*************
        * Channel 4  *
        *************/
        case NR40 :
            data |= 0xFF;                               /* Cant be read                 */
            break;
        case NR41 :
            data |= 0xFF;                               /* Cant be read                 */
            break;
        case NR42 :
            data = memory[NR42];
            break;
        case NR43 :
            data = memory[NR43];
            break;
        case NR44 :
            data = memory[NR44] | 0xBF;                 /* Only Bit 6 can be read       */
            break;
        /*************
        * Control    *
        *************/            
        case NR50 :
            data = memory[NR50];
            break;
        case NR51 :
            data = memory[NR51];
            break;            
        case NR52 :
            /* bits 4-6 are ALWAYS read as 1, regardless of what is written to them */
            data = memory[NR52];
            data |= 0x70;
            break;
        /*************
        * Wave       *
        *************/ 
        case 0xFF30 ... 0xFF3F :        /* WAVE pattern RAM */
            data = memory[address];
            break;
        
        default :
            data = 0xFF;
            break;
    }
    
    //printf("[SND] Read %x from %x, clock=%4d\n",data,address,soundstate.unifiedlengthclock);
    return data;

}

void soundReset(void)
{
    
    printf("[SND] Reset Sound Registers\n");
    
    memory[NR10] = 0x80;
	memory[NR11] = 0x00;
	memory[NR12] = 0x00;
    memory[NR13] = 0xFF;    /* No Change */
	memory[NR14] = 0xBF;
    memory[NR20] = 0xFF;    /* No Change */
	memory[NR21] = 0x3F;
	memory[NR22] = 0x00;
    memory[NR23] = 0xFF;    /* No Change */
	memory[NR24] = 0xBF;
	memory[NR30] = 0x7F;
	memory[NR31] = 0xFF;
	memory[NR32] = 0x9F;
    memory[NR33] = 0xFF;    /* No Change */
	memory[NR34] = 0xBF;
    memory[NR40] = 0xFF;    /* No Change */
	memory[NR41] = 0xFF;
	memory[NR42] = 0x00;
	memory[NR43] = 0x00;
	memory[NR44] = 0xBF;
	memory[NR50] = 0x00;
	memory[NR51] = 0x00;
    memory[NR52] = 0xF0;
    
    soundstate.enable = TRUE;

    /*************
    * Channel 1  *
    *************/ 
    soundstate.Chn1.dac = 0;
    soundstate.Chn1.trigger = 0;
    soundstate.Chn1.enable = 0;
    soundstate.Chn1.frequency = 0;
    soundstate.Chn1.length_cnt = 0;
    soundstate.Chn1.length_cnt_skip = FALSE;
    soundstate.Chn1.wave_duty = 0;
    soundstate.Chn1.envelope.volume = 0;
    soundstate.Chn1.envelope.increasing = 0;
    soundstate.Chn1.envelope.period = 0;
    soundstate.Chn1.sweep.period = 0;
    soundstate.Chn1.sweep.increasing = 0;
    soundstate.Chn1.sweep.shift = 0;
    soundstate.Chn1.sweep.enable = 0;
    soundstate.Chn1.sweep.shadowregister = 0;
    /*************
    * Channel 2  *
    *************/
    soundstate.Chn2.dac = 0;
    soundstate.Chn2.trigger = 0;
    soundstate.Chn2.enable = 0;
    soundstate.Chn2.frequency = 0;
    soundstate.Chn2.length_cnt = 0;
    soundstate.Chn2.length_cnt_skip = FALSE;
    soundstate.Chn2.wave_duty = 0;
    soundstate.Chn2.envelope.volume = 0;
    soundstate.Chn2.envelope.increasing = 0;
    soundstate.Chn2.envelope.period = 0;
    /*************
    * Channel 3  *
    *************/ 
    soundstate.Chn3.dac = 0;
    soundstate.Chn3.trigger = 0;
    soundstate.Chn3.enable = 0;
    soundstate.Chn3.frequency = 0;
    soundstate.Chn3.length_cnt = 0;
    soundstate.Chn3.length_cnt_skip = FALSE;
    soundstate.Chn3.volume = 0;
    /*************
    * Channel 4  *
    *************/
    soundstate.Chn4.dac = 0;
    soundstate.Chn4.trigger = 0;
    soundstate.Chn4.enable = 0;
    soundstate.Chn4.length_cnt = 0;
    soundstate.Chn4.length_cnt_skip = FALSE;
    soundstate.Chn4.noise = 0;
    soundstate.Chn4.envelope.volume = 0;
    soundstate.Chn4.envelope.increasing = 0;
    soundstate.Chn4.envelope.period = 0;
    /*************
    * Control    *
    *************/
    
    soundstate.S01_Vin_output = 0;
    soundstate.S02_Vin_output = 0;
    soundstate.S01_volume = 0;
    soundstate.S02_volume = 0;
    
    soundstate.fivebitcounter = 32;
    soundstate.unifiedlengthclock = 16384;
    soundstate.framesequencerstep = 0;
    soundstate.framesequencerclock = 8192;
}

void soundTurnOn(void)
{
    
}

void soundTurnOff(void)
{
    soundReset();
}