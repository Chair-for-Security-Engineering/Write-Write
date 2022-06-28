
/* See LICENSE file for license and copyright information */


#ifndef LIBTEA_H
#define LIBTEA_H


/* Top-level definition to propagate to all files. This header should be included
 * in every other Libtea source file.
 */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE  
#endif


/* Just as important as the top-level GNU_SOURCE definition! */
#if defined(__linux__) || defined(LINUX) || defined(__linux)
#define LIBTEA_LINUX 1
#endif


#if defined(__ANDROID__) || defined(__android__) || defined(ANDROID)
/* We use mostly LIBTEA_LINUX functionality for Android, so define both, and use the
 * presence or absence of LIBTEA_ANDROID to distinguish between the two operating
 * systems where needed.
 */
#define LIBTEA_LINUX 1
#define LIBTEA_ANDROID 1
#endif


#define LIBTEA_SUPPORT_CACHE 1
#define LIBTEA_SUPPORT_PAGING 1
#define LIBTEA_SUPPORT_INTERRUPTS 0
#define LIBTEA_SUPPORT_SGX 0
/* Currently we only have enclave support for Intel SGX. Future work could extend this to
 * other enclave types, e.g. ARM TrustZone.
 */
#define LIBTEA_SUPPORT_ENCLAVES LIBTEA_SUPPORT_SGX
/* See LICENSE file for license and copyright information */

#ifndef _LIBTEA_CONFIG_H_
#define _LIBTEA_CONFIG_H_


/*
 * Set to 1 if your CPU supports Hyperthreading/SMT and it is enabled.
 */
#define LIBTEA_HAVE_HYPERTHREADING 1


/* Enable if your x86 CPU supports the RDTSCP instruction and you are running on
 * Linux (do not use on Windows - worse resolution than RDTSC). Has no effect on
 * non-x86 CPUs.
 */
#define LIBTEA_RDTSCP 1


/* The number of entries to use in the covert channel lookup table. This must be a
 * value between 1 and 256 inclusive. At 256, all possible values of a single byte
 * can be encoded. Consider reducing the value (e.g. to 26 to encode only 'A' to 'Z')
 * if testing on a CPU with a very small number of TLB entries, as the larger the
 * lookup table, the greater the pressure on the TLB. (You can also reduce the size of
 * the offset with the LIBTEA_COVERT_CHANNEL_OFFSET parameter below.)
 */
#define LIBTEA_COVERT_CHANNEL_ENTRIES 256


/* The offset to use between entries in the covert channel lookup table. This should be
 * a small power of 2 so multiplication with it will be compiled into a SHL instruction
 * (on x86, or equivalent otherwise). If this does not occur, attacks with very short
 * transient windows, such as ZombieLoad, will break. 4096 (page size) is typically the
 * best value on Intel CPUs; consider also 2056, 1024, or 4096*4 (observed to work well
 * on some Armv8-A and AMD CPUs).
 */
#define LIBTEA_COVERT_CHANNEL_OFFSET 4096


/* This enables the experimental libtea_isolate_windows_core function. If this is
 * enabled, you need to link with the PSAPI library (use -lpsapi) when compiling.
 * Warning: this is disabled by default because linking with this library seems to
 * increase the amount of cache noise, e.g. when using Libtea's cache covert channel
 * functions.
 */
#define LIBTEA_ENABLE_WINDOWS_CORE_ISOLATION 0


/* This enables the experimental libtea_force_memory_deduplication function. If this
 * is enabled, you need to link with NTDLL (use -lntdll) when compiling.
 * Warning: this is disabled by default because linking with this library seems to
 * increase the amount of cache noise, e.g. when using Libtea's cache covert channel
 * functions.
 */
#define LIBTEA_ENABLE_WINDOWS_MEMORY_DEDUPLICATION 0


/*
 * Enabling this will disable all output of internal library messages to stdout.
 * Print functions explicitly called by the user will still print to stdout.
 */
#define LIBTEA_SILENT 0


/* Select which IRQ vector to use for your custom interrupt handler. Do not use values 0-31 (reserved for CPU exception handlers). */
#define LIBTEA_IRQ_VECTOR 45

/*
 * Configure APIC timer interval for next interrupt.
 *
 * NOTE: the exact timer interval value depends on the CPU frequency, and
 * hence remains inherently platform-specific. We empirically established
 * suitable timer intervals on the following platforms by tweaking and
 * observing the NOP microbenchmark erip results:
 *
 * Intel i7-6700 3.4GHz (Skylake), ucode unknown:        19
 * Intel i7-6500U 2.5GHz (Skylake), ucode unknown:       25
 * Intel i5-6200U 2.3GHz (Skylake), ucode unknown:       28
 * Intel i7-8650U 1.9GHz (Kaby Lake R), ucode unknown:   34
 * Intel i7-8650U 1.9GHz (Kaby Lake R), ucode 0xca:      54
 * Intel i9-9900K 3.6GHz (Coffee Lake R), ucode unknown: 21
 * Intel i5-1035G1 1GHz (Ice Lake), ucode 0x32:         135
 *
 * Please see the paper 'SGX-Step: A Practical Attack Framework for Precise
 * Enclave Execution Control' (Van Bulck et al., SysTEX 2017) and the
 * SGX-Step GitHub repository (https://github.com/jovanbulck/sgx-step) for
 * more details.
 *
 * Once you have established the correct timer interval for your platform,
 * uncomment #define LIBTEA_APIC_TIMER_INTERVAL below and insert the correct interval.
 */
//#define LIBTEA_APIC_TIMER_INTERVAL YOUR_VALUE_HERE
#ifndef LIBTEA_APIC_TIMER_INTERVAL
#if LIBTEA_SUPPORT_INTERRUPTS
  #ifdef _MSC_VER
  #pragma message ("You need to manually configure LIBTEA_APIC_TIMER_INTERVAL in libtea_config.h.")
  #else
  #warning You need to manually configure LIBTEA_APIC_TIMER_INTERVAL in libtea_config.h.
  #endif
#endif
#endif

#endif //_LIBTEA_CONFIG_H

/* See LICENSE file for license and copyright information */

#ifndef LIBTEA_MODULE_H
#define LIBTEA_MODULE_H

#ifdef __cplusplus
extern "C" {
#endif


#if LIBTEA_LINUX
#define LIBTEA_DEVICE_NAME "libtea"
#define LIBTEA_DEVICE_PATH "/dev/" LIBTEA_DEVICE_NAME


#else
#define LIBTEA_DEVICE_NAME "LibteaLink"
#define LIBTEA_DEVICE_PATH "\\\\.\\" LIBTEA_DEVICE_NAME
#pragma warning(disable : 4201)
#endif


/* Libtea Common Functionality */

#include <stddef.h>   //For size_t

/**
 * Structure to get/set system registers
 */
typedef struct {
    /** Register ID */
    size_t reg;
    /** Logical CPU core to modify the register on */
    int cpu;
    /** Value */
    size_t val;
} libtea_system_reg;


#if LIBTEA_LINUX
#define LIBTEA_IOCTL_MAGIC_NUMBER (long)0x3d17
#define LIBTEA_IOCTL_GET_SYSTEM_REG \
  _IOR(LIBTEA_IOCTL_MAGIC_NUMBER, 1, size_t)

#define LIBTEA_IOCTL_SET_SYSTEM_REG \
  _IOR(LIBTEA_IOCTL_MAGIC_NUMBER, 2, size_t)

#define LIBTEA_IOCTL_GET_KERNEL_PHYS_ADDR \
  _IOR(LIBTEA_IOCTL_MAGIC_NUMBER, 3, size_t)

#else
/* Due to repeated problems with including ntddk.h, manually define our own versions of these */
#define LIBTEA_FILE_DEVICE_UNKNOWN             0x00000022
#define LIBTEA_FILE_ANY_ACCESS                 0
#define LIBTEA_FILE_READ_ACCESS                0x0001
#define LIBTEA_METHOD_BUFFERED                 0
#define LIBTEA_CTL_CODE( DeviceType, Function, Method, Access ) (((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method) )
#define LIBTEA_IOCTL_GET_PHYS_ADDR LIBTEA_CTL_CODE(LIBTEA_FILE_DEVICE_UNKNOWN, 0x807, LIBTEA_METHOD_BUFFERED, LIBTEA_FILE_ANY_ACCESS)
#define LIBTEA_IOCTL_GET_SYSTEM_REG LIBTEA_CTL_CODE(LIBTEA_FILE_DEVICE_UNKNOWN, 0x80b, LIBTEA_METHOD_BUFFERED, LIBTEA_FILE_ANY_ACCESS)
#define LIBTEA_IOCTL_SET_SYSTEM_REG LIBTEA_CTL_CODE(LIBTEA_FILE_DEVICE_UNKNOWN, 0x80c, LIBTEA_METHOD_BUFFERED, LIBTEA_FILE_ANY_ACCESS)

#endif


#if LIBTEA_SUPPORT_PAGING

#if LIBTEA_LINUX

//Do not modify - these are definitions, not configuration variables.
//Use libtea_switch_flush_tlb_implementation at runtime to choose which to use
#define LIBTEA_FLUSH_TLB_KERNEL 1
#define LIBTEA_FLUSH_TLB_CUSTOM 0

#define LIBTEA_IOCTL_VM_RESOLVE \
  _IOR(LIBTEA_IOCTL_MAGIC_NUMBER, 4, size_t)

#define LIBTEA_IOCTL_VM_UPDATE \
  _IOR(LIBTEA_IOCTL_MAGIC_NUMBER, 5, size_t)

#define LIBTEA_IOCTL_VM_LOCK \
  _IOR(LIBTEA_IOCTL_MAGIC_NUMBER, 6, size_t)

#define LIBTEA_IOCTL_VM_UNLOCK \
  _IOR(LIBTEA_IOCTL_MAGIC_NUMBER, 7, size_t)

#define LIBTEA_IOCTL_READ_PAGE \
  _IOR(LIBTEA_IOCTL_MAGIC_NUMBER, 8, size_t)

#define LIBTEA_IOCTL_WRITE_PAGE \
  _IOR(LIBTEA_IOCTL_MAGIC_NUMBER, 9, size_t)

#define LIBTEA_IOCTL_GET_ROOT \
  _IOR(LIBTEA_IOCTL_MAGIC_NUMBER, 10, size_t)

#define LIBTEA_IOCTL_SET_ROOT \
  _IOR(LIBTEA_IOCTL_MAGIC_NUMBER, 11, size_t)

#define LIBTEA_IOCTL_GET_PAGESIZE \
  _IOR(LIBTEA_IOCTL_MAGIC_NUMBER, 12, size_t)

#define LIBTEA_IOCTL_FLUSH_TLB \
  _IOR(LIBTEA_IOCTL_MAGIC_NUMBER, 13, size_t)

#define LIBTEA_IOCTL_GET_PAT \
  _IOR(LIBTEA_IOCTL_MAGIC_NUMBER, 14, size_t)

#define LIBTEA_IOCTL_SET_PAT \
  _IOR(LIBTEA_IOCTL_MAGIC_NUMBER, 15, size_t)

#define LIBTEA_IOCTL_SWITCH_FLUSH_TLB_IMPLEMENTATION \
  _IOR(LIBTEA_IOCTL_MAGIC_NUMBER, 16, size_t)


#if LIBTEA_SUPPORT_SGX
#define LIBTEA_IOCTL_ENCLAVE_INFO \
  _IOWR(LIBTEA_IOCTL_MAGIC_NUMBER, 17, struct libtea_enclave_info)

#define LIBTEA_IOCTL_EDBGRD \
  _IOWR(LIBTEA_IOCTL_MAGIC_NUMBER, 18, libtea_edbgrd)
#endif

#else

#define LIBTEA_IOCTL_READ_PAGE LIBTEA_CTL_CODE(LIBTEA_FILE_DEVICE_UNKNOWN, 0x801, LIBTEA_METHOD_BUFFERED, LIBTEA_FILE_ANY_ACCESS)

#define LIBTEA_IOCTL_WRITE_PAGE LIBTEA_CTL_CODE(LIBTEA_FILE_DEVICE_UNKNOWN, 0x802, LIBTEA_METHOD_BUFFERED, LIBTEA_FILE_READ_ACCESS)

#define LIBTEA_IOCTL_GET_CR3 LIBTEA_CTL_CODE(LIBTEA_FILE_DEVICE_UNKNOWN, 0x803, LIBTEA_METHOD_BUFFERED, LIBTEA_FILE_ANY_ACCESS)

#define LIBTEA_IOCTL_FLUSH_TLB LIBTEA_CTL_CODE(LIBTEA_FILE_DEVICE_UNKNOWN, 0x804, LIBTEA_METHOD_BUFFERED, LIBTEA_FILE_ANY_ACCESS)

#define LIBTEA_IOCTL_READ_PHYS_VAL LIBTEA_CTL_CODE(LIBTEA_FILE_DEVICE_UNKNOWN, 0x805, LIBTEA_METHOD_BUFFERED, LIBTEA_FILE_ANY_ACCESS)

#define LIBTEA_IOCTL_WRITE_PHYS_VAL LIBTEA_CTL_CODE(LIBTEA_FILE_DEVICE_UNKNOWN, 0x806, LIBTEA_METHOD_BUFFERED, LIBTEA_FILE_ANY_ACCESS)

#define LIBTEA_IOCTL_SET_CR3 LIBTEA_CTL_CODE(LIBTEA_FILE_DEVICE_UNKNOWN, 0x808, LIBTEA_METHOD_BUFFERED, LIBTEA_FILE_ANY_ACCESS)

#define LIBTEA_IOCTL_SET_PAT LIBTEA_CTL_CODE(LIBTEA_FILE_DEVICE_UNKNOWN, 0x809, LIBTEA_METHOD_BUFFERED, LIBTEA_FILE_ANY_ACCESS)

#define LIBTEA_IOCTL_GET_PAT LIBTEA_CTL_CODE(LIBTEA_FILE_DEVICE_UNKNOWN, 0x80a, LIBTEA_METHOD_BUFFERED, LIBTEA_FILE_ANY_ACCESS)

#endif


/**
 * Structure containing the page-table entries of all levels.
 * The Linux names are aliased with the Intel names.
 */
typedef struct {
    /** Process ID */
    size_t pid;
    /** Virtual address */
    size_t vaddr;

    /** Page global directory / Page map level 5 */
    union {
        size_t pgd;
        size_t pml5;
    };
    /** Page directory 4 / Page map level 4 */
    union {
        size_t p4d;
        size_t pml4;
    };
    /** Page upper directory / Page directory pointer table */
    union {
        size_t pud;
        size_t pdpt;
    };
    /** Page middle directory / Page directory */
    union {
        size_t pmd;
        size_t pd;
    };
    /** Page table entry */
    size_t pte;
    /** Bitmask indicating which entries are valid/should be updated */
    size_t valid;
} libtea_page_entry;


/**
 * Structure to read/write physical pages
 */
#if LIBTEA_LINUX
typedef struct {
    /** Page-frame number */
    size_t pfn;
    /** Virtual address */
    size_t vaddr;
    /** Page size */
    size_t size;
    /** Page content */
    unsigned char* buffer;
} libtea_physical_page;
#else


#ifdef _MSC_VER
__pragma(pack(push, 1))
#else
#pragma pack(push, 1)
#endif


typedef struct {
    char content[4096];
    size_t paddr;
} libtea_physical_page;


#ifdef _MSC_VER
__pragma(pack(pop))
#else
#pragma pack(pop)
#endif

#endif


/**
 * Structure to get/set the root of paging
 */
typedef struct {
    /** Process id */
    size_t pid;
    /** Physical address of paging root */
    size_t root;
} libtea_paging_root;


#define LIBTEA_VALID_MASK_PGD (1<<0)
#define LIBTEA_VALID_MASK_P4D (1<<1)
#define LIBTEA_VALID_MASK_PUD (1<<2)
#define LIBTEA_VALID_MASK_PMD (1<<3)
#define LIBTEA_VALID_MASK_PTE (1<<4)

#endif


#if LIBTEA_SUPPORT_SGX

#ifndef __KERNEL__
#include <stdint.h>
#endif


struct libtea_enclave_info {
  uint64_t base;
  uint64_t size;
  uint64_t aep;
  uint64_t tcs;
};

typedef struct {
  uint64_t adrs;
  uint64_t val;
  int64_t  len;
  int      write;
} libtea_edbgrd;

typedef struct {
  uint64_t adrs;
} libtea_invpg;


#endif


#ifdef __cplusplus
}
#endif


#endif //LIBTEA_MODULE_H

/* See LICENSE file for license and copyright information */

#ifndef LIBTEA_COMMON_H
#define LIBTEA_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif




#include <fcntl.h>
#include <memory.h>
#include <setjmp.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>


#if LIBTEA_ANDROID
#define LIBTEA_SHELL "/system/bin/sh"
#elif LIBTEA_LINUX
#define LIBTEA_SHELL "/bin/sh"
#endif

#if LIBTEA_LINUX
#include <dirent.h>
#include <errno.h>
#include <linux/perf_event.h>
#include <pthread.h>
#include <sched.h>
#include <sys/fcntl.h> 
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <sys/sysinfo.h>
#include <unistd.h>


#define libtea_popen(cmd, type) popen(cmd, type)
#define libtea_pclose(fd) pclose (fd)
typedef void* HANDLE;
typedef int libtea_file;
typedef int* libtea_file_ptr;
typedef pid_t libtea_thread;

#else
#include <intrin.h>
#include <malloc.h>
#include <Windows.h> //Note: this must be included before Psapi.h (despite what the SpiderMonkey linter says), otherwise compilation fails as there are missing typedefs
#include <Psapi.h>
#pragma comment(lib, "Psapi")
#define libtea_popen(cmd, type) _popen(cmd, type)
#define libtea_pclose(fd) _pclose(fd)
#define LIBTEA_WINDOWS 1    /* This is hacky but the best we can do seeing as MSVC seems to provide no Windows identifier macros */
#define NO_WINDOWS_SUPPORT libtea_info("Error: %s not supported on Windows", __func__)
typedef HANDLE libtea_file;
typedef HANDLE* libtea_file_ptr;
typedef HANDLE libtea_thread;

#if defined(_MSC_VER) || defined(__clang__)
typedef size_t pid_t;
#endif
#endif

#if defined(_MSC_VER) && (!defined(MOZJS_MAJOR_VERSION) || MOZJS_MAJOR_VERSION < 63)   /* If using Visual Studio C++ compiler. We have to explicitly exclude Firefox after MOZJS v63 as they began using clang-cl */
#define libtea_inline __forceinline
#else                                                                                  /* Assume using GCC, MinGW GCC, or Clang */
#define libtea_inline inline __attribute__((always_inline,flatten))                    /* Assume using GCC */                                    
#define LIBTEA_INLINEASM 1
#endif


#if LIBTEA_SUPPORT_CACHE

#endif


typedef struct libtea_instance_t libtea_instance;


/**
 * Timer function
 */
typedef uint64_t (*libtea_timer_function)(libtea_instance*);


/**
 * Available high-resolution timers
 */
typedef enum {
  /** Native timer */
  LIBTEA_TIMER_NATIVE,
  /* APERF variants potentially invulnerable to DABANGG effects as they inc. at processor frequency? */
  /** AMD Zen 2+ only: APERF - higher resolution than rdtsc */
  LIBTEA_TIMER_NATIVE_AMD_ZEN2,
  /** AMD Zen only: APERF read via kernel driver (no rdpru support) */
  LIBTEA_TIMER_NATIVE_AMD_ZEN,
  /** Counting thread */
  LIBTEA_TIMER_COUNTING_THREAD,
  /** Linux perf interface */
  LIBTEA_TIMER_PERF,
  /** Monotonic clock */
  LIBTEA_TIMER_MONOTONIC_CLOCK
} libtea_timer;


typedef struct {
  unsigned char secret;
  HANDLE addr;
  int core;
} libtea_thread_data;


/* Eviction strategies are defined here rather than in libtea_cache.h as we require them for the instance definition */

/**
 * Structure containing the access strategy for eviction.
 */
typedef struct {
    int C, D, L, S;
} libtea_eviction_strategy;


/**
 * Structure containing virtual addresses of an eviction set.
 */
typedef struct {
    int addresses;
    void** address;
} libtea_eviction_set;


typedef struct {
  bool used;
  size_t n;
  void** congruent_virtual_addresses;
} libtea_congruent_address_cache_entry;


typedef struct {
  size_t mapping_size;
  void* mapping;
  libtea_file_ptr handle;
} libtea_memory;


typedef struct {
  libtea_congruent_address_cache_entry* congruent_address_cache;
  libtea_memory memory;
} libtea_eviction;


typedef struct {
    int has_pgd, has_p4d, has_pud, has_pmd, has_pt;
    int pgd_entries, p4d_entries, pud_entries, pmd_entries, pt_entries;
    int page_offset;
} libtea_paging_definition_t;


/**
 * Structure containing the local variables for a libtea instance.
 */
struct libtea_instance_t {
  #if LIBTEA_WINDOWS
  HANDLE module_fd;
  #else
  int module_fd; /* File descriptor for IOCTL access to kernel module */
  #endif
  libtea_timer_function timer;
  uint64_t measure_start;
  libtea_thread timing_thread;
  void* timing_thread_stack;
  volatile size_t thread_counter;
  libtea_thread leaky_thread;
  libtea_thread_data leaky_thread_data;
  int has_tm;
  int is_intel;
  #if LIBTEA_LINUX
  cpu_set_t cpumask;
  #endif
  int physical_cores;
  int logical_cores;
  int cpu_architecture;
  int perf_fd;
  size_t direct_physical_map;
  /** LLC Miss timing information */
  int llc_miss_threshold;
  /** LLC Hit timing information */
  int llc_hit_threshold;
  int llc_line_size;
  int llc_ways;
  int llc_sets;
  int llc_partitions;
  int llc_slices;
  int llc_size;
  int llc_set_mask;
  size_t physical_memory;
  libtea_file covert_channel_handle;
  void* covert_channel;
  libtea_eviction_strategy eviction_strategy;
  libtea_eviction_strategy prime_strategy;
  libtea_eviction* eviction;
  #if LIBTEA_SUPPORT_PAGING
  int umem_fd; /* File descriptor for read/write acess to /proc/umem */
  int mem_fd;  /* File descriptor for read/write access to /dev/mem */
  int pagesize;
  size_t paging_root;
  unsigned char* vmem;
  libtea_paging_definition_t paging_definition;
  #endif
  #if LIBTEA_LINUX
  int last_min_pstate;
  int last_max_pstate;
  int last_turbo_boost_setting;
  #endif
};


/* These need to be static so that each .c file including the header has its own copy */
#if LIBTEA_LINUX
static sigjmp_buf libtea__trycatch_buf;
#else
static jmp_buf libtea__trycatch_buf;
#endif
typedef void (*sighnd_t)(int);
static sighnd_t libtea__saved_sighandler[32];
static libtea_instance* libtea__instances[32];
void libtea__trycatch_segfault_handler(int signum);
static void libtea__try_start_prep();


/* Return values for "void" functions which can fail.
 * Functions which normally return a variable will return NULL on error. */
#define LIBTEA_SUCCESS 0
#define LIBTEA_ERROR -1

#define LIBTEA_READ 0
#define LIBTEA_WRITE 1
#define LIBTEA_READ_WRITE 2


/**
 * Initializes a libtea instance and initializes and acquires kernel module.
 *
 * :return: Returns a libtea instance
 */
libtea_instance* libtea_init();


/**
 * Initializes a libtea instance without the kernel module (paging, interrupts, and enclave functionality will be disabled).
 *
 * :return: Returns a libtea instance
 */
libtea_instance* libtea_init_nokernel();


/**
 * Cleans up the libtea instance and (if necessary) releases the kernel module.
 */
void libtea_cleanup(libtea_instance* instance);


/**
 * Accesses the provided address.
 *
 * :param addr: Virtual address
 */
#define libtea_access(addr) libtea__arch_access(addr)


/**
 * Accesses the provided address (with memory barriers).
 *
 * :param addr: Virtual address
 */
#define libtea_access_b(addr) libtea__arch_access_b(addr)


/**
 * Accesses the provided address speculatively. Success will vary depending on the microarchitecture
 * used (exact branch prediction implementation, ROB size etc).
 *
 * :param addr: Virtual address
 */
libtea_inline void libtea_access_speculative(void* addr);


/**
 * Prefetches the provided address.
 *
 * :param addr: Virtual address
 */
#define libtea_prefetch(addr) libtea__arch_prefetch(addr)


/**
 * Prefetches the provided address in anticipation of a write to the address.
 *
 * :param addr: Virtual address
 */
#define libtea_prefetch_anticipate_write(addr)  libtea__arch_prefetchw(addr)


/**
 * Flushes the provided address from the cache.
 *
 * :param addr: Virtual address
 */
#define libtea_flush(addr) libtea__arch_flush(addr)


/**
 * Flushes the provided address from the cache (with memory barriers).
 *
 * :param addr: Virtual address.
 */
#define libtea_flush_b(addr) libtea__arch_flush_b(addr)


/**
 * Begin memory barrier.
 */
#define libtea_barrier_start() libtea__arch_barrier_start()


/**
 * End memory barrier.
 *
 * Note: unnecessary on x86.
 *
 */
#define libtea_barrier_end() libtea__arch_barrier_end()


/**
 * Insert a speculation barrier.
 *
 */
#define libtea_speculation_barrier() libtea__arch_speculation_barrier()


/**
 * Returns the current timestamp.
 *
 * :param instance: The libtea instance
 * :return: The current timestamp
 */
libtea_inline uint64_t libtea_timestamp(libtea_instance* instance);


/**
 * Begins a timing measurement.
 *
 * :param instance: The libtea instance
 */
libtea_inline void libtea_measure_start(libtea_instance* instance);


/**
 * Ends a timing measurement and returns the elapsed time.
 *
 * :param instance: The libtea instance
 * :return: Elapsed time since the start of the measurement
 */
libtea_inline uint64_t libtea_measure_end(libtea_instance* instance);


/**
 * Configures which timer is used.
 *
 * Note: on most systems you will need to run as root to use LIBTEA_PERF_TIMER.
 * Otherwise it will fail silently (returning 0).
 *
 * :param instance: The libtea instance
 * :param timer: The timer to use
 */
libtea_inline static void libtea_set_timer(libtea_instance* instance, libtea_timer timer);


/**
 * Begins a try/catch block using signal handling.
 *
 * Usage: libtea_try_start() { ... }
 */
#if LIBTEA_LINUX
#define libtea_try_start()  libtea__try_start_prep(); if(!sigsetjmp(libtea__trycatch_buf, 1))
#else
#define libtea_try_start()  libtea__try_start_prep(); if(!setjmp(libtea__trycatch_buf))
#endif


/**
 * Ends the signal handling try/catch block and restores the
 * previous signal handlers.
 */
#define libtea_try_end()                                                                \
  do{                                                                                     \
    signal(SIGILL, libtea__saved_sighandler[0]);                                        \
    signal(SIGFPE, libtea__saved_sighandler[1]);                                        \
    signal(SIGSEGV, libtea__saved_sighandler[2]);                                       \
  } while(0)


/**
 * Aborts the signal handling try/catch block by triggering a segmentation fault.
 *
 * Note: this function assumes that NULL (the zero page) is not mapped into the process' memory.
 */
#define libtea_try_abort()  libtea_access(0)


/**
 * Aborts the signal handling try/catch block via a siglongjmp.
 */
#define libtea_try_abort_noexcept()  siglongjmp(libtea__trycatch_buf, 1)


/**
 * Begins a try/catch block using using transactional memory.
 *
 * Note: this function will throw an exception if you try to execute it without
 * a supported transactional memory implementation (Intel TSX or PowerPC HTM).
 *
 * Usage: libtea_try_start_tm() { ... }
 */
#define libtea_try_start_tm()  if(libtea__arch_transaction_begin())


/**
 * Ends the transactional try/catch block.
 *
 * Note: Intel TSX will segfault if this is used outside of a transaction
 * (i.e. a libtea_try_start_tm() block).
 */
#define libtea_try_end_tm() libtea__arch_transaction_end()


/**
 * Aborts the transactional try/catch block.
 */
#define libtea_try_abort_tm()  libtea__arch_transaction_abort()


#if LIBTEA_INLINEASM

/**
 * Starts a specpoline block (code within will only be executed transiently).
 *
 * :param label: A goto label to use in the inline assembly.
 *
 * Note: you must pass the same label to the corresponding libtea_speculation_end,
 * and you must use a different label each time you call libtea_speculation_start
 * within the same program, or it will fail to compile ("redefinition of label").
 */
#define libtea_speculation_start(label)  libtea__arch_speculation_start(label)

/**
 * Ends a specpoline block.
 *
 * :param label: A goto label to use in the inline assembly. (See notes for libtea_speculation_start.)
 */
#define libtea_speculation_end(label)  libtea__arch_speculation_end(label)

#endif


/**
 * Gets the sibling hyperthread of the provided core (Linux-only).
 *
 * :param logical_core: The logical core
 * :return: The id of the sibling hyperthread or LIBTEA_ERROR
 */
libtea_inline int libtea_get_hyperthread(int logical_core);


/**
 * Pins a process to the provided core.
 *
 * :param process: The process to pin
 * :param core: The core the process should be pinned to
 */
libtea_inline void libtea_pin_to_core(libtea_thread process, int core);


/**
 * Returns the physical address of the provided virtual address.
 *
 * Note: this function must be run with root privileges.
 *
 * :param instance: The libtea instance
 * :param addr: The virtual address
 * :return: The corresponding physical address or LIBTEA_ERROR
 */
libtea_inline size_t libtea_get_physical_address(libtea_instance* instance, size_t addr);


/**
 * Opens a shared memory region.
 *
 * Note: libtea only supports one shared memory region being open at
 * a time. You must close the shared memory when you finish using it using
 * libtea_close_shared_memory().
 *
 * :param size: Desired size of the region in bytes
 * :param windowsMapping: Returns the Windows mapping handle (ignored on Linux)
 *
 * :return: A void* or Handle pointer to the shared memory, or NULL if
 * an error occurred.
 */
libtea_inline HANDLE libtea_open_shared_memory(size_t size, libtea_file_ptr windowsMapping);


/**
 * Closes a shared memory region created with open_shared_memory.
 *
 * Note: libtea only supports one shared memory region being open at
 * a time.
 *
 * :param mem: Pointer or Handle to the shared memory region
 * :param windowsMapping: The Windows mapping handle (ignored on Linux)
 * :param size: Size of the region in bytes
 *
 * :return: LIBTEA_SUCCESS on success, LIBTEA_ERROR otherwise
 */
libtea_inline int libtea_close_shared_memory(HANDLE mem, libtea_file_ptr windowsMapping, size_t size);


/**
 * Starts a leaky thread.
 *
 * :param instance: The libtea instance.
 * :param type: The type of leaky thread to create. 1 for load loop, 2 for store loop, 3 for nop loop.
 * :param secret: A byte value to repeatedly load/store (ignored for nop loop, but you must still provide a value).
 * :param shared: A void pointer / HANDLE to a shared memory region, or NULL to not use shared memory.
 * :param core: The CPU core to lock the thread to.
 *
 * :return: A libtea_thread handle, or 0 (Linux) / NULL (Windows) if an error occurred.
 */
libtea_thread libtea_start_leaky_thread(libtea_instance* instance, int type, unsigned char secret, HANDLE shared, int core);


/**
 * Stops the victim thread initialized with libtea_start_leaky_thread().
 *
 * :param instance: The libtea instance.
 */
void libtea_stop_leaky_thread(libtea_instance* instance);


/**
 * Maps a page of the given file at the defined offset to the program's
 * address space and returns its address (Linux-only).
 *
 * Note: This function leaks memory.
 *
 * :param filename: The path to the file
 * :param filesize: Returns the size of the file (if not NULL)
 * :param fileHandle: Returns the file descriptor / handle
 * :param rw: LIBTEA_READ for a read-only mapping, LIBTEA_WRITE for write-only (Linux-only), LIBTEA_READ_WRITE for read-write
 * :param offset: The offset that should be mounted
 *
 * :return: Mapped address or NULL if any error occurs
 */
libtea_inline void* libtea_map_file_by_offset(const char* filename, size_t* filesize, libtea_file_ptr fileHandle, int rw, size_t offset);


/**
 * Maps an entire file and returns its address.
 *
 * Note: This function leaks memory. On Windows, you must also close the
 * underlying file (fileHandle) in addition to unmapping the file.
 *
 * :param filename: The path to the file
 * :param filesize: Returns the size of the file (if not NULL)
 * :param fileHandle: Returns the file descriptor / handle
 * :param windowsMapping: Returns the Windows mapping handle (ignored on Linux)
 * :param rw: LIBTEA_READ for a read-only mapping, LIBTEA_WRITE for write-only (Linux-only), LIBTEA_READ_WRITE for read-write
 * :return: Mapped address or NULL if any error occurs
 */
libtea_inline void* libtea_map_file(const char* filename, size_t* filesize, libtea_file_ptr fileHandle, libtea_file_ptr windowsMapping, int rw);


/**
 * Maps a region of memory (not backed by an underlying file).
 * This function exists to facilitate Linux/Windows cross-compatibility.
 *
 * Note: This function leaks memory. You should unmap the allocated
 * region with libtea_munmap().
 *
 * :param buffer_size: The size of the region to map
 * :param handle: Returns the Windows mapping handle (ignored on Linux)
 * :param rw: LIBTEA_READ for a read-only mapping, LIBTEA_WRITE for write-only (Linux-only), LIBTEA_READ_WRITE for read-write
 * :return: Pointer to the mapper region (or NULL on error)
 */
libtea_inline void* libtea_mmap(int buffer_size, libtea_file_ptr windowsMapping, int rw);


/**
 * Unmaps a memory-mapped file. This function exists to facilitate
 * Linux/Windows cross-compatibility.
 *
 * :param ptr: Pointer to the region to unmap
 * :param buffer_size: The size of the region (ignored on Windows)
 * :param fileHandle: File descriptor / handle
 * :param windowsMapping: The Windows mapping handle (ignored on Linux)
 * :return: LIBTEA_SUCCESS on success, LIBTEA_ERROR otherwise
 */
libtea_inline int libtea_munmap_file(void* ptr, int buffer_size, libtea_file_ptr fileHandle, libtea_file_ptr windowsMapping);


/**
 * Unmaps a (non file-backed) mapped region of memory. This function
 * exists to facilitate Linux/Windows cross-compatibility.
 *
 * :param ptr: Pointer to the region to unmap
 * :param buffer_size: The size of the region (ignored on Windows)
 * :param windowsMapping: The Windows mapping handle (ignored on Linux)
 * :return: LIBTEA_SUCCESS on success, LIBTEA_ERROR otherwise
 */
libtea_inline int libtea_munmap(void* ptr, int buffer_size, libtea_file_ptr windowsMapping);


/**
 * Finds the index of the nth largest integer in the list.
 *
 * :param list: The list
 * :param nmemb: Number of list entries
 * :param n: Value of n (0 == largest)
 * :return: The index
 */
libtea_inline int libtea_find_index_of_nth_largest_int(int* list, size_t nmemb, size_t n);


/**
 * Finds the index of the nth largest size_t in the list.
 *
 * :param list: The list
 * :param nmemb: Number of list entries
 * :param n: Value of n (0 == largest)
 * :return: The index
 */
libtea_inline int libtea_find_index_of_nth_largest_sizet(size_t* list, size_t nmemb, size_t n);


/**
 * Writes to a model-specific register (MSR) / system register.
 *
 * Note: requires the msr driver (x86 only) or the libtea driver.
 *
 * :param instance: The libtea instance
 * :param cpu: The core ID
 * :param reg: The register
 * :param val: The value
 * :return: LIBTEA_SUCCESS or LIBTEA_ERROR
 */
libtea_inline int libtea_write_system_reg(libtea_instance* instance, int cpu, uint32_t reg, uint64_t val);


/**
 * Reads from a model-specific register (MSR) / system register.
 *
 * Note: requires the msr driver (x86 only) or the libtea driver.
 *
 * :param instance: The libtea instance
 * :param cpu: The core ID
 * :param reg: The register
 * :return: The value of the register or LIBTEA_ERROR
 */
libtea_inline size_t libtea_read_system_reg(libtea_instance* instance, int cpu, uint32_t reg);


/**
 * Disables all hardware prefetchers (supported on Intel only).
 *
 * :param instance: The libtea instance
 */
libtea_inline void libtea_disable_hardware_prefetchers(libtea_instance* instance);


/**
 * Enables all hardware prefetchers (supported on Intel only).
 *
 * :param instance: The libtea instance
 */
libtea_inline void libtea_enable_hardware_prefetchers(libtea_instance* instance);


/**
 * Attempts to isolate the provided CPU core by removing it from the affinity mask of all
 * running user processes. It is unfortunately not possible to modify the affinity of
 * system processes.
 *
 * Note: only supported on Windows; must be run with administrator privileges. On Linux,
 * boot with the isolcpus=X parameter set or (preferred) use the cset-shield tool.
 *
 * This is an experimental function and is only enabled if LIBTEA_ENABLE_WINDOWS_CORE_ISOLATION
 * is set to 1 in libtea_config.h.
 *
 * :param core: The CPU core to isolate
 * :return: LIBTEA_SUCCESS on success, otherwise LIBTEA_ERROR
 */
#if LIBTEA_ENABLE_WINDOWS_CORE_ISOLATION
libtea_inline int libtea_isolate_windows_core(int core);
#endif


/**
 * Attempts to lock the CPU to a stable P-state for reproducible microarchitectural attack or
 * benchmark results. Disables Turbo Boost and sets both the minimum and maximum P-state to the
 * provided value (provided as a percentage of available performance).
 *
 * Note: only supported for Intel CPUs on Linux (depends on the intel_pstate module).
 *
 * :param instance: The libtea instance
 * :param perf_percentage: The integer percentage of available performance to lock to
 * :return: LIBTEA_SUCCESS on success, otherwise LIBTEA_ERROR
 */
libtea_inline int libtea_set_cpu_pstate(libtea_instance* instance, int perf_percentage);


/*
 * Restores the CPU P-state and Turbo Boost settings to their state prior to the last call of
 * libtea_set_cpu_pstate. Do not use without first calling libtea_set_cpu_pstate.
 *
 * Note: only supported for Intel CPUs on Linux (depends on the intel_pstate module).
 *
 * :param instance: The libtea instance
 * :return: LIBTEA_SUCCESS on success, otherwise LIBTEA_ERROR
 */
libtea_inline int libtea_restore_cpu_pstate(libtea_instance* instance);


#if LIBTEA_SILENT
#define libtea_info(msg, ...)
#else
#define libtea_info(msg, ...)                                         \
  do {                                                                  \
    printf("[" __FILE__ "] " msg "\n", ##__VA_ARGS__);      \
    fflush(stdout);                                                     \
  } while(0)

#endif

#define libtea_always_print_info(msg, ...)                            \
  do {                                                                  \
    printf("[" __FILE__ "] " msg "\n", ##__VA_ARGS__);      \
    fflush(stdout);                                                     \
  } while(0)


/* Use a hard assert for select Interrupt/Enclave functionality
 * where continuing execution in an invalid state could lead to
 * a system hang / unpredictable behavior.
 */
#define libtea_assert(cond)                                  \
    do {                                                     \
        if (!(cond))                                         \
        {                                                    \
            printf("Assertion '" #cond "' failed.");         \
            exit(EXIT_FAILURE);                              \
        }                                                    \
    } while(0);


#if !defined(_GNU_SOURCE) && LIBTEA_LINUX
int clone(int (*fn)(void *), void *child_stack,
                 int flags, void *arg, ...
                 /* pid_t *ptid, void *newtls, pid_t *ctid */ );
int sched_setaffinity(pid_t pid, size_t cpusetsize,
                             const cpu_set_t *mask);

int sched_getaffinity(pid_t pid, size_t cpusetsize,
                             cpu_set_t *mask);
int sched_getcpu(void);
#endif


#ifdef __cplusplus
}
#endif


#endif //LIBTEA_COMMON_H

/* See LICENSE file for license and copyright information */

#ifndef LIBTEA_ARCH_H
#define LIBTEA_ARCH_H

#ifdef __cplusplus
extern "C" {
#endif




/* Condition below will need adapting to support Windows on Arm, but MSVC is currently providing no helpful macros */
#if defined(__i386__) || defined(__x86_64__) || LIBTEA_WINDOWS
#define LIBTEA_X86 1
#elif defined(__aarch64__)
#define LIBTEA_AARCH64 1
#elif defined(__PPC64__) || defined(__ppc64__)
#define LIBTEA_PPC64 1
#endif


#if LIBTEA_LINUX || LIBTEA_INLINEASM
#define LIBTEA_NOP() asm volatile("nop")
#else
#define LIBTEA_NOP() __nop()
#endif

#if LIBTEA_LINUX
int libtea__arch_counting_thread(void* arg);
#else
DWORD WINAPI libtea__arch_counting_thread(LPVOID arg);
#endif


uint64_t libtea__arch_timestamp_native();
#if LIBTEA_X86
uint64_t libtea__arch_timestamp_native_amd_zen();
uint64_t libtea__arch_timestamp_native_amd_zen2();
#endif
uint64_t libtea__arch_timestamp_monotonic();
void libtea__trycatch_segfault_handler(int signum); /* Defined in libtea_common.c */
libtea_inline void libtea__arch_init_cpu_features(libtea_instance* instance);

#if LIBTEA_SUPPORT_CACHE
libtea_inline int libtea__arch_init_cache_info(libtea_instance* instance);
libtea_inline void libtea__arch_init_direct_physical_map(libtea_instance* instance);
libtea_inline void libtea__arch_init_eviction_strategy(libtea_instance* instance);
libtea_inline void libtea__arch_init_prime_strategy(libtea_instance* instance);
libtea_inline void libtea__arch_fast_cache_encode(libtea_instance* instance, void* addr);
#endif

libtea_inline int libtea__arch_transaction_begin();
libtea_inline void libtea__arch_transaction_end();
libtea_inline void libtea__arch_transaction_abort();


libtea_inline void libtea__arch_access(void* addr);


libtea_inline void libtea__arch_access_b(void* addr);


libtea_inline void libtea__arch_flush(void* addr);


libtea_inline void libtea__arch_flush_b(void* addr);


libtea_inline void libtea__arch_barrier_start();


libtea_inline void libtea__arch_barrier_end();


libtea_inline void libtea__arch_speculation_barrier();


libtea_inline int libtea__arch_write_system_reg(libtea_instance* instance, int cpu, uint32_t reg, uint64_t val);


libtea_inline size_t libtea__arch_read_system_reg(libtea_instance* instance, int cpu, uint32_t reg);


libtea_inline void libtea__arch_disable_hardware_prefetchers(libtea_instance* instance);


libtea_inline void libtea__arch_enable_hardware_prefetchers(libtea_instance* instance);


#ifdef __cplusplus
}
#endif

#endif //LIBTEA_ARCH_H

/* See LICENSE file for license and copyright information */

#ifndef LIBTEA_CACHE_H
#define LIBTEA_CACHE_H

#ifdef __cplusplus
extern "C" {
#endif





/**
 * Performs Flush+Reload on the provided address and returns hit/miss based
 * on the current threshold.
 *
 * :param instance: The libtea instance
 * :param addr: The address
 * :return: 1 if the address was in the cache, 0 if the address was not cached
 */
libtea_inline int libtea_flush_reload(libtea_instance* instance, void* addr);


/**
 * Calibrates the threshold to distinguish between a cache hit and cache
 * miss using Flush+Reload.
 *
 * :param instance: The libtea instance
 */
libtea_inline void libtea_calibrate_flush_reload(libtea_instance* instance);


/**
 * Returns the cache slice of the provided physical address.
 * 
 * Note: only supported on Intel CPUs (based on Maurice et al., 'Reverse Engineering
 * Intel Last-Level Cache Complex Addressing Using Performance Counters', RAID 2015).
 * 
 * :param instance: The libtea instance
 * :param paddr: The physical address
 * :return: Cache slice of the physical address
 */
libtea_inline int libtea_get_cache_slice(libtea_instance* instance, size_t paddr);


/**
 * Returns the cache set of the provided physical address.
 *
 * :param instance: The libtea instance
 * :param paddr: The physical address
 * :return: Cache set of the physical address
 */
libtea_inline int libtea_get_cache_set(libtea_instance* instance, size_t paddr);


/**
 * Builds an eviction set for the provided physical address.
 *
 * :param instance: The libtea instance
 * :param set: The built eviction set
 * :param paddr: The physical address
 * :return: LIBTEA_SUCCESS on success, LIBTEA_ERROR otherwise
 */
libtea_inline int libtea_build_eviction_set(libtea_instance* instance, libtea_eviction_set* set, size_t paddr);


/**
 * Runs eviction using the provided eviction set.
 *
 * :param instance: The libtea instance
 * :param set: The eviction set
 */
libtea_inline void libtea_evict(libtea_instance* instance, libtea_eviction_set set);


/**
 * Performs Evict+Reload using the provided eviction set.
 * 
 * :param instance: The libtea instance
 * :param addr: The virtual address
 * :param set: The eviction set that should be used
 * :return: 1 if addr was cached
 */
libtea_inline int libtea_evict_reload(libtea_instance* instance, void* addr, libtea_eviction_set set);


/**
 * Calibrates the threshold to distinguish between a cache hit and cache
 * miss using Evict+Reload.
 *
 * :param instance: The libtea instance
 */
libtea_inline void libtea_calibrate_evict_reload(libtea_instance* instance);


/**
 * Performs the prime step using the provided eviction set.
 * 
 * :param instance: The libtea instance
 * :param set: The eviction set that should be used
 */
libtea_inline void libtea_prime(libtea_instance* instance, libtea_eviction_set set);


/**
 * Performs Prime+Probe and builds an eviction set for the provided address if
 * one does not exist.
 * 
 * :param instance: The libtea instance
 * :param set: The eviction set
 * :return: The execution time of the probe step
 */
libtea_inline int libtea_prime_probe(libtea_instance* instance, libtea_eviction_set set);


/**
 * Calculates the slice ID of the virtual address by measuring with performance counters (requires MSR access).
 * 
 * Note: only supported on Intel CPUs (based on Maurice et al., 'Reverse Engineering
 * Intel Last-Level Cache Complex Addressing Using Performance Counters', RAID 2015).
 *
 * :param instance: The libtea instance
 * :param vaddr: The virtual address
 * :return: The slice id
 */
libtea_inline size_t libtea_measure_slice(libtea_instance* instance, void* address);


/**
 * Encodes the provided value into the cache.
 *
 * :param instance: The libtea instance
 * :param value: The value to encode
 */
libtea_inline void libtea_cache_encode(libtea_instance* instance, unsigned char value);


/**
 * Dereferences a pointer at the provided offset and encodes the dereferenced value into the cache.
 * This function is intended for use with SCFirefox.
 *
 * :param instance: The libtea instance
 * :param ptr: The (char*) pointer to dereference
 * :param offset: The offset to dereference the pointer at (e.g. offset=10 -> ptr[10])
 */
libtea_inline void libtea_cache_encode_dereference(libtea_instance* instance, char* ptr, int offset);


/**
 * Like libtea_cache_encode_dereference, but uses optimized assembly to encode within an extremely short
 * transient window. Currently supported on x86 only.
 *
 * :param instance: The libtea instance
 * :param addr: The pointer to dereference
 */
#define libtea_fast_cache_encode(instance, addr) libtea__arch_fast_cache_encode(instance, addr)


/**
 * Decodes a value in a given range from the cache.
 * 
 * Note: you must ensure that the 'from' and 'to' values you specify do not exceed the value of
 * instance->covert_channel_entries (255 by default).
 *
 * :param instance: The libtea instance
 * :param from: The index in the covert channel to start decoding from (inclusive)
 * :param to: The index in the covert channel to stop decoding at (inclusive)
 * :param use_mix: Whether to check the covert channel in a non-linear pattern to avoid hardware prefetching effects. Warning: can destroy the signal on some CPUs; always try without use_mix first.
 * :return: The decoded value or LIBTEA_ERROR on error
 */
libtea_inline int libtea_cache_decode_from_to(libtea_instance* instance, int from, int to, bool use_mix);


/**
 * Decodes a value encoded into the cache covert channel.
 *
 * :param instance: The libtea instance
 * :param use_mix: Whether to check the covert channel in a non-linear pattern to avoid hardware prefetching effects. Warning: can destroy the signal on some CPUs; always try without use_mix first.
 * :return: The decoded value or LIBTEA_ERROR on error
 */
libtea_inline int libtea_cache_decode(libtea_instance* instance, bool use_mix);


/**
 * Decodes a value encoded into the cache covert channel (ignoring null/0).
 *
 * :param instance: The libtea instance
 * :param use_mix: Whether to check the covert channel in a non-linear pattern to avoid hardware prefetching effects. Warning: can destroy the signal on some CPUs; always try without use_mix first.
 * :return: The decoded value or LIBTEA_ERROR on error
 */
libtea_inline int libtea_cache_decode_nonull(libtea_instance* instance, bool use_mix);


/**
 * Decodes a value encoded into the cache covert channel and updates a histogram.
 *
 * :param instance: The libtea instance
 * :param use_mix: Whether to check the covert channel in a non-linear pattern to avoid hardware prefetching effects. Warning: can destroy the signal on some CPUs; always try without use_mix first.
 * :param print: Whether to output the updated histogram to stdout
 * :param offset: The value to add to the covert channel index to print the actual encoded character (if using <256 entries)
 * :param from: The index in the covert channel to start decoding from (inclusive)
 * :param to: The index in the covert channel to stop decoding at (inclusive)
 * :param hist: The histogram to modify (expects an int array with 256 elements)
 */
libtea_inline void libtea_cache_decode_histogram_iteration(libtea_instance* instance, bool use_mix, bool print, int offset, int from, int to, int* hist);


/**
 * Prints a histogram of decoded cache covert channel values to stdout for the provided
 * number of iterations. 
 * 
 * Note: this function repeatedly clears the terminal window.
 * 
 * :param instance: The libtea instance
 * :param iterations: The number of iterations to repeat for
 * :param sleep_len: The number of microseconds to sleep for between iterations (0 to not sleep)
 * :param yield: If true, call sched_yield() / SwitchToThread() between iterations
 * :param use_mix: Whether to check the covert channel in a non-linear pattern to avoid hardware prefetching effects. Warning: can destroy the signal on some CPUs; always try without use_mix first.
 * :param activity: A pointer to a function which should be called before each decode, e.g. a call to the victim (can be NULL to do nothing)
 * :param offset: The value to add to the covert channel index to get the actual encoded character (if using <256 entries)
 * :param from: The index in the covert channel to start decoding from (inclusive)
 * :param to: The index in the covert channel to stop decoding at (inclusive)
 */
libtea_inline void libtea_print_cache_decode_histogram(libtea_instance* instance, int iterations, int sleep_len, bool yield, bool use_mix, void(*activity)(), int offset, int from, int to);


/**
 * Returns a histogram of decoded cache covert channel values over the provided number
 * of iterations as an int array.
 * 
 * Note: the returned array is malloc'd and must be manually freed.
 * (size = sizeof(int) * LIBTEA_COVERT_CHANNEL_ENTRIES)
 * 
 * :param instance: The libtea instance
 * :param iterations: The number of iterations to repeat for
 * :param sleep_len: The number of microseconds to sleep for between iterations (0 to not sleep)
 * :param yield: If true, call sched_yield() / SwitchToThread() between iterations
 * :param use_mix: Whether to check the covert channel in a non-linear pattern to avoid hardware prefetching effects. Warning: can destroy the signal on some CPUs; always try without use_mix first.
 * :param activity: A pointer to a function which should be called before each decode, e.g. a call to the victim (can be NULL to do nothing)
 * :param offset: The value to add to the covert channel index to get the actual encoded character (if using <256 entries)
 * :param from: The index in the covert channel to start decoding from (inclusive)
 * :param to: The index in the covert channel to stop decoding at (inclusive)
 */
libtea_inline int* libtea_numeric_cache_decode_histogram(libtea_instance* instance, int iterations, int sleep_len, bool yield, bool use_mix, void(*activity)(), int offset, int from, int to);


/*
 * Compares each decoded value with the expected value and returns the number of incorrect values.
 *
 * :param decoded: An array of values decoded from the cache covert channel
 * :param expected: An array of the expected/secret values
 * :param length: The length of the two arrays (must be equal)
 * :param print_results: If true, prints incorrect values and a success/fail summary to stdout, else runs silently
 * :return: The number of incorrect values
 */
int libtea_check_decoded(char* decoded, char* expected, int length, bool print_results);


/*
 * Calculates the percentage accuracy per decoded cache line (64 bytes) and prints the results
 * to stdout in CSV format.
 *
 * :param decoded: An array of values decoded from the cache covert channel
 * :param expected: An array of the expected/secret values
 * :param length: The length of the two arrays (must be equal)
 */
void libtea_check_decoded_per_cacheline(char* decoded, char* expected, int length);


/* Internal functions not included in API */
//---------------------------------------------------------------------------
libtea_inline static int libtea_init_cache(libtea_instance* instance);
libtea_inline static void libtea_cleanup_cache(libtea_instance* instance);


#define LIBTEA_ADDRESS_CACHE_SIZE (128)

#ifdef __cplusplus
}
#endif

#endif //LIBTEA_CACHE_H

/* See LICENSE file for license and copyright information */

#ifndef LIBTEA_ARCH_PAGING_H
#define LIBTEA_ARCH_PAGING_H

#ifdef __cplusplus
extern "C" {
#endif




typedef enum {LIBTEA_PGD, LIBTEA_PUD, LIBTEA_PMD, LIBTEA_PTE, LIBTEA_PAGE} libtea_page_level;


libtea_inline void libtea__arch_print_page_entry_line(size_t entry, int line);


libtea_inline void libtea__arch_get_paging_definitions(libtea_instance* instance);


libtea_inline size_t libtea__arch_set_pfn();


libtea_inline size_t libtea__arch_get_pfn(size_t pte);


libtea_inline char libtea__arch_get_mt(size_t mts, unsigned char mt);


libtea_inline const char* libtea__arch_mt_to_string(unsigned char mt);


libtea_inline size_t libtea__arch_set_mt(unsigned char mt);


libtea_inline unsigned char libtea__arch_find_mt(size_t mts, unsigned char type);


libtea_inline size_t libtea__arch_apply_mt(size_t entry, unsigned char mt);


libtea_inline unsigned char libtea__arch_extract_mt(size_t entry);


libtea_inline uint64_t libtea__arch_get_physical_base_address(libtea_page_entry entry, libtea_page_level level);


libtea_inline uint64_t libtea__arch_get_virtual_address_index(libtea_page_entry entry, libtea_page_level level);


#ifdef __cplusplus
}
#endif


#endif //LIBTEA_ARCH_PAGING_H
/* See LICENSE file for license and copyright information */

#ifndef LIBTEA_X86_PAGING_H
#define LIBTEA_X86_PAGING_H

#ifdef __cplusplus
extern "C" {
#endif


#if LIBTEA_X86


/** Page is present */
#define LIBTEA_PAGE_BIT_PRESENT 0
/** Page is writeable */
#define LIBTEA_PAGE_BIT_RW 1
/** Page is userspace addressable */
#define LIBTEA_PAGE_BIT_USER 2
/** Page write through */
#define LIBTEA_PAGE_BIT_PWT 3
/** Page cache disabled */
#define LIBTEA_PAGE_BIT_PCD 4
/** Page was accessed (raised by CPU) */
#define LIBTEA_PAGE_BIT_ACCESSED 5
/** Page was written to (raised by CPU) */
#define LIBTEA_PAGE_BIT_DIRTY 6
/** 4 MB (or 2MB) page */
#define LIBTEA_PAGE_BIT_PSE 7
/** PAT (only on 4KB pages) */
#define LIBTEA_PAGE_BIT_PAT 7
/** Global TLB entry PPro+ */
#define LIBTEA_PAGE_BIT_GLOBAL 8
/** Available for programmer */
#define LIBTEA_PAGE_BIT_SOFTW1 9
/** Available for programmer */
#define LIBTEA_PAGE_BIT_SOFTW2 10
/** Windows only: Prototype PTE. PTE is actually a "symlink" to an OS-managed Prototype PTE shared between multiple processes. This enables working set trimming of shared memory. */
#define LIBTEA_PAGE_BIT_PROTOTYPE LIBTEA_PAGE_BIT_SOFTW2
/** Available for programmer */
#define LIBTEA_PAGE_BIT_SOFTW3 11
/** Windows only: Transition PTE. The data is still valid, but the OS has cleared the present bit in anticipation of removing the page from the process' working set. */
#define LIBTEA_PAGE_BIT_TRANSITION LIBTEA_PAGE_BIT_SOFTW3
/** PAT (on 2MB or 1GB pages) */
#define LIBTEA_PAGE_BIT_PAT_LARGE 12
/** Available for programmer */
#define LIBTEA_PAGE_BIT_SOFTW4 58
/** Protection Keys, bit 1/4 */
#define LIBTEA_PAGE_BIT_PKEY_BIT0 59
/** Protection Keys, bit 2/4 */
#define LIBTEA_PAGE_BIT_PKEY_BIT1 60
/** Protection Keys, bit 3/4 */
#define LIBTEA_PAGE_BIT_PKEY_BIT2 61
/** Protection Keys, bit 4/4 */
#define LIBTEA_PAGE_BIT_PKEY_BIT3 62
/** No execute: only valid after cpuid check */
#define LIBTEA_PAGE_BIT_NX 63


/** Strong uncachable (nothing is cached, strong memory ordering, no speculative reads) */
#define LIBTEA_UNCACHEABLE        0
/** Write combining (consecutive writes are combined in a WC buffer and then written once, allows speculative reads, weak memory ordering) */
#define LIBTEA_WRITE_COMBINING    1
/** Write through (read accesses are cached, write accesses are written to cache and memory, allows speculative reads, speculative processor ordering) */
#define LIBTEA_WRITE_THROUGH      4
/** Write protected (only read accesses are cached, allows speculative reads, speculative processor ordering) */
#define LIBTEA_WRITE_PROTECTED    5
/** Write back (read and write accesses are cached, allows speculative reads, speculative processor ordering) */
#define LIBTEA_WRITE_BACK         6
/** Uncachable minus / UC- (same as strong uncacheable, except this setting can be overridden to write-combining using the MTRRs) */
#define LIBTEA_UNCACHEABLE_MINUS  7

#define LIBTEA_PAGE_PRESENT 1


// Returns a mask of the form:
// +----- n+1 -+- n --------- 0-+
// | 0  0  0   |  1  1  1  1  1 |
// +-----------+----------------+
#define LIBTEA_MASK_TO(m)			    ((UINT64_C(0x1) << ((m) + 1)) - 1 )

// Returns a mask of the form:
// +----- m+1 -+- m ------ n -+--- 0-+
// | 0  0  0   |  1  1  1  1  | 0  0 |
// +-----------+--------------+------+
// The ordered version requires n < m, the other CREATE_MASK checks this at runtime
#define LIBTEA_CREATE_MASK_ORDERED(n,m)	(LIBTEA_MASK_TO((m)) ^ (LIBTEA_MASK_TO((n)) >> 1))
#define LIBTEA_CREATE_MASK(n,m)			(((n) < (m)) ? (LIBTEA_CREATE_MASK_ORDERED((n), (m))) : (LIBTEA_CREATE_MASK_ORDERED((m), (n))))
#define M			                    (libtea_get_physical_address_width())
#define MASK_M			                ((uint64_t) ((INT64_C(0x1) << M) - 1))

#define LIBTEA_PUD_PS_SHIFT		        7
#define LIBTEA_PUD_PS_MASK		        (UINT64_C(0x1) << LIBTEA_PUD_PS_SHIFT)

#define LIBTEA_PMD_PS_SHIFT		        7
#define LIBTEA_PMD_PS_MASK		        (UINT64_C(0x1) << LIBTEA_PMD_PS_SHIFT)

#define LIBTEA_PTE_PHYS_SHIFT	        12
#define LIBTEA_PTE_PHYS_MASK	        (MASK_M << LIBTEA_PTE_PHYS_SHIFT)

#define LIBTEA_PGD_SHIFT		        39
#define LIBTEA_PGD_MASK			        (INT64_C(0x1ff) << LIBTEA_PGD_SHIFT)

#define LIBTEA_PUD_SHIFT		        30
#define LIBTEA_PUD_MASK			        (INT64_C(0x1ff) << LIBTEA_PUD_SHIFT)

#define LIBTEA_PMD_SHIFT		        21
#define LIBTEA_PMD_MASK			        (INT64_C(0x1ff) << LIBTEA_PMD_SHIFT)

#define LIBTEA_PTE_SHIFT		        12
#define LIBTEA_PTE_MASK			        (INT64_C(0x1ff) << LIBTEA_PTE_SHIFT)

#define LIBTEA_PAGE_SHIFT		        0
#define LIBTEA_PAGE_MASK		        (INT64_C(0xfff) << LIBTEA_PAGE_SHIFT)

#define LIBTEA_PAGE1GiB_SHIFT		    0
#define LIBTEA_PAGE1GiB_MASK		    (INT64_C(0x3FFFFFFF) << LIBTEA_PAGE1GiB_SHIFT)

#define LIBTEA_PAGE2MiB_SHIFT		    0
#define LIBTEA_PAGE2MiB_MASK		    (INT64_C(0x1FFFFF) << LIBTEA_PAGE2MiB_SHIFT)

#define LIBTEA_PAGE_SIZE_4KiB		    0x1000
#define LIBTEA_PAGE_SIZE_2MiB		    0x200000
#define LIBTEA_PAGE_SIZE_1GiB		    0x40000000

#define LIBTEA_PUD_PS(entry)			(((entry) & LIBTEA_PUD_PS_MASK) >> LIBTEA_PUD_PS_SHIFT)
#define LIBTEA_PMD_PS(entry)			(((entry) & LIBTEA_PMD_PS_MASK) >> LIBTEA_PMD_PS_SHIFT)

#define LIBTEA_PGD_PHYS(entry)			((entry) & (LIBTEA_CREATE_MASK(12, M-1)))
#define LIBTEA_PUD_PS_0_PHYS(entry)    	((entry) & (LIBTEA_CREATE_MASK(12, M-1)))
#define LIBTEA_PUD_PS_1_PHYS(entry)		((entry) & (LIBTEA_CREATE_MASK(30, M-1)))
#define LIBTEA_PMD_PS_0_PHYS(entry)		((entry) & (LIBTEA_CREATE_MASK(12, M-1)))
#define LIBTEA_PMD_PS_1_PHYS(entry)		((entry) & (LIBTEA_CREATE_MASK(21, M-1)))
#define LIBTEA_PT_PHYS(entry)			((entry) & (LIBTEA_CREATE_MASK(12, M-1)))

#define LIBTEA_PGD_INDEX(vaddr)			(vaddr & LIBTEA_PGD_MASK) >> LIBTEA_PGD_SHIFT
#define LIBTEA_PUD_INDEX(vaddr)			(vaddr & LIBTEA_PUD_MASK) >> LIBTEA_PUD_SHIFT
#define LIBTEA_PMD_INDEX(vaddr)			(vaddr & LIBTEA_PMD_MASK) >> LIBTEA_PMD_SHIFT
#define LIBTEA_PTE_INDEX(vaddr)			(vaddr & LIBTEA_PTE_MASK) >> LIBTEA_PTE_SHIFT
#define LIBTEA_PAGE_INDEX(vaddr)		(vaddr & LIBTEA_PAGE_MASK) >> LIBTEA_PAGE_SHIFT
#define LIBTEA_PAGE1GiB_INDEX(vaddr)	(vaddr & LIBTEA_PAGE1GiB_MASK) >> LIBTEA_PAGE1GiB_SHIFT
#define LIBTEA_PAGE2MiB_INDEX(vaddr)	(vaddr & LIBTEA_PAGE2MiB_MASK) >> LIBTEA_PAGE2MiB_SHIFT


/**
 * Struct to access the fields of the PGD
 */
#pragma pack(push,1)
typedef struct {
    size_t present : 1;
    size_t writeable : 1;
    size_t user_access : 1;
    size_t write_through : 1;
    size_t cache_disabled : 1;
    size_t accessed : 1;
    size_t ignored_3 : 1;
    size_t size : 1;
    size_t ignored_2 : 4;
    size_t pfn : 28;
    size_t reserved_1 : 12;
    size_t ignored_1 : 11;
    size_t execution_disabled : 1;
} libtea_pgd;
#pragma pack(pop)


/**
 * Struct to access the fields of the P4D
 */
typedef libtea_pgd libtea_p4d;


/**
 * Struct to access the fields of the PUD
 */
typedef libtea_pgd libtea_pud;


/**
 * Struct to access the fields of the PMD
 */
typedef libtea_pgd libtea_pmd;


/**
 * Struct to access the fields of the PMD when mapping a large page (2MB)
 */
#pragma pack(push,1)
typedef struct {
    size_t present : 1;
    size_t writeable : 1;
    size_t user_access : 1;
    size_t write_through : 1;
    size_t cache_disabled : 1;
    size_t accessed : 1;
    size_t dirty : 1;
    size_t size : 1;
    size_t global : 1;
    size_t ignored_2 : 3;
    size_t pat : 1;
    size_t reserved_2 : 8;
    size_t pfn : 19;
    size_t reserved_1 : 12;
    size_t ignored_1 : 11;
    size_t execution_disabled : 1;
} libtea_pmd_large;
#pragma pack(pop)


/**
 * Struct to access the fields of the PTE
 */
#pragma pack(push,1)
typedef struct {
    size_t present : 1;                     /* Windows note: if present==1 this is a hardware PTE and we can handle it like a normal PTE. Other notes below only apply when present==0. */
    size_t writeable : 1;
    size_t user_access : 1;
    size_t write_through : 1;
    size_t cache_disabled : 1;
    size_t accessed : 1;
    size_t dirty : 1;
    size_t size : 1;
    size_t global : 1;
    size_t ignored_2 : 3;                   /* Windows note (only if valid==0): if bit 10 is set and not bit 11, it is a Prototype PTE. If bit 11 is set and not bit 10, it is a Transition PTE. If neither are set, it is a Software PTE (paged out to page file). */
    size_t pfn : 28;
    size_t reserved_1 : 12;
    size_t ignored_1 : 11;
    size_t execution_disabled : 1;
} libtea_pte;
#pragma pack(pop)


#endif //LIBTEA_X86

#ifdef __cplusplus
}
#endif

#endif //LIBTEA_X86_PAGING_H
/* See LICENSE file for license and copyright information */

#ifndef LIBTEA_PAGING_H
#define LIBTEA_PAGING_H

#ifdef __cplusplus
extern "C" {
#endif




#if defined(_MSC_VER) && (!defined(MOZJS_MAJOR_VERSION) || MOZJS_MAJOR_VERSION < 63)   /* If using Visual Studio C++ compiler. We have to explicitly exclude Firefox after MOZJS v63 as they use the hybrid clang-cl */
#define LIBTEA_PAGE_ALIGN_CHAR __declspec(align(4096)) char
#else                                                                                  /* Assume using GCC, MinGW GCC, or Clang */
#define LIBTEA_PAGE_ALIGN_CHAR char __attribute__((aligned(4096)))
#endif


/* Assumptions - will not be true on less common arches */
#define LIBTEA_PFN_MASK                	0xfffULL
unsigned char libtea_page_shift = 12;


int libtea__paging_init(libtea_instance* instance);


/** Use the kernel to resolve and update paging structures */
#define LIBTEA_PAGING_IMPL_KERNEL       0
/** Use the user-space implemenation to resolve and update paging structures, using pread to read from the memory mapping */
#define LIBTEA_PAGING_IMPL_USER_PREAD   1
/** Use the user-space implementation that maps physical memory into user space to resolve and update paging structures */
#define LIBTEA_PAGING_IMPL_USER         2


typedef libtea_page_entry(*libtea_resolve_addr_func)(libtea_instance*, void*, pid_t);
typedef void (*libtea_update_addr_func)(libtea_instance*, void*, pid_t, libtea_page_entry*);
void libtea__cleanup_paging(libtea_instance* instance);


/**
 * Switch between kernel and user-space paging implementations.
 *
 * :param instance: The libtea instance
 * :param implementation: The implementation to use, either LIBTEA_PAGING_IMPL_KERNEL, LIBTEA_PAGING_IMPL_USER, or LIBTEA_PAGING_IMPL_USER_PREAD
 */
void libtea_set_paging_implementation(libtea_instance* instance, int implementation);


/**
 * Resolves the page table entries of all levels for a virtual address of a given process.
 *
 * :param instance: The libtea instance
 * :param address: The virtual address to resolve
 * :param pid: The PID of the process (0 for own process)
 *
 * :return: A structure containing the page table entries of all levels.
 */
libtea_resolve_addr_func libtea_resolve_addr;


/**
 * Updates one or more page table entries for a virtual address of a given process.
 * The TLB for the given address is flushed after updating the entries.
 *
 * :param instance: The libtea instance
 * :param address: The virtual address
 * :param pid: The PID of the process (0 for own process)
 * :param vm: A structure containing the values for the page table entries and a bitmask indicating which entries to update
 */
libtea_update_addr_func libtea_update_addr;


/**
 * Sets a bit in the page table entry of an address.
 *
 * :param instance: The libtea instance
 * :param address: The virtual address
 * :param pid: The PID of the process (0 for own process)
 * :param bit: The bit to set (one of LIBTEA_PAGE_BIT_*)
 */
void libtea_set_addr_page_bit(libtea_instance* instance, void* address, pid_t pid, int bit);


/**
 * Clears a bit in the page table entry of an address.
 *
 * :param instance: The libtea instance
 * :param address: The virtual address
 * :param pid: The PID of the process (0 for own process)
 * :param bit: The bit to clear (one of LIBTEA_PAGE_BIT_*)
 */
void libtea_clear_addr_page_bit(libtea_instance* instance, void* address, pid_t pid, int bit);


/**
 * Helper function to mark a page as present and ensure the kernel is aware of this.
 * Linux only. Use in preference to libtea_set_addr_page_bit to avoid system crashes
 * (only necessary for the special case of the present bit). 
 *
 * :param instance: The libtea instance
 * :param page: A pointer to the page (must be mapped within the current process)
 * :param prot: The mprotect protection flags to reapply to the page, e.g. PROT_READ
 * :return: LIBTEA_SUCCESS on success, else LIBTEA_ERROR
 */
int libtea_mark_page_present(libtea_instance* instance, void* page, int prot);


/**
 * Helper function to mark a page as not present and ensure the kernel is aware of this.
 * Linux only. Use in preference to libtea_clear_addr_page_bit to avoid system crashes
 * (only necessary for the special case of the present bit). 
 *
 * :param instance: The libtea instance
 * :param page: A pointer to the page (must be mapped within the current process)
 * :return: LIBTEA_SUCCESS on success, else LIBTEA_ERROR
 */
int libtea_mark_page_not_present(libtea_instance* instance, void* page);


/**
 * Returns the value of a bit from the page table entry of an address.
 *
 * :param instance: The libtea instance
 * :param address: The virtual address
 * :param pid: The PID of the process (0 for own process)
 * :param bit: The bit to get (one of LIBTEA_PAGE_BIT_*)
 *
 * :return: The value of the bit (0 or 1)
 *
 */
unsigned char libtea_get_addr_page_bit(libtea_instance* instance, void* address, pid_t pid, int bit);

/**
 * Reads the page frame number (PFN) from the page table entry of an address.
 *
 * IMPORTANT: check if this has returned 0 before you use the value!
 * On Windows, the PFN will be 0 of the page has not yet been committed
 * (e.g. if you have allocated but not accessed the page).
 *
 * :param instance: The libtea instance
 * :param address: The virtual address
 * :param pid: The PID of the process (0 for own process)
 *
 * :return: The PFN
 */
size_t libtea_get_addr_pfn(libtea_instance* instance, void* address, pid_t pid);

/**
 * Sets the PFN in the page table entry of an address.
 *
 * :param instance: The libtea instance
 * :param address: The virtual address
 * :param pid: The PID of the process (0 for own process)
 * :param pfn: The new PFN
 *
 */
void libtea_set_addr_pfn(libtea_instance* instance, void* address, pid_t pid, size_t pfn);


/**
 * Casts a paging structure entry to a structure with easy access to its fields.
 *
 * :param v: Entry to Cast
 * :param type: Data type of struct to cast to, e.g., libtea_pte
 *
 * :return: Struct of type "type" with easily-accessible fields
 */
#define libtea_cast(v, type) (*((type*)(&(v))))


/**
 * Returns a new page table entry where the PFN is replaced by the specified one.
 *
 * :param entry: The page table entry to modify
 * :param pfn: The new PFN
 *
 * :return: A new page table entry with the given PFN
 */
size_t libtea_set_pfn(size_t entry, size_t pfn);


/**
 * Returns the PFN of a page table entry.
 *
 * :param entry: The page table entry to extract the PFN from
 *
 * :return: The PFN
 */
size_t libtea_get_pfn(size_t entry);


/**
 * Retrieves the content of a physical page.
 *
 * :param instance: The libtea instance
 * :param pfn: The PFN of the page to read
 * :param buffer: A buffer that is large enough to hold the content of the page
 */
void libtea_read_physical_page(libtea_instance* instance, size_t pfn, char* buffer);


/**
 * Replaces the content of a physical page.
 *
 * :param instance: The libtea instance
 * :param pfn: The PFN of the page to update
 * :param content: A buffer containing the new content of the page (must be the size of the physical page)
 */
void libtea_write_physical_page(libtea_instance* instance, size_t pfn, char* content);


/**
 * Map a physical address range into this process' virtual address space.
 *
 * :param instance: The libtea instance
 * :param paddr: The physical address of the start of the range
 * :param length: The length of the physical memory range to map
 * :param prot: The memory protection settings for the virtual mapping (e.g. PROT_READ | PROT_WRITE)
 * :param use_dev_mem: Map with /dev/mem if true, else use libtea_umem. (Only /dev/mem supports PROT_EXEC.)
 * :return: A virtual address that can be used to access the physical range
 */
void* libtea_map_physical_address_range(libtea_instance* instance, size_t paddr, size_t length, int prot, bool use_dev_mem);


/**
 * Unmaps an address range that was mapped into this process' virtual address space with libtea_map_physical_address_range or
 * libtea_remap_address.
 * 
 * Note: supported on Linux only.
 *
 * :param vaddr: The virtual address of the mapping
 * :param length: The length of the range to unmap
 * :return: LIBTEA_SUCCESS on success, else LIBTEA_ERROR
 */
int libtea_unmap_address_range(size_t vaddr, size_t length);


/**
 * Creates an additional virtual mapping to the physical page backing the provided virtual address.
 * Uses libtea_get_physical_address_at_level internally, so can be used with addresses not in the
 * process' pagemap. Use libtea_unmap_address_range to free the mapping.
 * 
 * Note: supported on Linux x86 only.
 *
 * :param instance: The libtea instance
 * :param vaddr: The virtual address to remap
 * :param level: The page table level to resolve the address at
 * :param length: The length of the range to map
 * :param prot: The memory protection to use, e.g. PROT_READ
 * :param use_dev_mem: Map with /dev/mem if true, else use libtea_umem. (Only /dev/mem supports PROT_EXEC.)
 * :return: An additional virtual address for the underlying physical address, or MAP_FAILED on error
 */
void* libtea_remap_address(libtea_instance* instance, size_t vaddr, libtea_page_level level, size_t length, int prot, bool use_dev_mem);


/**
 * Returns the root of the paging structure (i.e., the value of CR3 on x86 / TTBR0 on ARM).
 *
 * :param instance: The libtea instance
 * :param pid: The process id (0 for own process)
 * :return: The paging root, i.e. the physical address of the first page table (i.e., the PGD)
 */
size_t libtea_get_paging_root(libtea_instance* instance, pid_t pid);


/**
 * Sets the root of the paging structure (i.e., the value of CR3 on x86 / TTBR0 on ARM).
 *
 * :param instance: The libtea instance
 * :param pid: The proccess id (0 for own process)
 * :param root: The new paging root, i.e. the new physical address of the first page table (i.e., the PGD)
 */
void libtea_set_paging_root(libtea_instance* instance, pid_t pid, size_t root);


/**
 * Flushes/invalidates the TLB for a given address on all CPUs.
 *
 * :param instance: The libtea instance
 * :param address: The address to invalidate
 */
void libtea_flush_tlb(libtea_instance* instance, void* address);


/**
 * A full serializing barrier specifically for paging (overwrites the paging root with its current value).
 *
 * :param instance: The libtea instance
 */
void libtea_paging_barrier(libtea_instance* instance);


/**
 * Changes the implementation used for flushing the TLB. Both implementations use the kernel module, but LIBTEA_FLUSH_TLB_KERNEL
 * uses the native kernel functionality and is much faster; it should be preferred unless your kernel does not support
 * flush_tlb_mm_range.
 *
 * Note: supported on Linux only.
 *
 * :param instance: The libtea instance
 * :param implementation: The implementation to use, either LIBTEA_FLUSH_TLB_KERNEL or LIBTEA_FLUSH_TLB_CUSTOM
 *
 * :return: LIBTEA_SUCCESS on success, otherwise LIBTEA_ERROR
 */
int libtea_switch_flush_tlb_implementation(libtea_instance* instance, int implementation);


/**
 * Returns the default page size of the system.
 *
 * :param instance: The libtea instance
 * :return: Page size of the system in bytes
 */
int libtea_get_pagesize(libtea_instance* instance);


/**
 * Returns the physical address width.
 * 
 * Note: supported on Linux x86 only.
 *
 * :return: Physical address width of the CPU
 */
uint64_t libtea_get_physical_address_width();


/**
 * Gets the physical address of the provided virtual address at the provided paging level.
 * Currently only supported on Linux x86.
 *
 * :param instance: The libtea instance
 * :param vaddr: The virtual address
 * :param level: Page level to resolve the physical address of
 * :return: The physical address
 */
size_t libtea_get_physical_address_at_level(libtea_instance* instance, size_t vaddr, libtea_page_level level);


/**
 * Reads the value of all memory types (x86 PATs / ARM MAIRs). This is equivalent to reading the MSR 0x277 (x86) / MAIR_EL1 (ARM).
 *
 * :param: The libtea instance
 * :return: The memory types in the same format as in the IA32_PAT MSR / MAIR_EL1
 */
size_t libtea_get_memory_types(libtea_instance* instance);


/**
 * Programs the value of all memory types (x86 PATs / ARM MAIRs). This is equivalent to writing to the MSR 0x277 (x86) / MAIR_EL1 (ARM) on all CPUs.
 *
 * :param instance: The libtea instance
 * :param mts: The memory types in the same format as in the IA32_PAT MSR / MAIR_EL1
 */
void libtea_set_memory_types(libtea_instance* instance, size_t mts);


/**
 * Reads the value of a specific memory type attribute (PAT/MAIR).
 *
 * :param instance: The libtea instance
 * :param mt: The PAT/MAIR ID (from 0 to 7)
 * :return: The PAT/MAIR value (LIBTEA_UNCACHEABLE, LIBTEA_UNCACHEABLE_MINUS,
                LIBTEA_WRITE_COMBINING, LIBTEA_WRITE_THROUGH, LIBTEA_WRITE_BACK,
                or LIBTEA_WRITE_PROTECTED)
 */
char libtea_get_memory_type(libtea_instance* instance, unsigned char mt);


/**
 * Programs the value of a specific memory type attribute (PAT/MAIR).
 *
 * :param instance: The libtea instance
 * :param mt: The PAT/MAIR ID (from 0 to 7)
 */
void libtea_set_memory_type(libtea_instance* instance, unsigned char mt, unsigned char value);


/**
 * Generates a bitmask of all memory type attributes (PAT/MAIR) which are programmed to the given value.
 *
 * :param instance: The libtea instance
 * :param type: A memory type (LIBTEA_UNCACHEABLE, LIBTEA_UNCACHEABLE_MINUS,
                LIBTEA_WRITE_COMBINING, LIBTEA_WRITE_THROUGH, LIBTEA_WRITE_BACK,
                or LIBTEA_WRITE_PROTECTED)
 *
 * :return: A bitmask where a set bit indicates that the corresponding PAT/MAIR has the given type
 */
unsigned char libtea_find_memory_type(libtea_instance* instance, unsigned char type);


/**
 * Returns the first memory type attribute (PAT/MAIR) that is programmed to the given memory type.
 *
 * :param instance: The libtea instance
 * :param type: A memory type (LIBTEA_UNCACHEABLE, LIBTEA_UNCACHEABLE_MINUS,
                LIBTEA_WRITE_COMBINING, LIBTEA_WRITE_THROUGH, LIBTEA_WRITE_BACK,
                or LIBTEA_WRITE_PROTECTED)
 *
 * :return: A PAT/MAIR ID, or -1 if no PAT/MAIR of this type was found
 */
int libtea_find_first_memory_type(libtea_instance* instance, unsigned char type);


/**
 * Returns a new page table entry which uses the given memory type (PAT/MAIR).
 *
 * :param entry: A page table entry
 * :param mt: The PAT/MAIR ID (from 0 to 7)
 *
 * :return: A new page table entry with the given memory type (PAT/MAIR)
 */
size_t libtea_apply_memory_type(size_t entry, unsigned char mt);


/**
 * Sets the memory type of the page to the specified type, e.g. LIBTEA_UNCACHEABLE
 * for strong uncacheable or LIBTEA_WRITE_BACK for write back caching.
 *
 * :param instance: The libtea instance
 * :param page: A pointer to the page
 * :param type: A memory type (LIBTEA_UNCACHEABLE, LIBTEA_UNCACHEABLE_MINUS,
                LIBTEA_WRITE_COMBINING, LIBTEA_WRITE_THROUGH, LIBTEA_WRITE_BACK, 
                or LIBTEA_WRITE_PROTECTED)
 *
 * :return: LIBTEA_SUCCESS on success, else LIBTEA_ERROR
 */
int libtea_set_page_cacheability(libtea_instance* instance, void* page, unsigned char type);


/**
 * Returns the memory type (i.e., PAT/MAIR ID) which is used by a page table entry.
 *
 * :param entry: A page table entry
 *
 * :return: A PAT/MAIR ID (between 0 and 7)
 */
unsigned char libtea_extract_memory_type(size_t entry);


/**
 * Returns a human-readable representation of a memory type (PAT/MAIR value).
 *
 * :param mt: A PAT/MAIR ID
 *
 * :return: A human-readable representation of the memory type
 */
const char* libtea_memory_type_to_string(unsigned char mt);


/**
 * Pretty prints a libtea_page_entry struct.
 *
 * :param entry: A libtea_page_entry struct
 */
void libtea_print_libtea_page_entry(libtea_page_entry entry);


/**
 * Pretty prints a page table entry.
 *
 * :param entry: A page table entry
 */
void libtea_print_page_entry(size_t entry);


/**
 * Prints a single line of the pretty-print representation of a page table entry.
 *
 * :param entry: A page table entry
 * :param line: The line to print (0 to 3)
 */
#define libtea_print_page_entry_line(entry, line)  libtea__arch_print_page_entry_line(entry, line)


/**
 * Forces a page combining scan across the whole system (Windows-only).
 * This is experimental and is only enabled if LIBTEA_ENABLE_WINDOWS_MEMORY_DEDUPLICATION
 * is set to 1 in libtea_config.h.
 *
 * :return: The number of pages combined
 */
#if LIBTEA_ENABLE_WINDOWS_MEMORY_DEDUPLICATION
long long libtea_force_memory_deduplication();
#endif


#ifdef __cplusplus
}
#endif

#endif //LIBTEA_PAGING_H

/* See LICENSE file for license and copyright information */

/* Start libtea_common.c */
//---------------------------------------------------------------------------

/* TODO debug compiler flags - Windows should ideally be -O2 but this seems to force the frame pointer
 * being omitted, which could cause problems with setjmp from a non main function.
 */




/* Internal functions not included in API */
//---------------------------------------------------------------------------
#if LIBTEA_LINUX
libtea_inline void libtea__thread_create(int* tid, void** stack_ptr, int (*fnc)(void*), void* arg);
#else
libtea_inline void libtea__thread_create(libtea_thread* thread, LPVOID ignored, LPTHREAD_START_ROUTINE func, LPVOID param);
libtea_inline void libtea__pin_thread_to_core(libtea_thread thread, int core);
#endif
libtea_inline void libtea__thread_cancel(libtea_thread thread, void* stack);
uint64_t libtea__timestamp_perf(libtea_instance* instance);
uint64_t libtea__timestamp_counting_thread(libtea_instance* instance);
libtea_inline static void libtea__set_timer(libtea_instance* instance, libtea_timer timer);
libtea_inline void libtea__init_counter_thread(libtea_instance* instance);
libtea_inline void libtea__cleanup_counter_thread(libtea_instance* instance);
libtea_inline static void libtea__init_perf(libtea_instance* instance);
libtea_inline void libtea__cleanup_perf(libtea_instance* instance);
libtea_inline int libtea__get_numeric_sys_cmd_output(const char* cmd);


void libtea__unblock_signal(int signum) {
  #if LIBTEA_LINUX
  sigset_t sigs;
  sigemptyset(&sigs);
  sigaddset(&sigs, signum);
  sigprocmask(SIG_UNBLOCK, &sigs, NULL);
  #else
  /* Can get away with doing nothing on Windows */
  #endif
}


void libtea__trycatch_segfault_handler(int signum) {
  int i;
  for(i = 1; i < 32; i++) {
    libtea__unblock_signal(i);
  }
  #if LIBTEA_LINUX
  siglongjmp(libtea__trycatch_buf, 1);
  #else
  longjmp(libtea__trycatch_buf, 1);
  #endif
}


libtea_inline void libtea__try_start_prep() {
  libtea__saved_sighandler[0] = signal(SIGILL, libtea__trycatch_segfault_handler);
  libtea__saved_sighandler[1] = signal(SIGFPE, libtea__trycatch_segfault_handler);
  libtea__saved_sighandler[2] = signal(SIGSEGV, libtea__trycatch_segfault_handler);
}


void libtea__sigill_handler(int signum) {
  #if LIBTEA_LINUX
  libtea__unblock_signal(SIGILL);
  longjmp(libtea__trycatch_buf, 1);
  #else
  longjmp(libtea__trycatch_buf, 1);
  #endif
}


#if LIBTEA_LINUX
libtea_inline void libtea__thread_create(int* tid, void** stack_ptr, int (*fnc)(void*), void* arg) {
  int stacksize = 4096;
  char* stack = (char*) mmap(0, stacksize, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
  if(stack_ptr) *stack_ptr = stack;
  /* CLONE_VM|CLONE_FS|CLONE_FILES|CLONE_PARENT|CLONE_IO */
  *tid = clone(fnc, stack + stacksize - sizeof(void*) * 2, 0x80008700, arg);
}
#else
void libtea__thread_create(libtea_thread* thread, LPVOID ignored, LPTHREAD_START_ROUTINE func, LPVOID param){
  *thread = CreateThread(NULL, 4096, func, param, 0, NULL);
  if(*thread==NULL) libtea_info("Error: failed to create thread in libtea__thread_create.");
}
#endif


libtea_inline void libtea__thread_cancel(libtea_thread thread, void* stack) {
  #if LIBTEA_LINUX
  if(thread > 0) {
    kill(thread, 2);
  }
  if(stack) {
    munmap(stack, 4096);
  }
  #else
  TerminateThread(thread, 1);
  #endif
}


libtea_inline uint64_t libtea__timestamp_perf(libtea_instance* instance) {
  #if LIBTEA_LINUX
  uint64_t result = 0;
  if (read(instance->perf_fd, &result, sizeof(result)) < (ssize_t)sizeof(result)) {
    return 0;
  }
  return result;
  #else 
  /* Closest Windows equivalent to the perf interface. Microsoft recommends over RDTSC. */
  LARGE_INTEGER time;
  QueryPerformanceCounter(&time);
  return (uint64_t)time.QuadPart;
  #endif
}


libtea_inline uint64_t libtea__timestamp_counting_thread(libtea_instance* instance) {
  return instance->thread_counter;
}


libtea_inline static void libtea__set_timer(libtea_instance* instance, libtea_timer timer) {
  libtea__cleanup_counter_thread(instance);

  if(timer == LIBTEA_TIMER_NATIVE) {
    instance->timer = (libtea_timer_function) libtea__arch_timestamp_native;
  }
  #if LIBTEA_X86
  else if(timer == LIBTEA_TIMER_NATIVE_AMD_ZEN2){
    instance->timer = (libtea_timer_function) libtea__arch_timestamp_native_amd_zen2;
  }
  else if(timer == LIBTEA_TIMER_NATIVE_AMD_ZEN){
    #if LIBTEA_LINUX
    if(instance->module_fd > 0){
    #else
    if(instance->module_fd != NULL){
    #endif
      instance->timer = (libtea_timer_function) libtea__arch_timestamp_native_amd_zen;
    }
    else{
      libtea_info("Could not set timer LIBTEA_TIMER_NATIVE_AMD_ZEN. Have you loaded the kernel module? Falling back to rdtsc.");
      instance->timer = (libtea_timer_function) libtea__arch_timestamp_native;
    }
  }
  #endif

  else if(timer == LIBTEA_TIMER_COUNTING_THREAD) {
    libtea__init_counter_thread(instance);
  }
  else if(timer == LIBTEA_TIMER_MONOTONIC_CLOCK) {
    instance->timer = (libtea_timer_function) libtea__arch_timestamp_monotonic;
  }
  else if(timer == LIBTEA_TIMER_PERF) {
    libtea__init_perf(instance);
  }
}


libtea_inline void libtea__init_counter_thread(libtea_instance* instance) {

  libtea__thread_create(&(instance->timing_thread), &(instance->timing_thread_stack), libtea__arch_counting_thread, (void*)&(instance->thread_counter));
  instance->timer = (libtea_timer_function) libtea__timestamp_counting_thread;

  #if LIBTEA_LINUX
  int current_cpu = sched_getcpu();
  sched_getaffinity(getpid(), sizeof(cpu_set_t), &(instance->cpumask));
  libtea_pin_to_core(getpid(), current_cpu);

  #if LIBTEA_HAVE_HYPERTHREADING
  /* Double check we *actually* have hyperthreading, even though the config claims we do */

  const char* cmd1 = LIBTEA_SHELL " -c 'cat /sys/devices/system/cpu/smt/active'";
  /* Older kernels do not provide this SMT control, so also try this */
  const char* cmd2 = LIBTEA_SHELL " -c 'cat /sys/devices/system/cpu/cpu*/topology/thread_siblings_list | grep - | wc -l'";

  if(libtea__get_numeric_sys_cmd_output(cmd1) || libtea__get_numeric_sys_cmd_output(cmd2)){
    int hyper = libtea_get_hyperthread(current_cpu);
    if(hyper != LIBTEA_ERROR) {
      libtea_pin_to_core(instance->timing_thread, hyper);
    }
    else libtea_info("Error: could not get hyperthread in libtea__init_counter_thread. Is hyperthreading present/enabled?");
  }

  #endif

  #else
  int core = GetCurrentProcessorNumber();
  libtea__pin_thread_to_core(instance->timing_thread, core);
  //TODO if hyperthreading pin to hyperthread
  #endif
}


libtea_inline void libtea__cleanup_counter_thread(libtea_instance* instance) {
  #if LIBTEA_LINUX
  if(instance->timing_thread > 0) {
    libtea__thread_cancel(instance->timing_thread, instance->timing_thread_stack);
    instance->timing_thread_stack = NULL;
    instance->timing_thread = 0;
    sched_setaffinity(getpid(), sizeof(cpu_set_t), &(instance->cpumask));
  }
  #else
  if(instance->timing_thread != NULL) {
    libtea__thread_cancel(instance->timing_thread, NULL);
  }
  #endif
}


libtea_inline static void libtea__init_perf(libtea_instance* instance) {
  #if LIBTEA_LINUX
  instance->timer = (libtea_timer_function) libtea__timestamp_perf;
  static struct perf_event_attr attr;
  attr.type = PERF_TYPE_HARDWARE;
  attr.config = PERF_COUNT_HW_CPU_CYCLES;
  attr.size = sizeof(attr);
  attr.exclude_kernel = 1;
  attr.exclude_hv = 1;
  attr.exclude_callchain_kernel = 1;
  instance->perf_fd = syscall(__NR_perf_event_open, &attr, 0, -1, -1, 0);
  if(instance->perf_fd <= 0) libtea_info("Could not initialize perf. Are you running with root privileges?");
  #else
  /* No need to do anything - on Windows we use QueryPerformanceCounter instead, which needs no initialization */
  #endif
}


libtea_inline void libtea__cleanup_perf(libtea_instance* instance) {
  #if LIBTEA_LINUX
  if(instance->perf_fd){
      close(instance->perf_fd);
  }
  #endif
  return;
}


#if LIBTEA_WINDOWS
/* Helper function to count set bits in the processor mask. From Windows Dev Center */
libtea_inline int libtea__windows_count_set_bits(ULONG_PTR bitMask){
  int lshift = sizeof(ULONG_PTR)*8 - 1;
  int bitSetCount = 0;
  ULONG_PTR bitTest = (ULONG_PTR)1 << lshift;
  for (int i = 0; i <= lshift; ++i) {
    bitSetCount += ((bitMask & bitTest) ? 1 : 0);
    bitTest /= 2;
  }

  return bitSetCount;
}
#endif


/* cmd must include LIBTEA_SHELL if you want to run a shell command. We assume
 * there is only one line of integer output less than 200 chars long, as in the
 * case of getconf.
 */
libtea_inline int libtea__get_numeric_sys_cmd_output(const char* cmd){
  char output[200];
  int ret_val = -1;
  int sscanf_ret = -1;
  FILE *fp = libtea_popen(cmd, "r");
  if (fp == NULL){
    libtea_info("Error: libtea_popen failed in libtea__get_numeric_sys_cmd_output");
    goto libtea__get_numeric_sys_cmd_output_end;
  }
  if(fgets(output, sizeof(output), fp) == NULL){
    libtea_info("Error: fgets failed in libtea__get_numeric_sys_cmd_output");
    goto libtea__get_numeric_sys_cmd_output_end;
  }
  sscanf(output, "%d", &ret_val);
  libtea__get_numeric_sys_cmd_output_end:
  if(fp) libtea_pclose(fp);
  if(ret_val == -1){
    libtea_info("Error: libtea__get_numeric_sys_cmd_output failed. Failed to execute command: %s.", cmd);
  }
  return ret_val;
}


#if LIBTEA_LINUX
int libtea__load_thread(void* param) {
#else
DWORD WINAPI libtea__load_thread(LPVOID param) {
#endif
  libtea_thread_data* data = (libtea_thread_data*) param;
  if(data->addr != NULL){
    unsigned char* ptr = (unsigned char*) data->addr;
    ptr[0] = data->secret;
    libtea_speculation_barrier();
    while(1){
      libtea_access(ptr);
    }
  }
  else{
    unsigned char addr[10];
    addr[0] = data->secret;
    libtea_speculation_barrier();
    while(1){
      libtea_access(addr);
    }
  }
}


#if LIBTEA_LINUX
int libtea__store_thread(void* param) {
#else
DWORD WINAPI libtea__store_thread(LPVOID param) {
#endif
  libtea_thread_data* data = (libtea_thread_data*) param;
  if(data->addr != NULL){
    unsigned char* ptr = (unsigned char*) data->addr;
    while(1){
      ptr[0] = data->secret;
    }
  }
  else{
    unsigned char addr[10];
    while(1){
      addr[0] = data->secret;
    }
  }
}


#if LIBTEA_LINUX
int libtea__nop_thread(void* param) {
#else
DWORD WINAPI libtea__nop_thread(LPVOID param) {
#endif
  while(1){
    /* Doesn't seem to get optimized out, at least on debug JSShell build on Windows */
    LIBTEA_NOP();
  }
}

#if LIBTEA_LINUX
bool libtea__write_int_to_file(const char* path, int data){
  FILE* file = fopen(path, "w");
  if(!file) return false;
  int retval = fprintf(file, "%i", data);
  fclose(file);
  if(retval == 1) return true;
  else return false;
}


int libtea__read_int_from_file(const char* path){
  int data = -1;
  FILE* file = fopen(path, "r");
  if(!file) return -1;
  int retval = fscanf(file, "%i", &data);
  fclose(file);
  if(retval == 1) return data;
  else return -1;
}


bool libtea__set_minimum_pstate(libtea_instance* instance, int perf_percentage, bool restore){
  const char* minimum_pstate_path = "/sys/devices/system/cpu/intel_pstate/min_perf_pct";
  if(restore) return libtea__write_int_to_file(minimum_pstate_path, instance->last_min_pstate);
  else {
    instance->last_min_pstate = libtea__read_int_from_file(minimum_pstate_path);
    return libtea__write_int_to_file(minimum_pstate_path, perf_percentage);
  }
}


bool libtea__set_maximum_pstate(libtea_instance* instance, int perf_percentage, bool restore){
  const char* maximum_pstate_path = "/sys/devices/system/cpu/intel_pstate/max_perf_pct";
  if(restore) return libtea__write_int_to_file(maximum_pstate_path, instance->last_max_pstate);
  else{
    instance->last_max_pstate = libtea__read_int_from_file(maximum_pstate_path);
    return libtea__write_int_to_file(maximum_pstate_path, perf_percentage);
  }
}


bool libtea__disable_turbo_boost(libtea_instance* instance, bool restore){
  const char* turbo_boost_disable_path = "/sys/devices/system/cpu/intel_pstate/no_turbo";
  if(restore) return libtea__write_int_to_file(turbo_boost_disable_path, instance->last_turbo_boost_setting);
  else{
    instance->last_turbo_boost_setting = libtea__read_int_from_file(turbo_boost_disable_path);
    return libtea__write_int_to_file(turbo_boost_disable_path, 1);
  }
}
#endif


/* Public functions included in API */
//---------------------------------------------------------------------------

libtea_instance* libtea_init(){
  libtea_instance* instance = libtea_init_nokernel();

  #if LIBTEA_LINUX
  instance->module_fd = open(LIBTEA_DEVICE_PATH, O_RDONLY);
  if (instance->module_fd < 0) {
    libtea_info("Could not open Libtea module: %s", LIBTEA_DEVICE_PATH);
    return NULL;
  }
  #if LIBTEA_SUPPORT_PAGING
  instance->mem_fd = open("/dev/mem", O_RDWR);  //This can be mapped PROT_EXEC, libtea_umem can't
  instance->umem_fd = open("/proc/libtea_umem", O_RDWR);
  #endif

  #else
  instance->module_fd = CreateFile(LIBTEA_DEVICE_PATH, GENERIC_ALL, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_SYSTEM, 0);
  if (instance->module_fd == INVALID_HANDLE_VALUE) {
    libtea_info("Could not open Libtea module: %s. Check that you have run LibteaLoader to load the driver and that you are running from an administrator command prompt.", LIBTEA_DEVICE_PATH);
    return NULL;
  }
  /* instance->umem_fd not supported, so leave it set to 0 */
  #endif

  #if LIBTEA_SUPPORT_PAGING
  libtea__paging_init(instance);
  #endif

  #if LIBTEA_SUPPORT_INTERRUPTS
  libtea__interrupts_init();
  #endif

  return instance;
}


libtea_instance* libtea_init_nokernel(){
  libtea_instance* instance = (libtea_instance*)malloc(sizeof(libtea_instance));
  if(!instance) return NULL;
  memset(instance, 0, sizeof(libtea_instance));

  #if LIBTEA_LINUX
  /* TODO currently this approach will provide the wrong number of physical cores if there are
   * multiple CPU packages.
   */
  const char* cmd1 = LIBTEA_SHELL " -c 'cat /sys/devices/system/cpu/cpu*/topology/core_id | wc -l'";
  const char* cmd2 = LIBTEA_SHELL " -c 'cat /sys/devices/system/cpu/cpu*/topology/core_id | uniq | wc -l'";
  instance->logical_cores = libtea__get_numeric_sys_cmd_output(cmd1);
  instance->physical_cores = libtea__get_numeric_sys_cmd_output(cmd2);
  if(instance->physical_cores <= 0 || instance->logical_cores <= 0){
    libtea_info("Error: Libtea could not obtain the number of cores. Is /proc/cpuinfo accessible, and are the grep, uniq, and wc binaries present?");
    return NULL;
  }
  
  #else
  PSYSTEM_LOGICAL_PROCESSOR_INFORMATION infoBuffer = NULL;
  PSYSTEM_LOGICAL_PROCESSOR_INFORMATION infoBufferHandle = NULL;
  DWORD length = 0;

  /* First attempt will fail but writes the size buffer we actually need into length */
  GetLogicalProcessorInformation(infoBuffer, &length); 

  if(GetLastError() != ERROR_INSUFFICIENT_BUFFER){
    libtea_info("Error getting processor information in Libtea initialization - cannot continue, returning NULL instance.");
    free(infoBuffer);
    return NULL;
  }
  infoBuffer = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION) malloc(length);
  bool success = GetLogicalProcessorInformation(infoBuffer, &length); 

  if(!success){
    libtea_info("Error getting processor information in Libtea initialization - cannot continue, returning NULL instance.");
    if(infoBuffer) free(infoBuffer);
    return NULL;
  }

  int physicalCores = 0;
  int logicalCores = 0;
  infoBufferHandle = infoBuffer;

  /* Parsing code adapted from Windows Dev Center */
  for (int i=0; i <= length; i += sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION)) {
    switch (infoBufferHandle->Relationship) {
      
      case RelationProcessorCore:
        physicalCores++;
        logicalCores += libtea__windows_count_set_bits(infoBufferHandle->ProcessorMask);
        break;

      default:
        break;
      }
      
      infoBufferHandle++;
  }

  free(infoBuffer);
  instance->physical_cores = physicalCores;
  instance->logical_cores = logicalCores;
  #endif

  libtea__arch_init_cpu_features(instance);

  /* Use best available timer */
  libtea__saved_sighandler[SIGILL] = signal(SIGILL, libtea__sigill_handler);
  bool done = false;
  int counter = 0;
  while(!done){
    if(counter == 0) libtea__set_timer(instance, LIBTEA_TIMER_NATIVE);
    else if(counter == 1 && (instance->is_intel && instance->logical_cores == 2 * instance->physical_cores)){
      libtea__set_timer(instance, LIBTEA_TIMER_COUNTING_THREAD);
    }
    else if(counter == 2 && (instance->is_intel && instance->logical_cores == 2 * instance->physical_cores)){
      libtea__set_timer(instance, LIBTEA_TIMER_MONOTONIC_CLOCK);
    }
    else if(counter == 1) libtea__set_timer(instance, LIBTEA_TIMER_PERF); 
    else libtea__set_timer(instance, LIBTEA_TIMER_MONOTONIC_CLOCK);

    if(!setjmp(libtea__trycatch_buf)) {
      uint64_t ts1 = libtea_timestamp(instance);
      int i;
      for(i = 0; i < 1000000; i++) {
        LIBTEA_NOP();
      }
      uint64_t ts2 = libtea_timestamp(instance);
      if(ts2 > ts1) done = true;
    }
    counter++;
  }
  signal(SIGILL, libtea__saved_sighandler[SIGILL]);

  #if LIBTEA_SUPPORT_CACHE
  if(libtea_init_cache(instance) != LIBTEA_SUCCESS) return NULL;
  #endif
  
  return instance;
}


void libtea_cleanup(libtea_instance* instance){

  if(instance != NULL){
    #if LIBTEA_SUPPORT_CACHE
    libtea_cleanup_cache(instance);
    #endif

    #if LIBTEA_SUPPORT_PAGING
    libtea__cleanup_paging(instance);
    #endif

    #if LIBTEA_SUPPORT_INTERRUPTS
    /* Ensure local APIC timer is restored on process exit */
    if (libtea_apic_lvtt){
      libtea_apic_timer_deadline(instance);
    }
    #endif

    #if LIBTEA_LINUX
    if(instance->module_fd > 0) close(instance->module_fd);
    libtea__cleanup_perf(instance);
    #else
    if(instance->module_fd != NULL) CloseHandle(instance->module_fd);
    #endif

    instance = NULL;
  }

};


/**
 * Accesses the given address speculatively. Success will vary depending on the microarchitecture
 * used (exact branch prediction implementation, ROB size etc).
 *
 * :param addr: Virtual address to access
 */
libtea_inline void libtea_access_speculative(void* addr){
  
  /* Pointer chasing to extend the transient window */
  long long unsigned int condition = 1;
  void* chase_me[9] = {0};
  chase_me[8] = &chase_me[7];
  chase_me[7] = &chase_me[6];
  chase_me[6] = &chase_me[5];
  chase_me[5] = &chase_me[4];
  chase_me[4] = &chase_me[3];
  chase_me[3] = &chase_me[2];
  chase_me[2] = &chase_me[1];
  chase_me[1] = &chase_me[0];
  chase_me[0] = &condition;
  #define libtea_pointer_chaser *((uintptr_t*) ********(uintptr_t********)chase_me[8])

  /* Optimum number of flushes varies, but don't want to put too much pressure on the cache hierarchy */
  libtea_flush(&chase_me[8]);

  /* Stall long enough for the above flushes to take effect */
  for(volatile int z = 0; z < 100; z++){ }

  if(libtea_pointer_chaser){
    libtea_access(addr);
  }

}


libtea_inline uint64_t libtea_timestamp(libtea_instance* instance) {
  return instance->timer(instance);
}


libtea_inline void libtea_measure_start(libtea_instance* instance) {
  instance->measure_start = instance->timer(instance);
}


libtea_inline uint64_t libtea_measure_end(libtea_instance* instance) {
  return instance->timer(instance) - instance->measure_start;
}


libtea_inline static void libtea_set_timer(libtea_instance* instance, libtea_timer timer) {
  libtea__set_timer(instance, timer);
}    


libtea_inline int libtea_get_hyperthread(int logical_core) {

  #if LIBTEA_LINUX
  char cpu_id_path[300];
  char buffer[16];
  snprintf(cpu_id_path, 300, "/sys/devices/system/cpu/cpu%d/topology/core_id", logical_core);

  FILE* f = fopen(cpu_id_path, "r");
  volatile int dummy = fread(buffer, 16, 1, f);
  fclose(f);
  int phys = atoi(buffer);
  int hyper = LIBTEA_ERROR;

  DIR* dir = opendir("/sys/devices/system/cpu/");
  struct dirent* entry;
  while((entry = readdir(dir)) != NULL) {
    if(entry->d_name[0] == 'c' && entry->d_name[1] == 'p' && entry->d_name[2] == 'u' && (entry->d_name[3] >= '0' && entry->d_name[3] <= '9')) {
      
      /* Check core is actually online */
      snprintf(cpu_id_path, 300, "/sys/devices/system/cpu/%s/online", entry->d_name);
      f = fopen(cpu_id_path, "r");
      /* Do continue to core_id check if it's NULL as sometimes this file does not exist, even though the CPU *is* online */
      if(f != NULL){
        dummy += fread(buffer, 16, 1, f);
        fclose(f);
        if(atoi(buffer) == 0) continue;
      }
      
      snprintf(cpu_id_path, 300, "/sys/devices/system/cpu/%s/topology/core_id", entry->d_name);
      f = fopen(cpu_id_path, "r");
      if(f != NULL){
        dummy += fread(buffer, 16, 1, f);
        fclose(f);
        int logical = atoi(entry->d_name + 3);
        if(atoi(buffer) == phys && logical != logical_core) {
          hyper = logical;
          break;
        }
      }
    }
  }
  closedir(dir);
  return hyper;
  
  #else
  NO_WINDOWS_SUPPORT;
  return 0;
  #endif
}


/* Can take a thread or process pid_t on Linux, Windows needs a separate internal function for threads */
libtea_inline void libtea_pin_to_core(libtea_thread process, int core) {
  #if LIBTEA_LINUX
  cpu_set_t mask;
  mask.__bits[0] = 1 << core;
  sched_setaffinity(process, sizeof(cpu_set_t), &mask);

  #else
  DWORD_PTR newAffinityMask = 0;
  newAffinityMask |= (1 << core);
  bool set_success = SetProcessAffinityMask(process, newAffinityMask);
  if(!set_success){
    libtea_info("Error: failed to set process affinity mask in libtea_pin_to_core.");
  }
  #endif
}

#if LIBTEA_WINDOWS
libtea_inline void libtea__pin_thread_to_core(libtea_thread thread, int core) {
  DWORD newAffinityMask = 0;
  newAffinityMask |= (1 << core);
  DWORD_PTR oldAffinityMask = SetThreadAffinityMask(thread, newAffinityMask);
  if(oldAffinityMask == 0){  //This does not need dereferencing because DWORD_PTR is *not* a pointer
    int error = GetLastError();
    libtea_info("Error: failed to set thread affinity mask in libtea__pin_thread_to_core, last error is %d.", error);
  }
}
#endif


libtea_inline size_t libtea_get_physical_address(libtea_instance* instance, size_t vaddr) {

  #if LIBTEA_LINUX
  int fd = open("/proc/self/pagemap", O_RDONLY);
  uint64_t virtual_addr = (uint64_t)vaddr;
  size_t value = 0;
  //TODO assuming 4KB pagesize - could use instance->pagesize but we only initialize it in paging init
  off_t offset = (virtual_addr / 4096) * sizeof(value);
  int got = pread(fd, &value, sizeof(value), offset);
  if (got != 8) {
     libtea_info("Error: pread failed (return value %d), could not read 8-byte physical address", got);
     return LIBTEA_ERROR;
  }
  close(fd);
  return (value << 12) | ((size_t)vaddr & 0xFFFULL);

  #else
  size_t val = 0;
  ULONG returnLength;
  DeviceIoControl(instance->module_fd, LIBTEA_IOCTL_GET_PHYS_ADDR, (LPVOID)&vaddr, sizeof(vaddr), (LPVOID)&val, sizeof(val), &returnLength, 0);
  return val;
  #endif

}


libtea_inline size_t libtea_get_kernel_physical_address(libtea_instance* instance, size_t vaddr) {

  #if LIBTEA_LINUX
  /* Use this function to get the physical address of a kernel-space virtual address. Kernel virtual->physical address conversion is simple arithmetic,
   * but we need some kernel specific variables, so we call the Libtea driver to invoke the kernel's virt_to_phys function.
   */
  void* addr = (void*) vaddr;
  ioctl(instance->module_fd, LIBTEA_IOCTL_GET_KERNEL_PHYS_ADDR, &addr);
  return (size_t) addr;

  #else
  NO_WINDOWS_SUPPORT;
  return LIBTEA_ERROR;
  #endif

}


/* Summary of how Libtea handles files and mapping/unmapping
 * ===========================================================
 * tl;dr: Windows adds an extra handle to keep track of with every mapping, which is irritating
 * as we try to create the illusion here that it works identically on Linux and Windows.
 * 
 * HANDLE libtea_open_shared_memory(size_t size, libtea_file_ptr windowsMapping)
 * Not file-backed.
 * Linux - mmap.
 * Windows - file mapping returned in windowsMapping, file view returned as ret arg

 * int libtea_close_shared_memory(HANDLE mem, libtea_file_ptr windowsMapping, size_t size);
 * Linux - munmap mem.
 * Windows - unmap mem, close windowsMapping.

 * void* libtea_map_file_by_offset(const char* filename, size_t* filesize, libtea_file_ptr fileHandle, int rw, size_t offset);
 * File-backed.
 * Linux - open file, mmap. File descriptor returned in fileHandle, mapping returned as ret arg.
 * Windows - not supported

 * void* libtea_map_file(const char* filename, size_t* filesize, libtea_file_ptr fileHandle, libtea_file_ptr windowsMapping, int rw);
 * File-backed.
 * Linux - open file, mmap. File descriptor returned in fileHandle, mapping returned as ret arg.
 * Windows - CreateFileA, CreateFileMappingA, MapViewOfFile. File descriptor returned as fileHandle, file mapping returned as windowsMapping, view returned as ret arg.

 * void* libtea_mmap(int buffer_size, libtea_file_ptr windowsMapping, int rw);
 * Not file-backed.
 * Linux - mmap, return mapping.
 * Windows - CreateFileMappingA, MapViewOfFile. File mapping returned in windowsMapping, view returned as ret arg.

 * int libtea_munmap_file(void* ptr, int buffer_size, libtea_file_ptr fileHandle, libtea_file_ptr windowsMapping);
 * File-backed.
 * Linux - munmap, close fileHandle.
 * Windows - UnmapViewOfFile, CloseHandle.

 * int libtea_munmap(void* ptr, int buffer_size, libtea_file_ptr windowsMapping);
 * Not file-backed.
 * Linux - munmap.
 * Windows - UnmapViewOfFile, CloseHandle if windowsMapping is provided.
*/


libtea_inline HANDLE libtea_open_shared_memory(size_t size, libtea_file_ptr windowsMapping){
  
  HANDLE mem;

  #if LIBTEA_LINUX
  mem = mmap(0, size, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS|MAP_POPULATE, -1, 0);
  if (mem == MAP_FAILED){
    return NULL;
  }

  #else
  *windowsMapping = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE | SEC_COMMIT, 0, size, "LIBTEALEAK");
  /* NULL returned if mapping failed, so we will just return NULL as specified in API */
  int err = GetLastError();
  if(err == ERROR_ALREADY_EXISTS){
    libtea_info("Error: can't create shared memory in libtea_open_shared_memory, region already mapped!");
    return NULL;
  }
  mem = MapViewOfFile(*windowsMapping, FILE_MAP_ALL_ACCESS, 0, 0, size);
  if(mem == NULL) {
    libtea_info("Error in libtea_open_shared_memory: MapViewOfFile failed.");
    CloseHandle(*windowsMapping);
    return NULL;
  }
  #endif 

  return mem;
}


libtea_inline int libtea_close_shared_memory(HANDLE mem, libtea_file_ptr windowsMapping, size_t size){
  
  #if LIBTEA_LINUX
  if(munmap(mem, size) < 0) {
    return LIBTEA_ERROR;
  }

  #else
  if( !UnmapViewOfFile(mem) ){
    return LIBTEA_ERROR;
  }
  if( !CloseHandle(*windowsMapping) ){
    return LIBTEA_ERROR;
  }
  #endif 

  return LIBTEA_SUCCESS;
}


libtea_thread libtea_start_leaky_thread(libtea_instance* instance, int type, unsigned char secret, HANDLE shared, int core){

  instance->leaky_thread_data.secret = secret;
  instance->leaky_thread_data.addr = shared;
  instance->leaky_thread_data.core = core;

  if(type==0) libtea__thread_create(&instance->leaky_thread, NULL, libtea__load_thread, &(instance->leaky_thread_data));
  else if(type==1) libtea__thread_create(&instance->leaky_thread, NULL, libtea__store_thread, &(instance->leaky_thread_data));
  else libtea__thread_create(&instance->leaky_thread, NULL, libtea__nop_thread, &(instance->leaky_thread_data));

  #if LIBTEA_LINUX
  libtea_pin_to_core(instance->leaky_thread, core);
  #else
  libtea__pin_thread_to_core(instance->leaky_thread, core);
  #endif

  return instance->leaky_thread;
}


void libtea_stop_leaky_thread(libtea_instance* instance){
  libtea__thread_cancel(instance->leaky_thread, NULL);
}


libtea_inline void* libtea_map_file_by_offset(const char* filename, size_t* filesize, libtea_file_ptr fileHandle, int rw, size_t offset) {

  #if LIBTEA_LINUX
  int prot1 = O_RDONLY;
  int prot2 = PROT_READ;
  if(rw == 1){
    prot1 = O_WRONLY;
    prot2 = PROT_WRITE;
  }
  else if(rw == 2){
    prot1 = O_RDWR;
    prot2 = PROT_READ | PROT_WRITE;
  }

  *fileHandle = open(filename, prot1);
  if (*fileHandle < 0) {
    return NULL;
  }
  void* mapping = mmap(0, 4096, prot2, MAP_SHARED, *fileHandle, offset & ~(0xFFF));
  if (mapping == MAP_FAILED) {
    close(*fileHandle);
    return NULL;
  }
  return (char*) mapping + (offset & 0xFFF);
  
  #else
  NO_WINDOWS_SUPPORT;
  return NULL;
  #endif
}


libtea_inline void* libtea_map_file(const char* filename, size_t* filesize, libtea_file_ptr fileHandle, libtea_file_ptr windowsMapping, int rw) {

  #if LIBTEA_LINUX
  int prot1 = O_RDONLY;
  int prot2 = PROT_READ;
  if(rw == 1){
    prot1 = O_WRONLY;
    prot2 = PROT_WRITE;
  }
  else if(rw == 2){
    prot1 = O_RDWR;
    prot2 = PROT_READ | PROT_WRITE;
  }

  *fileHandle = open(filename, prot1);
  if (*fileHandle < 0) {
    libtea_info("Error in libtea_map_file: open failed. Check the filename is correct.");
    return NULL;
  }
  struct stat filestat;
  if (fstat(*fileHandle, &filestat) == -1) {
    libtea_info("Error in libtea_map_file: fstat failed.");
    close(*fileHandle);
    return NULL;
  }
  void* mapping = mmap(0, filestat.st_size, prot2, MAP_SHARED, *fileHandle, 0);
  if (mapping == MAP_FAILED) {
    libtea_info("Error in libtea_map_file: mmap failed.");
    close(*fileHandle);
    return NULL;
  }
  if (filesize != NULL) {
    *filesize = filestat.st_size;
  }
  return mapping;
  
  #else
  //TODO change security attributes - this handle cannot be inherited by child processes
  
  int prot = FILE_MAP_READ;
  
  if(rw == 1){
    prot = FILE_MAP_WRITE;    /* This is actually read/write access, unfortunately we can't have write-only access on Windows */
  }
  else if(rw == 2){
    prot = FILE_MAP_ALL_ACCESS;
  }

  *fileHandle = CreateFileA(filename, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  if(*fileHandle == INVALID_HANDLE_VALUE) {
    libtea_info("Error in libtea_map_file: CreateFileA failed. Check the filename is correct. Last error is %lu", GetLastError());
    return NULL;
  }
  LARGE_INTEGER li_size;
  if( !GetFileSizeEx(*fileHandle, &li_size) ) {
    libtea_info("Error in libtea_map_file: GetFileSizeEx failed. fileHandle is %p, last error is %lu", *fileHandle, GetLastError());
    CloseHandle(*fileHandle);
    return NULL;
  }
  DWORD size = (DWORD)(li_size.QuadPart);
  *windowsMapping = CreateFileMappingA(*fileHandle, NULL, PAGE_READWRITE | SEC_COMMIT, 0, size, NULL);
  if(*windowsMapping == NULL) {
    libtea_info("Error in libtea_map_file: CreateFileMappingA failed. Last error is %lu", GetLastError());
    CloseHandle(*fileHandle);
    return NULL;
  }
  void* mapping = MapViewOfFile(*windowsMapping, prot, 0, 0, size);
  if(mapping == NULL) {
    libtea_info("Error in libtea_map_file: MapViewOfFile failed. Last error is %lu", GetLastError());
    CloseHandle(*windowsMapping);
    CloseHandle(*fileHandle);
    return NULL;
  }
  *filesize = (size_t)(size);  /* Warning: assuming 64-bit size_t...as we do elsewhere */
  return mapping;
  #endif

}


libtea_inline void* libtea_mmap(int buffer_size, libtea_file_ptr windowsMapping, int rw) {

  void* ptr;
  
  #if LIBTEA_LINUX
  int prot = PROT_READ;
  if(rw == 1){
    prot = PROT_WRITE;
  }
  else if(rw == 2){
    prot = PROT_READ | PROT_WRITE;
  }
  ptr = mmap(0, buffer_size, prot, MAP_PRIVATE|MAP_ANONYMOUS|MAP_POPULATE, -1, 0);
  if(ptr == MAP_FAILED) {
    return NULL;
  }

  #else
  int prot = FILE_MAP_READ;
  
  if(rw == 1){
    prot = FILE_MAP_WRITE;    /* This is actually read/write access, unfortunately we can't have write-only access on Windows */
  }
  else if(rw == 2){
    prot = FILE_MAP_ALL_ACCESS;
  }

  *windowsMapping = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE | SEC_COMMIT, 0, buffer_size, NULL);
  if(*windowsMapping == NULL) return NULL;
  ptr = MapViewOfFile(*windowsMapping, prot, 0, 0, buffer_size);
  if(ptr == NULL) {
    CloseHandle(*windowsMapping);
    return NULL;
  }
  #endif

  return ptr;
}


libtea_inline int libtea_munmap_file(void* ptr, int buffer_size, libtea_file_ptr fileHandle, libtea_file_ptr windowsMapping) {
  int ret = LIBTEA_SUCCESS;

  #if LIBTEA_LINUX
  if(munmap(ptr, buffer_size) < 0){
    ret = LIBTEA_ERROR;
  }
  if(close(*fileHandle) < 0){
    ret = LIBTEA_ERROR;
  }

  #else
  if( !UnmapViewOfFile(ptr) ){
    ret = LIBTEA_ERROR;
  }
  if( !CloseHandle(*windowsMapping) ){
    ret = LIBTEA_ERROR;
  }
  if( !CloseHandle(*fileHandle) ){
    ret = LIBTEA_ERROR;
  }
  #endif

  return ret;
}


libtea_inline int libtea_munmap(void* ptr, int buffer_size, libtea_file_ptr windowsMapping) {
  int ret = LIBTEA_SUCCESS;

  #if LIBTEA_LINUX
  if(munmap(ptr, buffer_size) < 0){
    ret = LIBTEA_ERROR;
  }

  #else
  if( !UnmapViewOfFile(ptr) ){
    ret = LIBTEA_ERROR;
  }
  if(!CloseHandle(*windowsMapping)){
    ret = LIBTEA_ERROR;
  }
  #endif

  return ret;
}



libtea_inline int libtea_find_index_of_nth_largest_int(int* list, size_t nmemb, size_t n) {
  int* sorted = (int*) malloc(sizeof(int)*nmemb);
  size_t* idx = (size_t*) malloc(sizeof(size_t)*nmemb);
  size_t i, j;
  int tmp;
  memset(sorted, 0, nmemb);
  for(i = 0; i < nmemb; i++) {
    sorted[i] = list[i];
    idx[i] = i;
  }
  for(i = 0; i < nmemb; i++) {
    int swaps = 0;
    for(j = 0; j < nmemb - 1; j++) {
      if(sorted[j] < sorted[j + 1]) {
        tmp = sorted[j];
        sorted[j] = sorted[j + 1];
        sorted[j + 1] = tmp;
        tmp = idx[j];
        idx[j] = idx[j + 1];
        idx[j + 1] = tmp;
        swaps++;
      }
    }
    if(!swaps) break;
  }
  int ret_val = idx[n];
  free(sorted);
  free(idx);
  return ret_val;
}


libtea_inline int libtea_find_index_of_nth_largest_sizet(size_t* list, size_t nmemb, size_t n) {
  int* sorted = (int*) malloc(sizeof(int)*nmemb);
  size_t* idx = (size_t*) malloc(sizeof(size_t)*nmemb);
  size_t i, j;
  size_t tmp;
  memset(sorted, 0, nmemb);
  for(i = 0; i < nmemb; i++) {
    sorted[i] = list[i];
    idx[i] = i;
  }
  for(i = 0; i < nmemb; i++) {
    int swaps = 0;
    for(j = 0; j < nmemb - 1; j++) {
      if(sorted[j] < sorted[j + 1]) {
        tmp = sorted[j];
        sorted[j] = sorted[j + 1];
        sorted[j + 1] = tmp;
        tmp = idx[j];
        idx[j] = idx[j + 1];
        idx[j + 1] = tmp;
        swaps++;
      }
    }
    if(!swaps) break;
  }
  int ret_val = idx[n];
  free(sorted);
  free(idx);
  return ret_val;
}


libtea_inline int libtea_write_system_reg(libtea_instance* instance, int cpu, uint32_t reg, uint64_t val){
  return libtea__arch_write_system_reg(instance, cpu, reg, val);
}


libtea_inline size_t libtea_read_system_reg(libtea_instance* instance, int cpu, uint32_t reg){
  return libtea__arch_read_system_reg(instance, cpu, reg);
}


libtea_inline void libtea_disable_hardware_prefetchers(libtea_instance* instance){
  libtea__arch_disable_hardware_prefetchers(instance);
}


libtea_inline void libtea_enable_hardware_prefetchers(libtea_instance* instance){
  libtea__arch_enable_hardware_prefetchers(instance);
}


/* Adapted from code at https://docs.microsoft.com/en-us/windows/win32/psapi/enumerating-all-processes */
#if LIBTEA_ENABLE_WINDOWS_CORE_ISOLATION
libtea_inline int libtea_isolate_windows_core(int core){
  #if LIBTEA_WINDOWS
  DWORD processes[4096] = {0};
  DWORD bytesReturned = 0;
  int numProcesses = 0;
  int numIsolated = 0;
  if (!EnumProcesses(processes, sizeof(processes), &bytesReturned)) return LIBTEA_ERROR;
  numProcesses = bytesReturned / sizeof(DWORD);
  if(numProcesses > 4096){
    numProcesses = 4096;
    libtea_info("Max supported number of processes (4096) exceeded. Will only attempt to isolate first 4096 processes.");
  }
  for (int i = 0; i < numProcesses; i++){
    if(processes[i] != 0){
      HANDLE process = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_SET_INFORMATION, FALSE, processes[i]);
      if(!process){
        /* Access is denied for system processes. Seems there's nothing we can do about this, even as admin or with
         * PROCESS_ALL_ACCESS privilege.
         */
        continue;
      }
      DWORD_PTR processAffinityMask;
      DWORD_PTR systemAffinityMask;
      bool get_success = GetProcessAffinityMask(process, &processAffinityMask, &systemAffinityMask);
      if(get_success){
        processAffinityMask &= ~(1 << core);
        bool set_success = SetProcessAffinityMask(process, processAffinityMask);
        if(set_success) numIsolated++;
      }
      CloseHandle(process);
    }
  }
  libtea_info("Managed to isolate %d processes out of %d processes total.", numIsolated, numProcesses);
  return LIBTEA_SUCCESS;
  
  #else
  libtea_info("Error: isolate_windows_core is only supported on Windows.");
  return LIBTEA_ERROR;
  #endif
}
#endif


libtea_inline int libtea_set_cpu_pstate(libtea_instance* instance, int perf_percentage){
  #if LIBTEA_LINUX
  if(libtea__set_minimum_pstate(instance, perf_percentage, false) && libtea__set_maximum_pstate(instance, perf_percentage, false) && libtea__disable_turbo_boost(instance, false)) return LIBTEA_SUCCESS;
  else return LIBTEA_ERROR;
  #else
  return LIBTEA_ERROR;
  #endif
}


libtea_inline int libtea_restore_cpu_pstate(libtea_instance* instance){
  #if LIBTEA_LINUX
  if(libtea__set_minimum_pstate(instance, 0, true) && libtea__set_maximum_pstate(instance, 0, true) && libtea__disable_turbo_boost(instance, true)) return LIBTEA_SUCCESS;
  else return LIBTEA_ERROR;
  #else
  return LIBTEA_ERROR;
  #endif
}


/* End libtea_common.c */
//---------------------------------------------------------------------------

/* See LICENSE file for license and copyright information */

/* Start libtea_x86_common.c */
//---------------------------------------------------------------------------

#if LIBTEA_X86


#include <inttypes.h>

#if LIBTEA_LINUX
#include <cpuid.h>
#include <sys/utsname.h>
#endif

#define RDPRU ".byte 0x0f, 0x01, 0xfd"

#if LIBTEA_LINUX
int libtea__arch_counting_thread(void* arg) {
#else
DWORD WINAPI libtea__arch_counting_thread(LPVOID arg) {
#endif

  #if LIBTEA_INLINEASM
  /* Note: libtea threads cannot use libc functions! */
  volatile size_t* counter = (volatile size_t*)arg;
  asm volatile("1: inc %%rax\n"
                "mov %%rax, (%%rcx)\n"
                "jmp 1b" : : "c"(counter), "a"(0));

  #else
  libtea__windows_counting_thread(arg);
  #endif

  return 0;
}


libtea_inline uint64_t libtea__arch_timestamp_native() {

  #if LIBTEA_INLINEASM
  uint64_t a, d;
  asm volatile("mfence");
  #if LIBTEA_RDTSCP
  asm volatile("rdtscp" : "=a"(a), "=d"(d) :: "rcx");
  #else
  asm volatile("rdtsc" : "=a"(a), "=d"(d));
  #endif
  a = (d << 32) | a;
  asm volatile("mfence");
  return a;

  #else
  unsigned int tsc_aux;
  _mm_mfence();
  #if LIBTEA_RDTSCP
  uint64_t time = __rdtscp(&tsc_aux);
  #else
  uint64_t time = __rdtsc();
  #endif
  _mm_mfence();
  return time;
  #endif

}


libtea_inline uint64_t libtea__arch_timestamp_native_amd_zen() {
  /* TODO - read from kernel driver */
  uint64_t dummy = 0;
  return dummy;
}


libtea_inline uint64_t libtea__arch_timestamp_native_amd_zen2() {
  uint64_t low = 0;
  uint64_t high = 0;
  
  #if LIBTEA_INLINEASM
  asm volatile("mfence");
  asm volatile(RDPRU
			     : "=a" (low), "=d" (high)
			     : "c" (1));
  asm volatile("mfence");
  
  #else
  //TODO libtea__windows_rdpru(low, high);
  high = 0;
  #endif

  high = (high << 32) | low;
  return high;
}


libtea_inline uint64_t libtea__arch_timestamp_monotonic() {

  #if LIBTEA_LINUX
  asm volatile("mfence");
  struct timespec t1;
  clock_gettime(CLOCK_MONOTONIC, &t1);
  uint64_t res = t1.tv_sec * 1000 * 1000 * 1000ULL + t1.tv_nsec;
  asm volatile("mfence");
  return res;

  #else
  //TODO replace - now using QPC in perf timer instead
  LARGE_INTEGER time;
  QueryPerformanceCounter(&time);
  return (uint64_t) time.QuadPart;
  #endif

}


void libtea__arch_init_cpu_features(libtea_instance* instance){

    /* CPU manufacturer ID string (12 ASCII chars) is returned in EBX, EDX,
     * ECX (in that order, which is why we need to reorder the array in
     * 'name'). The largest value that EAX can be set to before calling
     * cpuid (which can be used to identify Intel microarchitectures) is
     * returned in EAX.
     */
    #if LIBTEA_LINUX
    uint32_t name[4] = {0, 0, 0, 0};
    __cpuid(0, instance->cpu_architecture, name[0], name[2], name[1]);
    if(strcmp((char *) name, "GenuineIntel") == 0) {
      instance->is_intel = 1;
      /* Check for Intel TSX */
      if (__get_cpuid_max(0, NULL) >= 7) {
        uint32_t a, b, c, d;
        __cpuid_count(7, 0, a, b, c, d);
        instance->has_tm = (b & (1 << 11)) ? 1 : 0;
      }
    }
    else instance->is_intel = 0;

    #else
    int temp[4] = {0, 0, 0, 0};
    __cpuid(temp, 0);
    int name[4] = {0, 0, 0, 0};
    name[0] = temp[1];
    name[1] = temp[3];
    name[2] = temp[2];
    instance->cpu_architecture = temp[0]; 
    if(strcmp((char *) name, "GenuineIntel") == 0) {
      instance->is_intel = 1;
      if (temp[0] >= 7) {
        /* Check for Intel TSX: EAX=7, ECX=0; returned value in EBX */
        __cpuidex(temp, 7, 0);
        instance->has_tm = (temp[1] & (1 << 11)) ? 1 : 0;
      }
    }
    else{
      instance->is_intel = 0;
      instance->has_tm = 0;
    }
    #endif
}


libtea_inline int libtea__arch_transaction_begin(){

  #if LIBTEA_INLINEASM
  int ret = (~0u);
  asm volatile(".byte 0xc7,0xf8 ; .long 0" : "+a" (ret) :: "memory");
  return ret == (~0u);

  #else
  return _xbegin();
  #endif

}


libtea_inline void libtea__arch_transaction_end(){
  #if LIBTEA_INLINEASM
  asm volatile(".byte 0x0f; .byte 0x01; .byte 0xd5" ::: "memory");  /* TSX xend */
  #else
  _xend();
  #endif
}


libtea_inline void libtea__arch_transaction_abort(){
  #if LIBTEA_INLINEASM
  asm volatile(".byte 0xc6; .byte 0xf8; .byte 0x00" ::: "memory");  /* TSX xabort(0) */
  #else
  _xabort(0);
  #endif
}


libtea_inline void libtea__arch_access(void* addr) {

  #if LIBTEA_INLINEASM
  asm volatile("movq (%0), %%rax\n" : : "r"(addr) : "rax");

  #else
  volatile char* access = (char*) addr;
  volatile char dummy = access[0];
  #endif

}


libtea_inline void libtea__arch_access_b(void* addr) {

  #if LIBTEA_INLINEASM
  asm volatile("mfence");
  asm volatile("movq (%0), %%rax\n" : : "r"(addr) : "rax");
  asm volatile("mfence");

  #else
  _mm_mfence();
  volatile char* access = (char*) addr;
  volatile char dummy = access[0];
  _mm_mfence();
  #endif

}


libtea_inline void libtea__arch_prefetch(void* addr){

  #if LIBTEA_INLINEASM
  asm volatile ("prefetcht0 (%0)" : : "r" (addr));
  /* Options:
   * prefetcht0 (temporal data)—prefetch data into all levels of the cache hierarchy.
   * prefetcht1 (temporal data with respect to first level cache misses)—prefetch data into level 2 cache and higher.
   * prefetcht2 (temporal data with respect to second level cache misses)—prefetch data into level 3 cache and higher, or an implementation-specific choice.
   * prefetchnta (non-temporal data with respect to all cache levels)—prefetch data into non-temporal cache structure and into a location close to the processor, minimizing cache pollution.
   */
  #else
  _m_prefetch(addr);
  #endif

}


libtea_inline void libtea__arch_prefetchw(void* addr){
  
  #if LIBTEA_INLINEASM
  asm volatile ("prefetchw (%0)" : : "r" (addr));
  #else
  _m_prefetchw(addr);
  #endif

}


libtea_inline void libtea__arch_flush(void* addr) {

  #if LIBTEA_INLINEASM
  asm volatile("clflush 0(%0)\n" : : "c"(addr) : "rax");
  
  #else
  _mm_clflush(addr);
  #endif

}


libtea_inline void libtea__arch_flush_b(void* addr) {

  #if LIBTEA_INLINEASM
  asm volatile("mfence");
  asm volatile("clflush 0(%0)\n" : : "r"(addr) : "rax");
  asm volatile("mfence");
  
  #else
  _mm_mfence();
  _mm_clflush(addr);
  _mm_mfence();
  #endif

}


libtea_inline void libtea__arch_barrier_start() {

  #if LIBTEA_INLINEASM
  asm volatile("mfence");
  
  #else
  _mm_mfence();
  #endif

}


libtea_inline void libtea__arch_barrier_end() {
  
  #if LIBTEA_INLINEASM
  asm volatile("mfence");
  
  #else
  _mm_mfence();
  #endif

}


libtea_inline void libtea__arch_speculation_barrier() {
  /* Even though lfence is not fully serializing, we use it as a 
   * compromise due to the variable latency and weak uop ordering
   * guarantees of cpuid. See 'nanoBench: A Low-Overhead
   * Tool for Running Microbenchmarks on x86 Systems' for discussion.
   */

  #if LIBTEA_INLINEASM
  asm volatile("lfence");

  #else
  _mm_lfence();
  #endif

}


#if LIBTEA_INLINEASM
#define libtea__arch_speculation_start(label) asm goto ("call %l0" : : : "memory" : label##_retp);
#define libtea__arch_speculation_end(label) asm goto("jmp %l0" : : : "memory" : label); label##_retp: asm goto("lea %l0(%%rip), %%rax; movq %%rax, (%%rsp); ret" : : : "memory","rax" : label); label: asm volatile("nop");
#endif


int libtea__arch_write_system_reg(libtea_instance* instance, int cpu, uint32_t reg, uint64_t val) {

  #if LIBTEA_LINUX
  char msr_file_name[64];
  sprintf(msr_file_name, "/dev/cpu/%d/msr", cpu);
  
  int fd = open(msr_file_name, O_WRONLY);
  if(fd < 0) {
    goto libtea_write_system_reg_driver;
  }
  
  if(pwrite(fd, &val, sizeof(val), reg) != sizeof(val)) {
      close(fd);
      goto libtea_write_system_reg_driver;
  }

  close(fd);
  return LIBTEA_SUCCESS;
  #endif

  #if LIBTEA_LINUX
  libtea_write_system_reg_driver:
    if(instance->module_fd <= 0){
    #else
    if(instance->module_fd == NULL){
    #endif
      libtea_info("Either the msr driver or the Libtea driver must be loaded to read and write system registers.");
      return LIBTEA_ERROR;
    }
    
    #if LIBTEA_LINUX 
    ioctl(instance->module_fd, LIBTEA_IOCTL_SET_SYSTEM_REG, cpu, reg, val);
    
    #else
    DWORD returnLength;
    libtea_system_reg msr_info;
    msr_info.cpu = cpu;
    msr_info.reg = reg;
    msr_info.val = val;
    DeviceIoControl(instance->module_fd, LIBTEA_IOCTL_SET_SYSTEM_REG, (LPVOID)&msr_info, sizeof(libtea_system_reg), (LPVOID)&msr_info, sizeof(libtea_system_reg), &returnLength, 0);
    #endif

    return LIBTEA_SUCCESS;
}


size_t libtea__arch_read_system_reg(libtea_instance* instance, int cpu, uint32_t reg) {

  #if LIBTEA_LINUX
  size_t data = 0;
  char msr_file_name[64];
  sprintf(msr_file_name, "/dev/cpu/%d/msr", cpu);
  
  int fd = open(msr_file_name, O_RDONLY);
  if(fd < 0) {
    goto libtea_read_system_reg_driver;
  }
  
  if(pread(fd, &data, sizeof(data), reg) != sizeof(data)) {
      close(fd);
      goto libtea_read_system_reg_driver;
  }
  close(fd);
  return data;
  #endif

  #if LIBTEA_LINUX
  libtea_read_system_reg_driver:
    if(instance->module_fd <= 0){
    #else 
    if(instance->module_fd == NULL){
    #endif
      libtea_info("Either the msr driver or the libtea driver must be loaded to read and write system registers.");
      return LIBTEA_ERROR;
    }
    
    #if LIBTEA_LINUX 
    libtea_system_reg msr_info;
    msr_info.cpu = cpu;
    msr_info.reg = reg;
    msr_info.val = 0;
    ioctl(instance->module_fd, LIBTEA_IOCTL_GET_SYSTEM_REG, &msr_info);
    return msr_info.val;
    
    #else
    DWORD returnLength;
    libtea_system_reg msr_info;
    msr_info.cpu = cpu;
    msr_info.reg = reg;
    msr_info.val = 0;
    DeviceIoControl(instance->module_fd, LIBTEA_IOCTL_GET_SYSTEM_REG, (LPVOID)&msr_info, sizeof(libtea_system_reg), (LPVOID)&msr_info, sizeof(libtea_system_reg), &returnLength, 0);
    return msr_info.val;
    #endif

}


void libtea__arch_disable_hardware_prefetchers(libtea_instance* instance) {
  if(instance->is_intel) {
    for(int i = 0; i < instance->logical_cores; i++) {
        libtea_write_system_reg(instance, i, 0x1a4, 0xf);
    }
  }
  else libtea_info("Disabling prefetchers is only implemented for Intel CPUs.");
  /* MSRs to control this are undocumented on AMD Zen, unsure on Via */
}


void libtea__arch_enable_hardware_prefetchers(libtea_instance* instance) {
  if(instance->is_intel) {
    for(int i = 0; i < instance->logical_cores; i++) {
        libtea_write_system_reg(instance, i, 0x1a4, 0x0);
    }
  }
  else libtea_info("Disabling prefetchers is only implemented for Intel CPUs.");
  /* MSRs to control this are undocumented on AMD Zen, unsure on Via */
}

#endif //LIBTEA_X86


/* End libtea_x86_common.c */
//---------------------------------------------------------------------------

/* See LICENSE file for license and copyright information */

/* Start libtea_cache.c */
//---------------------------------------------------------------------------



/* Internal functions not included in API */
//---------------------------------------------------------------------------
libtea_inline void libtea__init_cache_info(libtea_instance* instance);
libtea_inline int libtea__log_2(const uint32_t x);
libtea_inline bool libtea__eviction_init(libtea_instance* instance);
libtea_inline void libtea__eviction_cleanup(libtea_instance* instance);
libtea_inline size_t libtea__eviction_get_lookup_index(libtea_instance* instance, size_t paddr);
libtea_inline bool find_congruent_addresses(libtea_instance* instance, size_t index, size_t paddr, size_t number_of_addresses);
libtea_inline static void libtea__cleanup_all(void);
//---------------------------------------------------------------------------

libtea_inline void libtea__init_cache_info(libtea_instance* instance) {

  /* If the arch-specific cache info method reports its info is incomplete, fallback to parsing sysfs */
  if(!libtea__arch_init_cache_info(instance)){

    instance->llc_partitions = 1;
    libtea_info("Assuming your LLC has only a single partition. If this is incorrect, please set the environment variable LIBTEA_LLC_PARTITIONS.");

    #if LIBTEA_LINUX
    const char* cmd1 = LIBTEA_SHELL " -c 'getconf LEVEL3_CACHE_SIZE'";
    instance->llc_size = libtea__get_numeric_sys_cmd_output(cmd1);
    if(instance->llc_size == -1) goto libtea__init_cache_info_err;

    /* getconf worked and LLC is level 3, so get all other params at this level, ignore level 4 victim caches */
    else if(instance->llc_size != 0){  
      const char* cmd2 = LIBTEA_SHELL " -c 'getconf LEVEL3_CACHE_ASSOC'";
      instance->llc_ways = libtea__get_numeric_sys_cmd_output(cmd2);
      const char* cmd3 = LIBTEA_SHELL " -c 'getconf LEVEL3_CACHE_LINESIZE'";
      instance->llc_line_size = libtea__get_numeric_sys_cmd_output(cmd3);
    }

    /* getconf worked but no level 3 cache, so try L2 */
    else{ 
      const char* cmd4 = LIBTEA_SHELL " -c 'getconf LEVEL2_CACHE_SIZE'";
      instance->llc_size = libtea__get_numeric_sys_cmd_output(cmd4);
      if(instance->llc_size <= 0) goto libtea__init_cache_info_err; //no L2 either, give up (assume L1 cannot be LLC)
      else { //LLC is L2
        const char* cmd5 = LIBTEA_SHELL " -c 'getconf LEVEL2_CACHE_ASSOC'";
        instance->llc_ways = libtea__get_numeric_sys_cmd_output(cmd5);
        const char* cmd6 = LIBTEA_SHELL " -c 'getconf LEVEL2_CACHE_LINESIZE'";
        instance->llc_line_size = libtea__get_numeric_sys_cmd_output(cmd6);
      }
    }
    return;

    libtea__init_cache_info_err:
      libtea_info("Error: could not automatically obtain the properties of the LLC. All subsequent library calls using cache properties will produce incorrect results. Please set the LIBTEA_LLC_* environment variables.");

    #else
    NO_WINDOWS_SUPPORT;
    #endif

  }
  
}

// ---------------------------------------------------------------------------
libtea_inline static void libtea_cleanup_cache(libtea_instance* instance) {
  libtea__eviction_cleanup(instance);
  libtea__cleanup_counter_thread(instance);
  libtea_munmap(instance->covert_channel, LIBTEA_COVERT_CHANNEL_OFFSET * LIBTEA_COVERT_CHANNEL_ENTRIES, &instance->covert_channel_handle);
  int i = 0;
  while(libtea__instances[i]) {
    if(libtea__instances[i] == instance) {
      libtea__instances[i] = NULL;
    }
    i++;
  }
}

// ---------------------------------------------------------------------------
libtea_inline int libtea__log_2(const uint32_t x) {
  if(x == 0) return 0;
  
  #if _MSC_VER
  return (31 - (int)__lzcnt(x));
  #else
  return (31 - __builtin_clz (x));
  #endif
}

// ---------------------------------------------------------------------------
libtea_inline bool libtea__eviction_init(libtea_instance* instance) {
  if (instance->eviction != NULL && instance->eviction != 0) {
    return false;
  }

  libtea_eviction* eviction = (libtea_eviction*) calloc(1, sizeof(libtea_eviction));
  if (eviction == NULL) {
    return false;
  }

  eviction->memory.mapping_size = 128 * 1024 * 1024;
  eviction->memory.mapping = (char*) libtea_mmap(eviction->memory.mapping_size, eviction->memory.handle, LIBTEA_READ_WRITE);
  if (eviction->memory.mapping == NULL) {
    free(eviction);
    return false;
  }
  time_t srand_init;
  srand((unsigned) time(&srand_init));
  for(int value = 0; value < (int)eviction->memory.mapping_size; value++){
    /* Initialize the pages to different values to avoid memory deduplication collapsing it */
    ((char*)eviction->memory.mapping)[value] = rand() % 256;
  }

  eviction->congruent_address_cache = (libtea_congruent_address_cache_entry*) calloc(instance->llc_sets * instance->llc_slices, sizeof(libtea_congruent_address_cache_entry));
  if (eviction->congruent_address_cache == NULL) {
    libtea_munmap(eviction->memory.mapping, eviction->memory.mapping_size, eviction->memory.handle);
    free(eviction);
    return false;
  }

  instance->eviction = eviction;

  return true;
}

// ---------------------------------------------------------------------------
libtea_inline void libtea__eviction_cleanup(libtea_instance* instance) {
  if (instance->eviction == NULL) {
    return;
  }

  if (instance->eviction->congruent_address_cache) {
    free(instance->eviction->congruent_address_cache);
  }

  if (instance->eviction->memory.mapping) {
    libtea_munmap(instance->eviction->memory.mapping, instance->eviction->memory.mapping_size, instance->eviction->memory.handle);
  }

  free(instance->eviction);
  instance->eviction = NULL;
}

// ---------------------------------------------------------------------------
libtea_inline size_t libtea__eviction_get_lookup_index(libtea_instance* instance, size_t paddr) {
  int slice_index = libtea_get_cache_slice(instance, paddr);
  int set_index = libtea_get_cache_set(instance, paddr);

  return (slice_index * instance->llc_sets) + set_index;
}

// ---------------------------------------------------------------------------
libtea_inline bool find_congruent_addresses(libtea_instance* instance, size_t index, size_t paddr, size_t number_of_addresses) {

  if (instance->eviction->congruent_address_cache[index].used == true) {
    if (instance->eviction->congruent_address_cache[index].n >= number_of_addresses) {
      return true;
    }
  }

  if (instance->eviction->congruent_address_cache[index].congruent_virtual_addresses == NULL || instance->eviction->congruent_address_cache[index].n < number_of_addresses) {
    instance->eviction->congruent_address_cache[index].congruent_virtual_addresses = (void**) realloc(instance->eviction->congruent_address_cache[index].congruent_virtual_addresses, (sizeof(libtea_congruent_address_cache_entry) * number_of_addresses));
  }

  size_t addr = 0;
  char* current = (char*) instance->eviction->memory.mapping;
  current += paddr % 4096;
  while (addr < number_of_addresses) {
      *current = addr + 1;
      size_t physical = libtea_get_physical_address(instance, (size_t)current);
      if (libtea__eviction_get_lookup_index(instance, physical) == index && physical != paddr) {
          instance->eviction->congruent_address_cache[index].congruent_virtual_addresses[addr] = current;
          addr++;
      }

      current += 4096;
  }

  if (addr != number_of_addresses) {
    return false;
  }

  instance->eviction->congruent_address_cache[index].n = addr;
  instance->eviction->congruent_address_cache[index].used = true;

  return true;
}

// ---------------------------------------------------------------------------
libtea_inline static void libtea__cleanup_all(void) {
  int i = 0;
  while(libtea__instances[i]) {
    libtea_cleanup(libtea__instances[i]);
    libtea__instances[i] = 0;
    i++;
  }
}

// ---------------------------------------------------------------------------
libtea_inline static int libtea_init_cache(libtea_instance* instance) {
  instance->llc_miss_threshold = 180;

  libtea__init_cache_info(instance);
  if(getenv("LIBTEA_LLC_SLICES")) {
    instance->llc_slices = atoi(getenv("LIBTEA_LLC_SLICES"));
    libtea_info("Configured LLC slice count with $LIBTEA_LLC_SLICES value %d", instance->llc_slices);
  }
  if(getenv("LIBTEA_LLC_LINE_SIZE")) {
    instance->llc_line_size = atoi(getenv("LIBTEA_LLC_LINE_SIZE"));
    libtea_info("Configured LLC line size with $LIBTEA_LLC_LINE_SIZE value %d", instance->llc_line_size);
  }
  if(getenv("LIBTEA_LLC_SETS")) {
    instance->llc_sets = atoi(getenv("LIBTEA_LLC_SETS"));
    libtea_info("Configured LLC set count with $LIBTEA_LLC_SETS value %d", instance->llc_sets);
  }
  if(getenv("LIBTEA_LLC_PARTITIONS")) {
    instance->llc_partitions = atoi(getenv("LIBTEA_LLC_PARTITIONS"));
    libtea_info("Configured LLC partitions count with $LIBTEA_LLC_PARTITIONS value %d", instance->llc_partitions);
  }

  if(!instance->llc_sets) instance->llc_sets = instance->llc_size / instance->llc_line_size / instance->llc_ways / instance->llc_partitions;
  int set_per_cpu = instance->llc_sets / instance->llc_slices;

  instance->llc_set_mask = ((1 << libtea__log_2(set_per_cpu)) - 1) << libtea__log_2(instance->llc_line_size);

  #if LIBTEA_LINUX
  struct sysinfo info;
  if(sysinfo(&info) < 0){
    libtea_info("Error: call to sysinfo failed when initializing Libtea cache functionality.");
    return LIBTEA_ERROR;
  }
  instance->physical_memory = (size_t) info.totalram * (size_t) info.mem_unit;
  
  #else
  MEMORYSTATUSEX memory_status;
  memory_status.dwLength = sizeof(memory_status);
  if(!GlobalMemoryStatusEx(&memory_status)){
    int err = GetLastError();
    libtea_info("Error: call to GlobalMemoryStatusEx failed with error code %d when initializing Libtea cache functionality.", err);
    return LIBTEA_ERROR;
  }
  instance->physical_memory = memory_status.ullTotalPhys; /* Amount of actual physical memory in bytes */
  #endif

  libtea__arch_init_direct_physical_map(instance);
  libtea__arch_init_eviction_strategy(instance);
  libtea__arch_init_prime_strategy(instance);

  instance->covert_channel = libtea_mmap(LIBTEA_COVERT_CHANNEL_OFFSET * LIBTEA_COVERT_CHANNEL_ENTRIES, &instance->covert_channel_handle, LIBTEA_READ_WRITE);
  time_t srand_init;
  srand((unsigned) time(&srand_init));
  for(int value = 0; value < LIBTEA_COVERT_CHANNEL_OFFSET * LIBTEA_COVERT_CHANNEL_ENTRIES; value++){
    /* Very very important to initialize the covert channel pages to different values to avoid memory deduplication collapsing the channel */
    ((char*)instance->covert_channel)[value] = rand() % 256;
  }
  for(int value = 0; value < LIBTEA_COVERT_CHANNEL_ENTRIES; value++) {
    libtea_flush((char*) instance->covert_channel + value * LIBTEA_COVERT_CHANNEL_OFFSET);
  }

  int instance_count;
  for(instance_count = 0; instance_count < (int)sizeof(libtea__instances) / (int)sizeof(libtea__instances[0]) - 1; instance_count++) {
    if(libtea__instances[instance_count] == NULL) {
      libtea__instances[instance_count] = instance;
      break;
    }
  }

  if(getenv("LIBTEA_HIT_THRESHOLD")) {
    instance->llc_hit_threshold = atoi(getenv("LIBTEA_HIT_THRESHOLD"));
    libtea_info("Configured LLC hit threshold with $LIBTEA_HIT_THRESHOLD value %d", instance->llc_hit_threshold);
  }
  if(getenv("LIBTEA_MISS_THRESHOLD")) {
    instance->llc_miss_threshold = atoi(getenv("LIBTEA_MISS_THRESHOLD"));
    libtea_info("Configured LLC miss threshold with $LIBTEA_MISS_THRESHOLD value %d", instance->llc_miss_threshold);
  }
  if(getenv("LIBTEA_EVICTION_STRATEGY")) {
    // C-D-L-S
    char* strategy = strdup(getenv("LIBTEA_EVICTION_STRATEGY"));
    instance->eviction_strategy.C = atoi(strtok(strategy, "-"));
    instance->eviction_strategy.D = atoi(strtok(NULL, "-"));
    instance->eviction_strategy.L = atoi(strtok(NULL, "-"));
    instance->eviction_strategy.S = atoi(strtok(NULL, "-"));
    libtea_info("Configured eviction strategy with $LIBTEA_EVICTION_STRATEGY value %s", strategy);
    free(strategy);
  }
  if(getenv("LIBTEA_PRIME_STRATEGY")) {
    // C-D-L-S
    char* strategy = strdup(getenv("LIBTEA_PRIME_STRATEGY"));
    instance->eviction_strategy.C = atoi(strtok(strategy, "-"));
    instance->eviction_strategy.D = atoi(strtok(NULL, "-"));
    instance->eviction_strategy.L = atoi(strtok(NULL, "-"));
    instance->eviction_strategy.S = atoi(strtok(NULL, "-"));
    libtea_info("Configured prime strategy with $LIBTEA_PRIME_STRATEGY value %s", strategy);
    free(strategy);
  }
  if(getenv("LIBTEA_DIRECT_PHYSICAL_MAP")) {
    instance->direct_physical_map = strtoull(getenv("LIBTEA_DIRECT_PHYSICAL_MAP"), NULL, 0);
    libtea_info("Configured direct physical map with $LIBTEA_DIRECT_PHYSICAL_MAP value %zu", instance->direct_physical_map);
  }

  if(getenv("LIBTEA_DUMP")) {
    printf("Libtea configuration\n");
    printf("* LLC: %d sets, %d ways, %d slices (line size: %d)\n", instance->llc_sets, instance->llc_ways, instance->llc_slices, instance->llc_line_size);
    printf("* Cache hit/miss threshold: [%d, %d]\n", instance->llc_hit_threshold, instance->llc_miss_threshold);
    printf("* CPU: %d physical / %d logical cores, %s, architecture: 0x%x, %s\n", instance->physical_cores, instance->logical_cores, instance->is_intel ? "Intel" : "Non-Intel", instance->cpu_architecture, instance->has_tm ? "with transactional memory support" : "no transactional memory support");
    
    #if LIBTEA_LINUX
    printf("* Memory: %zd bytes / Memory map @ 0x%zx\n", instance->physical_memory, instance->direct_physical_map);
    #else
    printf("* Memory: %zd bytes\n", instance->physical_memory);
    #endif

    printf("\n");
  }
    
  if (instance_count == 0) {
    atexit(libtea__cleanup_all);  /* Triggers cleanup later at exit, not now */
  }

  return LIBTEA_SUCCESS;
}



/* Public functions included in API */
//---------------------------------------------------------------------------

libtea_inline int libtea_flush_reload(libtea_instance* instance, void* addr) {
  libtea_measure_start(instance);
  libtea_access(addr);
  int time = (int)libtea_measure_end(instance);
  int hit = (time < instance->llc_miss_threshold) && (time > instance->llc_hit_threshold);
  libtea_flush_b(addr);  /* Noise substantially increases without a memory barrier here */
  return hit;
}


libtea_inline void libtea_calibrate_flush_reload(libtea_instance* instance) {
  size_t reload_time = 0, flush_reload_time = 0, i, count = 1000000;
  size_t dummy[16];
  size_t *ptr = dummy + 8;

  libtea_access(ptr);
  for (i = 0; i < count; i++) {
    libtea_measure_start(instance);
    libtea_access(ptr);
    reload_time += libtea_measure_end(instance);
  }
  for (i = 0; i < count; i++) {
    libtea_measure_start(instance);
    libtea_access(ptr);
    flush_reload_time += libtea_measure_end(instance);
    libtea_flush_b(ptr);
  }
  reload_time /= count;
  flush_reload_time /= count;

  if(!getenv("LIBTEA_HIT_THRESHOLD")){
    instance->llc_hit_threshold = 0;      /* There is no need to have a hit threshold on most systems */
  }
  if(!getenv("LIBTEA_MISS_THRESHOLD")){
    instance->llc_miss_threshold = (flush_reload_time + reload_time * 2) / 3;
  }
}


libtea_inline int libtea_get_cache_slice(libtea_instance* instance, size_t paddr) {

  if(!instance->is_intel){
    libtea_info("libtea_get_cache_slice is only supported on Intel CPUs. The returned value will be incorrect.");
    return 0;
  }

  static const int h0[] = { 6, 10, 12, 14, 16, 17, 18, 20, 22, 24, 25, 26, 27, 28, 30, 32, 33, 35, 36 };
  static const int h1[] = { 7, 11, 13, 15, 17, 19, 20, 21, 22, 23, 24, 26, 28, 29, 31, 33, 34, 35, 37 };
  static const int h2[] = { 8, 12, 13, 16, 19, 22, 23, 26, 27, 30, 31, 34, 35, 36, 37 };

  int count = sizeof(h0) / sizeof(h0[0]);
  int hash = 0;
  if(instance->llc_slices <= 1) return hash;

  for (int i = 0; i < count; i++) {
    hash ^= (paddr >> h0[i]) & 1;
  }
  if(instance->llc_slices == 2) return hash;

  count = sizeof(h1) / sizeof(h1[0]);
  int hash1 = 0;
  for (int i = 0; i < count; i++) {
    hash1 ^= (paddr >> h1[i]) & 1;
  }
  if(instance->llc_slices == 4) return hash | (hash1 << 1);

  count = sizeof(h2) / sizeof(h2[0]);
  int hash2 = 0;
  for (int i = 0; i < count; i++) {
    hash2 ^= (paddr >> h2[i]) & 1;
  }
  if(instance->llc_slices == 8) return (hash2 << 2) | (hash1 << 1) | hash;

  return 0;
}


libtea_inline int libtea_get_cache_set(libtea_instance* instance, size_t paddr) {
  return (paddr & instance->llc_set_mask) >> libtea__log_2(instance->llc_line_size);
}


libtea_inline int libtea_build_eviction_set(libtea_instance* instance, libtea_eviction_set* set, size_t paddr) {
  if (instance->eviction == NULL || instance->eviction == 0) {
    libtea__eviction_init(instance);
  }

  set->addresses = instance->eviction_strategy.S + instance->eviction_strategy.C + instance->eviction_strategy.D;
  set->address = NULL;

  size_t index = libtea__eviction_get_lookup_index(instance, paddr);

  if (find_congruent_addresses(instance, index, paddr, set->addresses) == false) {
    return LIBTEA_ERROR;
  }

  set->address = instance->eviction->congruent_address_cache[index].congruent_virtual_addresses;

  return LIBTEA_SUCCESS;
}


libtea_inline int libtea_build_eviction_set_vaddr(libtea_instance* instance, libtea_eviction_set* set, size_t vaddr) {
  size_t paddr = libtea_get_physical_address(instance, (size_t)vaddr);
  return libtea_build_eviction_set(instance, set, paddr);
}


libtea_inline void libtea_evict(libtea_instance* instance, libtea_eviction_set set) {
  int s, c, d;
  for(s = 0; s <= instance->eviction_strategy.S; s += instance->eviction_strategy.L) {
    for(c = 0; c <= instance->eviction_strategy.C; c += 1) {
      for(d = 0; d <= instance->eviction_strategy.D; d += 1) {
        libtea_access(set.address[s + d]);
      }
    }
  }
}


libtea_inline int libtea_evict_reload(libtea_instance* instance, void* addr, libtea_eviction_set set) { 
  libtea_measure_start(instance);
  libtea_access(addr);
  int time = (int)libtea_measure_end(instance);
  int hit = (time < instance->llc_miss_threshold) && (time > instance->llc_hit_threshold);
  libtea_evict(instance, set);
  return hit;
}


libtea_inline void libtea_calibrate_evict_reload(libtea_instance* instance) {
  size_t reload_time = 0, evict_reload_time = 0, i, count = 1000000;
  size_t dummy[16];
  size_t *ptr = dummy + 8;

  *ptr = 2;
  libtea_eviction_set ev;
  if(libtea_build_eviction_set_vaddr(instance, &ev, (size_t)ptr)) return;

  libtea_access(ptr);
  for (i = 0; i < count; i++) {
    libtea_measure_start(instance);
    libtea_access(ptr);
    reload_time += libtea_measure_end(instance);
  }
  for (i = 0; i < count; i++) {
    libtea_measure_start(instance);
    libtea_access(ptr);
    evict_reload_time += libtea_measure_end(instance);
    libtea_evict(instance, ev);
  }
  reload_time /= count;
  evict_reload_time /= count;

  instance->llc_hit_threshold = 0; /* There is no need to have a hit threshold on most systems */
  instance->llc_miss_threshold = (evict_reload_time + reload_time * 2) / 3;
}


libtea_inline void libtea_prime(libtea_instance* instance, libtea_eviction_set set) {
  int s, c, d;
  for(s = 0; s <= instance->prime_strategy.S; s += instance->prime_strategy.L) {
    for(c = 0; c <= instance->prime_strategy.C; c += 1) {
      for(d = 0; d <= instance->prime_strategy.D; d += 1) {
        libtea_access(set.address[s + d]);
      }
    }
  }
}


libtea_inline int libtea_prime_probe(libtea_instance* instance, libtea_eviction_set set) {
  libtea_measure_start(instance);
  libtea_prime(instance, set);
  return libtea_measure_end(instance);
}


libtea_inline size_t libtea_measure_slice(libtea_instance* instance, void* address) {

  if(!instance->is_intel){
    libtea_info("libtea_measure_slice is only supported on Intel CPUs. The returned value will be incorrect.");
    return 0;
  }

  int msr_unc_perf_global_ctr;
  int val_enable_ctrs;
  if(instance->cpu_architecture >= 0x16) {
    /* Skylake or newer */   
    msr_unc_perf_global_ctr = 0xe01;
    val_enable_ctrs = 0x20000000;
  }
  else {
    msr_unc_perf_global_ctr = 0x391;
    val_enable_ctrs = 0x2000000f;
  }
    
  /* Disable counters */
  if(libtea_write_system_reg(instance, 0, msr_unc_perf_global_ctr, 0x0)) {
    return -1ull;
  }

  /* Reset counters */
  for (int i = 0; i < instance->llc_slices; i++) {
    libtea_write_system_reg(instance, 0, 0x706 + i * 0x10, 0x0);
  }

  /* Select event to monitor */
  for (int i = 0; i < instance->llc_slices; i++) {
    libtea_write_system_reg(instance, 0, 0x700 + i * 0x10, 0x408f34);
  }

  /* Enable counting */
  if(libtea_write_system_reg(instance, 0, msr_unc_perf_global_ctr, val_enable_ctrs)) {
    return -1ull;
  }

  /* Monitor */
  int access = 10000;
  while (--access) {
    libtea_flush(address);
  }

  /* Read counter */
  size_t* cboxes = (size_t*) malloc(sizeof(size_t) * instance->llc_slices);
  for (int i = 0; i < instance->llc_slices; i++) {
    cboxes[i] = libtea_read_system_reg(instance, 0, 0x706 + i * 0x10);
  }
  free(cboxes);

  return libtea_find_index_of_nth_largest_sizet(cboxes, instance->llc_slices, 0);
}


libtea_inline void libtea_cache_encode(libtea_instance* instance, unsigned char value) {
  libtea_access((char*) instance->covert_channel + value * LIBTEA_COVERT_CHANNEL_OFFSET);
}


libtea_inline void libtea_cache_encode_dereference(libtea_instance* instance, char* ptr, int offset) {
  libtea_access((char*) instance->covert_channel + ptr[offset] * LIBTEA_COVERT_CHANNEL_OFFSET);
}

libtea_inline int libtea_cache_decode_from_to(libtea_instance* instance, int from, int to, bool use_mix) {
  if(use_mix){
    for(int i = 0; i < 256; i++) {
      int mix_i = ((i * 167) + 13) & 255;
      if(mix_i < from || mix_i > to) continue;
      if(libtea_flush_reload(instance, (char*) instance->covert_channel + mix_i * LIBTEA_COVERT_CHANNEL_OFFSET)) {
        return mix_i;
      }
    }
  }
  else{
    for(int i = from; i <= to; i++) {
      if(libtea_flush_reload(instance, (char*) instance->covert_channel + i * LIBTEA_COVERT_CHANNEL_OFFSET)) {
        return i;
      }
    }
  }
  return LIBTEA_ERROR;
}


libtea_inline int libtea_cache_decode(libtea_instance* instance, bool use_mix) {
  return libtea_cache_decode_from_to(instance, 0, LIBTEA_COVERT_CHANNEL_ENTRIES-1, use_mix);
}


libtea_inline int libtea_cache_decode_nonull(libtea_instance* instance, bool use_mix) {
  return libtea_cache_decode_from_to(instance, 1, LIBTEA_COVERT_CHANNEL_ENTRIES-1, use_mix);
}


libtea_inline void libtea_cache_decode_histogram_iteration(libtea_instance* instance, bool use_mix, bool print, int offset, int from, int to, int* hist){
  bool update = false;
  int decoded = libtea_cache_decode_from_to(instance, from, to, use_mix);
  if(decoded > 0 && decoded != LIBTEA_ERROR){
    hist[decoded]++;
    update = true;
  }

  /* Redraw histogram on update */
  if (print && update) {
    #if LIBTEA_LINUX
    printf("\x1b[2J");
    #else
    system("cls");
    #endif
    int max = 1;

    for (int i = from; i <= to; i++) {
      if (hist[i] > max) {
        max = hist[i];
      }
    }

    printf("\n");
    for (int i = from; i <= to; i++) {
      printf("%c: (%4d) ", i+offset, hist[i]);
      for (int k = 0; k < hist[i] * 60 / max; k++) {
        printf("#");
      }
      printf("\n");
    }

    fflush(stdout);
  }
}



libtea_inline void libtea_print_cache_decode_histogram(libtea_instance* instance, int iterations, int sleep_len, bool yield, bool use_mix, void(*activity)(), int offset, int from, int to) {

  int* hist = (int*) malloc(sizeof(int)*LIBTEA_COVERT_CHANNEL_ENTRIES);
  memset(hist, 0, sizeof(int)*LIBTEA_COVERT_CHANNEL_ENTRIES);
  bool update = false;
  
  for(int reps=0; reps<iterations; reps++){

    if(activity != NULL) activity();

    if(use_mix){
      for(int i=0; i<256; i++){
        int mix_i = ((i * 167) + 13) % 256;
        if(mix_i < from || mix_i > to) continue;
        if(libtea_flush_reload(instance, (char*) instance->covert_channel + mix_i * LIBTEA_COVERT_CHANNEL_OFFSET)) {
          hist[mix_i]++;
          update = true;
        }  
      }
    }
    else{
      for(int i=from; i<=to; i++){
        if(libtea_flush_reload(instance, (char*) instance->covert_channel + i * LIBTEA_COVERT_CHANNEL_OFFSET)) {
          hist[i]++;
          update = true;
        }  
      }
    }

    /* Redraw histogram on update */
    if (update) {
      #if LIBTEA_LINUX
      printf("\x1b[2J");
      #else
      system("cls");
      #endif
      int max = 1;

      for (int i = from; i <= to; i++) {
        if (hist[i] > max) {
          max = hist[i];
        }
      }

      printf("\n");
      for (int i = from; i <= to; i++) {
        printf("%c: (%4d) ", i+offset, hist[i]);
        for (int k = 0; k < hist[i] * 60 / max; k++) {
          printf("#");
        }
        printf("\n");
      }

      fflush(stdout);
    }
    update = false;

    if(sleep_len > 0) {
      #if LIBTEA_LINUX
      usleep(sleep_len);
      #else
      if((sleep_len % 1000) != 0) {
        libtea_info("Warning: Windows can only sleep with millisecond precision.\nPlease adjust sleep_len to be a multiple of 1000.");
      }
      Sleep(sleep_len / 1000);
      #endif
    }

    if(yield) {
      #if LIBTEA_LINUX
      sched_yield();
      #else
      SwitchToThread();
      #endif
    }

  }
  free(hist);
}


libtea_inline int* libtea_numeric_cache_decode_histogram(libtea_instance* instance, int iterations, int sleep_len, bool yield, bool use_mix, void(*activity)(), int offset, int from, int to) {

  int* hist = (int*) malloc(sizeof(int)*LIBTEA_COVERT_CHANNEL_ENTRIES);
  memset(hist, 0, sizeof(int)*LIBTEA_COVERT_CHANNEL_ENTRIES);
  
  for(int reps=0; reps<iterations; reps++){

    if(activity != NULL) activity();

    if(use_mix){
      for(int i=0; i<256; i++){
        int mix_i = ((i * 167) + 13) % 256;
        if(mix_i < from || mix_i > to) continue;
        if(libtea_flush_reload(instance, (char*) instance->covert_channel + mix_i * LIBTEA_COVERT_CHANNEL_OFFSET)) {
          hist[mix_i]++;
        }  
      }
    }
    else{
      for(int i=from; i<=to; i++){
        if(libtea_flush_reload(instance, (char*) instance->covert_channel + i * LIBTEA_COVERT_CHANNEL_OFFSET)) {
          hist[i]++;
        }  
      }
    }

    if(sleep_len > 0) {
      #if LIBTEA_LINUX
      usleep(sleep_len);
      #else
      if((sleep_len % 1000) != 0) {
        libtea_info("Warning: Windows can only sleep with millisecond precision.\nPlease adjust sleep_len to be a multiple of 1000.");
      }
      Sleep(sleep_len / 1000);
      #endif
    }

    if(yield) {
      #if LIBTEA_LINUX
      sched_yield();
      #else
      SwitchToThread();
      #endif
    }

  }
  return hist;
}


void libtea_check_decoded_per_cacheline(char* decoded, char* expected, int length) {
  int failed = 0;
  for (int cacheline=0; cacheline<(length/64); cacheline++){
    int failed=0;
    for(int i=0; i<64; i++){
      int offset = i + cacheline*64;
      if(decoded[offset] != expected[offset]){
        failed++;
      }
    }
    printf("%.02f,", (double)(64-failed)/64);
  }
  printf("\n");
}


int libtea_check_decoded(char* decoded, char* expected, int length, bool print_results) {
  int failed = 0;
  for (int i=0; i<length; i++) {
    if (decoded[i] != expected[i]) {
      failed++;
      if (print_results) libtea_info("Expected 0x%02x at %d but got 0x%02x", expected[i], i, decoded[i]);
    }
  }
  if (failed && print_results) {
    libtea_info("[FAIL] %d/%d bytes incorrect", failed, length);
  }
  else if (print_results) {
    libtea_info("[SUCCESS] Recovered all %d bytes correctly", length);
  }
  return failed;
}


/* End libtea_cache.c */
//---------------------------------------------------------------------------

/* See LICENSE file for license and copyright information */

/* Start libtea_x86_cache.c */
//---------------------------------------------------------------------------

#if LIBTEA_X86




int libtea__arch_init_cache_info(libtea_instance* instance){
  instance->llc_slices = instance->physical_cores;    /* This holds on Intel (exception handled below) and AMD Zen -> Epyc, Zen 2 */
  
  int level = 0;
  uint32_t eax, ebx, ecx, edx;

  if(instance->is_intel) {
    if(instance->cpu_architecture >= 0x16) { 
      /* If Skylake or newer */
      instance->llc_slices *= 2;
    }
    do {
      
      #if LIBTEA_LINUX
      asm volatile("cpuid" : "=a" (eax), "=b" (ebx), "=c" (ecx), "=d" (edx) : "a" (4), "c" (level));
      #else
      int cpuid_info[4] = {0, 0, 0, 0};
      __cpuidex(cpuid_info, 4, level);
      eax = cpuid_info[0];
      ebx = cpuid_info[1];
      ecx = cpuid_info[2];
      edx = cpuid_info[3];
      #endif
      
      int type = eax & 0x1f;
      if(!type) break;
      level++;
      instance->llc_line_size = (ebx & 0xfff) + 1;
      instance->llc_ways = ((ebx >> 22) & 0x3ff) + 1;
      instance->llc_sets = ecx + 1;
      instance->llc_partitions = ((ebx >> 12) & 0x3ff) + 1;
      instance->llc_size = instance->llc_line_size * instance->llc_ways * instance->llc_sets * instance->llc_partitions;
    } while(1);
    return 1;    /* Report cache data is complete */
  }

  /* Check if it is actually an AMD CPU and not Via, Centaur etc */
  
  #if LIBTEA_LINUX
  uint32_t temp = 0;
  uint32_t name[3] = {0, 0, 0};
  __cpuid(0, temp, name[0], name[2], name[1]);
  if(strcmp((char *) name, "AuthenticAMD") == 0) {
  
  #else
  int temp[4] = {0, 0, 0, 0};
  __cpuid(temp, 0);
  int name[4] = {0, 0, 0, 0};
  name[0] = temp[1];
  name[1] = temp[3];
  name[2] = temp[2];
  if(strcmp((char *) name, "AuthenticAMD") == 0) {
  #endif

    do {
    
      #if LIBTEA_LINUX
      asm volatile("cpuid" : "=a" (eax), "=b" (ebx), "=c" (ecx), "=d" (edx) : "a" (0x8000001D), "c" (level));
      #else
      int cpuid_info[4] = {0, 0, 0, 0};
      __cpuidex(cpuid_info, 0x8000001D, level);
      eax = cpuid_info[0];
      ebx = cpuid_info[1];
      ecx = cpuid_info[2];
      edx = cpuid_info[3];
      #endif
      
      int type = eax & 0xf;   /* bits 4:0 */
      if(!type && level == 0){
        /* If this happens, the CPU does not support CPUID topology extensions */
        return 0;
      }
      else if(!type) break;
      level++;
      instance->llc_line_size = (ebx & 0xfff) + 1;            /* Bits 11:0 of EBX */
      instance->llc_partitions = ((ebx >> 12) & 0x3ff) + 1;   /* Bits 21:12 of EBX */
      instance->llc_ways = ((ebx >> 22) & 0x3ff) + 1;         /* Bits 31:22 of EBX */
      instance->llc_sets = ecx + 1;
      instance->llc_size = instance->llc_line_size * instance->llc_ways * instance->llc_sets * instance->llc_partitions;
    } while(1);

    return 1;    /* Report cache data is complete */
  }

  else return 0; /* Report cache data is incomplete - parent function will parse from sysfs instead */
}


void libtea__arch_init_direct_physical_map(libtea_instance* instance){

  #if LIBTEA_LINUX
  struct utsname buf;
  uname(&buf);
  int major = atoi(strtok(buf.release, "."));
  int minor = atoi(strtok(NULL, "."));

  if((major == 4 && minor < 19) || major < 4) {
    instance->direct_physical_map = 0xffff880000000000ull;
  } else {
    instance->direct_physical_map = 0xffff888000000000ull;
  }
  #else
  /* No direct-physical map on Windows */
  instance->direct_physical_map = 0;
  #endif

}


void libtea__arch_init_eviction_strategy(libtea_instance* instance){
  instance->eviction_strategy.C = 4;
  instance->eviction_strategy.D = 5;
  instance->eviction_strategy.L = 5;
  instance->eviction_strategy.S = 20;
}


void libtea__arch_init_prime_strategy(libtea_instance* instance){
  instance->prime_strategy.C = 1;
  instance->prime_strategy.D = 2;
  instance->prime_strategy.L = 1;
  instance->prime_strategy.S = instance->llc_ways - instance->prime_strategy.D - 1;
}


void libtea__arch_fast_cache_encode(libtea_instance* instance, void* addr) {
  #if LIBTEA_INLINEASM
  asm volatile("movzx (%%rcx), %%rax; shl $12, %%rax; movq (%%rbx,%%rax,1), %%rbx" : : "c"(addr), "b"(instance->covert_channel) : "rax");
  #else
  libtea_info("libtea_fast_cache_encode is not supported when compiled without inline assembly support");
  #endif
}


#endif //LIBTEA_X86


/* End libtea_x86_cache.c */
//---------------------------------------------------------------------------

/* See LICENSE file for license and copyright information */

/* Start libtea_paging.c */
//---------------------------------------------------------------------------

#include <string.h>





typedef size_t(*libtea_phys_read)(libtea_instance*, size_t);
typedef void(*libtea_phys_write)(libtea_instance*, size_t, size_t);


/* Internal functions not part of public API */

libtea_page_entry libtea__resolve_addr_kernel(libtea_instance* instance, void* address, pid_t pid);
static libtea_page_entry libtea__resolve_addr_user_ext(libtea_instance* instance, void* address, pid_t pid, libtea_phys_read deref);
static libtea_page_entry libtea__resolve_addr_user(libtea_instance* instance, void* address, pid_t pid);
#if LIBTEA_LINUX
static libtea_page_entry libtea__resolve_addr_user_map(libtea_instance* instance, void* address, pid_t pid);
#endif
void libtea__update_addr_kernel(libtea_instance* instance, void* address, pid_t pid, libtea_page_entry* vm);
void libtea__update_addr_user_ext(libtea_instance* instance, void* address, pid_t pid, libtea_page_entry* vm, libtea_phys_write pset);
static void libtea__update_addr_user(libtea_instance* instance, void* address, pid_t pid, libtea_page_entry* vm);
#if LIBTEA_LINUX
static void libtea__update_addr_user_map(libtea_instance* instance, void* address, pid_t pid, libtea_page_entry* vm);
#endif
uint64_t libtea__get_physical_base_address(libtea_page_entry entry, libtea_page_level level);
uint64_t libtea__get_virtual_address_index(libtea_page_entry entry, libtea_page_level level);


static inline size_t libtea__phys_read_map(libtea_instance* instance, size_t address) {
  return *(size_t*)(instance->vmem + address);
}


static inline void libtea__phys_write_map(libtea_instance* instance, size_t address, size_t value) {
  *(size_t*)(instance->vmem + address) = value;
}


static inline size_t libtea__phys_read_pread(libtea_instance* instance, size_t address) {
  size_t val = 0;

  #if LIBTEA_LINUX
  pread(instance->umem_fd, &val, sizeof(size_t), address);

  #else
  ULONG returnLength;
  DeviceIoControl(instance->module_fd, LIBTEA_IOCTL_READ_PHYS_VAL, (LPVOID)&address, sizeof(address), (LPVOID)&val, sizeof(val), &returnLength, 0);
  #endif

  return val;
}


static inline void libtea__phys_write_pwrite(libtea_instance* instance, size_t address, size_t value) {

  #if LIBTEA_LINUX
  pwrite(instance->umem_fd, &value, sizeof(size_t), address);

  #else
  ULONG returnLength;
  size_t info[2];
  info[0] = address;
  info[1] = value;
  DeviceIoControl(instance->module_fd, LIBTEA_IOCTL_WRITE_PHYS_VAL, (LPVOID)&info, sizeof(info), (LPVOID)&info, sizeof(info), &returnLength, 0);
  #endif

}


libtea_page_entry libtea__resolve_addr_kernel(libtea_instance* instance, void* address, pid_t pid) {
  libtea_page_entry vm;
  vm.vaddr = (size_t)address;
  vm.pid = (size_t)pid;

  #if LIBTEA_LINUX
  ioctl(instance->module_fd, LIBTEA_IOCTL_VM_RESOLVE, (size_t)&vm);

  #else
  NO_WINDOWS_SUPPORT;
  #endif

  return vm;
}


static libtea_page_entry libtea__resolve_addr_user_ext(libtea_instance* instance, void* address, pid_t pid, libtea_phys_read deref) {
  size_t root = (pid == 0) ? instance->paging_root : libtea_get_paging_root(instance, pid);

  int pgdi, p4di, pudi, pmdi, pti;
  size_t addr = (size_t)address;
  pgdi = (addr >> (instance->paging_definition.page_offset
        + instance->paging_definition.pt_entries
        + instance->paging_definition.pmd_entries
        + instance->paging_definition.pud_entries
        + instance->paging_definition.p4d_entries)) % (1ull << instance->paging_definition.pgd_entries);
  p4di = (addr >> (instance->paging_definition.page_offset
        + instance->paging_definition.pt_entries
        + instance->paging_definition.pmd_entries
        + instance->paging_definition.pud_entries)) % (1ull << instance->paging_definition.p4d_entries);
  pudi = (addr >> (instance->paging_definition.page_offset
        + instance->paging_definition.pt_entries
        + instance->paging_definition.pmd_entries)) % (1ull << instance->paging_definition.pud_entries);
  pmdi = (addr >> (instance->paging_definition.page_offset
        + instance->paging_definition.pt_entries)) % (1ull << instance->paging_definition.pmd_entries);
  pti = (addr >> instance->paging_definition.page_offset) % (1ull << instance->paging_definition.pt_entries);

  libtea_page_entry resolved;
  memset(&resolved, 0, sizeof(resolved));
  resolved.vaddr = (size_t)address;
  resolved.pid = (size_t)pid;
  resolved.valid = 0;

  if(!root) return resolved;

  size_t pgd_entry, p4d_entry, pud_entry, pmd_entry, pt_entry;

  pgd_entry = deref(instance, root + pgdi * sizeof(size_t));
  if (libtea_cast(pgd_entry, libtea_pgd).present != LIBTEA_PAGE_PRESENT) {
    return resolved;
  }
  resolved.pgd = pgd_entry;
  resolved.valid |= LIBTEA_VALID_MASK_PGD;
  if (instance->paging_definition.has_p4d) {
    size_t pfn = (size_t)(libtea_cast(pgd_entry, libtea_pgd).pfn);
    p4d_entry = deref(instance, pfn * instance->pagesize + p4di * sizeof(size_t));
    resolved.valid |= LIBTEA_VALID_MASK_P4D;
  }
  else {
    p4d_entry = pgd_entry;
  }
  resolved.p4d = p4d_entry;

  if (libtea_cast(p4d_entry, libtea_p4d).present != LIBTEA_PAGE_PRESENT) {
    return resolved;
  }

  if (instance->paging_definition.has_pud) {
    size_t pfn = (size_t)(libtea_cast(p4d_entry, libtea_p4d).pfn);
    pud_entry = deref(instance, pfn * instance->pagesize + pudi * sizeof(size_t));
    resolved.valid |= LIBTEA_VALID_MASK_PUD;
  }
  else {
    pud_entry = p4d_entry;
  }
  resolved.pud = pud_entry;

  if (libtea_cast(pud_entry, libtea_pud).present != LIBTEA_PAGE_PRESENT) {
    return resolved;
  }

  if (instance->paging_definition.has_pmd) {
    size_t pfn = (size_t)(libtea_cast(pud_entry, libtea_pud).pfn);
    pmd_entry = deref(instance, pfn * instance->pagesize + pmdi * sizeof(size_t));
    resolved.valid |= LIBTEA_VALID_MASK_PMD;
  }
  else {
    pmd_entry = pud_entry;
  }
  resolved.pmd = pmd_entry;

  if (libtea_cast(pmd_entry, libtea_pmd).present != LIBTEA_PAGE_PRESENT) {
    return resolved;
  }

  #if LIBTEA_X86
    if (!libtea_cast(pmd_entry, libtea_pmd).size) {
  #endif

    /* Normal 4KB page */
    size_t pfn = (size_t)(libtea_cast(pmd_entry, libtea_pmd).pfn);
    pt_entry = deref(instance, pfn * instance->pagesize + pti * sizeof(size_t));
    resolved.pte = pt_entry;
    resolved.valid |= LIBTEA_VALID_MASK_PTE;
    if (libtea_cast(pt_entry, libtea_pte).present != LIBTEA_PAGE_PRESENT) {
      return resolved;
    }
  #if LIBTEA_X86
  }
  #endif

  return resolved;
}


static libtea_page_entry libtea__resolve_addr_user(libtea_instance* instance, void* address, pid_t pid) {
  return libtea__resolve_addr_user_ext(instance, address, pid, libtea__phys_read_pread);
}


#if LIBTEA_LINUX
static libtea_page_entry libtea__resolve_addr_user_map(libtea_instance* instance, void* address, pid_t pid) {
  return libtea__resolve_addr_user_ext(instance, address, pid, libtea__phys_read_map);
}
#endif


void libtea__update_addr_kernel(libtea_instance* instance, void* address, pid_t pid, libtea_page_entry* vm) {
  vm->vaddr = (size_t)address;
  vm->pid = (size_t)pid;

  #if LIBTEA_LINUX
  ioctl(instance->module_fd, LIBTEA_IOCTL_VM_UPDATE, (size_t)vm);

  #else
  NO_WINDOWS_SUPPORT;
  #endif

}


void libtea__update_addr_user_ext(libtea_instance* instance, void* address, pid_t pid, libtea_page_entry* vm, libtea_phys_write pset) {
  libtea_page_entry current = libtea_resolve_addr(instance, address, pid);
  size_t root = (pid == 0) ? instance->paging_root : libtea_get_paging_root(instance, pid);

  if(!root) return;

  size_t pgdi, p4di, pudi, pmdi, pti;
  size_t addr = (size_t)address;
  pgdi = (addr >> (instance->paging_definition.page_offset
       + instance->paging_definition.pt_entries
       + instance->paging_definition.pmd_entries
       + instance->paging_definition.pud_entries
       + instance->paging_definition.p4d_entries)) % (1ull << instance->paging_definition.pgd_entries);
  p4di = (addr >> (instance->paging_definition.page_offset
       + instance->paging_definition.pt_entries
       + instance->paging_definition.pmd_entries
       + instance->paging_definition.pud_entries)) % (1ull << instance->paging_definition.p4d_entries);
  pudi = (addr >> (instance->paging_definition.page_offset
       + instance->paging_definition.pt_entries
       + instance->paging_definition.pmd_entries)) % (1ull << instance->paging_definition.pud_entries);
  pmdi = (addr >> (instance->paging_definition.page_offset
       + instance->paging_definition.pt_entries)) % (1ull << instance->paging_definition.pmd_entries);
  pti = (addr >> instance->paging_definition.page_offset) % (1ull << instance->paging_definition.pt_entries);

  if ((vm->valid & LIBTEA_VALID_MASK_PTE) && (current.valid & LIBTEA_VALID_MASK_PTE)) {
    pset(instance, (size_t)libtea_cast(current.pmd, libtea_pmd).pfn * instance->pagesize + pti * (instance->pagesize / (1 << instance->paging_definition.pt_entries)), vm->pte);
  }
  if ((vm->valid & LIBTEA_VALID_MASK_PMD) && (current.valid & LIBTEA_VALID_MASK_PMD) && instance->paging_definition.has_pmd) {
    pset(instance, (size_t)libtea_cast(current.pud, libtea_pud).pfn * instance->pagesize + pmdi * (instance->pagesize / (1 << instance->paging_definition.pmd_entries)), vm->pmd);
  }
  if ((vm->valid & LIBTEA_VALID_MASK_PUD) && (current.valid & LIBTEA_VALID_MASK_PUD) && instance->paging_definition.has_pud) {
    pset(instance, (size_t)libtea_cast(current.p4d, libtea_p4d).pfn * instance->pagesize + pudi * (instance->pagesize / (1 << instance->paging_definition.pud_entries)), vm->pud);
  }
  if ((vm->valid & LIBTEA_VALID_MASK_P4D) && (current.valid & LIBTEA_VALID_MASK_P4D) && instance->paging_definition.has_p4d) {
    pset(instance, (size_t)libtea_cast(current.pgd, libtea_pgd).pfn * instance->pagesize + p4di * (instance->pagesize / (1 << instance->paging_definition.p4d_entries)), vm->p4d);
  }
  if ((vm->valid & LIBTEA_VALID_MASK_PGD) && (current.valid & LIBTEA_VALID_MASK_PGD) && instance->paging_definition.has_pgd) {
    pset(instance, root + pgdi * (instance->pagesize / (1 << instance->paging_definition.pgd_entries)), vm->pgd);
  }

  libtea_flush_tlb(instance, address);
}


static void libtea__update_addr_user(libtea_instance* instance, void* address, pid_t pid, libtea_page_entry* vm) {
  libtea__update_addr_user_ext(instance, address, pid, vm, libtea__phys_write_pwrite);
  libtea_flush_tlb(instance, address);
}


#if LIBTEA_LINUX
static void libtea__update_addr_user_map(libtea_instance* instance, void* address, pid_t pid, libtea_page_entry* vm) {
  libtea__update_addr_user_ext(instance, address, pid, vm, libtea__phys_write_map);
  libtea_flush_tlb(instance, address);
}
#endif


uint64_t libtea__get_physical_base_address(libtea_page_entry entry, libtea_page_level level){
  return libtea__arch_get_physical_base_address(entry, level);
}


uint64_t libtea__get_virtual_address_index(libtea_page_entry entry, libtea_page_level level){
  return libtea__arch_get_virtual_address_index(entry, level);
}


#define LIBTEA_B(val, bit) (!!((val) & (1ull << (bit))))


int libtea__paging_init(libtea_instance* instance) {

  #if LIBTEA_LINUX
  libtea_set_paging_implementation(instance, LIBTEA_PAGING_IMPL_KERNEL);
  #else
  libtea_set_paging_implementation(instance, LIBTEA_PAGING_IMPL_USER_PREAD);
  #endif

  instance->pagesize = libtea_get_pagesize(instance);
  #if LIBTEA_AARCH64
  if(instance->pagesize == 4096*4){
    libtea_page_shift = 14;
  }
  else if(instance->pagesize == 4096*16){
    libtea_page_shift = 16;
  }
  #endif

  libtea__arch_get_paging_definitions(instance);
  return 0;
}


void libtea__cleanup_paging(libtea_instance* instance) {
  #if LIBTEA_LINUX
  if(instance->umem_fd > 0) close(instance->umem_fd);
  if(instance->mem_fd > 0) close(instance->mem_fd);
  #endif
}


/* Public API (plus libtea_update_addr and libtea_resolve_addr versions) */


void libtea_set_paging_implementation(libtea_instance* instance, int implementation) {
  if (implementation == LIBTEA_PAGING_IMPL_KERNEL) {

    #if LIBTEA_LINUX
    libtea_resolve_addr = libtea__resolve_addr_kernel;
    libtea_update_addr = libtea__update_addr_kernel;

    #else
    libtea_info("Error: Libtea kernel paging implementation not supported on Windows.");
    #endif

  }
  else if (implementation == LIBTEA_PAGING_IMPL_USER_PREAD) {
    libtea_resolve_addr = libtea__resolve_addr_user;
    libtea_update_addr = libtea__update_addr_user;
    instance->paging_root = libtea_get_paging_root(instance, 0);
  }
  else if (implementation == LIBTEA_PAGING_IMPL_USER) {

    #if LIBTEA_LINUX
    libtea_resolve_addr = libtea__resolve_addr_user_map;
    libtea_update_addr = libtea__update_addr_user_map;
    instance->paging_root = libtea_get_paging_root(instance, 0);
    if (!instance->vmem) {
      instance->vmem = (unsigned char*)mmap(NULL, 32ull << 30ull, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_NORESERVE, instance->umem_fd, 0);
      libtea_info("Mapped physical memory to %p.", instance->vmem);
    }

    #else
    libtea_info("Error: Libtea user paging implementation not supported on Windows.");
    #endif

  }
  else {
    libtea_info("Error: invalid Libtea paging implementation.");
  }
}


void libtea_set_addr_page_bit(libtea_instance* instance, void* address, pid_t pid, int bit) {
  libtea_page_entry vm = libtea_resolve_addr(instance, address, pid);
  if (!(vm.valid & LIBTEA_VALID_MASK_PTE)) return;
  vm.pte |= (1ull << bit);
  vm.valid = LIBTEA_VALID_MASK_PTE;
  libtea_update_addr(instance, address, pid, &vm);
}


void libtea_clear_addr_page_bit(libtea_instance* instance, void* address, pid_t pid, int bit) {
  libtea_page_entry vm = libtea_resolve_addr(instance, address, pid);
  if (!(vm.valid & LIBTEA_VALID_MASK_PTE)) return;
  vm.pte &= ~(1ull << bit);
  vm.valid = LIBTEA_VALID_MASK_PTE;
  libtea_update_addr(instance, address, pid, &vm);
}


int libtea_mark_page_present(libtea_instance* instance, void* page, int prot) {
  #if LIBTEA_LINUX && LIBTEA_X86
  libtea_page_entry vm = libtea_resolve_addr(instance, page, 0);
  vm.pte |= ~(1ull << LIBTEA_PAGE_BIT_PRESENT);
  vm.valid = LIBTEA_VALID_MASK_PTE;
  //Must use mprotect so Linux is aware we unmapped the page (then restore unmitigated PTE) - otherwise system will crash
  if(mprotect((void*) (((uint64_t) page) & ~LIBTEA_PFN_MASK), 4096, prot) != 0) {
    return LIBTEA_ERROR;
  }
  libtea_update_addr(instance, page, 0, &vm);
  return LIBTEA_SUCCESS;
  #elif LIBTEA_LINUX
  libtea_info("libtea_mark_page_present is only supported on x86!");
  return LIBTEA_ERROR;
  #else
  NO_WINDOWS_SUPPORT;
  return LIBTEA_ERROR;
  #endif
}


int libtea_mark_page_not_present(libtea_instance* instance, void* page) {
  #if LIBTEA_LINUX && LIBTEA_X86
  libtea_page_entry vm = libtea_resolve_addr(instance, page, 0);
  vm.pte &= ~(1ull << LIBTEA_PAGE_BIT_PRESENT);
  vm.valid = LIBTEA_VALID_MASK_PTE;
  //Must use mprotect so Linux is aware we unmapped the page (then restore unmitigated PTE) - otherwise system will crash
  if(mprotect((void*) (((uint64_t) page) & ~LIBTEA_PFN_MASK), 4096, PROT_NONE) != 0) {
    return LIBTEA_ERROR;
  }
  libtea_update_addr(instance, page, 0, &vm);
  return LIBTEA_SUCCESS;
  #elif LIBTEA_LINUX
  libtea_info("libtea_mark_page_not_present is only supported on x86!");
  return LIBTEA_ERROR;
  #else
  NO_WINDOWS_SUPPORT;
  return LIBTEA_ERROR;
  #endif
}


unsigned char libtea_get_addr_page_bit(libtea_instance* instance, void* address, pid_t pid, int bit) {
  libtea_page_entry vm = libtea_resolve_addr(instance, address, pid);
  return !!(vm.pte & (1ull << bit));
}


size_t libtea_get_addr_pfn(libtea_instance* instance, void* address, pid_t pid) {
  libtea_page_entry vm = libtea_resolve_addr(instance, address, pid);
  if (!(vm.valid & LIBTEA_VALID_MASK_PTE)) return 0;
  else return libtea_get_pfn(vm.pte);
}


void libtea_set_addr_pfn(libtea_instance* instance, void* address, pid_t pid, size_t pfn) {
  libtea_page_entry vm = libtea_resolve_addr(instance, address, pid);
  if (!(vm.valid & LIBTEA_VALID_MASK_PTE)) return;
  vm.pte = libtea_set_pfn(vm.pte, pfn);
  vm.valid = LIBTEA_VALID_MASK_PTE;
  libtea_update_addr(instance, address, pid, &vm);
}


int libtea_get_pagesize(libtea_instance* instance) {

  #if LIBTEA_LINUX
  return getpagesize();

  #else
  SYSTEM_INFO sysinfo;
  GetSystemInfo(&sysinfo);
  return sysinfo.dwPageSize;
  #endif

}


size_t libtea_set_pfn(size_t pte, size_t pfn) {
  pte &= libtea__arch_set_pfn();
  pte |= pfn << 12;
  return pte;
}


size_t libtea_get_pfn(size_t pte) {
  return libtea__arch_get_pfn(pte);
}


void libtea_read_physical_page(libtea_instance* instance, size_t pfn, char* buffer) {
  #if LIBTEA_LINUX
  if (instance->umem_fd > 0) {
    pread(instance->umem_fd, buffer, instance->pagesize, pfn * instance->pagesize);
  }
  else {
    libtea_physical_page page;
    page.buffer = (unsigned char*)buffer;
    page.pfn = pfn;
    ioctl(instance->module_fd, LIBTEA_IOCTL_READ_PAGE, (size_t)&page);
  }
  #else
  DWORD returnLength;
  pfn *= instance->pagesize;
  DeviceIoControl(instance->module_fd, LIBTEA_IOCTL_READ_PAGE, (LPVOID)&pfn, sizeof(pfn), (LPVOID)buffer, 4096, &returnLength, 0);
  #endif
}


void libtea_write_physical_page(libtea_instance* instance, size_t pfn, char* content) {
  #if LIBTEA_LINUX
  if (instance->umem_fd > 0) {
    pwrite(instance->umem_fd, content, instance->pagesize, pfn * instance->pagesize);
  }
  else {
    libtea_physical_page page;
    page.buffer = (unsigned char*)content;
    page.pfn = pfn;
    ioctl(instance->module_fd, LIBTEA_IOCTL_WRITE_PAGE, (size_t)&page);
  }
  #else
  DWORD returnLength;
  libtea_physical_page page;
  if (instance->pagesize != 4096) {
    libtea_info("Error: page sizes other than 4096 not supported for Libtea paging on Windows.");
    return;
  }
  page.paddr = pfn * instance->pagesize;
  memcpy(page.content, content, instance->pagesize);
  DeviceIoControl(instance->module_fd, LIBTEA_IOCTL_WRITE_PAGE, (LPVOID)&page, sizeof(libtea_physical_page), (LPVOID)&page, sizeof(libtea_physical_page), &returnLength, 0);
  #endif
}


void* libtea_map_physical_address_range(libtea_instance* instance, size_t paddr, size_t length, int prot, bool use_dev_mem) {

  #if LIBTEA_LINUX
  size_t pfn = (paddr & ~LIBTEA_PFN_MASK);
  //TODO query PAT errors when trying to switch to LIBTEA_PAGING_IMPL_USER
  int fd = use_dev_mem ? instance->mem_fd : instance->umem_fd;

  char* map = (char*) mmap(0, length, prot, MAP_SHARED, fd, pfn);
  if (map == MAP_FAILED) {
    libtea_info("Error in libtea_map_physical_address_range, mmap errno %d", errno);
    return map;
  }
  uintptr_t vaddr = ((uintptr_t) map) | (paddr & LIBTEA_PFN_MASK);
  return (void*) vaddr;

  #else
  NO_WINDOWS_SUPPORT;
  return NULL;
  #endif

}

int libtea_unmap_address_range(size_t vaddr, size_t length){

  #if LIBTEA_LINUX	
  void* unmap_addr = (void*)(((uintptr_t) vaddr) & ~LIBTEA_PFN_MASK);
  if(munmap(unmap_addr, length)){
    libtea_info("Munmap failed in libtea_unmap_address_range, errno is %d. Tried to unmap memory range of size %zu at %p", errno, length, unmap_addr);
    return LIBTEA_ERROR;
  }
  else {
    return LIBTEA_SUCCESS;
  }
  
  #else
  NO_WINDOWS_SUPPORT;
  return LIBTEA_ERROR;	  
  #endif
}


size_t libtea_get_paging_root(libtea_instance* instance, pid_t pid) {

  #if LIBTEA_LINUX
  libtea_paging_root cr3;
  cr3.pid = (size_t)pid;
  cr3.root = 0;
  ioctl(instance->module_fd, LIBTEA_IOCTL_GET_ROOT, (size_t)&cr3);
  return cr3.root;

  #else
  size_t cr3 = 0;
  DWORD returnLength;
  if(!pid) pid = GetCurrentProcessId();
  DeviceIoControl(instance->module_fd, LIBTEA_IOCTL_GET_CR3, (LPVOID)&pid, sizeof(pid), (LPVOID)&cr3, sizeof(cr3), &returnLength, 0);
  return (cr3 & ~0xfff);
  #endif

}


void libtea_set_paging_root(libtea_instance* instance, pid_t pid, size_t root) {
  libtea_paging_root cr3;
  cr3.pid = (size_t)pid;
  cr3.root = root;

  #if LIBTEA_LINUX
  ioctl(instance->module_fd, LIBTEA_IOCTL_SET_ROOT, (size_t)&cr3);

  #else
  DWORD returnLength;
  if (!pid) pid = GetCurrentProcessId();
  size_t info[2];
  info[0] = pid;
  info[1] = root;
  DeviceIoControl(instance->module_fd, LIBTEA_IOCTL_SET_CR3, (LPVOID)info, sizeof(info), (LPVOID)info, sizeof(info), &returnLength, 0);
  #endif
}


void libtea_flush_tlb(libtea_instance* instance, void* address) {

  #if LIBTEA_LINUX
  ioctl(instance->module_fd, LIBTEA_IOCTL_FLUSH_TLB, (size_t)address);

  #else
  size_t vaddr = (size_t)address;
  DWORD returnLength;
  DeviceIoControl(instance->module_fd, LIBTEA_IOCTL_FLUSH_TLB, (LPVOID)&vaddr, sizeof(vaddr), (LPVOID)&vaddr, sizeof(vaddr), &returnLength, 0);
  #endif

}


void libtea_paging_barrier(libtea_instance* instance) {
  libtea__arch_speculation_barrier();
  libtea_set_paging_root(instance, 0, libtea_get_paging_root(instance, 0));
  libtea__arch_speculation_barrier();
}


int libtea_switch_flush_tlb_implementation(libtea_instance* instance, int implementation) {
  #ifdef LIBTEA_LINUX
  if(ioctl(instance->module_fd, LIBTEA_IOCTL_SWITCH_FLUSH_TLB_IMPLEMENTATION, (size_t) implementation) != 0) {
    return LIBTEA_SUCCESS;
  }
  else{
    return LIBTEA_ERROR;
  }
  #else
  NO_WINDOWS_SUPPORT;
  return LIBTEA_ERROR;
  #endif
}


size_t libtea_get_memory_types(libtea_instance* instance) {
  size_t mt = 0;

  #if LIBTEA_LINUX
  ioctl(instance->module_fd, LIBTEA_IOCTL_GET_PAT, (size_t)&mt);

  #else
  DWORD returnLength;
  DeviceIoControl(instance->module_fd, LIBTEA_IOCTL_GET_PAT, (LPVOID)&mt, sizeof(mt), (LPVOID)&mt, sizeof(mt), &returnLength, 0);
  #endif

  return mt;
}


void libtea_set_memory_types(libtea_instance* instance, size_t mts) {
  #if LIBTEA_LINUX
  ioctl(instance->module_fd, LIBTEA_IOCTL_SET_PAT, mts);

  #else
  DWORD returnLength;
  DeviceIoControl(instance->module_fd, LIBTEA_IOCTL_GET_PAT, (LPVOID)&mts, sizeof(mts), (LPVOID)&mts, sizeof(mts), &returnLength, 0);
  #endif

}


char libtea_get_memory_type(libtea_instance* instance, unsigned char mt) {
  size_t mts = libtea_get_memory_types(instance);
  return libtea__arch_get_mt(mts, mt);
}


void libtea_set_memory_type(libtea_instance* instance, unsigned char mt, unsigned char value) {
  size_t mts = libtea_get_memory_types(instance);
  mts &= libtea__arch_set_mt(mt);
  mts |= ((size_t)value << (mt * 8));
  libtea_set_memory_types(instance, mts);
}


unsigned char libtea_find_memory_type(libtea_instance* instance, unsigned char type) {
  size_t mts = libtea_get_memory_types(instance);
  return libtea__arch_find_mt(mts, type);
}


int libtea_find_first_memory_type(libtea_instance* instance, unsigned char type) {

  #if LIBTEA_LINUX
  return __builtin_ffs(libtea_find_memory_type(instance, type)) - 1;

  #else
  DWORD index = 0;
  if (BitScanForward64(&index, libtea_find_memory_type(instance, type))) {
    return index;
  }
  else {
    return -1;
  }
  #endif

}


size_t libtea_apply_memory_type(size_t entry, unsigned char mt) {
  return libtea__arch_apply_mt(entry, mt);
}


int libtea_set_page_cacheability(libtea_instance* instance, void* page, unsigned char type) {
  libtea_page_entry entry = libtea_resolve_addr(instance, page, 0);
  int available_mt = libtea_find_first_memory_type(instance, type);
  if (available_mt == -1) {
    return LIBTEA_ERROR;
  }
  entry.pte = libtea_apply_memory_type(entry.pte, available_mt);
  entry.valid = LIBTEA_VALID_MASK_PTE;
  libtea_update_addr(instance, page, 0, &entry);
  return LIBTEA_SUCCESS;
}


unsigned char libtea_extract_memory_type(size_t entry) {
  return libtea__arch_extract_mt(entry);
}


const char* libtea_memory_type_to_string(unsigned char mt) {
  return libtea__arch_mt_to_string(mt);
}


void libtea_print_libtea_page_entry(libtea_page_entry entry) {
  if (entry.valid & LIBTEA_VALID_MASK_PGD) {
    printf("PGD of address\n");
    libtea_print_page_entry(entry.pgd);
  }
  if (entry.valid & LIBTEA_VALID_MASK_P4D) {
    printf("P4D of address\n");
    libtea_print_page_entry(entry.p4d);
  }
  if (entry.valid & LIBTEA_VALID_MASK_PUD) {
    printf("PUD of address\n");
    libtea_print_page_entry(entry.pud);
  }
  if (entry.valid & LIBTEA_VALID_MASK_PMD) {
    printf("PMD of address\n");
    libtea_print_page_entry(entry.pmd);
  }
  if (entry.valid & LIBTEA_VALID_MASK_PTE) {
    printf("PTE of address\n");
    libtea_print_page_entry(entry.pte);
  }
}


#define libtea_paging_print_bit(fmt, bit)                                            \
  printf((fmt), (bit));                                                                \
  printf("|");


void libtea_print_page_entry(size_t entry) {
  for (int i = 0; i < 4; i++) {
    libtea_print_page_entry_line(entry, i);
  }
}


void* libtea_remap_address(libtea_instance* instance, size_t vaddr, libtea_page_level level, size_t length, int prot, bool use_dev_mem){

  #if LIBTEA_LINUX
  size_t paddr = libtea_get_physical_address_at_level(instance, vaddr, level);
  void* new_mapping = libtea_map_physical_address_range(instance, paddr, length, prot, use_dev_mem);
  return new_mapping;

  #else
  NO_WINDOWS_SUPPORT;
  return NULL;
  #endif
}


uint64_t libtea_get_physical_address_width(){

  #if LIBTEA_INLINEASM && LIBTEA_X86
	uint32_t eax, ebx, ecx, edx;
	static uint64_t width = 0;
	
	//Cache the result to avoid VM exits from CPUID
	if(width == 0){
		eax = 0x80000008;
		ebx = 0;
		ecx = 0;
		edx = 0;
	  asm volatile ("cpuid" : "=a" (eax), "=b" (ebx), "=c" (ecx), "=d" (edx) : "a" (eax), "b" (ebx), "c" (ecx), "d" (edx));
		width = (eax & 0xff);
	}
	return width;
  
  #else
  //TODO. is this also Intel only? AMD cpuid parsing is often different
  libtea_info("Error: libtea_get_physical_address_width is only supported on x86 and requires compiler support for inline assembly.");
  return 0;
  #endif

}


size_t libtea_get_physical_address_at_level(libtea_instance* instance, size_t vaddr, libtea_page_level level){

  #if LIBTEA_LINUX
  libtea_page_entry entry = libtea_resolve_addr(instance, (void*)vaddr, 0);
  uint64_t base = libtea__get_physical_base_address(entry, level);
  uint64_t index = libtea__get_virtual_address_index(entry, level);
  if(level == LIBTEA_PAGE){
    return base + index;
  }
  else {
    return base + index * 8;
  }

  #else
  NO_WINDOWS_SUPPORT;
  return LIBTEA_ERROR;
  #endif
}


//Code adapted from the WindowsInternals demos at https://github.com/zodiacon/WindowsInternals/blob/master/MemCombine/MemCombine.cpp
//Note: in order to compile this with MinGW GCC, you need to manually link in NTDLL with -lntdll flag
#if LIBTEA_WINDOWS
#pragma comment(lib, "ntdll")
#include <Winternl.h>

#ifdef __cplusplus
extern "C" NTSTATUS NTAPI RtlAdjustPrivilege(ULONG Privilege, BOOLEAN Enable, BOOLEAN Client, PBOOLEAN WasEnabled);
extern "C" NTSTATUS NTAPI NtSetSystemInformation(SYSTEM_INFORMATION_CLASS SystemInformationClass, PVOID SystemInformation, ULONG SystemInformationLength);
#else
extern NTSTATUS NTAPI RtlAdjustPrivilege(ULONG Privilege, BOOLEAN Enable, BOOLEAN Client, PBOOLEAN WasEnabled);
extern NTSTATUS NTAPI NtSetSystemInformation(SYSTEM_INFORMATION_CLASS SystemInformationClass, PVOID SystemInformation, ULONG SystemInformationLength);
#endif

typedef struct {
	HANDLE Handle;
	ULONG_PTR PagesCombined;
	ULONG Flags;
} libtea_memory_combine_information_ex;
#endif


#if LIBTEA_ENABLE_WINDOWS_MEMORY_DEDUPLICATION
long long libtea_force_memory_deduplication(){

  #if LIBTEA_WINDOWS
  BOOLEAN enabled;
  /* Request SE_PROF_SINGLE_PROCESS_PRIVILEGE == 13L */
  int status = RtlAdjustPrivilege(13L, true, false, &enabled);
  if(status != 0) {
    libtea_info("Could not obtain SE_PROF_SINGLE_PROCESS_PRIVILEGE in libtea_force_memory_deduplication: status %d.\n", status);
  }
  libtea_memory_combine_information_ex info = {0};
  /* Currently don't offer option to just combine "common pages" (pages which are all 0s or all 1s), but can implement this by setting info.Flags to 4 instead */
	status = NtSetSystemInformation((SYSTEM_INFORMATION_CLASS)130, &info, sizeof(info));
	if (status != 0) {
		libtea_info("Error calling NtSetSystemInformation in libtea_force_memory_deduplication: status %d\n", status);
		return status;
	}
	return (long long)info.PagesCombined;

  #else
  libtea_info("libtea_force_memory_deduplication is only supported on Windows.");
  return 0;
  #endif

}
#endif

/* See LICENSE file for license and copyright information */

/* Start libtea_x86_paging.c */
//---------------------------------------------------------------------------

#if LIBTEA_X86





libtea_inline void libtea__arch_print_page_entry_line(size_t entry, int line){
  if (line == 0 || line == 3) printf("+--+------------------+-+-+-+-+-+-+-+-+--+--+-+-+-+\n");
  if (line == 1) printf("|NX|       PFN        |H|?|?|?|G|S|D|A|UC|WT|U|W|P|\n");
  if (line == 2) {
    printf("|");
    libtea_paging_print_bit(" %d", LIBTEA_B(entry, LIBTEA_PAGE_BIT_NX));
    printf(" %16p |", (void*)((entry >> 12) & ((1ull << 40) - 1)));
    libtea_paging_print_bit("%d", LIBTEA_B(entry, LIBTEA_PAGE_BIT_PAT_LARGE));
    libtea_paging_print_bit("%d", LIBTEA_B(entry, LIBTEA_PAGE_BIT_SOFTW3));
    libtea_paging_print_bit("%d", LIBTEA_B(entry, LIBTEA_PAGE_BIT_SOFTW2));
    libtea_paging_print_bit("%d", LIBTEA_B(entry, LIBTEA_PAGE_BIT_SOFTW1));
    libtea_paging_print_bit("%d", LIBTEA_B(entry, LIBTEA_PAGE_BIT_GLOBAL));
    libtea_paging_print_bit("%d", LIBTEA_B(entry, LIBTEA_PAGE_BIT_PSE));
    libtea_paging_print_bit("%d", LIBTEA_B(entry, LIBTEA_PAGE_BIT_DIRTY));
    libtea_paging_print_bit("%d", LIBTEA_B(entry, LIBTEA_PAGE_BIT_ACCESSED));
    libtea_paging_print_bit(" %d", LIBTEA_B(entry, LIBTEA_PAGE_BIT_PCD));
    libtea_paging_print_bit(" %d", LIBTEA_B(entry, LIBTEA_PAGE_BIT_PWT));
    libtea_paging_print_bit("%d", LIBTEA_B(entry, LIBTEA_PAGE_BIT_USER));
    libtea_paging_print_bit("%d", LIBTEA_B(entry, LIBTEA_PAGE_BIT_RW));
    libtea_paging_print_bit("%d", LIBTEA_B(entry, LIBTEA_PAGE_BIT_PRESENT));
    printf("\n");
  }
}


libtea_inline void libtea__arch_get_paging_definitions(libtea_instance* instance){
  instance->paging_definition.has_pgd = 1;
  instance->paging_definition.has_p4d = 0;
  instance->paging_definition.has_pud = 1;
  instance->paging_definition.has_pmd = 1;
  instance->paging_definition.has_pt = 1;
  instance->paging_definition.pgd_entries = 9;
  instance->paging_definition.p4d_entries = 0;
  instance->paging_definition.pud_entries = 9;
  instance->paging_definition.pmd_entries = 9;
  instance->paging_definition.pt_entries = 9;
  instance->paging_definition.page_offset = 12;
}


libtea_inline size_t libtea__arch_set_pfn(){
  return  ~(((1ull << 40) - 1) << 12);
}


libtea_inline size_t libtea__arch_get_pfn(size_t pte){
  return (pte & (((1ull << 40) - 1) << 12)) >> 12;
}


libtea_inline char libtea__arch_get_mt(size_t mts, unsigned char mt){
  return ((mts >> (mt * 8)) & 7);
}


libtea_inline const char* libtea__arch_mt_to_string(unsigned char mt) {
  const char* mts[] = { "UC", "WC", "Rsvd", "Rsvd", "WT", "WP", "WB", "UC-", "Rsvd" };
  if (mt <= 7) return mts[mt];
  return NULL;
}


libtea_inline size_t libtea__arch_set_mt(unsigned char mt){
  return ~(7 << (mt * 8));
}


libtea_inline unsigned char libtea__arch_find_mt(size_t mts, unsigned char type){
  unsigned char found = 0;
  int i;
  for (i = 0; i < 8; i++) {
    if (((mts >> (i * 8)) & 7) == type) found |= (1 << i);
  }
  return found;
}


libtea_inline size_t libtea__arch_apply_mt(size_t entry, unsigned char mt) {
  entry &= ~((1ull << LIBTEA_PAGE_BIT_PWT) | (1ull << LIBTEA_PAGE_BIT_PCD) | (1ull << LIBTEA_PAGE_BIT_PAT));
  if (mt & 1) entry |= (1ull << LIBTEA_PAGE_BIT_PWT);
  if (mt & 2) entry |= (1ull << LIBTEA_PAGE_BIT_PCD);
  if (mt & 4) entry |= (1ull << LIBTEA_PAGE_BIT_PAT);
  return entry;
}


libtea_inline unsigned char libtea__arch_extract_mt(size_t entry){
  return (!!(entry & (1ull << LIBTEA_PAGE_BIT_PWT))) | ((!!(entry & (1ull << LIBTEA_PAGE_BIT_PCD))) << 1) | ((!!(entry & (1ull << LIBTEA_PAGE_BIT_PAT))) << 2);
}


libtea_inline uint64_t libtea__arch_get_physical_base_address(libtea_page_entry entry, libtea_page_level level){
  switch(level){
    case LIBTEA_PGD:
      libtea_info("TODO not implemented yet, returning pgd here instead of pgd_phys_address");
      return entry.pgd;
    case LIBTEA_PUD:
      return LIBTEA_PGD_PHYS(entry.pgd);
    case LIBTEA_PMD:
      if(!LIBTEA_PUD_PS(entry.pud)){
        libtea_info("WARNING: PUD assertion failed in libtea_get_physical_base_address, PMD address returned will be wrong");
      }
      return LIBTEA_PUD_PS_0_PHYS(entry.pud);
    case LIBTEA_PTE:
      if(!LIBTEA_PUD_PS(entry.pud) && !LIBTEA_PMD_PS(entry.pmd)){
        libtea_info("WARNING: PUD or PMD assertion failed in libtea_get_physical_base_address, PTE address returned will be wrong");
      }
      return LIBTEA_PMD_PS_0_PHYS(entry.pmd);
    case LIBTEA_PAGE:
    //Intentional fall-through
    default:
      if(LIBTEA_PUD_PS(entry.pud)){
        return LIBTEA_PUD_PS_1_PHYS(entry.pud);
      }
      if(LIBTEA_PMD_PS(entry.pmd)){
        return LIBTEA_PMD_PS_1_PHYS(entry.pmd);
      }
      return LIBTEA_PT_PHYS(entry.pte);
  }
}


libtea_inline uint64_t libtea__arch_get_virtual_address_index(libtea_page_entry entry, libtea_page_level level){
  switch(level){
    case LIBTEA_PGD:
      return LIBTEA_PGD_INDEX(entry.vaddr);
    case LIBTEA_PUD:
      return LIBTEA_PUD_INDEX(entry.vaddr);
    case LIBTEA_PMD:
    {
      if(!LIBTEA_PUD_PS(entry.pud)){
        libtea_info("WARNING: PUD assertion failed in libtea_get_virtual_address_index, PMD address returned will be wrong");
      }
      return LIBTEA_PMD_INDEX(entry.vaddr);
    }
    case LIBTEA_PTE:
    {
      if(!LIBTEA_PUD_PS(entry.pud) && !LIBTEA_PMD_PS(entry.pmd)){
        libtea_info("WARNING: PUD or PMD assertion failed in libtea_get_virtual_address_index, PTE address returned will be wrong");
      }
      return LIBTEA_PTE_INDEX(entry.vaddr);
    }
    case LIBTEA_PAGE:
    //Intentional fall-through
    default:
    {
      if(LIBTEA_PUD_PS(entry.pud)){
        return LIBTEA_PAGE1GiB_INDEX(entry.vaddr);
      }
      else if(LIBTEA_PMD_PS(entry.pmd)){
        return LIBTEA_PAGE2MiB_INDEX(entry.vaddr);
      }
      else return LIBTEA_PAGE_INDEX(entry.vaddr);
    }
  }
}


#endif //LIBTEA_X86


/* End libtea_x86_paging.c */
//---------------------------------------------------------------------------
#endif //LIBTEA_H