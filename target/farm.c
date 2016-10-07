/* This function marks the start of the farm */

#define CS211_ALIGNED __attribute__((aligned(0x10)))
#define CS211_NOP asm volatile ("nop")

extern int start_farm(void) CS211_ALIGNED;
int start_farm(void)
{
    CS211_NOP;
    return 1;
}

extern void setval_407(unsigned *p) CS211_ALIGNED;
void setval_407(unsigned *p)
{
    CS211_NOP;
    *p = 3281031256U;
}

extern unsigned addval_453(unsigned x) CS211_ALIGNED;
unsigned addval_453(unsigned x)
{
    CS211_NOP;
    return x + 2428995912U;
}

extern unsigned getval_397(void) CS211_ALIGNED;
unsigned getval_397(void)
{
    CS211_NOP;
    return 2425393240U;
}

extern unsigned getval_129(void) CS211_ALIGNED;
unsigned getval_129(void)
{
    CS211_NOP;
    return 3284633929U;
}

extern unsigned addval_398(unsigned x) CS211_ALIGNED;
unsigned addval_398(unsigned x)
{
    CS211_NOP;
    return x + 3251079496U;
}

extern void setval_200(unsigned *p) CS211_ALIGNED;
void setval_200(unsigned *p)
{
    CS211_NOP;
    *p = 2455284152U;
}

extern unsigned getval_445(void) CS211_ALIGNED;
unsigned getval_445(void)
{
    CS211_NOP;
    return 3347662944U;
}

extern unsigned addval_113(unsigned x) CS211_ALIGNED;
unsigned addval_113(unsigned x)
{
    CS211_NOP;
    return x + 2425641040U;
}

/* This function marks the middle of the farm */
extern int mid_farm(void) CS211_ALIGNED;
int mid_farm(void)
{
    CS211_NOP;
    return 1;
}

/* Add two arguments */
extern long add_xy(long x, long y) CS211_ALIGNED;
long add_xy(long x, long y)
{
    CS211_NOP;
    return x+y;
}

extern void setval_417(unsigned *p) CS211_ALIGNED;
void setval_417(unsigned *p)
{
    CS211_NOP;
    *p = 3286272360U;
}

extern unsigned getval_478(void) CS211_ALIGNED;
unsigned getval_478(void)
{
    CS211_NOP;
    return 2411974297U;
}

extern unsigned addval_216(unsigned x) CS211_ALIGNED;
unsigned addval_216(unsigned x)
{
    CS211_NOP;
    return x + 3269495112U;
}

extern unsigned addval_376(unsigned x) CS211_ALIGNED;
unsigned addval_376(unsigned x)
{
    CS211_NOP;
    return x + 3229929101U;
}

extern unsigned getval_111(void) CS211_ALIGNED;
unsigned getval_111(void)
{
    CS211_NOP;
    return 3223376281U;
}

extern unsigned getval_104(void) CS211_ALIGNED;
unsigned getval_104(void)
{
    CS211_NOP;
    return 3281047177U;
}

extern unsigned getval_117(void) CS211_ALIGNED;
unsigned getval_117(void)
{
    CS211_NOP;
    return 3353381192U;
}

extern unsigned getval_315(void) CS211_ALIGNED;
unsigned getval_315(void)
{
    CS211_NOP;
    return 3372797576U;
}

extern unsigned addval_155(unsigned x) CS211_ALIGNED;
unsigned addval_155(unsigned x)
{
    CS211_NOP;
    return x + 3223896713U;
}

extern unsigned addval_432(unsigned x) CS211_ALIGNED;
unsigned addval_432(unsigned x)
{
    CS211_NOP;
    return x + 2464188744U;
}

extern unsigned getval_330(void) CS211_ALIGNED;
unsigned getval_330(void)
{
    CS211_NOP;
    return 3286272072U;
}

extern unsigned addval_152(unsigned x) CS211_ALIGNED;
unsigned addval_152(unsigned x)
{
    CS211_NOP;
    return x + 3224945289U;
}

extern void setval_312(unsigned *p) CS211_ALIGNED;
void setval_312(unsigned *p)
{
    CS211_NOP;
    *p = 3232023177U;
}

extern unsigned addval_480(unsigned x) CS211_ALIGNED;
unsigned addval_480(unsigned x)
{
    CS211_NOP;
    return x + 3281047949U;
}

extern unsigned getval_427(void) CS211_ALIGNED;
unsigned getval_427(void)
{
    CS211_NOP;
    return 2430634312U;
}

extern unsigned addval_218(unsigned x) CS211_ALIGNED;
unsigned addval_218(unsigned x)
{
    CS211_NOP;
    return x + 3676361097U;
}

extern void setval_295(unsigned *p) CS211_ALIGNED;
void setval_295(unsigned *p)
{
    CS211_NOP;
    *p = 3373846153U;
}

extern unsigned getval_422(void) CS211_ALIGNED;
unsigned getval_422(void)
{
    CS211_NOP;
    return 3286272328U;
}

extern void setval_347(unsigned *p) CS211_ALIGNED;
void setval_347(unsigned *p)
{
    CS211_NOP;
    *p = 3674260105U;
}

extern void setval_271(unsigned *p) CS211_ALIGNED;
void setval_271(unsigned *p)
{
    CS211_NOP;
    *p = 3531918985U;
}

extern unsigned getval_321(void) CS211_ALIGNED;
unsigned getval_321(void)
{
    CS211_NOP;
    return 2425541001U;
}

extern unsigned getval_291(void) CS211_ALIGNED;
unsigned getval_291(void)
{
    CS211_NOP;
    return 2425409945U;
}

extern unsigned addval_384(unsigned x) CS211_ALIGNED;
unsigned addval_384(unsigned x)
{
    CS211_NOP;
    return x + 3229929113U;
}

extern unsigned addval_225(unsigned x) CS211_ALIGNED;
unsigned addval_225(unsigned x)
{
    CS211_NOP;
    return x + 2210648457U;
}

extern unsigned addval_272(unsigned x) CS211_ALIGNED;
unsigned addval_272(unsigned x)
{
    CS211_NOP;
    return x + 3375940233U;
}

extern unsigned addval_264(unsigned x) CS211_ALIGNED;
unsigned addval_264(unsigned x)
{
    CS211_NOP;
    return x + 3375944072U;
}

extern unsigned addval_448(unsigned x) CS211_ALIGNED;
unsigned addval_448(unsigned x)
{
    CS211_NOP;
    return x + 3375415945U;
}

extern unsigned getval_380(void) CS211_ALIGNED;
unsigned getval_380(void)
{
    CS211_NOP;
    return 3526939016U;
}

extern unsigned addval_389(unsigned x) CS211_ALIGNED;
unsigned addval_389(unsigned x)
{
    CS211_NOP;
    return x + 3682124425U;
}

extern void setval_461(unsigned *p) CS211_ALIGNED;
void setval_461(unsigned *p)
{
    CS211_NOP;
    *p = 3269495112U;
}

extern void setval_177(unsigned *p) CS211_ALIGNED;
void setval_177(unsigned *p)
{
    CS211_NOP;
    *p = 3372794504U;
}

extern void setval_474(unsigned *p) CS211_ALIGNED;
void setval_474(unsigned *p)
{
    CS211_NOP;
    *p = 3675836041U;
}

/* This function marks the end of the farm */
extern int end_farm(void) CS211_ALIGNED;
int end_farm(void)
{
    CS211_NOP;
    return 1;
}
