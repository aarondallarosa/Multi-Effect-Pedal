#include <iostream>
#include <sndfile.h>
#include <cmath>

// Open the file
SNDFILE* open_wav(const char* filename, SF_INFO* sfinfo){
    // Clears SF_INFO
    *sfinfo = SF_INFO();
    // Open the file to read
    SNDFILE* infile = sf_open(filename, SFM_READ, sfinfo);
    // Ensure the file is opened properly
    if (!infile) {
        std::cerr << "Error opening file " << filename << ": "
                  << sf_strerror(NULL) << std::endl;
        return nullptr;
    }
    return infile;
}

void tube_screamer(const float* input_buffer, float* output_buffer, float drive, float tone, float level, int num_samples){
    // Convwet parameters (0-10) to usable values
    // Gain from 12-118 Im still fine tuning this effect to minimize the digital sound
    float gain_val = 12.0f + ((118.0f - 12.0f) * (drive / 10.0f));
    // Determines LPF frequencies
    float alpha = tone / 10.0f;
    // (1/2) - 2 Times the volume
    float volume = 0.5f + ((2.5f - 0.5f) * (level / 10.0f));
    float prev_filtered = 0;
    float sample = 0;
    for (int n = 0; n < num_samples; ++n){
        // Apply simple gain stage and clipping
        sample = gain_val * input_buffer[n];
        sample = tanh(sample / 0.5f) * 0.5f;
        // Apply LPF filter
        float filtered = alpha * sample + (1.0f - alpha) * prev_filtered;
        prev_filtered = filtered;
        // Change volume
        output_buffer[n] = level * filtered;

    }
}



int main(){
    // Open the file
    SF_INFO sfinfo;
    SNDFILE* infile = open_wav("Clean_Riff.wav", &sfinfo);
    if (!infile) {
        return 1;
    }

    // Determine amount of samples and create arrays
    int num_samples = sfinfo.frames;
    float* input_buffer = new float[num_samples];
    float* output_buffer = new float[num_samples];
    sf_read_float(infile, input_buffer, num_samples);
    
    // Gather Values and verify
    float drive, tone, level;
    while(true){
    std::cout << "Enter Drive Value (0-10): ";
    std::cin >> drive;
    std::cout << "Enter Tone Value (0-10): ";
    std::cin >> tone;
    std::cout << "Enter Level Value (0-10): ";
    std::cin >> level;
    // Verify values
    if (drive >= 0 && drive <= 10 &&
        tone >= 0 && tone <= 10 &&
        level >= 0 && level <= 10){
            break;
        }
        else{
            std::cout << "Invalid Input\n";
        }
    }
    
    // Insert delay function
    tube_screamer(input_buffer, output_buffer, drive, tone, level, num_samples);

    // Write output file this is not used in RaspberryPi
    SF_INFO sfoutinfo = sfinfo; 
    sfoutinfo.format = SF_FORMAT_WAV | SF_FORMAT_FLOAT;
    SNDFILE* outfile = sf_open("Driven_Riff.wav", SFM_WRITE, &sfoutinfo);
    if (!outfile) {
        std::cerr << "Error opening output.wav for writing: "
        << sf_strerror(NULL) << std::endl;
        delete[] input_buffer;
        delete[] output_buffer;
        sf_close(infile);
        return 1;
    }
    
sf_write_float(outfile, output_buffer, num_samples);
sf_close(outfile);

    // close everything used
    sf_close(infile);
    delete[] input_buffer;
    delete[] output_buffer;
    return 0;
    }
