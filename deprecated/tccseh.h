#ifndef _TCCSEH_H
#define _TCCSEH_H

#ifndef NULL
#define NULL ((void *)0)
#endif

#ifndef _WINBASE_
void __stdcall RaiseException(unsigned long dwExceptionCode, unsigned long dwExceptionFlags, int nNumberOfArguments, const unsigned long *lpArguments);
#endif

#ifndef _INC_EXCPT
unsigned long __cdecl _exception_code(void);
#endif

struct _scope {int trylevel; void *filter, *handler, *unwind;};
#define __seh_scope static struct _scope _scopetable[] =
#define __scope(l,i) {l, &&_filter ## i, &&_handler ## i, &&_unwind ## i}

#define __seh_begin void *_seh[6]; \
    asm( \
    "movl $-1,-4(%%ebp);" \
    "movl %%eax,-8(%%ebp);" \
    "movl $_tcc_except_handler,-12(%%ebp);" \
    "movl %%fs:0,%%eax;" \
    "movl %%eax,-16(%%ebp);" \
    "movl %%esp,-24(%%ebp);" \
    "leal -16(%%ebp),%%eax;" \
    "movl %%eax,%%fs:0;": :"a"(_scopetable));

#define __seh_end asm( \
    "movl -16(%%ebp),%%eax;movl %%eax,%%fs:0;": : :"eax");

#define __try asm("incl -4(%ebp)"); do

#define __except(i,f) while(0); __leave(i); \
    _filter ## i: \
    asm("ret": :"a"(({f;}))); \
    _handler ## i: asm("push %0": :"r"(&&_end ## i)); do

#define __finally(i) while (0); \
    _unwind ## i: do

#define __end(i) while(0); asm("ret"); _end ## i: ;

#define __leave(i) asm("decl -4(%%ebp);push %0": :"r"(&&_end ## i)); goto _unwind ## i

#define EF_EXECUTE (-1)
#define EF_SEARCH 0
#define EF_HANDLE 1

#define throw(code) RaiseException(code, 0, 0, NULL)

#define __catch(i,code) __except(i,_exception_code() == (code) ? EF_HANDLE : EF_SEARCH)

#endif