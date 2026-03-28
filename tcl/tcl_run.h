
#ifndef TCL_RUN_H_
#define TCL_RUN_H_

#include"tcl_struct.h"
#include"tcl_type.h"
#include"tcl_string.h"


TCLR_Context *tclr_make_context(TCLR_Context *ctx,TCLR_FLAGS flag){
	// TCLR_NONE_LAYER = 0, TCLR_FULL_LAYER = 1, TCLR_NEGATIVE_LAYER = 2,
	TCLR_Context *cOut = malloc(sizeof(TCLR_Context));
	cOut->instruction = 0;
	cOut->parseStackIdx = 0;
	cOut->flags = flag;
	cOut->program = NULL;
	cOut->scope = malloc(sizeof(TCL_Scope) +
			sizeof(struct _TCL_KV) * TCL_MIN_CAPACITY);
	cOut->scope->length = 0;
	cOut->scope->capacity = TCL_MIN_CAPACITY;
	cOut->parent = ctx;
	cOut->vparent = NULL;
	cOut->arena = (ctx == NULL) ? NULL : ctx->arena;
	cOut->fnScope = (ctx == NULL) ? NULL : ctx->fnScope;
	//
	if(flag & TCLR_NEGATIVE_LAYER)
		cOut->vparent = ctx;

	return cOut;
}
void tclr_free_context(TCLR_Context *ctx){
	if(ctx->parent == NULL){
		// TODO
	}
	/* / ?????
	for(int i = 0;i < ctx->parseStackIdx;i++){
		free(ctx->parseStack[i]);
	} //  */
	free(ctx);
}
TCL_String *tclr_get_bracketStr(TCL_String *str,int32_t *index){
	str = str;
	index = index;
	// TODO
	return NULL;
}
// TODO?
TCL_String *tclr_get_var_slice(TCL_String *str,int32_t *index){
	int32_t begin = *index;
	int32_t ending;
	int32_t idx;
	char contains = 0;
	TCL_String *outStr;
	if(str->data[*index] == '{'){
		ending = _tcls_string_get_length_array(
				str->data,str->length,*index,0);
		outStr = _tcls_make_string_from_bound(str->data,begin + 1,ending - 1);
		return outStr;
		/*
		for(idx = begin;idx < ending;idx++){
			// no $ parsing?
			if(str->data[idx] == '$')
				contains = 1;
		}
		return NULL; // */
	}
	outStr = malloc(sizeof(TCL_String) +
			sizeof(char) * TCL_MIN_CAPACITY);
	outStr->capacity = TCL_MIN_CAPACITY;
	outStr->length = 0;
	outStr->refs = 0;

	contains = 1;
	for(idx = *index;idx < str->length;idx++){
		contains = 0;
		if(str->data[idx] >= 'A' && str->data[idx] <= 'Z')
			contains = 1;
		else if(str->data[idx] >= 'a' && str->data[idx] <= 'z')
			contains = 1;
		else if(str->data[idx] >= '0' && str->data[idx] <= '9')
			contains = 1;
		else if(str->data[idx] == '_')
			contains = 1;
		else if(str->data[idx] == '('){
			// TODO
		}
		if(contains == 0)break;
		if(outStr->capacity <= outStr->length){
			outStr->capacity *= 2;
			outStr = malloc(sizeof(TCL_String) +
			sizeof(char) * TCL_MIN_CAPACITY);
		}
		outStr->data[outStr->length] = str->data[idx];
		outStr->length++;
	}

	return outStr;
}
TCL_String *tclr_compile_str(TCLR_Context *ctx,int32_t *stack,TCL_String *base){
	TCL_String *outStr = malloc(sizeof(TCL_String) + sizeof(char) * base->length);
	outStr->length = 0;
	outStr->capacity = base->length;
	outStr->refs = 0;
	outStr->tags = 0;
	char state = 0;
	for(int32_t strIdx = 0;strIdx < base->length;strIdx++){
		if(base->data[strIdx] == '\\' && state == 0)
			state = 1;
		else state = 0;
		//
		if(base->data[strIdx] == '[' && base->data[strIdx + 1] == ']'){
			TCL_String *stStr = ctx->parseStack[*stack];
			(*stack)++;
			tcl_string_cp(&outStr,stStr);
			continue;
		}
		if(base->data[strIdx] == '$' && state == 0){
			int32_t ofVar = strIdx + 1;
			TCL_String *varStr = tclr_get_var_slice(base,&ofVar);
			// TODO indexing with ( and )
			TCL_String *fetch = tcl_get_from_scope(&(ctx->scope),varStr);
			free(varStr);
			if(fetch == NULL){
				printf("Variable not found! (475b4b5c-bc52-4517-82ac-a82fec7f7b25)\n");
			}
			tcl_string_cp(&outStr,fetch);
			//free(fetch);
			continue;
		}
		if(base->data[strIdx] == '$' && state == 1)
			outStr->length--;
		outStr->data[outStr->length] = base->data[strIdx];
		outStr->length++;
	}
	return outStr;
}

void tclr_step_instruction(TCLR_Context **ctx_ptr){
	TCLR_Context *ctx = *ctx_ptr;
	if(ctx == NULL)return;
	if((*ctx_ptr)->program->length <= ctx->instruction){
		TCLR_Context *freeing = ctx;
		*ctx_ptr = (*ctx_ptr)->parent;
		tclr_free_context(freeing);
		return;
	}
	struct TCLS_Cmd *curCmd = ctx->program->commands[ctx->instruction];
	if(curCmd->command == NULL){
		printf("(15f3ecff-6441-443f-b8c7-ea7b1656697e)\n");
	}
	struct TCLS_Cmd *execCmd = malloc(sizeof(struct TCLS_Cmd) + 
			sizeof(TCLS_Cmd) * curCmd->length);
	execCmd->length = curCmd->length;
	execCmd->capacity = curCmd->length;
	execCmd->stackDepth = curCmd->stackDepth;

	int32_t stOff = execCmd->stackDepth;
	execCmd->command = tclr_compile_str(ctx,&stOff,curCmd->command);
	ctx->parseStackIdx -= execCmd->stackDepth;
	struct TCLF_KV *fnIdx = tclf_get_function(ctx->fnScope,execCmd->command);
	if(fnIdx == NULL){
		printf("Not finding %.*s function.\n"
				,curCmd->command->length,curCmd->command->data);
		ctx->instruction++;
		// TODO if fnIdx->flags == TCLF_FN_PUSH
		return;
	}

	if(fnIdx->flags & TCLF_FN_RAW){
		for(int32_t i = 0;i < curCmd->length;i++){
			execCmd->arguments[i] = curCmd->arguments[i];
		}
	}
	else{
		for(int32_t i = 0;i < curCmd->length;i++){
			execCmd->arguments[i] = tclr_compile_str(
					ctx,&stOff,curCmd->arguments[i]);
		}
	}
	// TODO
	if(fnIdx->flags == TCLF_FN_NATIVE){
		((TCLF_NAT_FN)(fnIdx->natFn))(&ctx,curCmd);
	}
	ctx->instruction++;
}

#endif
