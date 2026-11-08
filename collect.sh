#!/usr/bin/env bash
set -euo pipefail

if [[ $# -ne 2 ]]; then
    echo "Usage: $0 INPUT_VIDEO_OR_IMAGE ITERATIONS" >&2
    exit 1
fi
input=$1
iterations=$2
if [[ ! -f "$input" || ! "$iterations" =~ ^[1-9][0-9]*$ ]]; then
    echo "Input must exist and iterations must be a positive integer." >&2
    exit 1
fi
if [[ ! -r /proc/device-tree/model ]] || ! grep -q 'Raspberry Pi' /proc/device-tree/model; then
    echo "Run this collection script on the Raspberry Pi, not WSL or a PC." >&2
    exit 1
fi
for tool in perf vcgencmd; do command -v "$tool" >/dev/null || { echo "Missing $tool" >&2; exit 1; }; done
mkdir -p results

{
    echo '$ uname -a'; uname -a
    echo '$ cat /etc/os-release'; cat /etc/os-release
    echo '$ cat /proc/device-tree/model'; tr '\0' '\n' </proc/device-tree/model
    echo '$ lscpu'; lscpu
    echo '$ g++ --version'; g++ --version
    echo '$ perf --version'; perf --version
    echo '$ make -B -n all'; make -B -n all
} >results/system.txt 2>&1
perf list >results/perf_list.txt 2>&1
./validate --input "$input" >results/correctness.txt 2>&1
cat /proc/sys/kernel/perf_event_paranoid >results/perf_event_paranoid.txt
perf stat -e cycles:u,instructions:u -- seq 1 1000000 >/dev/null 2>results/perf_access_test.txt || {
    echo "Counter access test failed; see results/perf_access_test.txt" >&2
    exit 1
}

echo "Letting the Pi idle for two minutes before the first block..."
sleep 120
baseline=$(vcgencmd measure_temp | sed -E 's/.*=([0-9.]+).*/\1/')
baseline_throttle=$(vcgencmd get_throttled)
printf 'baseline_temp=%s\nbaseline_%s\n' "$baseline" "$baseline_throttle" >results/thermal_baseline.txt
if ! awk -v current="$baseline" 'BEGIN { exit !(current < 80) }'; then
    echo "Baseline temperature is at least 80 C. Cool the Pi and rerun collection." >&2
    exit 1
fi

measure_block() {
    local name=$1 executable=$2 threads=$3
    local pre_temp pre_status pre_hex post_temp post_status post_hex
    while :; do
        pre_temp=$(vcgencmd measure_temp | sed -E 's/.*=([0-9.]+).*/\1/')
        if awk -v current="$pre_temp" -v base="$baseline" 'BEGIN { d=current-base; if (d<0) d=-d; exit !(d<=5 && current<80) }'; then break; fi
        echo "Waiting for temperature within 5 C of baseline $baseline C (now $pre_temp C)..."
        sleep 30
    done
    pre_status=$(vcgencmd get_throttled)
    pre_hex=${pre_status#throttled=}
    {
        echo "before_temp=${pre_temp} C"
        echo "before_${pre_status}"
    } >"results/${name}_thermal.txt"
    if (( (pre_hex & 15) != 0 )); then
        echo "Current power/thermal warning before $name; stop and correct it." >&2
        exit 1
    fi
    "$executable" --input "$input" --threads "$threads" --iterations "$iterations" >"results/${name}_warmup.txt" 2>&1
    local args=(--input "$input" --threads "$threads" --iterations "$iterations")
    perf stat -r 5 -e cycles:u,instructions:u -- "$executable" "${args[@]}" >"results/${name}_cycles_instructions.txt" 2>&1
    if [[ "$name" == original || "$name" == optimized_1t ]]; then
        perf stat -r 5 -e cache-references:u,cache-misses:u -- "$executable" "${args[@]}" >"results/${name}_cache.txt" 2>&1
    fi
    if [[ "$name" != original ]]; then
        perf stat -r 5 -e task-clock,context-switches,cpu-migrations,page-faults -- "$executable" "${args[@]}" >"results/${name}_os_activity.txt" 2>&1
    fi
    post_temp=$(vcgencmd measure_temp | sed -E 's/.*=([0-9.]+).*/\1/')
    post_status=$(vcgencmd get_throttled)
    post_hex=${post_status#throttled=}
    {
        echo "after_temp=${post_temp} C"
        echo "after_${post_status}"
    } >>"results/${name}_thermal.txt"
    if (( (post_hex & 15) != 0 || ((post_hex ^ pre_hex) & 983040) != 0 )); then
        echo "Thermal/power warning in $name. Discard and repeat this block." | tee "results/${name}_INVALID.txt" >&2
        exit 1
    fi
}

measure_block original ./sobel_original 1
measure_block optimized_1t ./sobel_optimized 1
measure_block optimized_2t ./sobel_optimized 2
measure_block optimized_4t ./sobel_optimized 4
measure_block optimized_8t ./sobel_optimized 8
echo "Collection complete. Inspect event support, counter active time, and variation in every perf file."
