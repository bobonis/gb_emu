#include "sound.h"
#include "memory.h"
#include "definitions.h"
#include <stdio.h>
#include <SDL2/SDL.h>
#include <stdint.h>

#define SDL_BUFFER_SAMPLES (1*1024);

/* Sound */
int samples_left_to_input;
int samples_left_to_output;
unsigned int buffer_next_output_sample;
unsigned int buffer_next_input_sample;
short samplebuffer[GB_BUFFER_SIZE/2];

#define debug_length 0
#define debug_sweep 0
#define debug_duty 0
#define debug_output 0
#define debug_sampler 0

int sampleslost = 0;

typedef struct {

     unsigned int fivebitcounter;
     unsigned int unifiedlengthclock;
     unsigned int framesequencerclock;
     unsigned int framesequencerstep;
     int sampler;
     unsigned int next_buffer_sample;

     struct { /* Master */

        unsigned int enable;
        unsigned char left_volume;
        unsigned char left_Vin_output;
        
        unsigned char right_volume;
        unsigned char right_Vin_output;
     
        int channel4_left_enable;
        int channel3_left_enable;
        int channel2_left_enable;
        int channel1_left_enable;
        int channel4_right_enable;
        int channel3_right_enable;
        int channel2_right_enable;
        int channel1_right_enable;
     } master;


     struct { /* Tone & Sweep */

         unsigned int dac;
         unsigned short frequency;
         unsigned char wave_duty;
         unsigned char volume;
         int timer;
         int output;

         struct {
             unsigned char volume;
             unsigned char increasing;
             unsigned char period;
             unsigned char timer;
             unsigned int enable;
             } envelope;

         struct {
             unsigned char timer;
             unsigned char period;
             unsigned char negate;
             unsigned char shift;
             unsigned int enable;
             unsigned short shadowregister;
             unsigned int negate_flag;
             } sweep;
             
         struct {
             unsigned int period;
             unsigned short counter;
             unsigned char phase;
            } duty;

         struct {
             unsigned int enable;
             int counter;
             unsigned int consecutive;
         } length;
         
        } Chn1;
         
     struct { /* Tone */

         unsigned int dac;
         unsigned short frequency;
         unsigned char wave_duty;
         unsigned char volume;
         int output;

         struct {
             unsigned char volume;
             unsigned char increasing;
             unsigned char period;
             unsigned char timer;
             unsigned int enable;
             } envelope;

         struct {
             unsigned int period;
             unsigned short counter;
             unsigned char phase;
            } duty;

         struct {
             unsigned int enable;
             int counter;
             unsigned int consecutive;
         } length;

         } Chn2;
         
     struct { /* Wave Output */

         unsigned short wave_ram[16];
         unsigned int dac;
         unsigned short frequency;
         unsigned char volume;
         int output;
         
         struct {
             unsigned int enable;
             int counter;
             unsigned int consecutive;
         } length;
         
         } Chn3;

     struct { //Noise

         unsigned int dac;
         unsigned char volume;
         unsigned int frequency;
         int output;

         struct {
             unsigned char volume;
             unsigned char increasing;
             unsigned char period;
             unsigned char timer;
             unsigned int enable;
             } envelope;

         struct {
             unsigned char shift;
             unsigned char width;
             unsigned char ratio;
             } noise;

         struct {
             unsigned int enable;
             int counter;
             unsigned int consecutive;
         } length;

         } Chn4;

} _GB_SOUND_HARDWARE_;

static _GB_SOUND_HARDWARE_ soundstate;

/************************************************************************************************************************
*
*                                             TIMER COUNTERS CLOCKS 
*
************************************************************************************************************************/

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
    soundstate.fivebitcounter -= 4;

    if (soundstate.fivebitcounter == 0) {
        soundTickProgrammableCounter();
        soundstate.fivebitcounter = 4;
    }

    soundstate.framesequencerclock -= 4;

    if (soundstate.framesequencerclock == 0) {
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
                soundTickEnvelope();
                break;
            default :
                break;
        }

        soundstate.framesequencerclock = 8192;
        soundstate.framesequencerstep += 1;
        soundstate.framesequencerstep &= 0x07;  /* 3-bit counter */
    }

    soundTickSampler();
}

/************************************************************************************************************************
*                                             LENGTH COUNTER
************************************************************************************************************************/
void soundTickLenghthCounter(void)
{
    /* On DMG no clocking when sound is disabled */
    if (soundstate.master.enable == FALSE) {
        return;
    }

    /* The clocking of the counter is enabled or disabled depending on the status of
       the 'length counter enable register' (AKA the 'consecutive' bit) */

    /************** Channel 1  **************/
    if ((soundstate.Chn1.length.consecutive == 1) && (soundstate.Chn1.length.counter > 0)) {
        soundstate.Chn1.length.counter -= 1;
        if (debug_length)
            printf(PRINT_BLUE"[SND][1] Length tick %2d, f = %d, t = %d"PRINT_RESET"\n",soundstate.Chn1.length.counter,soundstate.framesequencerstep,soundstate.framesequencerclock);
        if (soundstate.Chn1.length.counter == 0) { /* Length becoming 0 should clear status */
            if (debug_length)
                printf(PRINT_BLUE"[SND][1] Length counter expired"PRINT_RESET"\n");
            channelDisable(1);
        }
    }
    /************** Channel 2  **************/
    if ((soundstate.Chn2.length.consecutive == 1) && (soundstate.Chn2.length.counter > 0)) {
        soundstate.Chn2.length.counter -= 1;
        if (debug_length)
            printf(PRINT_BLUE"[SND][2] Length tick %2d, f = %d, t = %d"PRINT_RESET"\n",soundstate.Chn2.length.counter,soundstate.framesequencerstep,soundstate.framesequencerclock);
        if (soundstate.Chn2.length.counter == 0) { /* Length becoming 0 should clear status */
            if (debug_length)
                printf(PRINT_BLUE"[SND][2] Length counter expired"PRINT_RESET"\n");
            channelDisable(2);
        }
    }
    /************** Channel 3  **************/
    if ((soundstate.Chn3.length.consecutive == 1) && (soundstate.Chn3.length.counter > 0)) {
        soundstate.Chn3.length.counter -= 1;
        if (debug_length)
            printf(PRINT_BLUE"[SND][3] Length tick %2d, f = %d, t = %d"PRINT_RESET"\n",soundstate.Chn3.length.counter,soundstate.framesequencerstep,soundstate.framesequencerclock);
        if (soundstate.Chn3.length.counter == 0) { /* Length becoming 0 should clear status */
            if (debug_length)
                printf(PRINT_BLUE"[SND][3] Length counter expired"PRINT_RESET"\n");
            channelDisable(3);
        }
    }
    /************** Channel 4  **************/
    if ((soundstate.Chn4.length.consecutive == 1) && (soundstate.Chn4.length.counter > 0)) {
        soundstate.Chn4.length.counter -= 1;
        if (debug_length)
            printf(PRINT_BLUE"[SND][4] Length tick %2d, f = %d, t = %d"PRINT_RESET"\n",soundstate.Chn4.length.counter,soundstate.framesequencerstep,soundstate.framesequencerclock);
        if (soundstate.Chn4.length.counter == 0) { /* Length becoming 0 should clear status */
            if (debug_length)
                printf(PRINT_BLUE"[SND][4] Length counter expired"PRINT_RESET"\n");
            channelDisable(4);
        }
    }
}

/************************************************************************************************************************
*                                             SWEEP COUNTER
************************************************************************************************************************/
void soundTickSweepCounter(void) {
 
     /* On DMG no clocking when sound is disabled */
    if (soundstate.master.enable == FALSE) {
        return;
    }
    
    /* When the sweep's internal enabled flag is set and the sweep period is not zero,
       a new frequency is calculated and the overflow check is performed */

    if (soundstate.Chn1.sweep.enable) {
        soundstate.Chn1.sweep.timer -= 1;
        if (soundstate.Chn1.sweep.timer) {
            if (debug_sweep)
                printf(PRINT_GREEN"[SND][1] Sweep tick %2d, f = %d, t = %d"PRINT_RESET"\n",soundstate.Chn1.sweep.timer,soundstate.framesequencerstep,soundstate.framesequencerclock);
            //soundstate.Chn1.sweep.timer -= 1;
            return;
        }
    } else {
        return;
    }
    
    if (debug_sweep){
         printf(PRINT_GREEN"[SND][1] Sweep tick %2d, f = %d, t = %d"PRINT_RESET"\n",soundstate.Chn1.sweep.timer,soundstate.framesequencerstep,soundstate.framesequencerclock);    if ((soundstate.Chn1.sweep.period == 0) && (soundstate.Chn1.sweep.shift == 0))
    //if (debug_sweep)
         printf(PRINT_GREEN"[SND][1] Sweep counter expired"PRINT_RESET"\n");
         }
         //channelDisable(1);
         //soundstate.Chn1.sweep.enable = 0;


    if (soundstate.Chn1.sweep.period) {

        unsigned short frequency = channelCalculateSweepFreq();
        //printf("     [1] Calculated new frequency %d, shift=%d\n",frequency,soundstate.Chn1.sweep.shift);

        if ((frequency <= 2047) && (soundstate.Chn1.sweep.shift)) {
            soundstate.Chn1.sweep.shadowregister = frequency;
            memory[NR13] = frequency & 0x00FF;
            memory[NR14] &= 0xF8;
            memory[NR14] |= ((frequency >> 8) & 0x0007);

            soundstate.Chn1.frequency = frequency;

            channelCalculateSweepFreq();

            //soundstate.Chn1.sweep.timer = soundstate.Chn1.sweep.period << 1;
            //printf("     [1] Sweep timer reloaded %d\n",soundstate.Chn1.sweep.timer);
        }
        soundstate.Chn1.sweep.timer = soundstate.Chn1.sweep.period;

        printf("     [1] Sweep timer reloaded %d\n",soundstate.Chn1.sweep.timer);
    } else {
        soundstate.Chn1.sweep.timer = 8;
        printf("     [1] Sweep timer reloaded %d\n",soundstate.Chn1.sweep.timer);
    }
}

/*
  * Frequency calculation consists of taking the value in the frequency
  * shadow register, shifting it right by sweep shift, optionally negating
  * the value, and summing this with the frequency shadow register to produce
  * a new frequency
  */
unsigned short channelCalculateSweepFreq(void)
{
     unsigned short frequency = soundstate.Chn1.sweep.shadowregister >> soundstate.Chn1.sweep.shift;

     if (soundstate.Chn1.sweep.negate) {
         /* Subtract mode uses two's complement */
         soundstate.Chn1.sweep.negate_flag = TRUE;
         frequency = (~frequency) + 1;
         frequency &= 0x7FF;
         frequency = soundstate.Chn1.sweep.shadowregister + frequency;
         frequency &= 0x7FF;
     } else {
         frequency = soundstate.Chn1.sweep.shadowregister + frequency;
     }
     printf("     [1] Calculated new frequency %d\n",frequency);
     /* if this is greater than 2047, square 1 is disabled */
     if (frequency > 2047) {
         channelDisable(1);
         soundstate.Chn1.sweep.enable = 0;
         printf("*****************************\n");
         //soundstate.Chn1.length.consecutive = FALSE;
     }

     return frequency;
}



/************************************************************************************************************************
*                                             ENVELOPE
************************************************************************************************************************/
void soundTickEnvelope(void)
{
     /* On DMG no clocking when sound is disabled */
     if (soundstate.master.enable == FALSE) {
         return;
     }

     /*************
     * Channel 1  *
     *************/
     if (soundstate.Chn1.length.enable) {
         if ((soundstate.Chn1.envelope.enable) && (soundstate.Chn1.envelope.period)) {
             if (soundstate.Chn1.envelope.timer == 0) {

                 soundstate.Chn1.envelope.timer = soundstate.Chn1.envelope.period;

                 if (soundstate.Chn1.envelope.increasing) {
                     if (soundstate.Chn1.volume < 0x0F) {
                         soundstate.Chn1.volume += 1;
                         /* TODO update stereo volume */
                     } else {
                         soundstate.Chn1.envelope.enable = 0;
                     }
                 } else {
                     if (soundstate.Chn1.volume > 0x00) {
                         soundstate.Chn1.volume -= 1;
                         /* TODO update stereo volume */
                     } else {
                         soundstate.Chn1.envelope.enable = 0;
                     }
                 }
             } else {
                 soundstate.Chn1.envelope.timer -= 1;
             }
         }
     }
     /*************
     * Channel 2  *
     *************/
     if (soundstate.Chn2.length.enable) {
         if ((soundstate.Chn2.envelope.enable) && (soundstate.Chn2.envelope.period)) {
             if (soundstate.Chn2.envelope.timer == 0) {

                 soundstate.Chn2.envelope.timer = soundstate.Chn2.envelope.period;

                 if (soundstate.Chn2.envelope.increasing) {
                     if (soundstate.Chn2.volume < 0x0F) {
                         soundstate.Chn2.volume += 1;
                         /* TODO update stereo volume */
                     } else {
                         soundstate.Chn2.envelope.enable = 0;
                     }
                 } else {
                     if (soundstate.Chn2.volume > 0x00) {
                         soundstate.Chn2.volume -= 1;
                         /* TODO update stereo volume */
                     } else {
                         soundstate.Chn2.envelope.enable = 0;
                     }
                 }
             } else {
                 soundstate.Chn2.envelope.timer -= 1;
             }
         }
     }
     /*************
     * Channel 4  *
     *************/
     if (soundstate.Chn4.length.enable) {
         if ((soundstate.Chn4.envelope.enable) && (soundstate.Chn4.envelope.period)) {
             if (soundstate.Chn4.envelope.timer == 0) {

                 soundstate.Chn4.envelope.timer = soundstate.Chn4.envelope.period;

                 if (soundstate.Chn4.envelope.increasing) {
                     if (soundstate.Chn4.volume < 0x0F) {
                         soundstate.Chn4.volume += 1;
                         /* TODO update stereo volume */
                     } else {
                         soundstate.Chn4.envelope.enable = 0;
                     }
                 } else {
                     if (soundstate.Chn4.volume > 0x00) {
                         soundstate.Chn4.volume -= 1;
                         /* TODO update stereo volume */
                     } else {
                         soundstate.Chn4.envelope.enable = 0;
                     }
                 }
             } else {
                 soundstate.Chn4.envelope.timer -= 1;
             }
         }
     }


}
/************************************************************************************************************************
*                                             FREQUENCY COUNTER
************************************************************************************************************************/
void soundTickProgrammableCounter(void)
{
    /* On DMG no clocking when sound is disabled */
    if (soundstate.master.enable == FALSE) {
        return;
    }

    /************** Channel 1  **************/     
    if (soundstate.Chn1.length.enable == TRUE) {
        if (soundstate.Chn1.duty.counter == 0)
            exit(0);

        soundstate.Chn1.duty.counter -= 1;

        if (soundstate.Chn1.duty.counter == 0) {
            soundstate.Chn1.duty.counter = 2048 - soundstate.Chn1.frequency;
            soundTickDuty(1);
        }
    } 
    
    /************** Channel 2  **************/     
    if (soundstate.Chn2.length.enable == TRUE) {
        if (soundstate.Chn2.duty.counter == 0)
            exit(0);

        soundstate.Chn2.duty.counter -= 1;

        if (soundstate.Chn2.duty.counter == 0) {
            soundstate.Chn2.duty.counter = 2048 - soundstate.Chn2.frequency;
            soundTickDuty(2);
        }
    } 



}




void soundTickDuty(int channel)
{
    int output;
    unsigned char wave_duty = 0;
    unsigned char phase = 0;
    unsigned char volume = 0;
    
    switch (channel) {
        case 1 :    /************** Channel 1  **************/ 
            soundstate.Chn1.duty.phase += 1;
            soundstate.Chn1.duty.phase &= 0x07;
            wave_duty = soundstate.Chn1.wave_duty;
            phase = soundstate.Chn1.duty.phase;
            volume = soundstate.Chn1.volume;
            break;

        case 2 :    /************** Channel 2  **************/ 
            soundstate.Chn2.duty.phase += 1;
            soundstate.Chn2.duty.phase &= 0x07;
            wave_duty = soundstate.Chn2.wave_duty;
            phase = soundstate.Chn2.duty.phase;
            volume = soundstate.Chn1.volume;
            break;

        default :   /************** Channel X  **************/
            break;
    }
    
       
    switch (wave_duty) {
        case 0: output = (phase == 6); break; /* ______-_ */
        case 1: output = (phase >= 6); break; /* ______-- */
        case 2: output = (phase >= 4); break; /* ____---- */
        case 3: output = (phase <= 5); break; /* ------__ */
        default : output = 0; break;
    }

    if (output) {
        output = (int)volume * 127;
    } else {
        output = (int)volume * (-128);
    }
    
    if (debug_duty)
            printf(PRINT_RED"[SND][%d] Duty Output = %5d, Duty = %d, Phase = %d"PRINT_RESET"\n",channel,output,wave_duty,phase);

    switch (channel) {
        case 1 :    /************** Channel 1  **************/ 
            soundstate.Chn1.output = output;
            break;

        case 2 :    /************** Channel 2  **************/ 
            soundstate.Chn2.output = output;
            break;

        default :
            break;
    }
}

/************************************************************************************************************************
*
*                                             MEMORY MANAGEMENT 
*
************************************************************************************************************************/

void soundWriteRegister(unsigned short address,unsigned char value)
{
     printf("[SND] --> Wrote %2x at %x, DAC=%d%d%d%d, frame=%d, time=%4d\n",value,address,soundstate.Chn1.dac,soundstate.Chn2.dac,soundstate.Chn3.dac,soundstate.Chn4.dac,soundstate.framesequencerstep,soundstate.framesequencerclock);

     if ((address >= 0xFF30) && (address <= 0xFF3F)) {
         memory[address] = value;
     }

     /* Any writes to those registers are ignored while power remains off except of the
        length counters that are unaffected by power and can still be written while off */
     if (soundstate.master.enable == FALSE) {
         switch (address) {
             case NR52 :
                 if ((value & (1<<7)) == 0) { /* Turn audio processing unit off */
                     soundTurnOff();
                     /* Writing a 0 to bit position 7 in register $FF26(NR52) disables the power to the audio processing unit */
                     soundstate.master.enable = FALSE;
                     /* Unused bits 4,5,6 are 1, all other bits are 0 */
                     memory[NR52] = 0x70;
                 } else { /* Turn audio processing unit on */
                     soundTurnOn();
                 }
                 break;
             case NR11 :
                 soundstate.Chn1.length.counter = 64 - (value & 0x3F);
                 if (debug_length)
                    printf(PRINT_BLUE"[SND][1] Wrote new lenght = %d"PRINT_RESET"\n",soundstate.Chn1.length.counter);
                 break;
             case NR21 :
                 soundstate.Chn2.length.counter = 64 - (value & 0x3F);
                 break;
             case NR31 :
                 soundstate.Chn3.length.counter = 256 - value;
                 break;
             case NR41 :
                 soundstate.Chn4.length.counter = 64 - (value & 0x3F);
                 break;
             default :
                 break;
         }
         return;
     }

     switch (address) {
         /*************
         * Channel 1  *
         *************/
         case NR10 : /* 0xFF10 -PPP NSSS Sweep period, negate, shift */
             memory[NR10] = value;
             /* The sweep timers treat a period of 0 as 8 */
             soundstate.Chn1.sweep.period = (value & 0x70) >> 4;
             soundstate.Chn1.sweep.negate = (value & 0x08) >> 3;
             soundstate.Chn1.sweep.shift = value & 0x07;
             printf("[SND][1] Update sweep\n");
             printf("     [1] period= %d\n",soundstate.Chn1.sweep.period);
             printf("     [1] negate= %d\n",soundstate.Chn1.sweep.negate);
             printf("     [1] shift= %d\n",soundstate.Chn1.sweep.shift);
             /* Clearing the sweep negate mode bit in NR10 after at least one
                sweep calculation has been made using the negate mode since the
                last trigger causes the channel to be immediately disabled
              */
             if ((soundstate.Chn1.sweep.negate_flag) && !(soundstate.Chn1.sweep.negate)) {
                 channelDisable(1);
             }

             break;
         case NR11 : /* 0xFF11 DDLL LLLL Duty, Length load (64-L) */
             memory[NR11] = value;
             /* The 5-bit length value is subtracted from 64 and the result being written to the counter */
             soundstate.Chn1.length.counter = 64 - (value & 0x3F);
             if (debug_length)
                printf(PRINT_BLUE"[SND][1] Wrote new lenght = %d"PRINT_RESET"\n",soundstate.Chn1.length.counter);
             soundstate.Chn1.wave_duty = (value >> 6) & 0x03;
             break;
         case NR12 : /* 0xFF12 VVVV APPP Starting volume, Envelope add mode, period */

             /* Zombie mode */
             if (soundstate.Chn1.length.enable) {
                 /* If the old envelope period was zero and the envelope is still doing automatic updates, volume is incremented by 1 */
                 if (soundstate.Chn1.envelope.enable && (soundstate.Chn1.envelope.period == 0)) {
                     soundstate.Chn1.volume += 1;
                 /* otherwise if the envelope was in subtract mode, volume is incremented by 2 */
                 } else if (soundstate.Chn1.envelope.increasing == 0) {
                     soundstate.Chn1.volume += 2;
                 }

                 /* If the mode was changed (add to subtract or subtract to add), volume is set to 16-volume */
                 if (soundstate.Chn1.envelope.increasing != ((value & 0x08) >> 3))
                     soundstate.Chn1.volume = 0x10 - soundstate.Chn1.volume;

                 /* Only the low 4 bits of volume are kept after the above operations */
                 soundstate.Chn1.volume &= 0x0F;

                 /* TODO update stereo volume */
             }

             memory[NR12] = value;

             if (!(value & 0xF8)) { /* Disabling DAC should disable channel immediately */
                 soundstate.Chn1.dac = FALSE;
                 channelDisable(1);
             } else { /* Enabling DAC shouldn't re-enable channel */
                 soundstate.Chn1.dac = TRUE;
             }
             break;
         case NR13 : /* 0xFF13 FFFF FFFF Frequency LSB */
             memory[NR13] = value;
             //soundstate.Chn1.frequency = ((memory[NR14] & 0x07) << 8 ) | value;
             printf("[SND][1] Update frequency\n");
             printf("     [1] New frequency = %d\n",value);
             break;
         case NR14 : /* 0xFF14 TL-- -FFF Trigger, Length enable, Frequency MSB */
             memory[NR14] = value;
             //soundstate.Chn1.frequency = ((value & 0x07) << 8 ) | memory[NR13];
             printf("[SND][1] Update frequency\n");
             printf("     [1] New frequency = %d\n",((value & 0x07) << 8 ) | memory[NR13]);

            /* trigger channel */

            if ((value & 0x80) >> 7) {
                channelEnable(1);
            }

            /* lenghth counter */

            if ((value & 0x40) >> 6) {
                
                unsigned int old_consecutive = soundstate.Chn1.length.consecutive;
                soundstate.Chn1.length.consecutive = TRUE;
                /* when the frame sequencer's next step is one that doesn't clock the length counter
                   if the length counter was PREVIOUSLY disabled and now enabled */
                if ((soundstate.framesequencerstep % 2) && (old_consecutive == FALSE)) {
                    /* if the length counter is not zero, it is decremented */
                    if (soundstate.Chn1.length.counter) {
                        soundstate.Chn1.length.counter -= 1;
                        if (debug_length)
                            printf(PRINT_BLUE"[SND][1] Decreased Length = %d"PRINT_RESET"\n",soundstate.Chn1.length.counter);
                        /* If this decrement makes it zero and trigger is clear, the channel is disabled */
                        if (soundstate.Chn1.length.counter == 0){
                            channelDisable(1);
                        }
                    }
                }

            } else {
                soundstate.Chn1.length.consecutive = FALSE;
            }

            /* If a channel is triggered when the frame sequencer's next step is one that doesn't clock
            the length counter and the length counter is now enabled and length is being set to 64
            (256 for wave channel) because it was previously zero, it is set to 63 instead */
            if (( (value & 0x80) >> 7) ){
                if (soundstate.Chn1.length.counter == 0) {
                    if ((soundstate.Chn1.length.consecutive) && (soundstate.framesequencerstep % 2)) {
                        soundstate.Chn1.length.counter = 63;
                    } else {
                        soundstate.Chn1.length.counter = 64;
                    }
                    printf(PRINT_BLUE"[SND][1] Load new Length on trigger = %d"PRINT_RESET"\n",soundstate.Chn1.length.counter);
                }
            }



             if (( (value & 0x80) >> 7)) {

                /* Frequency Sweep */
                
                 printf("[SND][1] ** Trigger channel **\n");
                 soundstate.Chn1.sweep.negate_flag = FALSE;
                 /* Square 1's frequency is copied to the shadow register */
                 soundstate.Chn1.frequency = ((memory[NR14] & 0x07 ) << 8 ) | memory[NR13];
                 soundstate.Chn1.sweep.shadowregister = ((memory[NR14] & 0x07 ) << 8 ) | memory[NR13];
                 printf("     [1] Update shadowregister = %d\n",soundstate.Chn1.frequency);

                 /* The internal enabled flag is set if either the sweep period or shift are non-zero, cleared otherwise */
                 if ((soundstate.Chn1.sweep.period) || (soundstate.Chn1.sweep.shift)) {
                     printf("     [1] Sweep enabled\n");
                     soundstate.Chn1.sweep.enable = TRUE;
                     /* The sweep timer is reloaded */
                     if (soundstate.Chn1.sweep.period) {
                         soundstate.Chn1.sweep.timer = soundstate.Chn1.sweep.period;
                     } else {
                         soundstate.Chn1.sweep.timer = 8;
                     }
                     //if ((soundstate.framesequencerstep == 2) || (soundstate.framesequencerstep == 6))
                       // soundstate.Chn1.sweep.timer -= 1;
                     printf("     [1] Update timer = %d\n",soundstate.Chn1.sweep.timer);
                 } else {
                     printf("     [1] Sweep disabled\n");
                     soundstate.Chn1.sweep.enable = FALSE;
                     soundstate.Chn1.sweep.timer = 0;
                 }

                 /* If the sweep shift is non-zero, frequency calculation and the overflow check are performed immediately */
                 if (soundstate.Chn1.sweep.shift) {
                     unsigned short frequency = channelCalculateSweepFreq();
                     //printf("     [1] Calculated new frequency %d\n",frequency);
                 }

                 /* Envelope Calculation */

                 /* Channel volume is reloaded */
                 soundstate.Chn1.volume = (memory[NR12] >> 4) & 0x0F;
                 soundstate.Chn1.envelope.enable = TRUE;
                 soundstate.Chn1.envelope.increasing = (memory[NR12] & 0x08) >> 3;
                 soundstate.Chn1.envelope.period = memory[NR12] & 0x07;
                 /* Volume envelope timer is reloaded with period */
                 soundstate.Chn1.envelope.timer = soundstate.Chn1.envelope.period;

                 /* Programmable Counter init */

                 soundstate.Chn1.duty.counter = 2048 - soundstate.Chn1.frequency;
                 /* TODO update stereo volume */
             }


             /* if the channel's DAC is off, after the above actions occur the channel will be immediately disabled again */
             if (soundstate.Chn1.dac == OFF) {
                 channelDisable(1);
             }



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
             soundstate.Chn2.length.counter = 64 - (value & 0x3F);
             if (debug_length)
                printf(PRINT_BLUE"[SND][2] Wrote new lenght = %d"PRINT_RESET"\n",soundstate.Chn2.length.counter);
             soundstate.Chn2.wave_duty = (value >> 6) & 0x03;
             break;
         case NR22 : /* 0xFF17 VVVV APPP Starting volume, Envelope add mode, period */

             /* Zombie mode */
             if (soundstate.Chn2.length.enable) {
                 /* If the old envelope period was zero and the envelope is still doing automatic updates, volume is incremented by 1 */
                 if (soundstate.Chn2.envelope.enable && (soundstate.Chn2.envelope.period == 0)) {
                     soundstate.Chn2.volume += 1;
                 /* otherwise if the envelope was in subtract mode, volume is incremented by 2 */
                 } else if (soundstate.Chn2.envelope.increasing == 0) {
                     soundstate.Chn2.volume += 2;
                 }

                 /* If the mode was changed (add to subtract or subtract to add), volume is set to 16-volume */
                 if (soundstate.Chn2.envelope.increasing != ((value & 0x08) >> 3))
                     soundstate.Chn2.volume = 0x10 - soundstate.Chn2.volume;

                 /* Only the low 4 bits of volume are kept after the above operations */
                 soundstate.Chn2.volume &= 0x0F;

                 /* TODO update stereo volume */
             }

             memory[NR22] = value;

             if (!(value & 0xF8)) { /* Disabling DAC should disable channel immediately */
                 soundstate.Chn2.dac = FALSE;
                 channelDisable(2);
             } else { /* Enabling DAC shouldn't re-enable channel */
                 soundstate.Chn2.dac = TRUE;
             }
             break;
         case NR23 : /* 0xFF18 FFFF FFFF Frequency LSB */
             memory[NR23] = value;
             //soundstate.Chn2.frequency = ((memory[NR24] & 0x07) << 8 ) | value;
             break;
         case NR24 : /* 0xFF19 TL-- -FFF Trigger, Length enable, Frequency MSB */
             memory[NR24] = value;
             //soundstate.Chn2.frequency = ((value & 0x07) << 8 ) | memory[NR23];

            /* trigger channel */

            if ((value & 0x80) >> 7) {
                channelEnable(2);
                soundstate.Chn2.frequency = ((memory[NR24] & 0x07 ) << 8 ) | memory[NR23];
                soundstate.Chn2.duty.counter = 2048 - soundstate.Chn2.frequency;
            }

            /* lenghth counter */

            if ((value & 0x40) >> 6) {
                
                unsigned int old_consecutive = soundstate.Chn2.length.consecutive;
                soundstate.Chn2.length.consecutive = TRUE;
                /* when the frame sequencer's next step is one that doesn't clock the length counter
                   if the length counter was PREVIOUSLY disabled and now enabled */
                if ((soundstate.framesequencerstep % 2) && (old_consecutive == FALSE)) {
                    /* if the length counter is not zero, it is decremented */
                    if (soundstate.Chn2.length.counter) {
                        soundstate.Chn2.length.counter -= 1;
                        if (debug_length)
                            printf(PRINT_BLUE"[SND][2] Decreased Length = %d"PRINT_RESET"\n",soundstate.Chn2.length.counter);
                        /* If this decrement makes it zero and trigger is clear, the channel is disabled */
                        if (soundstate.Chn2.length.counter == 0){
                            channelDisable(2);
                        }
                    }
                }

            } else {
                soundstate.Chn2.length.consecutive = FALSE;
            }

            /* If a channel is triggered when the frame sequencer's next step is one that doesn't clock
            the length counter and the length counter is now enabled and length is being set to 64
            (256 for wave channel) because it was previously zero, it is set to 63 instead */
            if (( (value & 0x80) >> 7) ){
                if (soundstate.Chn2.length.counter == 0) {
                    if ((soundstate.Chn2.length.consecutive) && (soundstate.framesequencerstep % 2)) {
                        soundstate.Chn2.length.counter = 63;
                    } else {
                        soundstate.Chn2.length.counter = 64;
                    }
                    printf(PRINT_BLUE"[SND][2] Load new Length on trigger = %d"PRINT_RESET"\n",soundstate.Chn2.length.counter);
                }
            }

             if (( (value & 0x80) >> 7) ){

                 /* Envelope Calculation */

                 /* Channel volume is reloaded */
                 soundstate.Chn2.volume = (memory[NR12] >> 4) & 0x0F;
                 soundstate.Chn2.envelope.enable = TRUE;
                 soundstate.Chn2.envelope.increasing = (memory[NR12] & 0x08) >> 3;
                 soundstate.Chn2.envelope.period = memory[NR12] & 0x07;
                 /* Volume envelope timer is reloaded with period */
                 soundstate.Chn2.envelope.timer = soundstate.Chn2.envelope.period;

                 /* TODO update stereo volume */
             }

             /* if the channel's DAC is off, after the above actions occur the channel will be immediately disabled again */
             if (soundstate.Chn2.dac == OFF) {
                 channelDisable(2);
             }

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
             soundstate.Chn3.length.counter = 256 - value;
             if (debug_length)
                printf(PRINT_BLUE"[SND][3] Wrote new lenght = %d"PRINT_RESET"\n",soundstate.Chn3.length.counter);
             break;
         case NR32 : /* 0xFF1C -VV- ---- Volume code (00=0%, 01=100%, 10=50%, 11=25%) */
             memory[NR32] = value;
             soundstate.Chn3.volume = (value & 0x60) >> 5;
             if (!(value & 0xF8)) { /* Disabling DAC should disable channel immediately */
                 soundstate.Chn3.dac = FALSE;
                 channelDisable(3);
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
             //soundstate.Chn3.frequency = ((value & 0x07) << 8 ) | memory[NR33];

            /* trigger channel */

            if ((value & 0x80) >> 7) {
                channelEnable(3);
            }

            /* lenghth counter */

            if ((value & 0x40) >> 6) {
                
                unsigned int old_consecutive = soundstate.Chn3.length.consecutive;
                soundstate.Chn3.length.consecutive = TRUE;
                /* when the frame sequencer's next step is one that doesn't clock the length counter
                   if the length counter was PREVIOUSLY disabled and now enabled */
                if ((soundstate.framesequencerstep % 2) && (old_consecutive == FALSE)) {
                    /* if the length counter is not zero, it is decremented */
                    if (soundstate.Chn3.length.counter) {
                        soundstate.Chn3.length.counter -= 1;
                        if (debug_length)
                            printf(PRINT_BLUE"[SND][3] Decreased Length = %d"PRINT_RESET"\n",soundstate.Chn3.length.counter);
                        /* If this decrement makes it zero and trigger is clear, the channel is disabled */
                        if (soundstate.Chn3.length.counter == 0){
                            channelDisable(3);
                        }
                    }
                }

            } else {
                soundstate.Chn3.length.consecutive = FALSE;
            }

            /* If a channel is triggered when the frame sequencer's next step is one that doesn't clock
            the length counter and the length counter is now enabled and length is being set to 64
            (256 for wave channel) because it was previously zero, it is set to 63 instead */
            if (( (value & 0x80) >> 7) ){
                if (soundstate.Chn3.length.counter == 0) {
                    if ((soundstate.Chn3.length.consecutive) && (soundstate.framesequencerstep % 2)) {
                        soundstate.Chn3.length.counter = 255;
                    } else {
                        soundstate.Chn3.length.counter = 256;
                    }
                    printf(PRINT_BLUE"[SND][3] Load new Length on trigger = %d"PRINT_RESET"\n",soundstate.Chn3.length.counter);
                }
            }

             /* if the channel's DAC is off, after the above actions occur the channel will be immediately disabled again */
             if (soundstate.Chn3.dac == OFF) {
                 channelDisable(3);
             }

             break;
         /*************
         * Channel 4  *
         *************/
         case NR40 : /* 0xFF1F ---- ---- Not used */
             memory[NR40] = 0xFF;
             break;
         case NR41 : /* 0xFF20 --LL LLLL Length load (64-L) */
             memory[NR41] = value;
             soundstate.Chn4.length.counter = 64 - (value & 0x3F);
             if (debug_length)
                printf(PRINT_BLUE"[SND][4] Wrote new lenght = %d"PRINT_RESET"\n",soundstate.Chn4.length.counter);
             break;
         case NR42 : /* 0xFF21 VVVV APPP Starting volume, Envelope add mode, period */

             /* Zombie mode */
             if (soundstate.Chn4.length.enable) {
                 /* If the old envelope period was zero and the envelope is still doing automatic updates, volume is incremented by 1 */
                 if (soundstate.Chn4.envelope.enable && (soundstate.Chn4.envelope.period == 0)) {
                     soundstate.Chn4.volume += 1;
                 /* otherwise if the envelope was in subtract mode, volume is incremented by 2 */
                 } else if (soundstate.Chn4.envelope.increasing == 0) {
                     soundstate.Chn4.volume += 2;
                 }

                 /* If the mode was changed (add to subtract or subtract to add), volume is set to 16-volume */
                 if (soundstate.Chn4.envelope.increasing != ((value & 0x08) >> 3))
                     soundstate.Chn4.volume = 0x10 - soundstate.Chn4.volume;

                 /* Only the low 4 bits of volume are kept after the above operations */
                 soundstate.Chn4.volume &= 0x0F;

                 /* TODO update stereo volume */
             }

             memory[NR42] = value;

             if (!(value & 0xF8)) { /* Disabling DAC should disable channel immediately */
                 soundstate.Chn4.dac = FALSE;
                 channelDisable(4);
             } else { /* Enabling DAC shouldn't re-enable channel */
                 soundstate.Chn4.dac = TRUE;
             }
             break;
         case NR43 : /* 0xFF22 SSSS WDDD Clock shift, Width mode of LFSR, Divisor code */
             memory[NR43] = value;
             soundstate.Chn4.noise.shift = (value & 0xF0) >> 4;
             soundstate.Chn4.noise.width = (value & 0x08) >> 3;
             soundstate.Chn4.noise.ratio = (value & 0x07);

             /* Using a noise channel clock shift of 14 or 15 results in the LFSR receiving no clocks */
             if (soundstate.Chn4.noise.shift > 13) {
                soundstate.Chn4.frequency = 0;
                return;
             }

             /* Frequency = 524288 Hz / r / 2^(s+1)    For r=0 use r=0.5 instead */            
             const unsigned int NoiseFreqRatio[8] = {1048576,524288,262144,174763,131072,104858,87381,74898 };
             soundstate.Chn4.frequency = NoiseFreqRatio[soundstate.Chn4.noise.ratio] >> (soundstate.Chn4.noise.shift + 1);
            
             //if(Sound.Chn4.outfreq > (1<<18)) Sound.Chn4.outfreq = 1<<18;


             break;
         case NR44 : /* 0xFF23 TL-- ---- Trigger, Length enable */
             memory[NR44] = value;

            /* trigger channel */

            if ((value & 0x80) >> 7) {
                channelEnable(4);
            }

            /* lenghth counter */

            if ((value & 0x40) >> 6) {
                
                unsigned int old_consecutive = soundstate.Chn4.length.consecutive;
                soundstate.Chn4.length.consecutive = TRUE;
                /* when the frame sequencer's next step is one that doesn't clock the length counter
                   if the length counter was PREVIOUSLY disabled and now enabled */
                if ((soundstate.framesequencerstep % 2) && (old_consecutive == FALSE)) {
                    /* if the length counter is not zero, it is decremented */
                    if (soundstate.Chn4.length.counter) {
                        soundstate.Chn4.length.counter -= 1;
                        if (debug_length)
                            printf(PRINT_BLUE"[SND][4] Decreased Length = %d"PRINT_RESET"\n",soundstate.Chn4.length.counter);
                        /* If this decrement makes it zero and trigger is clear, the channel is disabled */
                        if (soundstate.Chn4.length.counter == 0){
                            channelDisable(4);
                        }
                    }
                }

            } else {
                soundstate.Chn4.length.consecutive = FALSE;
            }

            /* If a channel is triggered when the frame sequencer's next step is one that doesn't clock
            the length counter and the length counter is now enabled and length is being set to 64
            (256 for wave channel) because it was previously zero, it is set to 63 instead */
            if (( (value & 0x80) >> 7) ){
                if (soundstate.Chn4.length.counter == 0) {
                    if ((soundstate.Chn4.length.consecutive) && (soundstate.framesequencerstep % 2)) {
                        soundstate.Chn4.length.counter = 63;
                    } else {
                        soundstate.Chn4.length.counter = 64;
                    }
                    printf(PRINT_BLUE"[SND][4] Load new Length on trigger = %d"PRINT_RESET"\n",soundstate.Chn4.length.counter);
                }
            }



             if (( (value & 0x80) >> 7) ){

                 /* Envelope Calculation */

                 /* Channel volume is reloaded */
                 soundstate.Chn4.volume = (memory[NR12] >> 4) & 0x0F;
                 soundstate.Chn4.envelope.enable = TRUE;
                 soundstate.Chn4.envelope.increasing = (memory[NR12] & 0x08) >> 3;
                 soundstate.Chn4.envelope.period = memory[NR12] & 0x07;
                 /* Volume envelope timer is reloaded with period */
                 soundstate.Chn4.envelope.timer = soundstate.Chn4.envelope.period;

                 /* TODO update stereo volume */
             }

             /* if the channel's DAC is off, after the above actions occur the channel will be immediately disabled again */
             if (soundstate.Chn4.dac == OFF) {
                 channelDisable(4);
             }


             break;
         /*************
         * Control    *
         *************/
         case NR50 : /* 0xNR50 FF24 ALLL BRRR Vin L enable, Left vol, Vin R enable, Right vol */
             memory[NR50] = value;
             soundstate.master.right_volume = value & 0x07;
             soundstate.master.right_Vin_output = (value & 0x08) >> 3;
             soundstate.master.left_volume = (value & 0x70) >> 4;
             soundstate.master.left_Vin_output = (value & 0x80) >> 7;
             break;
         case NR51: /* 0xFF25 NW21 NW21 Left enables, Right enables */
             memory[NR51] = value;
             soundstate.master.channel4_left_enable = (value & 0x80) >> 7;
             soundstate.master.channel3_left_enable = (value & 0x40) >> 6;
             soundstate.master.channel2_left_enable = (value & 0x20) >> 5;
             soundstate.master.channel1_left_enable = (value & 0x10) >> 4;
             soundstate.master.channel4_right_enable = (value & 0x08) >> 3;
             soundstate.master.channel3_right_enable = (value & 0x04) >> 2;
             soundstate.master.channel2_right_enable = (value & 0x02) >> 1;
             soundstate.master.channel1_right_enable = (value & 0x01);
             break;
         case NR52 : /* 0xFF26 P--- NW21 Power control/status, Channel length statuses */
             if ((value & (1<<7)) == 0) { /* Turn audio processing unit off */
                 soundTurnOff();
                 /* Writing a 0 to bit position 7 in register $FF26(NR52) disables the power to the audio processing unit */
                 soundstate.master.enable = FALSE;
                 /* Unused bits 4,5,6 are 1, all other bits are 0 */
                 memory[NR52] = 0x70;
             } else { /* Turn audio processing unit on */
                 soundTurnOn();

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
     printf("[SND][MEM] Read %x from %x, DAC=%d%d%d%d, frame=%d, time=%4d\n",data,address,soundstate.Chn1.dac,soundstate.Chn2.dac,soundstate.Chn3.dac,soundstate.Chn4.dac,soundstate.framesequencerstep,soundstate.framesequencerclock);
     return data;
}

/************************************************************************************************************************
*
*                                             INITIALIZATIONS 
*
************************************************************************************************************************/

void soundReset(void)
{

     printf("[SND] Reset Sound Registers\n");
     soundResetBufferPointers();
     soundstate.master.enable = TRUE;
     soundResetRegisters();
     soundResetControl();
     soundResetChannel(1);
     soundResetChannel(2);
     soundResetChannel(3);
     soundResetChannel(4);
   
     soundstate.next_buffer_sample = 0;
     soundstate.framesequencerstep = 0;
     soundstate.framesequencerclock = 8192;
     soundstate.fivebitcounter = 32;
}

void soundResetRegisters(void)
{
     memory[NR10] = 0x80;
     memory[NR11] = 0x00;
     memory[NR12] = 0x00;
     memory[NR13] = 0x00;    /* No Change */
     memory[NR14] = 0xBF;
     memory[NR20] = 0xFF;    /* No Change */
     memory[NR21] = 0x3F;
     memory[NR22] = 0x00;
     memory[NR23] = 0x00;    /* No Change */
     memory[NR24] = 0xBF;
     memory[NR30] = 0x7F;
     memory[NR31] = 0xFF;
     memory[NR32] = 0x9F;
     memory[NR33] = 0x00;    /* No Change */
     memory[NR34] = 0xBF;
     memory[NR40] = 0xFF;    /* No Change */
     memory[NR41] = 0xFF;
     memory[NR42] = 0x00;
     memory[NR43] = 0x00;
     memory[NR44] = 0xBF;
     memory[NR50] = 0x00;
     memory[NR51] = 0x00;
     memory[NR52] = 0xF0;
}

void soundResetControl()
{
     /*************
     * Control    *
     *************/

     soundstate.master.right_Vin_output = 0;
     soundstate.master.left_Vin_output = 0;
     soundstate.master.right_volume = 0;
     soundstate.master.left_volume = 0;

     //soundstate.fivebitcounter = 32;
     //soundstate.unifiedlengthclock = 16384;
     //soundstate.framesequencerstep = 0;
     //soundstate.framesequencerclock = 8192;
     soundstate.sampler = 0;
     
     /*
     soundstate.master.channel4_left_enable = 0;
     soundstate.master.channel3_left_enable = 0;
     soundstate.master.channel2_left_enable = 0;
     soundstate.master.channel1_left_enable = 0;
     soundstate.master.channel4_right_enable = 0;
     soundstate.master.channel3_right_enable = 0;
     soundstate.master.channel2_right_enable = 0;
     soundstate.master.channel1_right_enable = 0;
     */
}

void soundResetChannel(unsigned int channel)
{
     switch (channel) {
         case 1 :
     /*************
     * Channel 1  *
     *************/
     soundstate.Chn1.dac = 0;
     soundstate.Chn1.frequency = 0;
     soundstate.Chn1.wave_duty = 0;
     soundstate.Chn1.output = 0;
     soundstate.Chn1.envelope.increasing = 0;
     soundstate.Chn1.envelope.period = 0;
     soundstate.Chn1.envelope.timer = 0;
     soundstate.Chn1.envelope.enable = 0;
     soundstate.Chn1.sweep.timer = 0;
     soundstate.Chn1.sweep.period = 0;
     soundstate.Chn1.sweep.negate = 0;
     soundstate.Chn1.sweep.negate_flag = 0;
     soundstate.Chn1.sweep.shift = 0;
     soundstate.Chn1.sweep.enable = 0;
     soundstate.Chn1.sweep.shadowregister = 0;
     soundstate.Chn1.duty.period = 0;
     soundstate.Chn1.duty.phase = 0;
     soundstate.Chn1.duty.counter = 0;
     soundstate.Chn1.timer = 0;
     soundstate.Chn1.volume = 0;
     soundstate.Chn1.length.enable = 0;
     //soundstate.Chn1.length.counter = 0;
     //soundstate.Chn1.length.consecutive = 0;
             break;
         case 2 :
     /*************
     * Channel 2  *
     *************/
     soundstate.Chn2.dac = 0;
     soundstate.Chn2.frequency = 0;
     soundstate.Chn2.wave_duty = 0;
     soundstate.Chn2.output = 0;
     soundstate.Chn2.envelope.volume = 0;
     soundstate.Chn2.envelope.increasing = 0;
     soundstate.Chn2.envelope.period = 0;
     soundstate.Chn2.envelope.timer = 0;
     soundstate.Chn2.envelope.enable = 0;
     soundstate.Chn2.duty.period = 0;
     soundstate.Chn2.duty.phase = 0;
     soundstate.Chn2.duty.counter = 0;
     soundstate.Chn2.volume = 0;
     soundstate.Chn2.length.enable = 0;
     //soundstate.Chn2.length.counter = 0;
     //soundstate.Chn2.length.consecutive = 0;
             break;
         case 3 :
     /*************
     * Channel 3  *
     *************/
     soundstate.Chn3.dac = 0;
     soundstate.Chn3.frequency = 0;
     soundstate.Chn3.volume = 0;
     soundstate.Chn3.output = 0;
     soundstate.Chn3.length.enable = 0;
     //soundstate.Chn3.length.counter = 0;
     //soundstate.Chn3.length.consecutive = 0;
             break;
         case 4 :
     /*************
     * Channel 4  *
     *************/
     soundstate.Chn4.dac = 0;
     /* On system reset, this shift register is loaded with a value of 1 */
     soundstate.Chn4.noise.shift = 1;
     soundstate.Chn4.noise.width = 0;
     soundstate.Chn4.noise.ratio = 0;
     soundstate.Chn4.envelope.volume = 0;
     soundstate.Chn4.envelope.increasing = 0;
     soundstate.Chn4.envelope.period = 0;
     soundstate.Chn4.envelope.timer = 0;
     soundstate.Chn4.envelope.enable = 0;
     soundstate.Chn4.volume = 0;
     soundstate.Chn4.output = 0;
     soundstate.Chn4.frequency = 0;
     soundstate.Chn4.length.enable = 0;
     //soundstate.Chn4.length.counter = 0;
     //soundstate.Chn4.length.consecutive = 0;
             break;
         default :
             break;
     }
}

void soundTurnOn(void)
{
     if (soundstate.master.enable == FALSE) {
         soundstate.master.enable = TRUE;
         memory[NR52] = 0x80;
         printf("[SND] ##### Sound Enabled #####\n");
         soundstate.framesequencerstep = 0;
         soundstate.framesequencerclock = 8192;
         soundstate.sampler = 0;

     }

}

void soundTurnOff(void)
{
     printf("[SND][MASTER] Sound Disabled\n");
     
     soundResetRegisters();
     soundResetControl();
     soundResetBufferPointers();
     soundResetChannel(1);
     soundResetChannel(2);
     soundResetChannel(3);
     soundResetChannel(4);
}


void channelEnable(unsigned int channel)
{
    printf("[SND][%d] Channel Enabled\n",channel);
    switch (channel) {
        case 1 :
            soundstate.Chn1.length.enable = TRUE;
            setBit(NR52,0,TRUE);
            break;
        case 2 :
            soundstate.Chn2.length.enable = TRUE;
            setBit(NR52,1,TRUE);
            break;
        case 3 :
            soundstate.Chn3.length.enable = TRUE;
            setBit(NR52,2,TRUE);
            break;
        case 4 :
            soundstate.Chn4.length.enable = TRUE;
            setBit(NR52,3,TRUE);
            break;
        default :
            break;
     }
}


void channelDisable(unsigned int channel)
{
    printf("[SND][%d] Channel Disabled\n",channel);
    switch (channel) {
        case 1 :
            soundstate.Chn1.length.enable = FALSE;
            soundstate.Chn1.envelope.enable = FALSE;
            setBit(NR52,0,FALSE);
            break;
        case 2 :
            soundstate.Chn2.length.enable = FALSE;
            soundstate.Chn2.envelope.enable = FALSE;
            setBit(NR52,1,FALSE);
            break;
        case 3 :
            soundstate.Chn3.length.enable = FALSE;
            setBit(NR52,2,FALSE);
            break;
        case 4 :
            soundstate.Chn4.length.enable = FALSE;
            soundstate.Chn4.envelope.enable = FALSE;
            setBit(NR52,3,FALSE);
            break;
        default :
            break;
     }
}



/************************************************************************************************************************
*
*                                             SAMPLING FUNCTIONS 
*
************************************************************************************************************************/

void soundTickSampler(void)
{
    /* On DMG no clocking when sound is disabled */
    if (soundstate.master.enable == FALSE) {
        return;
    }

    soundstate.sampler += 4;
    
    //4194304 Hz CPU / 22050 Hz sound output.
    static int samplerate = CLOCKSPEED / AUDIO_FREQUENCY;

    if (samples_left_to_output > samples_left_to_input - (GB_BUFFER_SAMPLES/2)) {
        if (soundstate.sampler > (samplerate + 4)) {
            soundstate.sampler -= (samplerate + 4);
            soundMix();
        }
    } else {
        if (soundstate.sampler > (samplerate - 4)) {
            soundstate.sampler -= (samplerate - 4);
            soundMix();
        }
    }

}

void soundMix(void)
{

    if (samples_left_to_input < 1) {
        sampleslost += 1;
        return;
    }
    
    samples_left_to_input--;
    samples_left_to_output++;

   // printf("[SND][X] Samples input - %d, Samples output - %d\n",samples_left_to_input,samples_left_to_output);

    if (soundstate.master.enable == 0) {
        samplebuffer[buffer_next_input_sample++] = 0;
        samplebuffer[buffer_next_input_sample++] = 0;
        buffer_next_input_sample &= GB_BUFFER_SAMPLES-1;
        return;
    }
    
    /* mixing */  
    
    short left_sample = 0;
    short right_sample = 0;
    
    if(soundstate.master.channel1_left_enable) left_sample += soundstate.Chn1.output;
    if(soundstate.master.channel2_left_enable) left_sample += soundstate.Chn2.output;
    if(soundstate.master.channel3_left_enable) left_sample += soundstate.Chn3.output;
    if(soundstate.master.channel4_left_enable) left_sample += soundstate.Chn4.output;
    
    //left_sample = (left_sample * 512) - 16384;
    left_sample *= soundstate.master.left_volume;

//    switch(soundstate.master.left_volume) {
//        case 0: left_sample >>= 3;                                      break;  /* 12.5% */
//        case 1: left_sample >>= 2;                                      break;  /* 25.0% */
//        case 2: left_sample = (left_sample >> 2) + (left_sample >> 3);  break;  /* 37.5% */
//        case 3: left_sample >>= 1;                                      break;  /* 50.0% */
//        case 4: left_sample = (left_sample >> 1) + (left_sample >> 3);  break;  /* 62.5% */
//        case 5: left_sample -= (left_sample >> 2);                      break;  /* 75.0% */
//        case 6: left_sample -= (left_sample >> 3);                      break;  /* 87.5% */
//        default :                                                       break;  /*100.0% */
//    }
    
    if(soundstate.master.channel1_right_enable) right_sample += soundstate.Chn1.output;
    if(soundstate.master.channel2_right_enable) right_sample += soundstate.Chn2.output;
    if(soundstate.master.channel3_right_enable) right_sample += soundstate.Chn3.output;
    if(soundstate.master.channel4_right_enable) right_sample += soundstate.Chn4.output;
    
    //right_sample = (right_sample * 512) - 16384;
    right_sample *= soundstate.master.right_volume;
//    switch(soundstate.master.right_volume) {
//        case 0: right_sample >>= 3;                                         break;  /* 12.5% */
//        case 1: right_sample >>= 2;                                         break;  /* 25.0% */
//        case 2: right_sample = (right_sample >> 2) + (right_sample >> 3);   break;  /* 37.5% */
//        case 3: right_sample >>= 1;                                         break;  /* 50.0% */
//        case 4: right_sample = (right_sample >> 1) + (right_sample >> 3);   break;  /* 62.5% */
//        case 5: right_sample -= (right_sample >> 2);                        break;  /* 75.0% */
//        case 6: right_sample -= (right_sample >> 3);                        break;  /* 87.5% */
//        default :                                                           break;  /*100.0% */
//    }

  
    //printf("[SOUND] Left Sample: %d -- Right Sample: %d\n",left_sample,right_sample);

    samplebuffer[buffer_next_input_sample++] = left_sample;
    samplebuffer[buffer_next_input_sample++] = right_sample;
    buffer_next_input_sample &= GB_BUFFER_SAMPLES-1;

    if (debug_sampler)
        printf(PRINT_MAGENTA"[SND][X] Master Output : L = %5d - R = %5d, Sample = %3d"PRINT_RESET"\n",left_sample,right_sample,buffer_next_input_sample);

}



/************************************************************************************************************************
*
*                                             PLAYBACK FUNCTIONS 
*
************************************************************************************************************************/
void audioInit(void)
{

    if ( SDL_Init( SDL_INIT_AUDIO ) < 0 ) { 
        printf( "[SDL][AUDIO] Failed to initialize: %s\n", SDL_GetError() );
        return; 
    }
    
    /* print the list of audio backends */
    int numAudioDrivers = SDL_GetNumAudioDrivers();
    int i;
    printf("[SDL][AUDIO] %d audio backends compiled into SDL:\n", numAudioDrivers);
    for(i=0;i<numAudioDrivers;i++) {
        printf("[SDL][    %d] %s\n", i,SDL_GetAudioDriver(i));
    }

    int numAudioDevices = SDL_GetNumAudioDevices(0);
    printf("[SDL][AUDIO] %d audio devices reported from SDL:\n", numAudioDevices);
    for (i = 0; i < numAudioDevices; ++i) {
        printf("[SDL][    %d] %s\n", i + 1, SDL_GetAudioDeviceName(i, 0));
    }

    /* print the audio driver we are using */
    printf("[SDL][AUDIO] Audio driver: %s\n", SDL_GetCurrentAudioDriver());

    SDL_AudioSpec want, have;
    SDL_zero(want);

    want.freq = AUDIO_FREQUENCY;
    want.format = AUDIO_S16SYS;
    want.channels = 2; /* 1, 2, 4, or 6 */
    want.samples = SDL_BUFFER_SAMPLES; /* power of 2, or 0 and env SDL_AUDIO_SAMPLES is used */
    want.callback = &update_stream; /* can not be NULL */   
    want.userdata = NULL;

    printf("[SDL][AUDIO] Desired  - frequency: %d format: f %d s %d be %d sz %d channels: %d samples: %d\n", 
    want.freq, SDL_AUDIO_ISFLOAT(want.format), SDL_AUDIO_ISSIGNED(want.format), SDL_AUDIO_ISBIGENDIAN(want.format), SDL_AUDIO_BITSIZE(want.format), want.channels, want.samples);

/*
    if (SDL_OpenAudio(&audiospec, &audiospec_init) < 0)
    {
        printf("[ERROR] Couldn't open audio: %s\n", SDL_GetError());
        return;
    }
*/
    /* open audio device, allowing any changes to the specification */
    SDL_AudioDeviceID dev = SDL_OpenAudioDevice(NULL, 0, &want, &have, SDL_AUDIO_ALLOW_ANY_CHANGE);
    /* print the audio driver we are using */
    printf("[SDL][AUDIO] Selected audio driver: %s\n", SDL_GetCurrentAudioDriver());

    if(!dev) {
        printf("[SDL][AUDIO] Failed to open audio device: %s\n", SDL_GetError());
        SDL_Quit();
        return;
    }

    printf("[SDL][AUDIO] Obtained - frequency: %d format: f %d s %d be %d sz %d channels: %d samples: %d\n", 
    have.freq, SDL_AUDIO_ISFLOAT(have.format), SDL_AUDIO_ISSIGNED(have.format), SDL_AUDIO_ISBIGENDIAN(have.format), SDL_AUDIO_BITSIZE(have.format), have.channels, have.samples);
    
    SDL_PauseAudioDevice(dev, 0); /* play! */
    //SDL_PauseAudio(0);
}


void update_stream(void * userdata, unsigned char * stream, int len)
{
    //memset(stream, 0, len/2);
    if (samples_left_to_output < len/4){
        printf("[SND][X] Update stream without enough samples: have - %d, want - %d\n",samples_left_to_output, len/4);
        return;
    } 
    
    if(soundstate.master.enable == 0) {
        memset(stream,0,(size_t)len);
        return;
    }

    
    samples_left_to_input += len/4;
    //printf("[SND][X] Samples left to input = %d\n", samples_left_to_input);
    samples_left_to_output -= len/4;

    if (debug_output)
        printf(PRINT_RED"[SND][X] Samples : Out = %5d - In = %5d,lost = %d"PRINT_RESET"\n",samples_left_to_output,samples_left_to_input,sampleslost);
    
    sampleslost = 0;
    int i;
    int16_t * buf = (int16_t *)stream;

    
    for (i = 0; i < len/2; i++) {
        buf[i] = samplebuffer[buffer_next_output_sample++];
        buffer_next_output_sample &= GB_BUFFER_SAMPLES-1;
    }
}

void soundResetBufferPointers(void)
{
    buffer_next_input_sample = 0;
    buffer_next_output_sample = 0;
    samples_left_to_input = GB_BUFFER_SAMPLES;
    samples_left_to_output = 0;
}

