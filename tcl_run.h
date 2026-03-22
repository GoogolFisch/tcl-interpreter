
#ifndef TCL_RUN_H_
#define TCL_RUN_H_

#include"tcl_type.h"
#include"tcl_string.h"

#define TCLR_STACK_SIZE 64

enum TCLR_FLAGS{
	TCLR_NONE_LAYER = 0,
	TCLR_FULL_LAYER = 1,
	TCLR_NEGATIVE_LAYER = 2,
}

struct TCLR_Context{
	//
	int32_t instruction;
	int32_t parseStackIdx;
	TCLR_FLAGS flags;
	TCLS_Commands *program;
	TCL_String (*parseStack)[TCLR_STACK_SIZE];
	struct TCL_Scope *scope;
	struct TCLR_Context *parent;
	struct TCLR_Context *vparent;
};

typedef struct TCLR_Context TCLR_Context;

void tclr_free_context(TCLR_Context *ctx){

	for(int i = 0;i < ctx->parseStackIdx;i++){
		free(ctx->parseStack[i]);
	}
	free(ctx);
}

void tclr_step_instruction(TCLR_Context **ctx_ptr){
	TCLR_Context *ctx = *ctx_ptr;
	if(ctx == NULL)return;
	if((*ctx_ptr)->program->length <= (*ctx)->instruction){
		TCLR_Context *freeing = *ctx;
		*ctx_ptr = (*ctx_ptr)->parent;
		tlcr_free_context(freeing);
		return;
	}
	struct _TCLS_Cmd *curCmd = (*ctx_ptr)->program->commands[(*ctx_ptr)->instruction];
}

#endif
