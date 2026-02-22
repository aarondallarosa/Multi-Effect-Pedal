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


void carbon_copy(const float* input_buffer, float* output_buffer, float regen, float delay, float mix, bool warm, int num_samples){
    // Determine the real values for paramters
    float regen_val = regen / 10.0f * 0.95f;
    // 20ms to 900ms delay. The real pedal goes to 1200 but that is extreme
    float delay_val = 20.0f + ((900.0f - 20.0f) * (delay / 10.0f));
    // Determines the ratio of the dry to delayed signal
    float mix_val = mix / 10.0f;
    int D = static_cast<int>((delay_val / 1000.0f) * 44100.0f);
    // Alpha for the LPF. When warm is toggled the cut off frequencies are lower
    float alpha;
    if (warm == true){
        alpha = 0.16f;
    }
    else {
        alpha = 0.41f;
    }
    
    // Create a buffer
    float* feedback_buffer = new float[D];
    for (int i = 0; i < D; ++i) {
        feedback_buffer[i] = 0.0f;
    }
    // Initialize values prior to loop to avoid negative samples
    int index = 0;
    float last_lpf = 0.0f;
    // Loop through and modify signal
    for (int n = 0; n < num_samples; ++n){
        // Create values that will be updated after the current iteration
        float dry = input_buffer[n];
        float delayed = feedback_buffer[index];
        output_buffer[n] = ((1.0f - mix_val) * dry) + (mix_val * delayed);

        // Apply LPF
        last_lpf = (1 - alpha) * delayed + alpha * last_lpf;

        // Update the feedback and iterate index 
        feedback_buffer[index] = dry + (regen_val * last_lpf);
        index++;
        if (index >= D){
            index = 0;
        }
    }
    delete[] feedback_buffer;
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
    float delay, regen, mix;
    int warm_int;
    bool warm;
    while(true){
    std::cout << "Enter Delay Value (0-10): ";
    std::cin >> delay;
    std::cout << "Enter Regen Value (0-10): ";
    std::cin >> regen;
    std::cout << "Enter Mix Value (0-10): ";
    std::cin >> mix;
    std::cout << "Enter Warm (0 or 1): ";
    std::cin >> warm_int;
    warm = (warm_int == 1);
    // Verify values
    if (delay >= 0 && delay <= 10 &&
        regen >= 0 && regen <= 10 &&
        mix >= 0 && mix <= 10 &&
        (warm_int == 0 || warm_int == 1)){
            break;
        }
        else{
            std::cout << "Invalid Input\n";
        }
    }
    
    // Insert delay function
    carbon_copy(input_buffer, output_buffer, regen, delay, mix, warm, num_samples);

    // Write output file this is not used in RaspberryPi
    SF_INFO sfoutinfo = sfinfo;
    sfoutinfo.format = SF_FORMAT_WAV | SF_FORMAT_FLOAT;
    SNDFILE* outfile = sf_open("out.wav", SFM_WRITE, &sfoutinfo);
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
