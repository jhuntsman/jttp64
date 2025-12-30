#ifndef JTTP_SOUND_H
#define JTTP_SOUND_H

// Direct SID Register Pointers
#define SID_V2_CTRL *((volatile unsigned char*)0xD40B)
#define SID_V3_CTRL *((volatile unsigned char*)0xD412)
#define SID_FLT_LO *((volatile unsigned char*)0xD415)
#define SID_FLT_HI *((volatile unsigned char*)0xD416)
#define SID_FLT_CTRL *((volatile unsigned char*)0xD417)
#define SID_FLT_VOL *((volatile unsigned char*)0xD418)

void init_sid_enhanced();
void play_thud();
void play_dragon_snore(unsigned char phase);

#endif  // JTTP_SOUND_H
