#include "idt.h"
#include "lib.h"

/*
 * divide_by_zero()
 *   DESCRIPTION: Initializes divide by zero exception
 *   SIDE EFFECTS: Will be called when divide by zero occurs
 */
void divide_by_zero(){
    clear();
    printf("Exception - Divide by Zero \n");
    while(1){}
}

/*
 * debug()
 *   DESCRIPTION: Initializes debug exception
 *   SIDE EFFECTS: Will be called when debug exception occurs
 */
void debug(){
    clear();
    printf("Exception - Debug \n");
    while(1){}
}

/*
 * NMI_Interrupt()
 *   DESCRIPTION: Initializes NMI_Interrupt exception
 *   SIDE EFFECTS: Will be called when NMI_Interrupt exception occurs
 */
void NMI_Interrupt(){
    clear();                  
    printf("NMI \n");
    while(1){}
}

/*
 * Breakpoint()
 *   DESCRIPTION: Initializes Breakpoint exception
 *   SIDE EFFECTS: Will be called when Breakpoint exception occurs
 */
void Breakpoint(){
    clear();
    printf("Breakpoint \n");
    while(1){}
}

/*
 * Overflow()
 *   DESCRIPTION: Initializes overflow exception
 *   SIDE EFFECTS: Will be called when overflow exception occurs
 */
void Overflow(){
    clear();
    printf("Overflow \n");
    while(1){}
}

/*
 * Bounds_range_exceeded()
 *   DESCRIPTION: Initializes Bounds Range Exceeded exception
 *   SIDE EFFECTS: Will be called when Bounds Range Exceeded exception occurs
 */
void Bounds_range_exceeded(){
    clear();
    printf("Bounds Range Exceeded \n");
    while(1){}
}

/*
 * Invalid_opcode()
 *   DESCRIPTION: Initializes Invalid Opcode exception
 *   SIDE EFFECTS: Will be called when Invalid Opcode exception occurs
 */
void Invalid_opcode(){
    clear();
    printf("Invalid Opcode \n");
    while(1){}
}

/*
 * Device_not_avaliable()
 *   DESCRIPTION: Initializes Device Not Avaliable exception
 *   SIDE EFFECTS: Will be called when Device Not Avaliable exception occurs
 */
void Device_not_avaliable(){
    clear();
    printf("Device Not Avaliable \n");
    while(1){}
}

/*
 * Double_fault()
 *   DESCRIPTION: Initializes Double Fault exception
 *   SIDE EFFECTS: Will be called when Double Fault exception occurs
 */
void Double_fault(){
    clear();
    printf("Double Fault \n");
    while(1){}
}

/*
 * Coprocessor_segment_overrun()
 *   DESCRIPTION: Initializes Coprocessor Segment Overrun exception
 *   SIDE EFFECTS: Will be called when Coprocessor Segment Overrun occurs
 */
void Coprocessor_segment_overrun(){
    clear();
    printf("Coprocessor Segment Overrun \n");
    while(1){}
}

/*
 * Invalid_TSS()
 *   DESCRIPTION: Initializes Invalid TSS exception
 *   SIDE EFFECTS: Will be called when Invalid TSS exception occurs
 */
void Invalid_TSS(){
    clear();
    printf("Invalid TSS \n");
    while(1){}
}

/*
 * Segment_not_present()
 *   DESCRIPTION: Initializes Segment Not Present exception
 *   SIDE EFFECTS: Will be called when Segment Not Present exception occurs
 */
void Segment_not_present(){
    clear();
    printf("Segment Not Present \n");
    while(1){}
}

/*
 * Stack_segment_present()
 *   DESCRIPTION: Initializes Stack Segment Fault exception
 *   SIDE EFFECTS: Will be called when Stack Segment Fault exception occurs
 */
void Stack_segment_present(){
    clear();
    printf("Stack Segment Fault \n");
    while(1){}
}

/*
 * General_protection_fault
 *   DESCRIPTION: Initializes General Protection Fault exception
 *   SIDE EFFECTS: Will be called when General Protection Fault exception occurs
 */
void General_protection_fault(){
    clear();
    printf("General Protection Fault \n");
    while(1){}
}

/*
 * Page_fault()
 *   DESCRIPTION: Initializes Page Fault exception
 *   SIDE EFFECTS: Will be called when Page Fault exception occurs
 */
void Page_fault(){
    //clear();
    printf("Page Fault \n");
    while(1){}
}

/*
 * x87_FPU_error()
 *   DESCRIPTION: Initializes x87 FPU Error exception
 *   SIDE EFFECTS: Will be called when x87 FPU Error exception occurs
 */
void x87_FPU_error(){
    clear();
    printf("x87 FPU Error \n");
    while(1){}
}   

/*
 * Alignment_check()
 *   DESCRIPTION: Initializes Alignment Check exception
 *   SIDE EFFECTS: Will be called when Alignment Check exception occurs
 */
void Alignment_check(){
    clear();
    printf("Alignment Check \n");
    while(1){}
}

/*
 * Machine_check()
 *   DESCRIPTION: Initializes Machine Check exception
 *   SIDE EFFECTS: Will be called when Machine Check exception occurs
 */
void Machine_check(){
    clear();
    printf("Machine Check \n");
    while(1){}
}

/*
 * SIMD_Floating_Point_Exception()
 *   DESCRIPTION: Initializes SIMD Floating Point Exception
 *   SIDE EFFECTS: Will be called when SIMD Floating Point Exception occurs
 */
void SIMD_Floating_Point_Exception(){
    clear();
    printf("SIMD Floating Point Exception \n");
    while(1){}
}

/*
 * Virtualization_Exception
 *   DESCRIPTION: Initializes Virtualization Exception
 *   SIDE EFFECTS: Will be called when Virtualization Exception occurs
 */
void Virtualization_Exception() {
    clear();
    printf("Virtualization Exception\n");
    while(1);
}

/*
 * Control_Protection_Exception()
 *   DESCRIPTION: Initializes Control_Protection_Exception
 *   SIDE EFFECTS: Will be called when Control_Protection_Exception occurs
 */
void Control_Protection_Exception() {
    clear();
    printf("Control_Protection_Exception\n");
    while(1);
}

/*
 * Hypervisor_Injection_Exception()
 *   DESCRIPTION: Initializes Hypervisor_Injection_Exception
 *   SIDE EFFECTS: Will be called when Hypervisor_Injection_Exception occurs
 */
void Hypervisor_Injection_Exception() {
    clear();
    printf("Hypervisor_Injection_Exception\n");
    while(1);
}

/*
 * VMM_Communication_Exception()
 *   DESCRIPTION: Initializes VMM_Communication_Exception
 *   SIDE EFFECTS: Will be called when VMM_Communication_Exception occurs
 */
void VMM_Communication_Exception() {
    clear();
    printf("VMM_Communication_Exception\n");
    while(1);
}

/*
 * Security_Exception()
 *   DESCRIPTION: Initializes Security_Exception
 *   SIDE EFFECTS: Will be called when Security_Exception occurs
 */
void Security_Exception() {
    clear();
    printf("Security_Exception\n");
    while(1);
}



