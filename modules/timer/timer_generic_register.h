/**
 * @internal
 * @file
 * @brief TCx_t structure declaration
 *
 * TCx_t is a register mapping declaration which maps only useful registers
 * common to TC0, TC1, TC2, TC4 and TC5.
 */

/* 16-bit Timer/Counter generic register mapping */
typedef struct TCx_struct
{
    register8_t CTRLA;  /* Control  Register A */
    register8_t unused_0x01;
    register8_t unused_0x02; 
    register8_t unused_0x03;
    register8_t unused_0x04;
    register8_t unused_0x05;
    register8_t unused_0x06;
    register8_t INTCTRLB;  /* Interrupt Control Register B */
    register8_t unused_0x08;
    register8_t unused_0x09;
    register8_t unused_0x0A;
    register8_t unused_0x0B;
    register8_t unused_0x0C;
    register8_t unused_0x0D;
    register8_t unused_0x0E;
    register8_t unused_0x0F;
    register8_t unused_0x10;
    register8_t unused_0x11;
    register8_t unused_0x12;
    register8_t unused_0x13;
    register8_t unused_0x14;
    register8_t unused_0x15;
    register8_t unused_0x16;
    register8_t unused_0x17;
    register8_t unused_0x18;
    register8_t unused_0x19;
    register8_t unused_0x1A;
    register8_t unused_0x1B;
    register8_t unused_0x1C;
    register8_t unused_0x1D;
    register8_t unused_0x1E;
    register8_t unused_0x1F;
    _WORDREGISTER(CNT);  /* Count */
    register8_t unused_0x22;
    register8_t unused_0x23;
    register8_t unused_0x24;
    register8_t unused_0x25;
    register8_t unused_0x26;
    register8_t unused_0x27;
    _WORDREGISTER(CCA);  /* Compare or Capture A */
} TCx_t;


