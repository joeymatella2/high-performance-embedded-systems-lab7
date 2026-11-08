#include "input.h"

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <stdexcept>

namespace {
int positive(const std::string& value) {
    size_t used = 0;
    int result = std::stoi(value, &used);
    if (used != value.size() || result < 1)
        throw std::invalid_argument("numeric options must be positive integers");
    return result;
}
int nonnegative(const std::string& value) {
    size_t used = 0;
    int result = std::stoi(value, &used);
    if (used != value.size() || result < 0)
        throw std::invalid_argument("--frame must be a nonnegative integer");
    return result;
}
}

Options parse_options(int argc, char** argv, bool benchmark)
{
    Options options;
    for (int i = 1; i < argc; ++i) {
        const std::string key = argv[i];
        if (key == "--no-display") continue; // Accepted for the lab's sample command.
        if (i + 1 >= argc) throw std::invalid_argument("missing value for " + key);
        const std::string value = argv[++i];
        if (key == "--input") options.input = value;
        else if (key == "--iterations") options.iterations = positive(value);
        else if (key == "--threads") options.threads = positive(value);
        else if (key == "--width") options.width = positive(value);
        else if (key == "--height") options.height = positive(value);
        else if (key == "--frame") options.frame = nonnegative(value);
        else throw std::invalid_argument("unknown option: " + key);
    }
    if (options.input.empty()) throw std::invalid_argument("--input is required");
    if (benchmark && options.iterations < 1)
        throw std::invalid_argument("--iterations is required");
    if (!benchmark && options.iterations != 0)
        throw std::invalid_argument("validate does not accept --iterations");
    if (options.width < 3 || options.height < 3)
        throw std::invalid_argument("width and height must be at least 3");
    return options;
}

cv::Mat load_frame(Options& options)
{
    cv::setNumThreads(1);
    cv::Mat frame = cv::imread(options.input, cv::IMREAD_COLOR);
    options.input_is_video = frame.empty();
    if (frame.empty()) {
        cv::VideoCapture capture(options.input);
        capture.set(cv::CAP_PROP_POS_FRAMES, options.frame);
        if (!capture.isOpened() || !capture.read(frame) || frame.empty())
            throw std::runtime_error("could not read image or selected video frame: " + options.input);
    }
    cv::Mat resized;
    cv::resize(frame, resized, cv::Size(options.width, options.height));
    return resized;
}
