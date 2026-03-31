
#ifndef FUNC_LOAD_H_
#define FUNC_LOAD_H_

#include<stdint.h>
#include<stdlib.h>
#include<stdio.h>
#include"../tcl/include.h"

#define insert_natFunction(ctx,string,fn) do{                                  \
    TCL_String *str = tcl_create_cstring(string);                              \
    tcl_set_string_arena(&((*ctx)->arena),str);                                \
    tclf_insert_natFunction(&((*ctx)->fnScope),                                \
            str,(void(*)())(void*)fn);                                         \
}while(0)


void load_init(TCLR_Context **ctx);


#endif
