# Lab 7: grayscale and Sobel performance counters

This package adapts the scalar fixed-point implementation from Lab 2 and the
pointer-based threaded implementation from Lab 6. Both produce the same
cropped grayscale/Sobel image. The reference is built with `-O0`; the optimized
kernel is built with `-O3 -ftree-vectorize`. All versions use the same `g++`
and OpenCV installation on the machine where `make` is run. See `Makefile` for
every exact compile and link command (`make -B -n all` prints them).

## Build and correctness

On the **Raspberry Pi** with OpenCV development files installed:

```bash
make clean
make all
./validate --input /path/to/video.mp4 | tee results/correctness.txt
```

Create `results/` first if running validation manually. `validate` reads the
video frame selected by `Options.frame` in `input.h` (initially index 60),
or loads the supplied image file. It resizes the input to 3840x2160, compares
all five configurations pixel by pixel, and exits nonzero on any mismatch. Use
`--width W --height H` on both `validate` and every benchmark command only if
you intentionally choose a different common size. The default is the 4K size
used in earlier labs. To select another zero-based video frame for the entire
experiment, change `Options.frame` in `input.h` and rebuild with
`make clean && make all`. `collect.sh` does not pass a frame argument, so all
configurations use the same compiled default.

For the representative image-based experiment, use the included lossless
`inputs/frame2000.png`. It was extracted from zero-based frame 2000 of the
earlier-lab `bbb_sunflower_2160p_60fps_normal.mp4` with OpenCV 4.6.0 and is
3840x2160. Its SHA-256 is
`fdcc323bff5c3765f5ef89f493ce4943caac26b48fcef76fa97cf46b7e98ef4f`.
When input is an image, `Options.frame` is ignored. The benchmark labels the
input kind in its output.

## Benchmark commands

```bash
./sobel_original --input /path/to/video.mp4 --threads 1 --iterations 500
./sobel_optimized --input /path/to/video.mp4 --threads 1 --iterations 500
./sobel_optimized --input /path/to/video.mp4 --threads 2 --iterations 500
./sobel_optimized --input /path/to/video.mp4 --threads 4 --iterations 500
./sobel_optimized --input /path/to/video.mp4 --threads 8 --iterations 500
```

For frame 2000, replace `/path/to/video.mp4` in every command with
`inputs/frame2000.png`. The same image must be used for validation, timing
calibration, and all counter sets. Do not combine its results with the earlier
frame-60 video results.

Replace `500` with one iteration count that makes **every** configuration run
for at least two seconds. Keep that number fixed for all configurations. The
benchmark opens/decodes/resizes only one frame before timing, allocates the
gray and output images plus worker-vector capacity once, repeats processing
without I/O, and prints a checksum after the loop. `perf` still counts process
startup and cleanup; enough iterations make that work small in proportion.
The optimized family uses the same grayscale/Sobel kernels, dataflow, flags,
input, and output for every thread count. Worker threads are started and joined
on each iteration, as in Lab 6; that overhead is part of the measured design.

## Collect evidence on the Pi

```bash
chmod +x collect.sh
./collect.sh /path/to/video.mp4 500
```

For the frame-2000 image experiment, run
`./collect.sh inputs/frame2000.png ITERATIONS`, where `ITERATIONS` has been
recalibrated on the Pi so every configuration lasts at least two seconds.
The script writes to `results/`; move or copy any earlier `results/` directory
before starting a new collection.

`collect.sh` refuses to run outside a Raspberry Pi. It saves system details,
`perf list`, the access test, correctness, one warm-up per configuration, the
11 required `perf stat -r 5` outputs, and before/after thermal status under
`results/`. It waits two minutes before the first block and for each later
block to return within 5 C of the baseline. It stops if a current throttle
condition or newly latched condition appears. If a block is marked invalid,
discard its counter files and rerun that block after conditions stabilize.
Do not run the benchmark with `sudo`.

Inspect each perf file for `<not supported>` or counter active time well below
100%. Record unsupported values as N/A. If counters are multiplexed, split
that set into smaller commands and repeat the whole measurement. The script
does not change `perf_event_paranoid`; follow the lab statement's temporary
procedure on your own Pi only if the access test fails. Save the original
value and restore it afterward.

The script's exact perf sets are:

```bash
perf stat -r 5 -e cycles:u,instructions:u -- ./sobel_original --input /path/to/video.mp4 --threads 1 --iterations 500
perf stat -r 5 -e cache-references:u,cache-misses:u -- ./sobel_original --input /path/to/video.mp4 --threads 1 --iterations 500
perf stat -r 5 -e cycles:u,instructions:u -- ./sobel_optimized --input /path/to/video.mp4 --threads 1 --iterations 500
perf stat -r 5 -e cache-references:u,cache-misses:u -- ./sobel_optimized --input /path/to/video.mp4 --threads 1 --iterations 500
perf stat -r 5 -e task-clock,context-switches,cpu-migrations,page-faults -- ./sobel_optimized --input /path/to/video.mp4 --threads 1 --iterations 500
perf stat -r 5 -e cycles:u,instructions:u -- ./sobel_optimized --input /path/to/video.mp4 --threads 2 --iterations 500
perf stat -r 5 -e task-clock,context-switches,cpu-migrations,page-faults -- ./sobel_optimized --input /path/to/video.mp4 --threads 2 --iterations 500
perf stat -r 5 -e cycles:u,instructions:u -- ./sobel_optimized --input /path/to/video.mp4 --threads 4 --iterations 500
perf stat -r 5 -e task-clock,context-switches,cpu-migrations,page-faults -- ./sobel_optimized --input /path/to/video.mp4 --threads 4 --iterations 500
perf stat -r 5 -e cycles:u,instructions:u -- ./sobel_optimized --input /path/to/video.mp4 --threads 8 --iterations 500
perf stat -r 5 -e task-clock,context-switches,cpu-migrations,page-faults -- ./sobel_optimized --input /path/to/video.mp4 --threads 8 --iterations 500
```

`perf` writes statistics to standard error. The script saves both standard
output and standard error for every measured run.

## Calculations and report

Use the averaged results from each `-r 5` output. Divide elapsed time,
cycles, instructions, and cache counts by the common iteration count to get
per-frame values. Calculate IPC = instructions / cycles, cache miss rate =
cache misses / cache references * 100%, effective CPUs = task-clock (ms) /
elapsed time (ms), and speedup(N) = optimized-1T time / optimized-NT time.
Keep rates and counts from the same counter set. Use `report_outline.md` to
organize the PDF after the Pi measurements exist; do not fill it with assumed
counter values.
