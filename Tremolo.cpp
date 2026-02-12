#include <iostream>
#include <sndfile.h>

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

void TR2(const float* input_buffer, float* output_buffer, float rate, float depth, float wave, int num_samples){
    // Determine the real values for paramters
    float rate_val = 1.0f + ((13.0f - 1.0f) * (rate / 10.0f)); 
    float depth_val = depth / 10.0f;
    float wave_val = wave / 10.0f;
    float phase = 0.0f;
    float LFO, square, triangle;
    float phase_inc = rate_val / 44100.0f;
    // Loop through and modify signal
    for (int n = 0; n < num_samples; ++n){
        // Create values that will be updated after the current iteration
        if (phase >= 0.5){
            triangle = 2.0f - 2.0f * phase;
            square = 0.0f;
        }
        else {
            triangle = 2.0f * phase;
            square = 1.0f;
        }
        LFO = (1 - wave_val) * triangle + wave_val * square;
        output_buffer[n] = input_buffer[n] * ((1.0f - depth_val) + (depth_val * LFO));
        phase += phase_inc;
        if (phase >= 1){
            phase -= 1;
        }
    }
}



int main(){
    // Open the file
    SF_INFO sfinfo;
    SNDFILE* infile = open_wav("gtgtg.wav", &sfinfo);
    if (!infile) {
        return 1;
    }

    // Determine amount of samples and create arrays
    int num_samples = sfinfo.frames;
    float* input_buffer = new float[num_samples];
    float* output_buffer = new float[num_samples];
    sf_read_float(infile, input_buffer, num_samples);
    
    // Gather Values and verify
    float rate, depth, wave;
    while(true){
    std::cout << "Enter Rate Value (0-10): ";
    std::cin >> rate;
    std::cout << "Enter Depth Value (0-10): ";
    std::cin >> depth;
    std::cout << "Enter Wave Value (0-10): ";
    std::cin >> wave;
    // Verify values
    if (rate >= 0 && rate <= 10 &&
        depth >= 0 && depth <= 10 &&
        wave >= 0 && wave <= 10){
            break;
        }
        else{
            std::cout << "Invalid Input\n";
        }
    }
    
    // Insert delay function
    TR2(input_buffer, output_buffer, rate, depth, wave, num_samples);

    // Write output file this is not used in RaspberryPi
    SF_INFO sfoutinfo = sfinfo;
    sfoutinfo.format = SF_FORMAT_WAV | SF_FORMAT_FLOAT;
    SNDFILE* outfile = sf_open("Mixed.wav", SFM_WRITE, &sfoutinfo);
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