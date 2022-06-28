#define _GNU_SOURCE

#include "math.h"
#include <unistd.h>
#include <sched.h>
#include "libtea.h"
#define RUNS 3000

double get_mean(uint64_t* results, int size);
void print_hist(uint64_t* results_0, uint64_t* results_1, int size);
int to_freq_domain(uint64_t* group_0, uint64_t* group_1, int size, uint64_t** out1, uint64_t** out2);
void demo(uint64_t* target, uint64_t* candidate_0, uint64_t* candidate_1);

libtea_instance* instance;
uint64_t results_0[RUNS];
uint64_t results_1[RUNS];


void demo(uint64_t* target, uint64_t* candidate_0, uint64_t* candidate_1){ 
    
    uint64_t time;
    volatile int decision = 0;
    volatile int decision_ctr = 0;

    int success_ctr = 0, failure_ctr = 0;
    int group_0_ctr = 0, group_1_ctr = 0;
    double group_0_mean = 0, group_1_mean = 0;

    clock_t before = clock();

    // Main Loop
    for(int i = 0; i < 2*RUNS; i++){
        // Switch between the candidate and the random address every 2nd iteration
        decision = decision_ctr < 2 ? 1 : 0;
        decision_ctr ++;
        decision_ctr %= 4;

        asm volatile(
            "cpuid\n\t"                         // Make sure the pipeline is empty on start - not strictly reuqired
            "clflush (%[target])\n\t"           // Flush the target address
            "test %[decision], %[decision]\n\t" // test if decision = 0
            "lea (%[candidate_0]), %%rax\n\t"   // rax = candidate0 (random)
            "lea (%[candidate_1]), %%rbx\n\t"   // rbx = candidate1 (colliding)
            "cmove %%rax, %%rcx\n\t"            // conditional move -> if decision == 0, rcx=rax
            "cmovne %%rbx, %%rcx\n\t"           // else -> rcx = rbx
            "movq %%rax, (%%rcx)\n\t"           // write something to rcx
            "cpuid\n\t"                         // serialize
            "rdtscp\n\t"                        // start timing measurement 
            "shl $32, %%rdx\n\t"                // combine the timestamp
            "or %%rdx, %%rax\n\t"
            "mov %%rax, %%r15\n\t"              // move the full timestamp to r15 so it doesn't get lost
            "movq  %%rdx, (%[target])\n\t"      // Write to the target address
            //"mfence\n\t"
            "cpuid\n\t"                         // serializing to make sure the measurement contains the whole operation
            "rdtscp\n\t"                        // stop timing measurement
            "shl $32, %%rdx\n\t"                // combine the timestamp
            "or %%rdx, %%rax\n\t"
            "sub %%r15, %%rax\n\t"  	        // compute the difference
            "mov %%rax, %[out]\n\t"
            : [out]"=r"(time) : [decision]"r"(decision), [candidate_1]"r"(candidate_1), [candidate_0]"r"(candidate_0), [target]"r"(target) : "rax", "rbx", "rcx", "rdx", "r15"
        );
        // Assign the measurement to the respective group if the measured time seems valid.
        if(time < 4000){ 
            if(decision == 1){
                results_0[group_1_ctr] = time;
                group_1_ctr++;
                group_1_mean += (int) time; 
            }else{
                results_1[group_0_ctr] = time;
                group_0_ctr++;
                group_0_mean += (int) time; 
            }
        }
    }  
        
        
    // Compute the means and print the result.
    group_0_mean /= group_0_ctr;
    group_1_mean /= group_1_ctr;
    size_t vpaddr = libtea_get_physical_address(instance, (size_t)target);
    size_t paddr_0 = libtea_get_physical_address(instance, (size_t)candidate_0);
    size_t paddr_1 = libtea_get_physical_address(instance, (size_t)candidate_1);
    int victim_set = libtea_get_cache_set(instance, vpaddr);
    int candidate_0_set = libtea_get_cache_set(instance, paddr_0);
    int candidate_1_set = libtea_get_cache_set(instance, paddr_1);
    int victim_slice = libtea_get_cache_slice(instance, vpaddr);
    int candidate_0_slice = libtea_get_cache_slice(instance, paddr_0);
    int candidate_1_slice = libtea_get_cache_slice(instance, paddr_1);
    printf("-------------------------------\n");
    if (group_0_mean-group_1_mean > 10) {
        if (victim_set == candidate_0_set){
            success_ctr++;
            printf("Success #%d / #%d\n", success_ctr, success_ctr+failure_ctr);
        }else{
            failure_ctr++;
            printf("Failure #%d / #%d\n", failure_ctr, success_ctr+failure_ctr);
        }
    }else if(group_0_mean-group_1_mean < -10) {
        if (victim_set == candidate_1_set){
            success_ctr++;
            printf("Success #%d / #%d\n", success_ctr, success_ctr+failure_ctr);
        }else{
            failure_ctr++;
            printf("Failure #%d / #%d\n", failure_ctr, success_ctr+failure_ctr);
        }
    }else{
        printf("Result ambiguous. Please retry.\n");
    }
    printf("Victim:\t \t%p\t%p\nCandidate1:\t%p\t%p\nCandidate2:\t%p\t%p\n", (void*) target, (void*) vpaddr, (void*) candidate_0, (void*) paddr_0, (void*) candidate_1, (void*) paddr_1);
    printf("Victim Set:   %d,\t Candidate Set:   %d,\t Candidate Set:   %d\n", victim_set, candidate_0_set, candidate_1_set);
    printf("Victim Slice: %d,\t Candidate Slice: %d,\t Candidate Slice: %d\n", victim_slice, candidate_0_slice, candidate_1_slice);
    printf("Means: Group 0: %6.6f, Group 1: %6.6f, Diff: %6.6f\n", group_0_mean, group_1_mean, group_1_mean-group_0_mean);
    //print_hist(results_1, results_0, RUNS);
        
    printf("############\n\n");
    clock_t difference = clock() - before;
    long msec = difference * 1000 / CLOCKS_PER_SEC;
    printf("Took %ld seconds %ld milliseconds\n",
        msec/1000, msec%1000);

    printf("\nResult: %d matches, thereof %d false positives.\n\n", success_ctr+failure_ctr, failure_ctr);
}


void setup_libtea(){
    instance = libtea_init();
    if (!instance){
        printf("Libtea initialization failed. Please install libtea and try again.\n");
        exit(1);
    }
}

void print_hist(uint64_t* group_0, uint64_t* group_1, int size){
    // Construct the histogram
    uint64_t minval = 0xFFFFFFFFF, maxval = 0;
    for(int i=0; i<size; i++){
        if (group_0[i] < minval){
            minval = group_0[i];
        }
        if (group_1[i] < minval){
            minval = group_1[i];
        }
        if (group_0[i] > maxval){
            maxval = group_0[i];
        }
        if (group_1[i] > maxval){
            maxval = group_1[i];
        }
    }
    int freqsize = maxval - minval + 1 ;

    int frequency[2][freqsize]; 
    for (int i=0; i<freqsize; i++) { 
        frequency[0][i] = 0;  
        frequency[1][i] = 0;
    }

    for(int i=0; i<size; i++){
        frequency[0][group_0[i] - minval] ++;
        frequency[1][group_1[i] - minval] ++;
    }

    for(int i = 0; i< freqsize; i+=2){
        printf("%3ld\t %7d \t %7d\n", i+minval, frequency[0][i], frequency[1][i]);
    }
}

double get_mean(uint64_t* results, int size){
    double mean = 0;
    for(int i=0; i < size; i++){
        mean += (int) results[i];
    }
    return mean/(size*1.0);
}

int to_freq_domain(uint64_t* group_0, uint64_t* group_1, int size, uint64_t** out1, uint64_t** out2){
    int minval = 0xFFFFFF, maxval = 0;
    for(int i=0; i<size; i++){
        if (group_0[i] < minval){
            minval = group_0[i];
        }
        if (group_1[i] < minval){
            minval = group_1[i];
        }
        if (group_0[i] > maxval){
            maxval = group_0[i];
        }
        if (group_1[i] > maxval){
            maxval = group_1[i];
        }
    }
    int freqsize = maxval - minval + 1 ;

    //int frequency[2][freqsize]; 
    *out1 = (uint64_t*) malloc(freqsize*sizeof(uint64_t));
    *out2 = (uint64_t*) malloc(freqsize*sizeof(uint64_t));
    for (int i=0; i<freqsize; i++) { 
        (*out1)[i] = 0;  
        (*out2)[i] = 0;
    }

    for(int i=0; i<size; i++){
        (*out1)[group_0[i] - minval] ++;
        (*out2)[group_1[i] - minval] ++;
    }

    return freqsize;
}


int main(){
    srand(time(NULL));

    if(geteuid() != 0)
    {
        printf("Program must be run as root. Exit.\n");
        exit(1);
    }

    setup_libtea();    

    printf("%d\n", instance->llc_sets);
    printf("%d\n", instance->llc_slices);
    
    // Select a target address and one that does not collide. Make sure these are not in the same cache line.
    uint64_t* target = malloc(256);
    target[0] = 0;
    uint64_t* random_address = malloc(256);
    random_address[0] = 0;

    size_t paddr = libtea_get_physical_address(instance, (size_t)(target));
    if(paddr == LIBTEA_ERROR){
        printf("Could not get physical address...\n");
    }

    // Use libtea to construct an eviction set. We use addresses from the eviction set since these are likely to collide wiht W+W
    libtea_eviction_set ev;
    if (!libtea_build_eviction_set(instance, &ev, paddr)==LIBTEA_SUCCESS){
        printf("Could not get eviction set...\n");
    }

    // Search the eviction set for a colliding address. On CPUs that use 10 or bits for cache indexing, this is not required
    // If libtea_get_phyiscal_address fails, modify the function and close fd after reading. This is a bug in libtea.
    size_t ev_paddr = libtea_get_physical_address(instance, (size_t)(ev.address[0]));
    size_t *colliding_address;
    for(int i = 0; i < ev.addresses; i++){
        ev_paddr = libtea_get_physical_address(instance, (size_t)(ev.address[i]));
        if((ev_paddr & 0xFF00) == (paddr & 0xFF00)){
            colliding_address = (size_t*) ev.address[i];
            printf("Found colliding candidate!\n");
            break;
        }
    }
    if(!colliding_address){
        printf("Failed!\n");
        exit(1);
    }


    demo(target, random_address, ev.address[0]);

    free(random_address);
    free(target);
    return 0;
}