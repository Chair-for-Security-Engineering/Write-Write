# Write-After-Write

This repository contains some code for our Write-After-Write paper. It includes three porgrams:
- Demo: A minimal example that shows the timing difference of Write+Write
- Evsets: Bottom-up construction of LLC eviction sets
- Covert: A synchronization demo using the Write-After-Write clock across CPU cores

## Libtea Dependecies
The **demo** program has a dependecy on Libtea, for installation instructions see [Libtea](https://github.com/libtea/frameworks).
It is sufficient to build the cache variant using `make libtea-x86-cache`. 
The demo program uses the library to get a verified minimal eviciton set and access physical addresses. +

For the **evsets** program, libtea is optional and can be used to verify the results. 

The `lib` folder contains a copy of the Libtea repository. Feel free to install it from here or from the official repo.

## Minimal Example (Demo)
The code for the minimal example is located in `src/minimal_example`. To build the program, simply run 
`make demo`. Make sure that libtea ist installed before you execute the program.

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
A value larger than 10 indicates a collision with the right candiate, a value smaller than -10 with the left respectively.

**Note:** After building the eviction set with libtea, the pyhsical addresses are checked for a collision on 10 bits. 
Depending on the cache configuration, a set collision does not automatically yield a collision with Write+Write.

## Eviction Set Construction (Evsets)
The code for the eviction set example is located in `src/evsets`. To build the program, simply run 
`make ev`. There are several configuration options that can (and should) be adjusted in `write+write.h`:
- Use `#define USE_LIBTEA` if you have libtea installed and want to verify the output
- Use `#define BENCH` to build eviction sets as fast as possible. Also, disable Libtea in this scenario. 
This option basically disables all printf statements within the timed code.
- `#define RUNS 30` defines the number of repetitions for each canidate measured. 30 should give a fairly
good detection rate but if performance doesn't matter, it does not hurt to increase this to 300 or so. 
- `#define CACHE_MISS_THRESHOLD 200` defines the cache miss threshold. This varies drastically across CPUs. In
virtualized environments, this is generally higher. For standard desktop level CPUs try something like 100. 
- `#define OUTLIER_THRESHOLD 1400` This is a threshold value for which the program rejets a write timing measurement
and retries. If your program stalls, increase this. If you get a lot of false positives, decrease this. For most CPUs
values in the range of 900 to 1500 worked well.
- `#define MEM_SIZE 13000000` The size of the array that is searched for eviction set addresses. Best somewhere between
9000000 and 20000000. 
- `#define CACHE_ASSOC 16` set the associativity of you LLC. 

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
Reduction was successfull
```
If the output ends with `The obtained eviction set is too small...`, you can retry or adjust the parametes.
If you get many false positives, try to adjust the `OUTLIER_THRESHOLD` or the `RUNS`. If you have a lot of
successes but still no eviction set, try to adjust `CACHE_MISS_THRESHOLD`, `MEM_SIZE` or `CACHE_ASSOC`.

## Covert Channel Synchronization (Covert)

The code for the covert channel example is located in `src/clock_sync`. To build the program, simply run 
`make covert`. There are a few parameters that can be adjusted. 

In `covert.h`:
- `#define COVERT_CHANNEL_THRESHOLD 200` adjusts the threshold for the covert channel.
For Flush+Reload, this is the threshold distinguishing cache hits from misses.

In `covert.c`:
- `#define HAS_RDTSCP` if your machine supports the `rdtscp` instruction. A combination of `rdtsc` and `mfence`
is used instead.
- `#define FLUSH_FLUSH` or `#define FLUSH_RELOAD` for the respective channel. Remember to also adjust the threshold.
Never define both, we don't know what happens then. <sub>Probably nothing, but still, don't.<sub>

In `synchronization.h`:
- `#define CLK_MOVING_AVERAGE_WINDOW 10` selects the volatility of the moving average
- `#define RING_BUFFER_SIZE 2000` sets the ring buffer size

To run the program, simply type `./covert [clock divider]`. For clock divider, use any integer. The default  is 10.
The programm will wait for approx. 10 seconds and then start transmitting the message. This is to de-synchronize the
threads if something does not work - after all, we started the threads at the same time so they are somewhat synchronized
to begin with. 
The output should look something like this:
```
./covert 
Clock divider is set to 10
Sender waits for 10 seconds...
Sender starts to transmit
Sender done (30.296867 s).
Receiver done.
in:  010010000110010101101100011011000110111100100001
out: 01001000011001010110110001101100011011110010000
```
As you see above, the last symbol is missing - this is a bug since the receiver is stopped to early. For high
clock divider parameters, it might be that there are zeros at the end. Fix it if you like ;-) 
