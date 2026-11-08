#pragma once

#include <opencv2/core.hpp>
#include <thread>
#include <vector>

// Both implementations write a cropped (rows - 2) x (cols - 2) CV_8UC1 image.
// The caller owns and allocates gray and output before repeated processing.
void process_original(const cv::Mat& input, cv::Mat& gray, cv::Mat& output);
void process_optimized(const cv::Mat& input, cv::Mat& gray, cv::Mat& output,
                       int thread_count, std::vector<std::thread>& workers);

void check_buffers(const cv::Mat& input, const cv::Mat& gray,
                   const cv::Mat& output);
unsigned long long checksum(const cv::Mat& image);
