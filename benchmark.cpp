#include "image_processing.h"
#include "input.h"

#include <chrono>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <thread>
#include <vector>

int main(int argc, char** argv)
{
    try {
        const Options options = parse_options(argc, argv, true);
#ifdef ORIGINAL_BUILD
        if (options.threads != 1)
            throw std::invalid_argument("sobel_original requires --threads 1");
#endif
        const cv::Mat input = load_frame(options);
        cv::Mat gray(input.rows, input.cols, CV_8UC1);
        cv::Mat output(input.rows - 2, input.cols - 2, CV_8UC1);
        std::vector<std::thread> workers;
        workers.reserve(options.threads);

        const auto start = std::chrono::steady_clock::now();
        for (int i = 0; i < options.iterations; ++i) {
#ifdef ORIGINAL_BUILD
            process_original(input, gray, output);
#else
            process_optimized(input, gray, output, options.threads, workers);
#endif
        }
        const auto stop = std::chrono::steady_clock::now();
        const double milliseconds =
            std::chrono::duration<double, std::milli>(stop - start).count();
#ifdef ORIGINAL_BUILD
        std::cout << "configuration=original\n";
#else
        std::cout << "configuration=optimized-" << options.threads << "T\n";
#endif
        std::cout << "threads=" << options.threads << '\n'
                  << "iterations=" << options.iterations << '\n'
                  << "input=" << options.input << '\n'
                  << "width=" << options.width << '\n'
                  << "height=" << options.height << '\n'
                  << "video_frame_index=" << options.frame << '\n'
                  << "checksum=" << checksum(output) << '\n'
                  << std::fixed << std::setprecision(3)
                  << "processing_ms=" << milliseconds << '\n'
                  << "processing_ms_per_frame=" << milliseconds / options.iterations << '\n';
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << '\n';
        return 1;
    }
}
