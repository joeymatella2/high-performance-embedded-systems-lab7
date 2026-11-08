#include "image_processing.h"
#include "input.h"

#include <algorithm>
#include <iostream>
#include <thread>
#include <vector>

int main(int argc, char** argv)
{
    try {
        const Options options = parse_options(argc, argv, false);
        const cv::Mat input = load_frame(options);
        cv::Mat gray(input.rows, input.cols, CV_8UC1);
        cv::Mat reference(input.rows - 2, input.cols - 2, CV_8UC1);
        cv::Mat output(input.rows - 2, input.cols - 2, CV_8UC1);
        std::vector<std::thread> workers;
        workers.reserve(8);
        process_original(input, gray, reference);
        std::cout << "configuration,checksum,max_pixel_difference,differing_pixels,pass\n";
        std::cout << "original," << checksum(reference) << ",0,0,yes\n";
        bool passed = true;
        for (int threads : {1, 2, 4, 8}) {
            process_optimized(input, gray, output, threads, workers);
            int maximum = 0;
            unsigned long long different = 0;
            for (int y = 0; y < output.rows; ++y) {
                const unsigned char* a = reference.ptr<unsigned char>(y);
                const unsigned char* b = output.ptr<unsigned char>(y);
                for (int x = 0; x < output.cols; ++x) {
                    const int delta = std::abs(int(a[x]) - int(b[x]));
                    maximum = std::max(maximum, delta);
                    if (delta != 0) ++different;
                }
            }
            passed = passed && different == 0;
            std::cout << "optimized-" << threads << "T," << checksum(output)
                      << ',' << maximum << ',' << different << ','
                      << (different == 0 ? "yes" : "NO") << '\n';
        }
        return passed ? 0 : 2;
    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << '\n';
        return 1;
    }
}
