/**
** @file mruby/presym.h - Preallocated Symbols
**
** See Copyright Notice in mruby.h
*/

#ifndef MRUBY_PRESYM_H
#define MRUBY_PRESYM_H

#undef MRB_PRESYM_MAX
#define MRB_PRESYM_CSYM(sym, num) MRB_PRESYM__##sym = (num<<1),
#define MRB_PRESYM_QSYM(str, sym, num) MRB_PRESYM_q_##sym = (num<<1),
#define MRB_PRESYM_SYM(sym, num) 

enum mruby_presym {
#include <../build/presym.inc>
};

#undef MRB_PRESYM_CSYM
#undef MRB_PRESYM_QSYM
#undef MRB_PRESYM_SYM

#define MRB_SYM(sym) MRB_PRESYM__##sym
#define MRB_QSYM(sym) MRB_PRESYM_q_##sym
#endif  /* MRUBY_PRESYM_H */
