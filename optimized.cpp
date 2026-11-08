#include "image_processing.h"

#include <algorithm>
#include <condition_variable>
#include <mutex>
#include <stdexcept>

namespace {
struct Barrier {
    explicit Barrier(int count) : count(count) {}
    std::mutex mutex;
    std::condition_variable condition;
    int arrived = 0;
    const int count;
    void wait() {
        std::unique_lock<std::mutex> lock(mutex);
        ++arrived;
        if (arrived == count) condition.notify_all();
        else condition.wait(lock, [this] { return arrived == count; });
    }
};

void gray_rows(const cv::Mat& input, cv::Mat& gray, int first, int last) {
    for (int y = first; y < last; ++y) {
        const unsigned char* src = input.ptr<unsigned char>(y);
        unsigned char* dst = gray.ptr<unsigned char>(y);
        for (int x = 0; x < input.cols; ++x) {
            dst[x] = static_cast<unsigned char>(
                (54 * src[3*x + 2] + 183 * src[3*x + 1] + 18 * src[3*x]) >> 8);
        }
    }
}

void sobel_rows(const cv::Mat& gray, cv::Mat& output, int first, int last) {
    for (int y = std::max(first, 1); y < std::min(last, gray.rows - 1); ++y) {
        const unsigned char* top = gray.ptr<unsigned char>(y - 1);
        const unsigned char* mid = gray.ptr<unsigned char>(y);
        const unsigned char* bot = gray.ptr<unsigned char>(y + 1);
        unsigned char* dst = output.ptr<unsigned char>(y - 1);
        for (int x = 1; x < gray.cols - 1; ++x) {
            const int gx = (top[x+1] + 2*mid[x+1] + bot[x+1]) -
                           (top[x-1] + 2*mid[x-1] + bot[x-1]);
            const int gy = (bot[x-1] + 2*bot[x] + bot[x+1]) -
                           (top[x-1] + 2*top[x] + top[x+1]);
            const int ax = gx < 0 ? -gx : gx;
            const int ay = gy < 0 ? -gy : gy;
            int magnitude = ax > ay ? ax + (ay >> 1) : ay + (ax >> 1);
            if (magnitude > 255) magnitude = 255;
            dst[x - 1] = static_cast<unsigned char>(magnitude);
        }
    }
}
} // namespace

// Adapted from Lab 6: same pointer-based two-stage kernel for every thread count.
// The caller reuses the image buffers and reserved worker vector across iterations.
void process_optimized(const cv::Mat& input, cv::Mat& gray, cv::Mat& output,
                       int thread_count, std::vector<std::thread>& workers)
{
    check_buffers(input, gray, output);
    if (thread_count < 1 || workers.capacity() < static_cast<size_t>(thread_count))
        throw std::invalid_argument("reserve worker slots before processing");
    Barrier barrier(thread_count);
    workers.clear();
    for (int i = 0; i < thread_count; ++i) {
        const int first = static_cast<int>(1LL * i * input.rows / thread_count);
        const int last = static_cast<int>(1LL * (i + 1) * input.rows / thread_count);
        workers.emplace_back([&, first, last] {
            gray_rows(input, gray, first, last);
            barrier.wait();
            sobel_rows(gray, output, first, last);
        });
    }
    for (auto& worker : workers) worker.join();
}
