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
#include <SDL/SDL.h>

#define SDL_BUFFER_SAMPLES (1024);

/* Sound */
int samples_left_to_input;
int samples_left_to_output;
unsigned int buffer_next_output_sample;
unsigned int buffer_next_input_sample;
short samplebuffer[GB_BUFFER_SIZE/2];


typedef struct {

     unsigned int enable;

     unsigned int fivebitcounter;
     unsigned int unifiedlengthclock;
     unsigned int framesequencerclock;
     unsigned int framesequencerstep;
     unsigned int sampler;
     unsigned int next_buffer_sample;

     struct { /* Master */
     
        unsigned char left_volume;
        unsigned char left_Vin_output;
        
        unsigned char right_volume;
        unsigned char right_Vin_output;
     
        unsigned int channel4_left_enable;
        unsigned int channel3_left_enable;
        unsigned int channel2_left_enable;
        unsigned int channel1_left_enable;
        unsigned int channel4_right_enable;
        unsigned int channel3_right_enable;
        unsigned int channel2_right_enable;
        unsigned int channel1_right_enable;
     } master;


     struct { /* Tone & Sweep */

         unsigned int dac;
         unsigned int trigger;
         unsigned int enable;
         unsigned short frequency;
         unsigned char length_cnt;
         unsigned int length_cnt_skip;
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
             unsigned char phase;
            } duty;
         
        } Chn1;
         
     struct { /* Tone */

         unsigned int dac;
         unsigned int trigger;
         unsigned int enable;
         unsigned short frequency;
         unsigned char length_cnt;
         unsigned int length_cnt_skip;
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
             unsigned char phase;
            } duty;
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
         int output;
         } Chn3;

     struct { //Noise

         unsigned int dac;
         unsigned int trigger;
         unsigned int enable;
         unsigned char length_cnt;
         unsigned int length_cnt_skip;
         unsigned char noise;
         unsigned char volume;
         int output;

         struct {
             unsigned char volume;
             unsigned char increasing;
             unsigned char period;
             unsigned char timer;
             unsigned int enable;
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
    
    //soundTickSampler();
    
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
                 soundTickEnvelope();
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
     soundTickDuty();
     //run
     soundTickSampler();
}

void soundTickSampler(void)
{
    soundstate.sampler += 4;
    //4194304 Hz CPU / 22050 Hz sound output.
    if (samples_left_to_output > samples_left_to_input - (GB_BUFFER_SAMPLES/2)) {
        if (soundstate.sampler > (191+4)) {
            soundstate.sampler -= (191+4);
            soundMix();
            
        }
    } else {
        if (soundstate.sampler > (191-4) ) {
            soundstate.sampler -= (191-4);
            soundMix();
        }
    }
}

void soundMix(void)
{
    if (samples_left_to_input < 1) return;
    
    samples_left_to_input--;
    samples_left_to_output++;
    
    if (soundstate.enable == 0) {
        samplebuffer[buffer_next_input_sample++] = 0;
        samplebuffer[buffer_next_input_sample++] = 0;
        buffer_next_input_sample &= GB_BUFFER_SAMPLES-1;
        return;
    }
    
    /* mixing */  
    
    int left_sample = 0;
    int right_sample = 0;
    
    if(soundstate.master.channel1_left_enable) left_sample += soundstate.Chn1.output;
    if(soundstate.master.channel2_left_enable) left_sample += soundstate.Chn1.output;
    if(soundstate.master.channel3_left_enable) left_sample += soundstate.Chn1.output;
    if(soundstate.master.channel4_left_enable) left_sample += soundstate.Chn1.output;
    
    left_sample = (left_sample * 512) - 16384;

    switch(soundstate.master.left_volume) {
        case 0: left_sample >>= 3;                                      break;  /* 12.5% */
        case 1: left_sample >>= 2;                                      break;  /* 25.0% */
        case 2: left_sample = (left_sample >> 2) + (left_sample >> 3);  break;  /* 37.5% */
        case 3: left_sample >>= 1;                                      break;  /* 50.0% */
        case 4: left_sample = (left_sample >> 1) + (left_sample >> 3);  break;  /* 62.5% */
        case 5: left_sample -= (left_sample >> 2);                      break;  /* 75.0% */
        case 6: left_sample -= (left_sample >> 3);                      break;  /* 87.5% */
        default :                                                       break;  /*100.0% */
    }
    
    if(soundstate.master.channel1_right_enable) right_sample += soundstate.Chn1.output;
    if(soundstate.master.channel2_right_enable) right_sample += soundstate.Chn1.output;
    if(soundstate.master.channel3_right_enable) right_sample += soundstate.Chn1.output;
    if(soundstate.master.channel4_right_enable) right_sample += soundstate.Chn1.output;
    
    right_sample = (right_sample * 512) - 16384;

    switch(soundstate.master.left_volume) {
        case 0: right_sample >>= 3;                                         break;  /* 12.5% */
        case 1: right_sample >>= 2;                                         break;  /* 25.0% */
        case 2: right_sample = (right_sample >> 2) + (right_sample >> 3);   break;  /* 37.5% */
        case 3: right_sample >>= 1;                                         break;  /* 50.0% */
        case 4: right_sample = (right_sample >> 1) + (right_sample >> 3);   break;  /* 62.5% */
        case 5: right_sample -= (right_sample >> 2);                        break;  /* 75.0% */
        case 6: right_sample -= (right_sample >> 3);                        break;  /* 87.5% */
        default :                                                           break;  /*100.0% */
    }

  
    //printf("[SOUND] Left Sample: %d -- Right Sample: %d\n",left_sample,right_sample);
    samplebuffer[buffer_next_input_sample++] = left_sample;
    samplebuffer[buffer_next_input_sample++] = right_sample;
    buffer_next_input_sample &= GB_BUFFER_SAMPLES-1;

}

void soundTickDuty(void)
{
    unsigned int output = 0;
    /*************
    * Channel 1  *
    *************/
    if (soundstate.Chn1.duty.period &&(--soundstate.Chn1.duty.period == 0)) {
        soundstate.Chn1.duty.period = 4 * (2048 - soundstate.Chn1.frequency);
        soundstate.Chn1.duty.phase += 1;
        soundstate.Chn1.duty.phase &= 0x07;
        
        switch (soundstate.Chn1.wave_duty) {
            case 0: output = (soundstate.Chn1.duty.phase == 6); break; /* ______-_ */
            case 1: output = (soundstate.Chn1.duty.phase >= 6); break; /* ______-- */
            case 2: output = (soundstate.Chn1.duty.phase >= 4); break; /* ____---- */
            case 3: output = (soundstate.Chn1.duty.phase <= 5); break; /* ------__ */
            default : output = 0; break;
        }
        printf("[SOUND DUTY] period = %d\n",soundstate.Chn1.duty.period);
    }
    
    soundstate.Chn1.output = 0;
    
    if (soundstate.Chn1.enable) {
        if (output) {
            soundstate.Chn1.output = soundstate.Chn1.volume;
            printf("%d", soundstate.Chn1.output);
        }
    }
    
    /*************
    * Channel 2  *
    *************/
    output = 0;
    if (soundstate.Chn2.duty.period &&(--soundstate.Chn2.duty.period == 0)) {
        soundstate.Chn2.duty.period = 4 * (2048 - soundstate.Chn2.frequency);
        soundstate.Chn2.duty.phase += 1;
        soundstate.Chn2.duty.phase &= 0x07;
        
        switch (soundstate.Chn2.wave_duty) {
            case 0: output = (soundstate.Chn2.duty.phase == 6); break; /* ______-_ */
            case 1: output = (soundstate.Chn2.duty.phase >= 6); break; /* ______-- */
            case 2: output = (soundstate.Chn2.duty.phase >= 4); break; /* ____---- */
            case 3: output = (soundstate.Chn2.duty.phase <= 5); break; /* ------__ */
            default : output = 0; break;
        }
    }
    
    soundstate.Chn2.output = 0;
    
    if (soundstate.Chn2.enable) {
        if (output) {
            soundstate.Chn2.output = soundstate.Chn2.volume;
        }
    }
}


void soundTickEnvelope(void)
{
     /* On DMG no clocking when sound is disabled */
     if (soundstate.enable == FALSE) {
         return;
     }

     /*************
     * Channel 1  *
     *************/
     if (soundstate.Chn1.enable) {
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
     if (soundstate.Chn2.enable) {
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
     if (soundstate.Chn4.enable) {
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


void soundTickLenghthCounter(void)
{

     /* It is a single clock, running at 256hz, probably divided directly from the GB
        processor's clock (4.194304Mhz). It is used to clock the length counters of
        each channel, when the length counter enable bit (see above) for that
        particular channel is on. It is common to all channels, so when it clocks for
        one channel it clocks all of them. */

     /* On DMG no clocking when sound is disabled */
     if (soundstate.enable == FALSE) {
         return;
     }
     //soundstate.unifiedlengthclock -=4;

     //if (soundstate.unifiedlengthclock == 0) {
         //soundstate.unifiedlengthclock = 16384;

      /*   if (soundstate.Chn1.enable | soundstate.Chn2.enable | soundstate.Chn3.enable | soundstate.Chn4.enable |
             soundstate.Chn1.trigger  | soundstate.Chn2.trigger  | soundstate.Chn3.trigger  | soundstate.Chn4.trigger)
         printf("[SND] CH1 %d-%d %2d, CH2 %d-%d %2d, CH3 %d-%d %2d, CH4 %d-%d %2d, FF26=%x\n",
soundstate.Chn1.trigger,soundstate.Chn1.enable,soundstate.Chn1.length_cnt,
soundstate.Chn2.trigger,soundstate.Chn2.enable,soundstate.Chn2.length_cnt,
soundstate.Chn3.trigger,soundstate.Chn3.enable,soundstate.Chn3.length_cnt,
soundstate.Chn4.trigger,soundstate.Chn4.enable,soundstate.Chn4.length_cnt, memory[NR52] ); */

         /* The clocking of the counter is enabled or disabled depending on the status of
            the 'length counter enable register' (AKA the 'consecutive' bit) in register
            set 5, a 1 enabling the counter, a 0 disabling it. */

         //printf ("[SND] CH1=%2d, CH2=%2d, CH3=%2d, CH4=%2d\n",soundstate.Chn1.length_cnt,soundstate.Chn2.length_cnt,soundstate.Chn3.length_cnt,soundstate.Chn4.length_cnt);
         /*************
         * Channel 1  *
         *************/
         if ((soundstate.Chn1.enable == 1) && (soundstate.Chn1.length_cnt > 0)) {
                 soundstate.Chn1.length_cnt -= 1;

                 if (soundstate.Chn1.length_cnt == 0) { /* Length becoming 0 should clear status */
                     channelDisable(1);
                 }
         }
         /*************
         * Channel 2  *
         *************/
         if ((soundstate.Chn2.enable == 1) && (soundstate.Chn2.length_cnt > 0)) {
                 soundstate.Chn2.length_cnt -= 1;

                 if (soundstate.Chn2.length_cnt == 0) { /* Length becoming 0 should clear status */
                     channelDisable(2);
                 }
         }
         /*************
         * Channel 3  *
         *************/
         if ((soundstate.Chn3.enable == 1) && (soundstate.Chn3.length_cnt > 0)) {
                 soundstate.Chn3.length_cnt -= 1;

                 if (soundstate.Chn3.length_cnt == 0) { /* Length becoming 0 should clear status */
                     channelDisable(3);
                 }
         }
         /*************
         * Channel 4  *
         *************/
         if ((soundstate.Chn4.enable == 1) && (soundstate.Chn4.length_cnt > 0)) {
                 soundstate.Chn4.length_cnt -= 1;

                 if (soundstate.Chn4.length_cnt == 0) { /* Length becoming 0 should clear status */
                     channelDisable(4);
                 }
         }
     //}
}

void soundTickSweepCounter(void) {
     /* When the sweep's internal enabled flag is set and the sweep period is not zero,
        a new frequency is calculated and the overflow check is performed
     */

     if (soundstate.Chn1.sweep.enable) {
         if (soundstate.Chn1.sweep.timer) {
             printf("[SND][1] Sweep timer tick %d\n",soundstate.Chn1.sweep.timer);
             soundstate.Chn1.sweep.timer -= 1;

             return;
         }
     } else {
         return;
     }


     printf("[SND][1] Sweep timer tick %d\n",soundstate.Chn1.sweep.timer);

     if (soundstate.Chn1.sweep.period) {

         unsigned short frequency = channelCalculateSweepFreq();
         printf("     [1] Calculated new frequency %d\n",frequency);

         if ((frequency <= 2047) && (soundstate.Chn1.sweep.shift)) {
             soundstate.Chn1.sweep.shadowregister = frequency;
             memory[NR13] = frequency & 0x00FF;
             memory[NR14] &= 0xF8;
             memory[NR14] |= ((frequency >> 8) & 0x0007);

             soundstate.Chn1.frequency = frequency;

             channelCalculateSweepFreq();

             soundstate.Chn1.sweep.timer = soundstate.Chn1.sweep.period - 1;
             soundstate.Chn1.duty.period = 4 * (2048 - soundstate.Chn1.frequency);
             printf("     [1] Sweep timer reloaded %d\n",soundstate.Chn1.sweep.timer);
         }
     } else {
         soundstate.Chn1.sweep.timer = 8 - 1;
         printf("     [1] Sweep timer reloaded %d\n",soundstate.Chn1.sweep.timer);
     }



}

void soundWriteRegister(unsigned short address,unsigned char value)
{

     printf("[SND] --> Wrote %2x at %x, DAC=%d%d%d%d, frame=%d, time=%4d\n",value,address,soundstate.Chn1.dac,soundstate.Chn2.dac,soundstate.Chn3.dac,soundstate.Chn4.dac,soundstate.framesequencerstep,soundstate.framesequencerclock);

     if ((address >= 0xFF30) && (address <= 0xFF3F)) {
         memory[address] = value;
     }

     /* Any writes to those registers are ignored while power remains off except of the
        length counters that are unaffected by power and can still be written while off */
     if (soundstate.enable == FALSE) {
         switch (address) {
             case NR52 :
                 if ((value & (1<<7)) == 0) { /* Turn audio processing unit off */
                     soundTurnOff();
                     /* Writing a 0 to bit position 7 in register $FF26(NR52) disables the power to the audio processing unit */
                     soundstate.enable = FALSE;
                     /* Unused bits 4,5,6 are 1, all other bits are 0 */
                     memory[NR52] = 0x70;
                 } else { /* Turn audio processing unit on */
                     soundTurnOn();
                 }
                 break;
             case NR11 :
                 soundstate.Chn1.length_cnt = 64 - (value & 0x3F);
                 break;
             case NR21 :
                 soundstate.Chn2.length_cnt = 64 - (value & 0x3F);
                 break;
             case NR31 :
                 soundstate.Chn3.length_cnt = 256 - value;
                 break;
             case NR41 :
                 soundstate.Chn4.length_cnt = 64 - (value & 0x3F);
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
             soundstate.Chn1.length_cnt = 64 - (value & 0x3F);
             soundstate.Chn1.wave_duty = (value >> 6) & 0x03;
             break;
         case NR12 : /* 0xFF12 VVVV APPP Starting volume, Envelope add mode, period */

             /* Zombie mode */
             if (soundstate.Chn1.enable) {
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
             soundstate.Chn1.frequency = ((memory[NR14] & 0x07) << 8 ) | value;
             printf("[SND][1] Update frequency\n");
             printf("     [1] New frequency = %d\n",soundstate.Chn1.frequency);
             break;
         case NR14 : /* 0xFF14 TL-- -FFF Trigger, Length enable, Frequency MSB */
             memory[NR14] = value;
             soundstate.Chn1.frequency = ((value & 0x07) << 8 ) | memory[NR13];
             printf("[SND][1] Update frequency\n");
             printf("     [1] New frequency = %d\n",soundstate.Chn1.frequency);

             if (( (value & 0x80) >> 7) ){ /* trigger channel */
                 channelEnable(1);
             }

             /* Frequency Sweep */

             if (( (value & 0x80) >> 7)) {
                 printf("[SND][1] ** Trigger channel **\n");
                 soundstate.Chn1.sweep.negate_flag = FALSE;
                 /* Square 1's frequency is copied to the shadow register */
                 soundstate.Chn1.sweep.shadowregister = soundstate.Chn1.frequency;
                 printf("     [1] Update shadowregister = %d\n",soundstate.Chn1.frequency);

                 /* The internal enabled flag is set if either the sweep period or shift are non-zero, cleared otherwise */
                 if ((soundstate.Chn1.sweep.period) || (soundstate.Chn1.sweep.shift)) {
                     printf("     [1] Sweep enabled\n");
                     soundstate.Chn1.sweep.enable = TRUE;
                     /* The sweep timer is reloaded */
                     if (soundstate.Chn1.sweep.period) {
                         soundstate.Chn1.sweep.timer = soundstate.Chn1.sweep.period - 1;
                     } else {
                         soundstate.Chn1.sweep.timer = 8 - 1;
                     }
                     printf("     [1] Update timer = %d\n",soundstate.Chn1.sweep.timer);
                 } else {
                     printf("     [1] Sweep disabled\n");
                     soundstate.Chn1.sweep.enable = FALSE;
                     soundstate.Chn1.sweep.timer = 0;
                 }

                 /* If the sweep shift is non-zero, frequency calculation and the overflow check are performed immediately */
                 if (soundstate.Chn1.sweep.shift) {
                     unsigned short frequency = channelCalculateSweepFreq();
                     printf("     [1] Calculated new frequency %d\n",frequency);
                 }

                 /* Envelope Calculation */

                 /* Channel volume is reloaded */
                 soundstate.Chn1.volume = (memory[NR12] >> 4) & 0x0F;
                 soundstate.Chn1.envelope.enable = TRUE;
                 soundstate.Chn1.envelope.increasing = (memory[NR12] & 0x08) >> 3;
                 soundstate.Chn1.envelope.period = memory[NR12] & 0x07;
                 /* Volume envelope timer is reloaded with period */
                 soundstate.Chn1.envelope.timer = soundstate.Chn1.envelope.period;

                 /* TODO update stereo volume */
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
                             channelDisable(1);
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
                 if ((soundstate.Chn1.enable) && (soundstate.framesequencerstep % 2)) {
                     soundstate.Chn1.length_cnt = 63;
                 } else {
                     soundstate.Chn1.length_cnt = 64;
                 }
             }

             soundstate.Chn1.duty.period = 4 * (2048 - soundstate.Chn1.frequency);

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
             soundstate.Chn2.length_cnt = 64 - (value & 0x3F);
             soundstate.Chn2.wave_duty = (value >> 6) & 0x03;
             break;
         case NR22 : /* 0xFF17 VVVV APPP Starting volume, Envelope add mode, period */

             /* Zombie mode */
             if (soundstate.Chn2.enable) {
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
             soundstate.Chn2.frequency = ((memory[NR24] & 0x07) << 8 ) | value;
             break;
         case NR24 : /* 0xFF19 TL-- -FFF Trigger, Length enable, Frequency MSB */
             memory[NR24] = value;
             soundstate.Chn2.frequency = ((value & 0x07) << 8 ) | memory[NR23];

             if (( (value & 0x80) >> 7) ){ /* trigger channel */
                 channelEnable(2);
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

             if ( ( (value & 0x40) >> 6)  ) {
                 /* when the frame sequencer's next step is one that doesn't clock the length counter
                    if the length counter was PREVIOUSLY disabled and now enabled */
                 if ((soundstate.framesequencerstep % 2) && (soundstate.Chn2.enable == FALSE)) {
                     /* if the length counter is not zero, it is decremented */
                     if (soundstate.Chn2.length_cnt) {
                         soundstate.Chn2.length_cnt -= 1;
                         /* If this decrement makes it zero and trigger is clear, the channel is disabled */
                         if (soundstate.Chn2.length_cnt == 0){
                             channelDisable(2);
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

             soundstate.Chn2.duty.period = 4 * (2048 - soundstate.Chn2.frequency);

             /* if the channel's DAC is off, after the above actions occur the channel will be immediately disabled again */
             if (soundstate.Chn2.dac == OFF) {
                 channelDisable(2);
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
             soundstate.Chn3.frequency = ((value & 0x07) << 8 ) | memory[NR33];

             if (( (value & 0x80) >> 7) ){ /* trigger channel */
                 channelEnable(3);
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
                             channelDisable(3);
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
                 channelDisable(3);
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

             /* Zombie mode */
             if (soundstate.Chn4.enable) {
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
             soundstate.Chn4.noise = value;
             break;
         case NR44 : /* 0xFF23 TL-- ---- Trigger, Length enable */
             memory[NR44] = value;

             if (( (value & 0x80) >> 7) ){ /* trigger channel */
                 channelEnable(4);
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

             if ( ( (value & 0x40) >> 6)  ) {
                 /* when the frame sequencer's next step is one that doesn't clock the length counter
                    if the length counter was PREVIOUSLY disabled and now enabled */
                 if ((soundstate.framesequencerstep % 2) && (soundstate.Chn4.enable == FALSE)) {
                     /* if the length counter is not zero, it is decremented */
                     if (soundstate.Chn4.length_cnt) {
                         soundstate.Chn4.length_cnt -= 1;
                         /* If this decrement makes it zero and trigger is clear, the channel is disabled */
                         if (soundstate.Chn4.length_cnt == 0){
                             channelDisable(4);
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
                 channelDisable(4);
             }

             printf("[SND] Lenght after: %d\n",soundstate.Chn4.length_cnt);

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
                 soundstate.enable = FALSE;
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

     //printf("[SND] <-- Read %x from %x, frame=%d, time=%4d\n",data,address,soundstate.framesequencerstep,soundstate.framesequencerclock);
     return data;

}

void soundReset(void)
{

     printf("[SND] Reset Sound Registers\n");
     soundstate.enable = TRUE;
     soundResetRegisters();
     soundResetControl();
     soundResetChannel(1);
     soundResetChannel(2);
     soundResetChannel(3);
     soundResetChannel(4);

     soundstate.Chn1.length_cnt = 0;
     soundstate.Chn2.length_cnt = 0;
     soundstate.Chn3.length_cnt = 0;
     soundstate.Chn4.length_cnt = 0;

     soundstate.Chn1.enable = 0;
     soundstate.Chn2.enable = 0;
     soundstate.Chn3.enable = 0;
     soundstate.Chn4.enable = 0;
     
     soundstate.next_buffer_sample = 0;
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

     soundstate.fivebitcounter = 32;
     soundstate.unifiedlengthclock = 16384;
     soundstate.framesequencerstep = 0;
     soundstate.framesequencerclock = 8192;
     soundstate.sampler = 0;
     
     soundstate.master.channel4_left_enable = 0;
     soundstate.master.channel3_left_enable = 0;
     soundstate.master.channel2_left_enable = 0;
     soundstate.master.channel1_left_enable = 0;
     soundstate.master.channel4_right_enable = 0;
     soundstate.master.channel3_right_enable = 0;
     soundstate.master.channel2_right_enable = 0;
     soundstate.master.channel1_right_enable = 0;
}

void soundResetChannel(unsigned int channel)
{
     switch (channel) {
         case 1 :
     /*************
     * Channel 1  *
     *************/
     soundstate.Chn1.dac = 0;
     soundstate.Chn1.trigger = 0;
     soundstate.Chn1.enable = 0;
     soundstate.Chn1.frequency = 0;
     //soundstate.Chn1.length_cnt = 0;
     soundstate.Chn1.length_cnt_skip = FALSE;
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
     soundstate.Chn1.volume = 0;
             break;
         case 2 :
     /*************
     * Channel 2  *
     *************/
     soundstate.Chn2.dac = 0;
     soundstate.Chn2.trigger = 0;
     soundstate.Chn2.enable = 0;
     soundstate.Chn2.frequency = 0;
     //soundstate.Chn2.length_cnt = 0;
     soundstate.Chn2.length_cnt_skip = FALSE;
     soundstate.Chn2.wave_duty = 0;
     soundstate.Chn2.output = 0;
     soundstate.Chn2.envelope.volume = 0;
     soundstate.Chn2.envelope.increasing = 0;
     soundstate.Chn2.envelope.period = 0;
     soundstate.Chn2.envelope.timer = 0;
     soundstate.Chn2.envelope.enable = 0;
     soundstate.Chn2.duty.period = 0;
     soundstate.Chn2.duty.phase = 0;
     soundstate.Chn2.volume = 0;
             break;
         case 3 :
     /*************
     * Channel 3  *
     *************/
     soundstate.Chn3.dac = 0;
     soundstate.Chn3.trigger = 0;
     soundstate.Chn3.enable = 0;
     soundstate.Chn3.frequency = 0;
     //soundstate.Chn3.length_cnt = 0;
     soundstate.Chn3.length_cnt_skip = FALSE;
     soundstate.Chn3.volume = 0;
     soundstate.Chn3.output = 0;
             break;
         case 4 :
     /*************
     * Channel 4  *
     *************/
     soundstate.Chn4.dac = 0;
     soundstate.Chn4.trigger = 0;
     soundstate.Chn4.enable = 0;
     //soundstate.Chn4.length_cnt = 0;
     soundstate.Chn4.length_cnt_skip = FALSE;
     soundstate.Chn4.noise = 0;
     soundstate.Chn4.envelope.volume = 0;
     soundstate.Chn4.envelope.increasing = 0;
     soundstate.Chn4.envelope.period = 0;
     soundstate.Chn4.envelope.timer = 0;
     soundstate.Chn4.envelope.enable = 0;
     soundstate.Chn4.volume = 0;
     soundstate.Chn1.output = 0;
             break;
         default :
             break;
     }
}

void soundTurnOn(void)
{
     if (soundstate.enable == FALSE) {
         soundstate.enable = TRUE;
         memory[NR52] = 0x80;
         printf("[SND] ##### Sound Enabled #####\n");
         soundstate.framesequencerstep = 0;
         soundstate.framesequencerclock = 8192;
         soundstate.sampler = 0;

     }

}

void soundTurnOff(void)
{
     printf("[SND] ##### Sound Disabled #####\n");
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
             soundstate.Chn1.trigger = TRUE;
             setBit(NR52,0,TRUE);
             break;
         case 2 :
             soundstate.Chn2.trigger = TRUE;
             setBit(NR52,1,TRUE);
             break;
         case 3 :
             soundstate.Chn3.trigger = TRUE;
             setBit(NR52,2,TRUE);
             break;
         case 4 :
             soundstate.Chn4.trigger = TRUE;
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
             soundstate.Chn1.trigger = FALSE;
             soundstate.Chn1.envelope.enable = FALSE;
             setBit(NR52,0,FALSE);
             break;
         case 2 :
             soundstate.Chn2.trigger = FALSE;
             soundstate.Chn2.envelope.enable = FALSE;
             setBit(NR52,1,FALSE);
             break;
         case 3 :
             soundstate.Chn3.trigger = FALSE;
             setBit(NR52,2,FALSE);
             break;
         case 4 :
             soundstate.Chn4.trigger = FALSE;
             soundstate.Chn4.envelope.enable = FALSE;
             setBit(NR52,3,FALSE);
             break;
         default :
             break;
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
         soundstate.Chn1.sweep.negate_flag = TRUE;
         frequency = soundstate.Chn1.sweep.shadowregister - frequency;
     } else {
         frequency = soundstate.Chn1.sweep.shadowregister + frequency;
     }

     /* if this is greater than 2047, square 1 is disabled */
     if (frequency > 2047) {
         channelDisable(1);
     }

     return frequency;
}









void audioInit(void)
{

    if ( SDL_Init( SDL_INIT_AUDIO ) < 0 ) { 
        printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
        return; 
    }
    
    SDL_AudioSpec audiospec;
    
    audiospec.freq = 22050;
    audiospec.format = AUDIO_S16SYS;
    audiospec.channels = 2;
    audiospec.samples = SDL_BUFFER_SAMPLES;
    audiospec.callback = update_stream;
    audiospec.userdata = NULL;

    if (SDL_OpenAudio(&audiospec, NULL) < 0)
    {
        printf("[ERROR] Couldn't open audio: %s\n", SDL_GetError());
        return;
    }
    
    SDL_PauseAudio(0);

}

static void update_stream(void * userdata, unsigned char * stream, int len)
{
    if (samples_left_to_output < len/4) return;

    samples_left_to_input += len/4;
    samples_left_to_output -= len/4;

    int i;
    
    for (i = 0; i < len/2; i++) {
        stream[i] = samplebuffer[buffer_next_output_sample++] / 128;
        buffer_next_output_sample &= GB_BUFFER_SAMPLES-1;
    }
    printf("[SOUND] SDL stream has been updated\n");
}

void soundResetBufferPointers(void)
{
    buffer_next_input_sample = 0;
    buffer_next_output_sample = 0;
    samples_left_to_input = GB_BUFFER_SAMPLES;
    samples_left_to_output = 0;
}