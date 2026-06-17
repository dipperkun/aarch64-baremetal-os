#include "mmu.h"
#include "types.h"
#include "uart.h"

#define VIRT_KERNEL_BASE 0xffff000000000000UL
#define PHYS_KERNEL_BASE 0x40000000UL
#define KERNEL_SIZE      (128 * 1024 * 1024)  /* 128MB */

/* Level 3 page table entries (4KB pages) */
#define PTE_VALID        (1UL << 0)
#define PTE_TYPE_PAGE    (3UL << 0)  /* Page descriptor */
#define PTE_ATTR_IDX(x)  ((x) << 2)
#define PTE_NS           (1UL << 5)  /* Non-secure */
#define PTE_AP_RW_EL1    (0UL << 6)  /* Read/write at EL1 */
#define PTE_AP_RO_EL1    (2UL << 6)  /* Read-only at EL1 */
#define PTE_SH_INNER     (3UL << 8)  /* Shareable */
#define PTE_AF           (1UL << 10) /* Access flag */
#define PTE_NG           (1UL << 11) /* Not global */

/* MAIR indices */
#define MAIR_DEVICE_NGNRNE  0  /* Device nGnRnE */
#define MAIR_DEVICE_NGNRE   1  /* Device nGnRE */
#define MAIR_DEVICE_GRE     2  /* Device GRE */
#define MAIR_NORMAL_NC      3  /* Normal non-cacheable */
#define MAIR_NORMAL         4  /* Normal cacheable */

/* Allocate page tables - statically allocated for simplicity */
#define MAX_TABLES 512
static uint64_t page_tables[MAX_TABLES][512] __attribute__((aligned(4096)));
static int table_count = 0;

static uint64_t *alloc_table(void)
{
    if (table_count >= MAX_TABLES) {
        uart_puts("[MMU] ERROR: Out of page tables\n");
        return NULL;
    }
    
    uint64_t *table = page_tables[table_count++];
    
    /* Clear table */
    for (int i = 0; i < 512; i++) {
        table[i] = 0;
    }
    
    return table;
}

void mmu_init(void)
{
    /* Allocate root page table (PGD) */
    uint64_t *pgd = alloc_table();
    uint64_t pgd_pa = (uint64_t)pgd;  /* Physical address */
    
    /* Map kernel: 0xffff000000000000 -> 0x40000000 */
    /* For 4-level paging at 48-bit VA space:
     * Level 0: PGD  (covers 512GB)
     * Level 1: PUD  (covers 1GB)
     * Level 2: PMD  (covers 2MB)
     * Level 3: PTE  (covers 4KB)
     */
    
    /* Get PGD entry index for kernel VA */
    uint64_t va_index = (VIRT_KERNEL_BASE >> 39) & 0x1ff;
    
    /* Allocate and setup level 1 table (PUD) */
    uint64_t *pud = alloc_table();
    uint64_t pud_entry = (uint64_t)pud | PTE_VALID | (3UL << 0);
    pgd[va_index] = pud_entry;
    
    /* Setup level 2 entries (2MB chunks) */
    for (uint64_t i = 0; i < (KERNEL_SIZE / (2 * 1024 * 1024)); i++) {
        uint64_t pmd_va_index = ((VIRT_KERNEL_BASE >> 21) & 0x1ff) + i;
        uint64_t pmd_entry = (PHYS_KERNEL_BASE + (i * 2 * 1024 * 1024)) |
                             PTE_VALID | (1UL << 0) |  /* Block descriptor */
                             PTE_ATTR_IDX(MAIR_NORMAL) |
                             PTE_AP_RW_EL1 |
                             PTE_SH_INNER |
                             PTE_AF;
        
        /* Get or create PMD table */
        uint64_t pud_index = (pmd_va_index >> 9) & 0x1ff;
        if (!(pud[pud_index] & PTE_VALID)) {
            uint64_t *pmd = alloc_table();
            pud[pud_index] = (uint64_t)pmd | PTE_VALID | (3UL << 0);
        }
        
        uint64_t *pmd = (uint64_t *)(pud[pud_index] & ~0xfffUL);
        uint64_t pmd_entry_index = pmd_va_index & 0x1ff;
        pmd[pmd_entry_index] = pmd_entry;
    }
    
    /* Setup MAIR (Memory Attribute Indirection Register) */
    uint64_t mair_el1 = 0;
    mair_el1 |= (0x00UL << (MAIR_DEVICE_NGNRNE * 8));  /* Device nGnRnE */
    mair_el1 |= (0x04UL << (MAIR_DEVICE_NGNRE * 8));   /* Device nGnRE */
    mair_el1 |= (0x0cUL << (MAIR_DEVICE_GRE * 8));     /* Device GRE */
    mair_el1 |= (0x44UL << (MAIR_NORMAL_NC * 8));      /* Normal NC */
    mair_el1 |= (0xffUL << (MAIR_NORMAL * 8));         /* Normal WB */
    asm volatile("msr mair_el1, %0" : : "r"(mair_el1));
    
    /* Setup TCR (Translation Control Register) */
    uint64_t tcr_el1 = 0;
    tcr_el1 |= (16UL << 0);   /* T0SZ = 16 (48-bit VA) */
    tcr_el1 |= (2UL << 14);   /* TG0 = 4KB pages */
    tcr_el1 |= (3UL << 12);   /* SH0 = Inner shareable */
    tcr_el1 |= (1UL << 10);   /* ORGN0 = Write-back */
    tcr_el1 |= (1UL << 8);    /* IRGN0 = Write-back */
    tcr_el1 |= (1UL << 37);   /* EPD1 = Disable TTBR1 lookups */
    asm volatile("msr tcr_el1, %0" : : "r"(tcr_el1));
    
    /* Enable MMU: Set bit 0 of SCTLR_EL1 */
    uint64_t sctlr_el1;
    asm volatile("mrs %0, sctlr_el1" : "=r"(sctlr_el1));
    sctlr_el1 |= 1;  /* Enable bit */
    sctlr_el1 |= (1 << 2);  /* Data cache */
    sctlr_el1 |= (1 << 12); /* Instruction cache */
    asm volatile("msr sctlr_el1, %0" : : "r"(sctlr_el1));
    
    /* Install TTBR0 */
    asm volatile("msr ttbr0_el1, %0" : : "r"(pgd_pa));
    
    /* TLB invalidation */
    asm volatile("tlbi vmalle1is");
    asm volatile("dsb sy");
    asm volatile("isb");
    
    uart_puts("[MMU] Virtual memory enabled (48-bit VA space)\n");
}

void mmu_map_page(uint64_t vaddr, uint64_t paddr, uint32_t flags)
{
    /* TODO: Dynamic page mapping */
    (void)vaddr;
    (void)paddr;
    (void)flags;
}
