#include <mbed.h>
#include "mbed_mem_trace.h"
#include "logger.h"

namespace GSH {

    class MemoryUtils 
    {
    public:
        static void print_memory_info() {
            // allocate enough room for every thread's stack statistics
            int cnt = osThreadGetCount();
            mbed_stats_stack_t *stats = (mbed_stats_stack_t*) malloc(cnt * sizeof(mbed_stats_stack_t));
        
            cnt = mbed_stats_stack_get_each(stats, cnt);
            for (int i = 0; i < cnt; i++) {
                GSH_DEBUG("Thread: 0x%X, Stack size: %u / %u\r\n", stats[i].thread_id, stats[i].max_size, stats[i].reserved_size);
            }
            free(stats);
        
            // Grab the heap statistics
            mbed_stats_heap_t heap_stats;
            mbed_stats_heap_get(&heap_stats);
            GSH_DEBUG("Heap size: %u / %u bytes\r\n", heap_stats.current_size, heap_stats.reserved_size);
        }
    };

}