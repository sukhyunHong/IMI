#ifndef __ASM_ISO_MODULE_H
#define __ASM_ISO_MODULE_H

typedef struct {
    unsigned long *ttbr;
    unsigned long sp_val;
    unsigned long asid;
} dom_data;

#endif /* __ASM_ISO_MODULE_H */