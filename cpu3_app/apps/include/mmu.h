#ifndef MMU_H_
#define MMU_H_

void mmu_table_init(void);
void Test_VirtualMMU(unsigned int va);
void mmu_init(void);
void mmu_enable(void);

#endif

