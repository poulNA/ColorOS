#include "paging.h"

/*
 * paging_init
 *   DESCRIPTION: Initializes paging
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Fills in paging directory/page table and enables paging
 */
void paging_init() {
    int i;
    // Blank paging
    //set each entry to not present
    for(i = 0; i < ENTRIES; i++){
        // This sets the following flags to the pagings:
        //   Supervisor: Only kernel-mode can access them
        //   Not Present: The paging table is not present
        paging_directory[i].P = 0;
        paging_directory[i].RW = 0;
        paging_directory[i].US = 0;
        paging_directory[i].PWT = 0;
        paging_directory[i].PCD = 1;
        paging_directory[i].A = 0;
        paging_directory[i].avl = 0;
        paging_directory[i].PS = 1;
        paging_directory[i].G = 1;
        paging_directory[i].AVL = 0;
        paging_directory[i].index_31_12 = 0;
    }

     // 0-4mb (marked as present and 4KB pages)
    paging_directory[0].P = 1;
    paging_directory[0].RW = 1;
    paging_directory[0].US = 0;
    paging_directory[0].PWT = 0;
    paging_directory[0].PCD = 0;
    paging_directory[0].A = 0;
    paging_directory[0].avl = 0;
    paging_directory[0].PS = 0;
    paging_directory[0].G = 1;
    paging_directory[0].AVL = 0;
    paging_directory[0].index_31_12 = ((uint32_t)paging_table) >> 12; // paging table address is 20 bits long

    // 4-8mb (marked as present and 4 MB pages)
    paging_directory[1].P = 1;
    paging_directory[1].RW = 1;
    paging_directory[1].US = 0;
    paging_directory[1].PWT = 0;
    paging_directory[1].PCD = 1;
    paging_directory[1].A = 0;
    paging_directory[1].avl = 0;
    paging_directory[1].PS = 1;
    paging_directory[1].AVL = 0;
    paging_directory[1].G = 1;
    paging_directory[1].index_31_12 = 1 << 10; // paging table address is 10 bits long

    // holds the physical address where we want to start mapping these pagings to.
    // in this case, we want to map these pagings to the very beginning of memory.
 
    //we will fill all 1024 entries in the paging table, mapping 4 megabytes
    for(i = 0; i < ENTRIES; i++){
        // As the address is paging aligned, it will always leave 12 bits zeroed.
        // Those bits are used by the attributes ;)
        paging_table[i].P = 0;
        paging_table[i].RW = 0;
        paging_table[i].US = 0;
        paging_table[i].PWT = 0;
        paging_table[i].PCD = 1;
        paging_table[i].A = 0;
        paging_table[i].D = 0;
        paging_table[i].PAT = 0;
        paging_table[i].G = 1;
        paging_table[i].AVL = 0;
        paging_table[i].index_31_12 = 0;
    }

    // set the video memory page in the paging table to be present
    paging_table[VID_START].P = 1;
    paging_table[VID_START].RW = 1;
    paging_table[VID_START].US = 0;
    paging_table[VID_START].PWT = 0;
    paging_table[VID_START].PCD = 0;
    paging_table[VID_START].A = 0;
    paging_table[VID_START].D = 0;
    paging_table[VID_START].PAT = 0;
    paging_table[VID_START].G = 1;
    paging_table[VID_START].AVL = 0;
    paging_table[VID_START].index_31_12 = VID_START;

    // Load paging Directory
    loadPagingDirectory((unsigned int*)paging_directory);
    // Enable paging 
    enablePaging();
}
