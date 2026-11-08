#include "image_processing.h"

#include <cmath>
#include <stdexcept>

// Scalar Lab 2 reference: fixed-point BGR grayscale and approximate Sobel.
// Keeping this output definition makes exact pixel comparison meaningful.
void process_original(const cv::Mat& input, cv::Mat& gray, cv::Mat& output)
{
    check_buffers(input, gray, output);
    for (int y = 0; y < input.rows; ++y) {
        for (int x = 0; x < input.cols; ++x) {
            const cv::Vec3b pixel = input.at<cv::Vec3b>(y, x);
            gray.at<unsigned char>(y, x) = static_cast<unsigned char>(
                (54 * pixel[2] + 183 * pixel[1] + 18 * pixel[0]) >> 8);
        }
    }

    for (int y = 1; y < gray.rows - 1; ++y) {
        for (int x = 1; x < gray.cols - 1; ++x) {
            const int gx =
                (gray.at<unsigned char>(y - 1, x + 1) +
                 2 * gray.at<unsigned char>(y, x + 1) +
                 gray.at<unsigned char>(y + 1, x + 1)) -
                (gray.at<unsigned char>(y - 1, x - 1) +
                 2 * gray.at<unsigned char>(y, x - 1) +
                 gray.at<unsigned char>(y + 1, x - 1));
            const int gy =
                (gray.at<unsigned char>(y + 1, x - 1) +
                 2 * gray.at<unsigned char>(y + 1, x) +
                 gray.at<unsigned char>(y + 1, x + 1)) -
                (gray.at<unsigned char>(y - 1, x - 1) +
                 2 * gray.at<unsigned char>(y - 1, x) +
                 gray.at<unsigned char>(y - 1, x + 1));
            const int ax = std::abs(gx);
            const int ay = std::abs(gy);
            int magnitude = ax > ay ? ax + (ay >> 1) : ay + (ax >> 1);
            if (magnitude > 255) magnitude = 255;
            output.at<unsigned char>(y - 1, x - 1) =
                static_cast<unsigned char>(magnitude);
        }
    }
}
