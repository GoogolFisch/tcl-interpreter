
#ifndef TCL_RUN_H_
#define TCL_RUN_H_

#include"tcl_type.h"
#include"tcl_string.h"

struct TCLR_Context{
	//
	int32_t instruction;
	struct TCL_Scope *scope;
	struct TCLR_Context *parent;
};

typedef struct TCLR_Context TCLR_Context;

void tclr_step_instruction(TCLR_Context **ctx_ptr){
	TCLR_Context *ctx = *ctx_ptr;
	if(ctx == NULL)return;
}

#endif
