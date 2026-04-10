
#ifndef FUNC_LOAD_H_
#define FUNC_LOAD_H_

#include<stdint.h>
#include<stdlib.h>
#include<stdio.h>
#include"../tcl/include.h"

#define insert_natFunction(ctx,string,fn,fee) do{                              \
    TCL_String *str = tcl_create_cstring(string);                              \
    tcl_set_string_arena(&((*ctx)->arena),str);                                \
    tclf_insert_natFunction(&((*ctx)->fnScope),                                \
            str,(TCLF_NAT_Fn)(void*)fn,(void*)fee);                            \
}while(0)
#define insert_anyFunction(inserter,ctx,string,fn,fee) do{                     \
    TCL_String *str = tcl_create_cstring(string);                              \
    tcl_set_string_arena(&((*ctx)->arena),str);                                \
    inserter(&((*ctx)->fnScope),                                               \
            str,(TCLF_NAT_Fn)(void*)fn,(void*)fee);                            \
}while(0)


void load_init(TCLR_Context **ctx);


#endif
