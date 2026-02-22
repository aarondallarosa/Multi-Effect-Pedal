#ifndef EFFECTS_H
#define EFFECTS_H

// Overdrive effect
void tube_screamer(const float* input_buffer, float* output_buffer, 
                   float drive, float tone, float level, int num_samples);

// Tremolo effect
void TR2(const float* input_buffer, float* output_buffer, 
         float rate, float depth, float wave, int num_samples);

// Digital Delay effect
void carbon_copy(const float* input_buffer, float* output_buffer, 
                 float regen, float delay, float mix, bool warm, int num_samples);

#endif // EFFECTS_H
