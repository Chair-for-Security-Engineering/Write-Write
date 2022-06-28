# Write-After-Write

This repository contains some code for our Write-After-Write paper published at RAID'22 (https://doi.org/10.1145/3545948.3545987). It includes three programs:
- Demo: A minimal example that shows the timing difference of Write+Write
- Evsets: Bottom-up construction of LLC eviction sets
- Clock Demo: A synchronization demo using the Write-After-Write clock across CPU cores

The programs have been tested on several CPUs. Note that we cannot guarantee that the code will run on your CPU. We cannot provide 
support -- however, if you have a question, feel free to open an issue here and we'll try to answer it.
The code is for demonstration purposes only. Any use is at your own risk. Please note that use for criminal purposes is prohibited and 
will be prosecuted. The authors are not liable for any damage caused. 

## Libtea Dependencies
The **demo** program has a dependency on Libtea, for installation instructions see [Libtea](https://github.com/libtea/frameworks).
It is sufficient to build the cache variant using `make libtea-x86-cache`. 
The demo program uses the library to get a verified minimal eviction set and access physical addresses. +

For the **evsets** program, libtea is optional and can be used to verify the results. 

The `libtea` submodule contains the Libtea repository. 

## Minimal Example (Demo)
The code for the minimal example is located in `src/minimal_example`. To build the program, simply run 
`make demo`. Make sure that libtea is installed before you execute the program.

Run the program with `sudo ./demo`. The output should look like this:
```
Found colliding candidate!
-------------------------------
Success #1 / #1
Victim:         0x55873aab6460  0x5d9270460
Candidate1:     0x55873aab6570  0x5d9270570
Candidate2:     0x7faa86208460  0x503770460
Victim Set:   17,        Candidate Set:   21,    Candidate Set:   17
Victim Slice: 4,         Candidate Slice: 0,     Candidate Slice: 4
Means: Group 0: 1241.187312, Group 1: 1256.021369, Diff: 14.834057
############

Took 0 seconds 4 milliseconds

Result: 1 matches, thereof 0 false positives.
```
If the victim set matches one of the candidate sets, the difference should be larger than 10 or smaller than -10. 
A value larger than 10 indicates a collision with the right candidate, a value smaller than -10 with the left respectively.

**Note:** After building the eviction set with libtea, the physical  addresses are checked for a collision on 10 bits. 
Depending on the cache configuration, a set collision does not automatically yield a collision with Write+Write.

## Eviction Set Construction (Evsets)
The code for the eviction set example is located in `src/evsets`. To build the program, simply run 
`make ev`. There are several configuration options that can (and should) be adjusted in `write+write.h`:
- Use `#define USE_LIBTEA` if you have libtea installed and want to verify the output
- Use `#define BENCH` to build eviction sets as fast as possible. Also, disable Libtea in this scenario. 
This option basically disables all printf statements within the timed code.
- `#define RUNS 30` defines the number of repetitions for each candidate  measured. 30 should give a fairly
good detection rate but if performance doesn't matter, it does not hurt to increase this to 300 or so. 
- `#define CACHE_MISS_THRESHOLD 200` defines the cache miss threshold. This varies drastically across CPUs. In
virtualized environments, this is generally higher. For standard desktop level CPUs try something like 100. 
- `#define OUTLIER_THRESHOLD 1400` This is a threshold value for which the program rejets a write timing measurement
and retries. If your program stalls, increase this. If you get a lot of false positives, decrease this. For most CPUs
values in the range of 900 to 1500 worked well.
- `#define MEM_SIZE 13000000` The size of the array that is searched for eviction set addresses. Best somewhere between
9000000 and 20000000. 
- `#define CACHE_ASSOC 16` set the associativity of your LLC. 

The values above worked well on the Xeon E-2224G.

To run the program, simply type `./ev_sets`. If you configured to use libtea, run it as root.
The output should look something like this:

```
...
-------------------------------
Success #173 / #205
Victim:         0x55d0ea828680
Candidate1:     0x7fdf11958680
Candidate2:     0x7fdf11960680
Victim Set:   986,       Candidate Set:   410,   Candidate Set:   986
Victim Slice: 1,         Candidate Slice: 3,     Candidate Slice: 3
Victim:          52ac6f680,
Candidate:       533e06680,
Candidate:       3ebcaf680
Means: Group 0: 1343.205059, Group 1: 1359.590894, Diff: 16.385835
############

Time taken 0 seconds 98 milliseconds

Result: 205 matches, thereof 32 false positives.

-----------  EV SET  -----------
Victim: 0x52ac6f680      Cache Set:  986, Cache Slice: 1

     0: 0x57df0f680      Cache Set:  986, Cache Slice: 1
     1: 0x51e35f680      Cache Set:  986, Cache Slice: 1
     2: 0x4b30ff680      Cache Set:  986, Cache Slice: 1
     3: 0x412e1f680      Cache Set:  986, Cache Slice: 1
     4: 0x43f82f680      Cache Set:  986, Cache Slice: 1
     5: 0x60545f680      Cache Set:  986, Cache Slice: 1
     6: 0x352e5f680      Cache Set:  986, Cache Slice: 1
     7: 0x21a44f680      Cache Set:  986, Cache Slice: 1
     8: 0x349c6f680      Cache Set:  986, Cache Slice: 1
     9: 0x3d9f8f680      Cache Set:  986, Cache Slice: 1
    10: 0x4c22ef680      Cache Set:  986, Cache Slice: 1
    11: 0x5583cf680      Cache Set:  986, Cache Slice: 1
    12: 0x4c13af680      Cache Set:  986, Cache Slice: 1
    13: 0x3480bf680      Cache Set:  986, Cache Slice: 1
    14: 0x458a9f680      Cache Set:  986, Cache Slice: 1
    15: 0x55cf4f680      Cache Set:  986, Cache Slice: 1
Reduction was successful
```
If the output ends with `The obtained eviction set is too small...`, you can retry or adjust the parameters.
If you get many false positives, try to adjust the `OUTLIER_THRESHOLD` or the `RUNS`. If you have a lot of
successes but still no eviction set, try to adjust `CACHE_MISS_THRESHOLD`, `MEM_SIZE` or `CACHE_ASSOC`.

## Covert Channel Synchronization (Covert)

The code for the covert channel example is located in `src/clock_demo`. To build the program, simply run 
`make`. If the code does not work out of the box, there are a few parameters that can be adjusted.

In `demo.c`:
- In line 154 is a hardcoded outlier threshold. You may need to adapt it to your CPU. Un-comment the printf statement in line 153 and 
choose a threshold that is just high enough to allow approx. 90% of the times printed. Remove the printf and try again.
- Try to change the `RING_BUFFER_SIZE` (line 10) or the `CLK_MOVING_AVERAGE_WINDOW` which selects the volatility of the moving average.

The program can be executed using `./demo [name] [core] [divider]`. To run the program, type for example `./demo a 1 1 & sleep 20; ./demo b 2 1`. 
This will create two text files (`a.txt` and `b.txt`) which contain timestamps when the clock changes from high to low and vice versa.
After some time, the program terminates. You can use `clock_eval.py` to analyze the results. It should look something like this:

![alt text](https://github.com/Chair-for-Security-Engineering/Write-Write/blob/master/src/clock_demo/sync.png)
