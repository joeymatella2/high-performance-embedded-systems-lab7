#pragma once

#include <opencv2/core.hpp>
#include <string>

struct Options {
    std::string input;
    int iterations = 0;
    int threads = 1;
    int width = 3840;
    int height = 2160;
    int frame = 60;
};

Options parse_options(int argc, char** argv, bool benchmark);
cv::Mat load_frame(const Options& options);
