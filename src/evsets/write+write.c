#include "write+write.h"


struct eviction_set_t* get_evset(uint64_t* addr_space, uint64_t* victim, uint64_t addr_space_size){ 
    
    uint64_t time;
    volatile int decision = 0;
    volatile int decision_ctr = 0;
    int success_ctr = 0, failure_ctr = 0;

    // Initialize the eviction set list.
    struct eviction_set_t *ev_set = malloc(sizeof(struct eviction_set_t));
    struct eviction_set_t *current = ev_set;

    // Get the set bits that are controlled by the virtual address
    uint64_t victim_set = (uint64_t) victim & 0xFC0;
    uint64_t addr_space_set = (uint64_t) addr_space & 0xFC0;

    // Make sure the start address is within the allocated array.
    uint64_t *start_address;
    if (addr_space_set < victim_set){
        addr_space += victim_set;
    }

    // Align the lower 12 bits of the victim and the candidate addresses, ignore the offset
    start_address = (uint64_t*) (((uint64_t)addr_space & 0xFFFFFFFFFFFFF000 ) | victim_set);

    #ifndef BENCH 
    printf("%p\n%p\n", (void*) victim, (void*) start_address);
    #endif

    // Some variables for the main loop
    int ctr;
    double mean[2] = {0, 0};
    void* candidate_0;
    void* candidate_1;

    #ifndef TRY_UNTIL_SUCCESS
    clock_t before = clock();
    #endif //TRY_UNTIL_SUCCESS
    // Main loop
    for (uint64_t i = 0; i < addr_space_size-2*0x1000; i+=2*0x1000)
    {
        ctr = 0;
        // Set the candidate addresses
        candidate_0 = (void*) &(start_address[i]);
        candidate_1 = (void*) &(start_address[i+0x1000]);

        while((ctr) != 2*RUNS){
            decision = (ctr & 0x2) >> 1;
            
            retry: 
            asm volatile(
                "cpuid\n\t"                         // Clear all active instructions before we start. This is not strictly required
                "clflush (%[victim])\n\t"           // Flush the victim address
                "test %[decision], %[decision]\n\t" // test if decision = 0
                "lea (%[candidate_0]), %%rax\n\t"   // rax = *candidate0
                "lea (%[candidate_1]), %%rbx\n\t"   // rbx = *candidate1
                "cmove %%rax, %%rcx\n\t"            // conditional move -> if decision == 1, rcx=rax
                "cmovne %%rbx, %%rcx\n\t"           // else -> rcx = rbx
                "movq %%rax, (%%rcx)\n\t"           // write to rcx
                "mfence\n\t"                    
                "cpuid\n\t"                         // serialization
                "nop\n\t"                           // alignment
                "nop\n\t"
                "nop\n\t"                           // We found that this reduces the number of outliers in the measurements
                "nop\n\t"                           // but it also works without...
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
                "rdtscp\n\t"                        // start the timing
                "shl $32, %%rdx\n\t"                // combine the timestamp
                "or %%rdx, %%rax\n\t"
                "mov %%rax, %%r15\n\t"              // move timestamp out of the way
                "movq %%rdx, (%[victim])\n\t"       // write to the victim address
                "mfence\n\t"
                "cpuid\n\t"                         // serialization
                "nop\n\t"                           // nops for imporved stability of timing measurement
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
                "rdtscp\n\t"                        // get the timestamp
                "shl $32, %%rdx\n\t"                // combine it
                "or %%rdx, %%rax\n\t"
                "sub %%r15, %%rax\n\t"              // compute the difference from the first timestamp
                "mov %%rax, %[out]\n\t"
                : [out]"=r"(time) : [decision]"r"(decision), [candidate_0]"r"(candidate_0), [candidate_1]"r"(candidate_1), [victim]"r"(victim) : "rax", "rbx", "rcx", "rdx", "r15"
            );


            if(time > OUTLIER_THRESHOLD){ // OUTLIER_THRESHOLD is kinda important in finetuning the evset construction. Ideal value depends on the CPU.
                goto retry; // sorry... ¯\_('_')_/¯
            }
            // Store the measured time
            mean[decision] += time;
            ctr++;
            
        }  
        // Compute the means
        mean[0] /= RUNS;
        mean[1] /= RUNS;

        // Check if we have a significant difference in means.
        if(fabs(mean[0]-mean[1]) > 10){
            #ifdef USE_LIBTEA 
            size_t vpaddr = libtea_get_physical_address(instance, (size_t)victim);
            size_t paddr = libtea_get_physical_address(instance, (size_t)candidate_0);
            size_t paddr2 = libtea_get_physical_address(instance, (size_t)candidate_1);
            int victim_set = libtea_get_cache_set(instance, vpaddr);
            int candidate_0_set = libtea_get_cache_set(instance, paddr);
            int candidate_1_set = libtea_get_cache_set(instance, paddr2);
            int victim_slice = libtea_get_cache_slice(instance, vpaddr);
            int candidate_0_slice = libtea_get_cache_slice(instance, paddr);
            int candidate_1_slice = libtea_get_cache_slice(instance, paddr2);
            #ifndef BENCH
            printf("-------------------------------\n");
            #endif //BENCH
            #else
            #ifndef BENCH 
            printf("Found candidate\n");
            
            #endif // BENCH
            success_ctr++;
            #endif // USE LIBTEA
            // If the difference is positive, candidate 0 collides
            if (mean[0]-mean[1] > 0) {
                #ifdef USE_LIBTEA
                if (victim_set == candidate_0_set){
                    success_ctr++;
                    #ifndef BENCH
                    printf("Success #%d / #%d\n", success_ctr, success_ctr+failure_ctr);
                    
                    //print_hist(results_coll, results_no_coll, group_0_ctr);
                    if (victim_slice == candidate_0_slice){
                        printf("Eviction Set Candidate!\n");
                    }
                    #endif
                }else{
                    failure_ctr++;
                    #ifndef BENCH
                    printf("Failure #%d / #%d\n", failure_ctr, success_ctr+failure_ctr);
                    #endif
                }
                #endif
                // Add the address to the eviction set list.
                current->address = candidate_0;
                current->next = (struct eviction_set_t*) malloc(sizeof(struct eviction_set_t));
                current = current->next;
            }else{ // Mean is negative --> candidate 1 collides
                #ifdef USE_LIBTEA
                if (victim_set == candidate_1_set){
                    success_ctr++;
                    #ifndef BENCH
                    printf("Success #%d / #%d\n", success_ctr, success_ctr+failure_ctr);
                    if (victim_slice == candidate_1_slice){
                        printf("Eviction Set Candidate!\n");
                    }
                    #endif //BENCH
                }else{
                    failure_ctr++;
                    #ifndef BENCH
                    printf("Failure #%d / #%d\n", failure_ctr, success_ctr+failure_ctr);
                    #endif //BENCH
                }
                #endif
                // Add the address to the eviction set.
                current->address = candidate_1;
                current->next = (struct eviction_set_t*) malloc(sizeof(struct eviction_set_t));
                current = current->next;
            }
            #ifndef BENCH
            printf("Victim:\t \t%p\nCandidate1:\t%p\nCandidate2:\t%p\n", (void*) victim, candidate_0, candidate_1);
            #ifdef USE_LIBTEA
            printf("Victim Set:   %d,\t Candidate Set:   %d,\t Candidate Set:   %d\n", victim_set, candidate_0_set, candidate_1_set);
            printf("Victim Slice: %d,\t Candidate Slice: %d,\t Candidate Slice: %d\n", victim_slice, candidate_0_slice, candidate_1_slice);
            printf("Victim:\t\t %8lx,\nCandidate:\t %8lx,\t\nCandidate:\t %8lx\n", vpaddr, paddr,paddr2);
            #endif // USE_LIBTEA
            printf("Means: Group 0: %6.6f, Group 1: %6.6f, Diff: %6.6f\n", mean[0], mean[1], mean[1]-mean[0]);
            #endif // BENCH
        }
        
    }
    #ifndef TRY_UNTIL_SUCCESS
    // Print timing stats.
    clock_t difference = clock() - before;
    printf("############\n\n");
    long msec = difference * 1000 / CLOCKS_PER_SEC;
    printf("Time taken %ld seconds %ld milliseconds\n",
        msec/1000, msec%1000);
    

    #ifdef USE_LIBTEA
    printf("\nResult: %d matches, thereof %d false positives.\n\n", success_ctr+failure_ctr, failure_ctr);
    #else
    printf("\nResult: %d matches.\n\n", success_ctr);
    #endif
    #endif // TRY_UNTIL_SUCCESS
    return ev_set;
}

/**
 * @brief Returns true if the ev_set was successfully reduced to a minimal ev-set
 * 
 * @param victim -> the victim address
 * @param ev_set_ptr -> a pointer to the start pointer of the eviction set
 * @return true if successful
 * @return false else
 */
bool reduce_evset(uint64_t *victim, struct eviction_set_t **ev_set_ptr){
    struct eviction_set_t *ev_set = *ev_set_ptr;
    int len = get_evset_len(ev_set);
    int abort_ctr = 0;

    // If this fails CACHE_ASSOC times, chances are that something went wrong...
    while(abort_ctr < CACHE_ASSOC+2){
        // get the current eviction set 
        ev_set = *ev_set_ptr;
        // Check whether the current eviction set is functional, if not return false
        if(test_evset(victim, ev_set) == false){
            return false;
        }
        // If the eviction set works and has length w, we are done -> return true.
        if(len == CACHE_ASSOC){
            // We do a double check whether it actually works...
            if(test_evset(victim, ev_set) == false){
                return false;
            }
            return true;
        }
        
        // Remove the first element from the eviction set
        struct eviction_set_t *removed_element = ev_set;
        *ev_set_ptr = ev_set->next;
        removed_element->next = NULL;

        // Recursive call to this function with a smaller eviction set. 
        // If this returns true, the smaller eviction set is functional and we can free the removed element.
        if(reduce_evset(victim, ev_set_ptr) == true){
            free(removed_element);
            return true;
        }
        // If the reduced eviction set did not work, we need to re-insert the removed element and try again.
        append_evset_address(*ev_set_ptr, removed_element);
        abort_ctr ++;
    }
    return false;
}

/**
 * @brief Appends an address to the eviction set list.
 */
void append_evset_address(struct eviction_set_t* ev_set, struct eviction_set_t* e){
    struct eviction_set_t *current = ev_set;
    while(current->next->next != NULL){
        current = current->next;
    }
    struct eviction_set_t *last = current->next;
    current->next = e;
    e->next = last;
}

void merge_evsets(struct eviction_set_t** a, struct eviction_set_t** b){
    if(*a == NULL){
        *a = *b;
    }else{
        struct eviction_set_t *current = *a;
        while(current->next->next != NULL){
            current = current->next;
        }
        free(current->next); // delete the old end-item
        current->next = *b;
    }
    
}

/**
 * @brief Get the length of the eviction set
 */
int get_evset_len(struct eviction_set_t* ev_set){
    int i = 0;
    struct eviction_set_t *current = ev_set;
    while(current->next != NULL){
        i++;
        current = current->next;
    }
    return i;
}

bool test_evset(uint64_t *victim, struct eviction_set_t *ev_set){
    uint64_t t_probe = 0;
    // Filter measurements that are not plausible
    while(t_probe < 30 || t_probe > 400){
        // Access the victim address
        asm volatile("movq (%0), %%rax\n" : : "r"(victim) : "rax");

        // We access the eviction set addresses multiple times to make sure that they really are cached
        struct eviction_set_t *current = ev_set;
        struct eviction_set_t *prev = ev_set;
        while(current->next != NULL){
            // Access the current and the previous ev-address
            asm volatile("movq (%0), %%rax\n" : : "r"(current->address) : "rax");
            asm volatile("movq (%0), %%rax\n" : : "r"(prev->address) : "rax");
            prev = current;
            current = current->next;
        }
        // Second iteration to REALLY make sure the victim was replaced if it collides...
        current = ev_set;
        while(current->next != NULL){
            asm volatile("movq (%0), %%rax\n" : : "r"(current->address) : "rax");
            current = current->next;
        }

        // Measure the access time to the victim
        asm volatile(
            "nop\n\t" //alignment
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
            "rdtscp\n\t"                        // Start measurement
            "shl $32, %%rdx\n\t"                // combine the timestamp
            "or %%rdx, %%rax\n\t"
            "mov %%rax, %%r15\n\t"              // Move the timestamp out of the way
            "movq (%[victim]), %%rdx\n\t"       // Access the victim address
            "mfence\n\t"                        // Make sure rtscp isn't executed out of order
            "nop\n\t"                           // Nops for improved accuracy
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
            "rdtscp\n\t"                        // End the timing measurement
            "shl $32, %%rdx\n\t"                // Combine the timestamp
            "or %%rdx, %%rax\n\t"
            "sub %%r15, %%rax\n\t"              // Compute the difference
            "mov %%rax, %[out]"
        : [out]"=r"(t_probe) : [victim]"r"(victim) : "rax", "rbx", "rcx", "rdx", "r15");
    }

    if (t_probe > CACHE_MISS_THRESHOLD){ // very basic test of whether ev evicts the target.
        return true;
    }else{
        return false;
    }
}



void print_evset(struct eviction_set_t* ev_set, uint64_t* victim){
    struct eviction_set_t *current = ev_set;
    int i = 0;

    printf("-----------  EV SET  -----------\n");

    #if defined(USE_LIBTEA) || defined(VERIFY)
    size_t paddr = libtea_get_physical_address(instance, (size_t)victim);
    int set = libtea_get_cache_set(instance, paddr);
    int slice = libtea_get_cache_slice(instance, paddr);
    printf("Victim: %p\t Cache Set: %4d, Cache Slice: %d\n\n", (void*) paddr, set, slice);
    #endif


    while(current->next != NULL){
        #if defined(USE_LIBTEA) || defined(VERIFY)
        paddr = libtea_get_physical_address(instance, (size_t)current->address);
        set = libtea_get_cache_set(instance, paddr);
        slice = libtea_get_cache_slice(instance, paddr);
        printf("    %2d: %p\t Cache Set: %4d, Cache Slice: %d\n", i, (void*) paddr, set, slice);
        #else
        printf("    %2d: %p\n", i, current->address);
        #endif
        ++i;
        current = current->next;
    }
}

void calibrate(uint64_t *victim){
    printf("Hit / Miss stats\n");
    uint64_t t_probe;
    asm volatile("clflush (%0)\n\t"::"r"(victim):);
    // Measure the access time to the victim
    asm volatile(
        "nop\n\t" //alignment
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
        "rdtscp\n\t"                        // Start measurement
        "shl $32, %%rdx\n\t"                // combine the timestamp
        "or %%rdx, %%rax\n\t"
        "mov %%rax, %%r15\n\t"              // Move the timestamp out of the way
        "movq (%[victim]), %%rdx\n\t"       // Access the victim address
        "mfence\n\t"                        // Make sure rtscp isn't executed out of order
        "nop\n\t"                           // Nops for improved accuracy
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
        "rdtscp\n\t"                        // End the timing measurement
        "shl $32, %%rdx\n\t"                // Combine the timestamp
        "or %%rdx, %%rax\n\t"
        "sub %%r15, %%rax\n\t"              // Compute the difference
        "mov %%rax, %[out]"
    : [out]"=r"(t_probe) : [victim]"r"(victim) : "rax", "rbx", "rcx", "rdx", "r15");
    printf("Miss: %lu\n", t_probe);

    asm volatile(
        "nop\n\t" //alignment
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
        "rdtscp\n\t"                        // Start measurement
        "shl $32, %%rdx\n\t"                // combine the timestamp
        "or %%rdx, %%rax\n\t"
        "mov %%rax, %%r15\n\t"              // Move the timestamp out of the way
        "movq (%[victim]), %%rdx\n\t"       // Access the victim address
        "mfence\n\t"                        // Make sure rtscp isn't executed out of order
        "nop\n\t"                           // Nops for improved accuracy
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
        "rdtscp\n\t"                        // End the timing measurement
        "shl $32, %%rdx\n\t"                // Combine the timestamp
        "or %%rdx, %%rax\n\t"
        "sub %%r15, %%rax\n\t"              // Compute the difference
        "mov %%rax, %[out]"
    : [out]"=r"(t_probe) : [victim]"r"(victim) : "rax", "rbx", "rcx", "rdx", "r15");
    
    printf("Hit: %lu\n", t_probe);

    asm volatile(
        "nop\n\t" //alignment
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
        "rdtscp\n\t"                        // Start measurement
        "shl $32, %%rdx\n\t"                // combine the timestamp
        "or %%rdx, %%rax\n\t"
        "mov %%rax, %%r15\n\t"              // Move the timestamp out of the way
        "movq (%[victim]), %%rdx\n\t"       // Access the victim address
        "mfence\n\t"                        // Make sure rtscp isn't executed out of order
        "nop\n\t"                           // Nops for improved accuracy
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
        "rdtscp\n\t"                        // End the timing measurement
        "shl $32, %%rdx\n\t"                // Combine the timestamp
        "or %%rdx, %%rax\n\t"
        "sub %%r15, %%rax\n\t"              // Compute the difference
        "mov %%rax, %[out]"
    : [out]"=r"(t_probe) : [victim]"r"(victim) : "rax", "rbx", "rcx", "rdx", "r15");
    printf("Hit: %lu\n", t_probe);
    asm volatile("mfence");
    asm volatile("clflush (%0)\n\t"::"r"(victim):);
    // Measure the access time to the victim
    asm volatile(
        "nop\n\t" //alignment
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
        "rdtscp\n\t"                        // Start measurement
        "shl $32, %%rdx\n\t"                // combine the timestamp
        "or %%rdx, %%rax\n\t"
        "mov %%rax, %%r15\n\t"              // Move the timestamp out of the way
        "movq (%[victim]), %%rdx\n\t"       // Access the victim address
        "mfence\n\t"
        "cpuid\n\t"                        // Make sure rtscp isn't executed out of order
        "nop\n\t"                           // Nops for improved accuracy
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
        "rdtscp\n\t"                        // End the timing measurement
        "shl $32, %%rdx\n\t"                // Combine the timestamp
        "or %%rdx, %%rax\n\t"
        "sub %%r15, %%rax\n\t"              // Compute the difference
        "mov %%rax, %[out]"
    : [out]"=r"(t_probe) : [victim]"r"(victim) : "rax", "rbx", "rcx", "rdx", "r15");
    printf("Miss: %lu\n", t_probe);
}


#if defined(USE_LIBTEA) || defined(VERIFY)
void setup_libtea(){
    instance = libtea_init();
    if (!instance){
        printf("Libtea initialization failed\n");
    }
}
#endif

int main(){
    srand(time(NULL));

    #if defined(USE_LIBTEA) || defined(VERIFY)
    if(geteuid() != 0)
    {
        printf("Warning: USE_LIBTEA enabled but you are not root. Printed output will not be correct.\nContinuing in 5s.\n");
        sleep(5);
    }

    setup_libtea();    
    instance->llc_set_mask =  65472;
    instance->llc_slices = 8;
    printf("Sets: %d, Slices %d\n", instance->llc_sets, instance->llc_slices);
    #endif // USE_LIBTEA

    // Allocate the array in which eviction set addresses are searched
    uint64_t addr_space_size = MEM_SIZE;
    uint64_t* addr_space = (uint64_t*) malloc(addr_space_size*sizeof(uint64_t));
    addr_space[0] = 0;
    
    // Select a random target address.
    uint64_t* victim = (uint64_t*) malloc(8);
    victim[0] = 0;
    calibrate(victim);

    // Start eviction set construction
    struct eviction_set_t *ev_set = NULL;
    
    #ifndef TRY_UNTIL_SUCCESS
    ev_set = get_evset(addr_space, victim, addr_space_size);

    // If the eviction set is valid, reduce it to minimal eviction set. 
    // Make sure you adjusted the cache miss threshold for this to work.
    if (test_evset(victim, ev_set)){
        clock_t before = clock();

        if (reduce_evset(victim, &ev_set)){
            printf("Reduction was successfull\n");
        }else{
            printf("Reduction algorithm failed\n");
        }

        clock_t difference = clock() - before;
        long msec = difference * 1000 / CLOCKS_PER_SEC;
        printf("Evset reduction took %ld seconds %ld milliseconds\n",
        msec/1000, msec%1000);
    }else{
        printf("The obtained eviction set is too small...\n");
    }
    #else
    clock_t before = clock();
    for(int i = 0; i < addr_space_size-100*4096; i+=100*4096){
        struct eviction_set_t *res = get_evset(addr_space+i, victim, 100*4096);
        merge_evsets(&ev_set, &res);
        if (test_evset(victim, ev_set)){
            if (reduce_evset(victim, &ev_set)){
                printf("Reduction was successfull\n");
                break;
            }   
        }
    }
    clock_t difference = clock() - before;
    long msec = difference * 1000 / CLOCKS_PER_SEC;
    printf("Evset took %ld seconds %ld milliseconds\n",
    msec/1000, msec%1000);
    #endif //TRY_UNTIL_SUCCESS
    print_evset(ev_set, victim);
    free(addr_space);
    free(victim);
    return 0;
}