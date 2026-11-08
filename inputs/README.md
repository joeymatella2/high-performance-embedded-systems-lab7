# Frame 2000 benchmark input

`frame2000.png` is a lossless 3840x2160 RGB image extracted from zero-based
frame 2000 of the earlier-lab video
`bbb_sunflower_2160p_60fps_normal.mp4` using OpenCV 4.6.0 on WSL. The video
was opened with `cv::VideoCapture`, positioned with
`CAP_PROP_POS_FRAMES = 2000`, decoded with `read`, resized to 3840x2160, and
saved with `cv::imwrite`. All measured configurations must use this exact PNG.

SHA-256:
`fdcc323bff5c3765f5ef89f493ce4943caac26b48fcef76fa97cf46b7e98ef4f`

Using a saved image removes repeated video opening and seeking from each
`perf stat` process. Image loading and resizing still occur before the
processing loop and remain included in the process-wide `perf` totals.
