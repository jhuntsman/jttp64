#include <c64/sid.h>

void init_sid_enhanced() {
    unsigned char* sid_reg = (unsigned char*)0xD400;

    // 1. Hard reset all 29 SID registers to 0
    for (unsigned char i = 0; i < 29; i++) sid_reg[i] = 0;

    // 2. Master Volume + Filter Type
    // 0x1F = Volume 15 + Low-Pass Filter enabled
    sid_reg[24] = 0x1F;

    // 3. Filter Cutoff (The "Muffle" control)
    // Range is 0-2047. We want it low for a deep sound.
    // Try 0x15 for deep rumble, 0x30 for more "hiss"
    sid_reg[21] = 0x00;  // Cutoff Low
    sid_reg[22] = 0x20;  // Cutoff High (Main control)

    // 4. Filter Routing + Resonance
    // 0xF0 = Maximum Resonance (sharper, more character)
    // 0x07 = Route Voice 1, 2, AND 3 through the filter
    sid_reg[23] = 0xF7;
}

void play_thud() {
    sid.voices[0].freq = 0x0800;  // Low frequency for a "heavy" sound
    sid.voices[0].attdec = 0x00;  // Attack: 0, Decay: 0 (Instant start)
    sid.voices[0].susrel = 0x40;  // Sustain: 0, Release: 4 (Fast fade out)

    // Trigger the Noise waveform (0x80) with the Gate bit (0x01)
    sid.voices[0].ctrl = 0x81;

    // Small delay to let the sound start
    for (int i = 0; i < 200; i++);

    // Release the gate to start the decay/release phase
    sid.voices[0].ctrl = 0x80;
}

void play_dragon_snore(const unsigned char phase) {
    if (phase == 0) {  // INHALE
        // Voice 1 (Index 0) - Pulse Rattle
        sid.voices[0].freq = 0x0180;
        sid.voices[0].attdec = 0x8A;
        sid.voices[0].pwm = 0x0200;  // 11-bit Pulse Width
        sid.voices[0].ctrl = 0x41;   // Pulse + Gate

        // Voice 2 (Index 1) - Low Triangle
        sid.voices[1].freq = 0x00C0;
        sid.voices[1].attdec = 0x9A;
        sid.voices[1].ctrl = 0x11;  // Triangle + Gate
    } else {                        // EXHALE
        // Voice 1 (Index 0) - Noise Rumble
        sid.voices[0].freq = 0x0080;
        sid.voices[0].attdec = 0x09;
        sid.voices[0].ctrl = 0x81;

        // Subtle tweak: slightly open the filter during exhale to
        // simulate the dragon opening its mouth
        *((volatile unsigned char*)0xD416) = 0x28;

        // Voice 2 (Index 1) - Deep Sawtooth
        sid.voices[1].freq = 0x0070;
        sid.voices[1].attdec = 0x0C;
        sid.voices[1].ctrl = 0x21;  // Sawtooth + Gate
    }
}


