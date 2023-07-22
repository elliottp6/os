
    ; write level 2 pagetable
    lea eax, [es:di + 0x2000] ; put address of PD into EAX
    or eax, PAGE_FLAG_PRESENT | PAGE_FLAG_WRITE
    mov [es:di + 0x1000], eax ; store value of EAX into first PDPT
 
    ; write level 1 pagetable
    lea eax, [es:di + 0x3000] ; put address of PT into EAX
    or eax, PAGE_FLAG_PRESENT | PAGE_FLAG_WRITE
    mov [es:di + 0x2000], eax ; store value of EAX into first PDE

    ; write level 0 pagetable
    ; note that if we're using 2MB pages, then this will actually write the level 1 pagetable (and we won't need the full 16KB, just 12KB)
    push di ; save DI
    lea di, [di + PT_OFFSET] ;  point DI to the PT_OFFSET
    mov eax, PAGE_FLAG_PRESENT | PAGE_FLAG_WRITE | PAGE_FLAG_HUGE; EAX starts pointing to memory address 0 w/ flags
.page_table_loop:
    mov [es:di], eax ; first page table entry points to memory address 0
    add eax, PAGE_SIZE ; next one points to PAGE_SIZE later physical address
    add di, 8 ; each page table entry is an 8 byte pointer
    cmp eax, PAGE_SIZE * 512 ; stop @ 512 pages
    jb .page_table_loop ; jump if eax is below 2MB
    pop di ; restore DI
    ret
    
