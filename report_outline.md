# Lab 7 report outline — fill only with measured Pi data

## Title and executive summary
State the Pi model, the best valid configuration, its measured improvement,
and the main counter-based reason for the recommendation.

## Implementations and experimental method
Describe the Lab 2 scalar reference, Lab 6 pointer/threaded implementation,
exact compiler flags, common output definition, benchmark loop, frame source
and size, iteration count, compiler and kernel versions, Pi cooling/power,
counter availability, repetitions, thermal checks, and discarded blocks.

## Correctness evidence
Table 1: configuration, checksum, maximum pixel difference, differing pixels,
pass. Explain what the checksum and full-image comparison each establish.

## Results
Table 2: original versus optimized-1T, with time, cycles, instructions,
instructions per cycle (IPC), cache references, misses, and miss rate.
Table 3: 1/2/4/8 threads, with time, speedup, cycles, instructions, IPC,
effective CPUs, context switches, and migrations. Include units and normalize
per frame where specified. Figure 1: time and speedup versus thread count.
Show one sample calculation each for IPC, miss rate, and effective CPU use.

## Analysis and discussion
Start with a source-level optimization hypothesis and the counter changes
predicted **before** reading results. Test it against instructions, cycles,
IPC, time, and cache data. Discuss the most unexpected result. Explain thread
scaling using process-wide totals and OS activity, including what the 8T data
does or does not support. Discuss unsupported events, multiplexing, noise,
thermal/power warnings, process scope, and one additional measurement that
could test an uncertain explanation. Refer to every table and figure.

## Recommendation and conclusion
Recommend one configuration using correctness, elapsed time, resource use,
and implementation complexity. Cite at least three numbers, two from
counter-derived metrics. Finish with a testable next experiment in the
required “If I change ... then ... because ... I would test ... while holding
... constant” form.

Export a polished 4–6 page PDF with 11–12 point text, approximately 1-inch
margins, page numbers, numbered/captioned tables and figures, and a maximum
of 8 pages of main content. Submit raw `results/` files separately.
