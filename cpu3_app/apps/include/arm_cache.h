#ifndef ARM_CACHE_H_ 
#define ARM_CACHE_H_

int arm_dcache_state_query(void);
void arm_dcache_enable(void);
void arm_dcache_disable(void);
void arm_dcache_invalidate(void);
void arm_dcache_invalidate_line(const void * addr);
void arm_dcache_invalidate_mlines(const void * addr, size_t length);
void arm_dcache_flush(void);
void arm_dcache_flush_line(const void * addr);
void arm_dcache_flush_mlines(const void * addr, size_t length);
int arm_icache_state_query(void);
void arm_icache_enable(void);
void arm_icache_disable(void);
void arm_icache_invalidate(void);
void arm_icache_invalidate_is(void);
void arm_icache_invalidate_line(const void * addr);
void arm_icache_invalidate_mlines(const void * addr, size_t length);

#endif
