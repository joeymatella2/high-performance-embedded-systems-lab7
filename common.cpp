#include "image_processing.h"

#include <stdexcept>

void check_buffers(const cv::Mat& input, const cv::Mat& gray,
                   const cv::Mat& output)
{
    if (input.type() != CV_8UC3 || input.rows < 3 || input.cols < 3 ||
        gray.type() != CV_8UC1 || gray.size() != input.size() ||
        output.type() != CV_8UC1 || output.rows != input.rows - 2 ||
        output.cols != input.cols - 2)
        throw std::invalid_argument("expected BGR input and preallocated gray/cropped output");
}

unsigned long long checksum(const cv::Mat& image)
{
    unsigned long long total = 0;
    for (int y = 0; y < image.rows; ++y) {
        const unsigned char* row = image.ptr<unsigned char>(y);
        for (int x = 0; x < image.cols; ++x) total += row[x];
    }
    return total;
}
