#include "util.h"

#define CLK_MOVING_AVERAGE_WINDOW 10
#define RING_BUFFER_SIZE 2000
int clock_divider_rate = 1;

struct clock_thread_args_t{
    bool clk;
    bool end;
    bool edge_detected;
    bool sync_edge;
    int cpu_core;
    char* name;
};

/**
 * @brief Generates the clock signal.
 */
void *clk_gen(void* arguments){
    volatile struct clock_thread_args_t *args = (struct clock_thread_args_t *)arguments;
    
    // init params
    args->clk = false;
    args->edge_detected = false;

    // set CPU affinity
    cpu_set_t  mask;
    CPU_ZERO(&mask);
    CPU_SET(args->cpu_core, &mask);

    if (sched_setaffinity(0, sizeof(mask), &mask))
	{
        printf("Failed to pin thread to core %d\n", args->cpu_core);
    }
    
    // Initilaize the ring buffers to compute the averages
    int64_t ring_buffer[RING_BUFFER_SIZE] = {0};
    int ring_buffer_counter = 0;
    int64_t ring_buffer2[RING_BUFFER_SIZE] = {0};
    int ring_buffer_counter2 = 0;

    // Select some address to probe the latency
    uint64_t* addr = (uint64_t*) malloc(8);

    uint64_t time = 0, timestamp = 0;
    uint64_t last_edge_ts = 0;
    // initialized with some average number we found. Over time, this is updated but it's good to have a somewhat correct mean period
    uint64_t mean_clk_period = 2147722305; 
    
    bool ready = true;
    bool internal_clk = false;
    bool sync = false;
    int clk_divider = 0;
    

    // starting timestamp
    asm volatile(
        #ifdef HAS_RDTSCP
        "rdtscp\n\t"                // get timestamp
        #else
        "lfence\n\t"
        "rdtsc\n\t"
        #endif  
        "shl $32, %%rdx\n\t"       // rdx << 32
        "or %%rdx, %%rax\n\t"      // rax = rdx | rax
        "mov %%rax, %[ts]\n\t"     // return rax
        :[ts]"=r"(last_edge_ts)::"eax", "ecx", "edx");

    uint64_t last_out = last_edge_ts;

    while(!args->end){
        time = 0;

        asm volatile(
            //"mfence\n\t"
            "cpuid\n\t"                 // reduce noise by serializing
            "nop\n\t"                   // reduce noise by nops
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            #ifdef HAS_RDTSCP
            "rdtscp\n\t"                    // start timestamp
            #else
            "lfence\n\t"
            "rdtsc\n\t"
            #endif
            "shl $32, %%rdx\n\t"            // combine the timestamp
            "or %%rdx, %%rax\n\t"
            "mov %%rax, %%r15\n\t"          // mov timestamp to r15 
            "movq %%rdx, (%[addr])\n\t"     // write to the address
            //"mfence\n\t"
            "cpuid\n\t"                     // serialize
            "nop\n\t"                       // nops for better measurement
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            #ifdef HAS_RDTSCP
            "rdtscp\n\t"                    // end timestamp
            #else
            "lfence\n\t"
            "rdtsc\n\t"
            #endif
            "shl $32, %%rdx\n\t"            // combine the timestamp
            "or %%rdx, %%rax\n\t"
            "sub %%r15, %%rax\n\t"          // compute delta
            "mov %%rax, %[res]\n\t"         // output the timestamp
            "mov %%r15, %[ts]\n\t"          // and the delta
            "cpuid\n\t"
            : [res]"=r"(time), [ts]"=r"(timestamp) : [addr]"r"(addr): "rax", "ebx", "rdx", "rcx", "r15");
        
        // Filter outliers
        if (time < 1500){
            // insert measurement to ring buffer
            ring_buffer[ring_buffer_counter] = time;
            ring_buffer_counter ++;
            // wrap the ring buffer if we are at the end
            if(ring_buffer_counter == RING_BUFFER_SIZE){
                ring_buffer_counter = 0;
            }

            // compute the average of the measured times
            double avg = get_avg(ring_buffer, RING_BUFFER_SIZE);
        
            // Put the difference of the current measurement - the average to the second ring buffer
            ring_buffer2[ring_buffer_counter2] = time-avg;
            ring_buffer_counter2 ++;
            // Wrap the second ring buffer if we are at the end
            if(ring_buffer_counter2 == RING_BUFFER_SIZE){
                ring_buffer_counter2 = 0;
                ready = true;
            }
            
            // compute mean over second ring buffer
            double normalized = get_avg(ring_buffer2, RING_BUFFER_SIZE);
            
            // Spike detection in ring buffer 2 (positive change)
            if(normalized > 17 && internal_clk == false && ready){
                // Adjust the mean clk period
                mean_clk_period = mean_clk_period - \
                    (mean_clk_period / CLK_MOVING_AVERAGE_WINDOW) + \
                    ((timestamp - last_edge_ts) / CLK_MOVING_AVERAGE_WINDOW);
                
                last_edge_ts = timestamp;
                
                internal_clk = true;
                ready = false; // do not detect spikes for the next few iterations
                ring_buffer_counter2 = 0;
                sync = true; // internal notification for edge                
            }

            // Spike detection in ring buffer 2 (negative change)
            if(normalized < -17 && internal_clk == true && ready){
                // Adjust the mean clk period
                mean_clk_period = mean_clk_period - \
                    (mean_clk_period / CLK_MOVING_AVERAGE_WINDOW) + \
                    ((timestamp - last_edge_ts) / CLK_MOVING_AVERAGE_WINDOW);

                last_edge_ts = timestamp;

                internal_clk = false;
                ready = false; // do not detect spikes for the next few iterations
                ring_buffer_counter2 = 0; 
                sync = true; // internal notification for edge
            }
        }
        // Divide the mean clock period in smaller chunks
        // leave some margin at the end in case the next clk is earlier
        double output_period = mean_clk_period / (clock_divider_rate*1.05);

        // Periodically change clk based on timing or if an edge is reported
        if(((int64_t)timestamp - last_out >= output_period && clk_divider < clock_divider_rate-2) || sync){
            last_out = timestamp;
            args->clk = !args->clk;
            args->edge_detected = true;
            clk_divider ++;
            if(sync){
                args->sync_edge = true;
                sync = false;
                clk_divider = 0;
            }else{
                args->sync_edge = false;
            }
            
        }
    }
}