/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/*  Interrupt masks to determine which interrupts
 *  are enabled and disabled
 *  I set them to 0xFF here, because Initializing them inside the
 *  enable_irq function would mess up the masking
 *  It wouldn't function the way it's supposed to
 */
uint8_t master_mask = 0xFF; /* IRQs 0-7 */
uint8_t slave_mask = 0xFF; /* IRQs 8-15 */

/*
 * i8259_init
 *  DESCRIPTION:
 *          This function Initializes the correct bytes to the MASTER PIC
 *          And SLAVE PIC
 *
 *  INPUT: none
 *
 *  OUTPUT: none
 *  SOURCE: https://courses.engr.illinois.edu/ece391/secure/references/8259A_PIC_Datasheet.pdf
 *          Helpful pages: 9-12, 13-14
*/
void
i8259_init(void)
{
    /*
     *  After writing to ICW1, Interrupt Controller Initialization sequqnce begins
     *  Initialization sequence done on both the master and slave
     *  And upon sequence Initialization, 4 things happen(page 10-12)
     *  > Interrupt Mask register is cleared
     *  > Slave mode address is set to 7
     *  > Priority 7 is assgined to IRQ7 inpuit
     *  > Special mask mode is cleared
    */
    outb(ICW1, MASTER_8259_PORT);
    outb(ICW1, SLAVE_8259_PORT);

    /* First we wrote to ICW1, now we write to ICW2 */
    outb(ICW2_MASTER, MASTER_8259_PORT_DATA); /* 0x21 */
    outb(ICW2_SLAVE, SLAVE_8259_PORT_DATA);   /* 0xA1 */

    /* Now we write to ICW3 */
    outb(ICW3_MASTER, MASTER_8259_PORT_DATA);
    outb(ICW3_SLAVE, SLAVE_8259_PORT_DATA);

    /* Now we write to ICW4 */
    outb(ICW4, MASTER_8259_PORT_DATA);
    outb(ICW4, SLAVE_8259_PORT_DATA);

    /* enable the slave PIC */
    enable_irq(SLAVE_IRQ2);
}

/*
 * enable_irq
 *  DESCRIPTION:
 *          This function is responsbile for enabling(unmask) the specified IRQ.
 *          Function sets the ACTIVE LOW IRQ.
 *
 *  INPUT: irq_num(either MASTER or SLAVE PIC)
 *
 *  OUTPUT: none
 *
 *  SIDE EFFECTS: Enables(ACTIVE LOW) an IRQ on either the MASTER or SLAVE, according to the input
 *
 *  SOURCE: https://courses.engr.illinois.edu/ece391/secure/references/8259A_PIC_Datasheet.pdf
 *          Helpful pages: 16
 *          https://en.wikipedia.org/wiki/Interrupt
 *          http://wiki.osdev.org/8259_PIC
*/
void
enable_irq(uint32_t irq_num)
{
    /* Master -> 0, 1, 2, 3, 4, 5, 6, 7
     * Slave -> 8, 9, 10, 11, 12, 13, 14, 15
     * If it's outside of bound then return from function
     */
    if(irq_num < 0 || irq_num > 15)
        return;

    /* Initialized to 1111 1110, incase if it's IRQ_0 or IRQ_8 */
    uint8_t interrupt_masking = 0xFE;
    int i = 0; // Will be used for a for loop

    /* If irq_num is between 8-15, its referring to SLAVE PIC */
    if(irq_num >= 8 && irq_num <= 15)
    {
        for(i = 8; i < irq_num; i++)
        {
            /*
             * Explanation behind left shifting the mask
             * Example :- 0xFE = 1111 1110, if left shifted then
             *                 1111 1100 ,this would be wrong... need to bring
             * the back, otherwise we wouldn't ignore IRQ0
             * ADD one to left shifted. SO 1111 1101 , now it's correct
            */
            interrupt_masking = interrupt_masking << 1;
            interrupt_masking = interrupt_masking + 1;
        }

        /* NOW, we have the interrupt_masking ready to use on the SLAVE_MASK
         * Example:- If we want to interrupt from IRQ_2 on slave then
         *           interrupt_masking = 0xFB = 1111 1011
         *           slave_mask =        0xFF = 1111 1111
         * USING & (AND)               = 0xFB = 1111 1011
         *               IRQ_2 IS ENABLED TO HAVE INTERRUPTS
        */
        slave_mask = slave_mask & interrupt_masking;

        /* Sent interrupt data to 0xA1 */
        outb(slave_mask, SLAVE_8259_PORT_DATA);
        return;
    }

    /* If the irq_num is between 0-7, its referring to Master PIC */
    if(irq_num >= 0 && irq_num <= 7)
    {
        for(i = 0; i < irq_num; i++)
        {
            interrupt_masking = interrupt_masking << 1;
            interrupt_masking = interrupt_masking + 1;
        }

        /* ENABLE the IRQ_X that was disabled before */
        master_mask = master_mask & interrupt_masking;

        /* Send the data in slave_mask to 0x21 */
        outb(master_mask, MASTER_8259_PORT_DATA);
        return;
    }

}

/*
 * disable_irq
 *  DESCRIPTION:
 *          This function is responsbile for disabling(mask) the specified IRQ.
 *
 *  INPUT: irq_num(either MASTER or SLAVE PIC)
 *
 *  OUTPUT: none
 *
 *  SIDE EFFECTS: Disables(mask) an IRQ on either the MASTER or SLAVE, according to the input
 *
 *  SOURCE: https://courses.engr.illinois.edu/ece391/secure/references/8259A_PIC_Datasheet.pdf
 *          Helpful pages: 16
 *          https://en.wikipedia.org/wiki/Interrupt
 *          http://wiki.osdev.org/8259_PIC
*/
void
disable_irq(uint32_t irq_num)
{
    /* Master -> 0, 1, 2, 3, 4, 5, 6, 7
     * Slave -> 8, 9, 10, 11, 12, 13, 14, 15
     * If it's outside of bound then return from function
     */
    if(irq_num < 0 || irq_num > 15)
        return;

    /* Initialized to 0000 0001, incase if it's IRQ_0 or IRQ_8 */
    uint8_t interrupt_masking = 0x01;
    int i = 0; // Will be used for a for loop.

    /* If irq_num is between 8-15, its referring to SLAVE PIC */
    if(irq_num >= 8 && irq_num <= 15)
    {
        for(i = 8; i < irq_num; i++)
        {
            /*
             * Explanation behind left shifting the mask
             * Example :- 0x01 = 0000 0001, if left shift
             *                   0000 0010
            */
            interrupt_masking = interrupt_masking << 1;
        }

        /*
         * NOW, the interrupt_masking is ready to be used on slave_mask
         * Example :- If we want to disable interrupts from IRQ_2 on the slave then
         *            interrupt_masking = 0x04 = 0000 0100
         *            slave_mask =        0xFB = 1111 1011
         * USING | (OR)          =        0xFF = 1111 1111
         *               IRQ_2 IS DISABALED FROM INTERRUPTS
        */
        slave_mask = slave_mask | interrupt_masking;

        /* Send the data in slave_mask to 0xA1 */
        outb(slave_mask, SLAVE_8259_PORT_DATA);
        return;
    }

    /* If the irq_num is between 0-7, its referring to the Master PIC */
    if(irq_num >= 0 && irq_num <= 7)
    {
        for(i = 0; i < irq_num; i++)
            interrupt_masking = interrupt_masking << 1;

        /* DISABLED the IRQ_X that was enabled before */
        master_mask = master_mask | interrupt_masking;

        /* Send the data in slave_mask to 0x21 */
        outb(master_mask, MASTER_8259_PORT_DATA);
        return;
    }
}

/*
 * send_eoi
 *  DESCRIPTION:
 *          Send end-of-interrupt signal for the specified IRQ
 *
 *  INPUT: irq_num(either MASTER or SLAVE PIC)
 *
 *  OUTPUT: none
 *
 *  SIDE EFFECTS: Sends EOI to finish interrupt occruing at irq_num
 *
 *  SOURCE: https://courses.engr.illinois.edu/ece391/secure/references/IA32-ref-manual-vol-3.pdf
 *          Pages: 314
 *          http://lxr.free-electrons.com/source/arch/x86_64/kernel/i8259.c?v=2.4.37
 *          Function: mask_and_ack_8259A(unsigned int irq)
 *          Lines:- 245 - 314.
*/
void
send_eoi(uint32_t irq_num)
{
    /*  Send EOI to Master PIC
     *  EOI = 0x60 = 0110 0000, the 3rd bit(indexed 0), is always 0
     *  According to OCW2 info on https://courses.engr.illinois.edu/ece391/secure/references/8259A_PIC_Datasheet.pdf PAGE 13
     *  We can only use the bits 0, 1, 2 with EOI. Other's are reversed/used by Intel
    */
    if(irq_num >= 0 && irq_num <= 7)
    {
        /*
         * If irq_num is on master, then EOI OR irq_num gives you the info
         * needed to finish the interrupt. Send that data to port: MASTER_8259_PORT
        */
        outb( (EOI | irq_num), MASTER_8259_PORT);
    }

    if(irq_num >= 8 && irq_num <= 15)
    {
        /*
         * If irq_num is on slave, we need to bring it in the range 0-7, becuase
         * we can only use bits 0, 1, 2 to represent the irq_num.
         * Send the EOI data to Slave to finish the interrupt and also to the
         * master, so master can finish any interrupts happening at IRQ_2 on the MASTER
        */
        irq_num = irq_num - 8; // change the range from 8-15 to 0-7 for purposes of using EOI
        outb( (EOI | irq_num), SLAVE_8259_PORT);
        outb( (EOI | 0x02), MASTER_8259_PORT);
    }
    return;
}
