/**
 * @cond internal
 * @file
 */
#include "timer.h"

/// Scheduling configuration for a single timer channel
typedef struct {
  uint16_t period;  ///< event period, in ticks
  void (*callback)(void);  ///< callback to execute
} timer_event_t;

typedef struct timer_struct {
  TCx_t *const tc;
  timer_event_t events[4];
} timer_t;

#define TIMER_EXPR(xn) \
    static void timer##xn##_init(void);
TIMER_ALL_APPLY_EXPR(TIMER_EXPR)
#undef TIMER_EXPR

void timer_init(void)
{
#define TIMER_EXPR(xn)  timer##xn##_init();
  TIMER_ALL_APPLY_EXPR(TIMER_EXPR)
#undef TIMER_EXPR
}

TCx_t *timer_get_tc(const timer_t *t)
{
  return t->tc;
}

void timer_set_callback(timer_t *t, timer_channel_t ch, uint16_t period, intlvl_t intlvl, timer_callback_t cb)
{
  const uint8_t ich = ch-TIMER_CHA;
  TCx_t *const tc = t->tc;
  INTLVL_DISABLE_ALL_BLOCK() {
    tc->INTCTRLB = (tc->INTCTRLB & ~(3 << 2*ich)) | (intlvl << 2*ich);
    (&tc->CCA)[ich] = tc->CNT + period;
    t->events[ich].period = period;
    t->events[ich].callback = cb;
  }
}

void timer_clear_callback(timer_t *t, timer_channel_t ch)
{
  const uint8_t ich = ch-TIMER_CHA;
  TCx_t *const tc = t->tc;
  INTLVL_DISABLE_ALL_BLOCK() {
    tc->INTCTRLB = (tc->INTCTRLB & ~(3 << 2*ich)) | (0 << 2*ich);
    t->events[ich].callback = 0;
  }
}


// Include template for each enabled timer

// Command to duplicate TIMERC0 code for each timer.
// Vimmers will use it with " :*! ".
//   python -c 'import sys; s=sys.stdin.read(); print "\n".join(s.replace("C0",x+n) for x in "CDEF" for n in "01")'

#ifdef TIMERC0_ENABLED
# define XN_(p,s)  p ## C0 ## s
# include "timerxn.inc.c"
#endif

#ifdef TIMERC1_ENABLED
# define XN_(p,s)  p ## C1 ## s
# include "timerxn.inc.c"
#endif

#ifdef TIMERD0_ENABLED
# define TIMER_T timer0_t
# define XN_(p,s)  p ## D0 ## s
# include "timerxn.inc.c"
#endif

#ifdef TIMERD1_ENABLED
# define XN_(p,s)  p ## D1 ## s
# include "timerxn.inc.c"
#endif

#ifdef TIMERE0_ENABLED
# define XN_(p,s)  p ## E0 ## s
# include "timerxn.inc.c"
#endif

#ifdef TIMERE1_ENABLED
# define XN_(p,s)  p ## E1 ## s
# include "timerxn.inc.c"
#endif

#ifdef TIMERF0_ENABLED
# define XN_(p,s)  p ## F0 ## s
# include "timerxn.inc.c"
#endif

#ifdef TIMERF1_ENABLED
# define XN_(p,s)  p ## F1 ## s
# include "timerxn.inc.c"
#endif

#ifdef TIMERC4_ENABLED
# define XN_(p,s) p ## C4 ## s
# include "timerxn.inc.c"
#endif

#ifdef TIMERC5_ENABLED
# define XN_(p,s) p ## C5 ## s
# include "timerxn.inc.c"
#endif

#ifdef TIMERD5_ENABLED
# define XN_(p,s) p ## D5 ## s
# include "timerxn.inc.c"
#endif

///@endcond
