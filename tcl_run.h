
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
	cOut->scope = NULL;
	cOut->parent = ctx;
	cOut->vparent = NULL;
	if(flag & TCLR_NEGATIVE_LAYER)
		cOut->vparent = ctx;

	return cOut;
}
void tclr_free_context(TCLR_Context *ctx){
	if(ctx->parent == NULL){
		// TODO
	}
	for(int i = 0;i < ctx->parseStackIdx;i++){
		free(ctx->parseStack[i]);
	}
	free(ctx);
}
TCL_String *tclr_get_var_slice(TCL_String *str,int32_t *index){
	int32_t beginn = *index;
	int32_t ending;
	int32_t idx;
	char contains = 0;
	if(str->data[*index] == '{'){
		ending = _tcls_string_get_length_array(
				str->data,str->length,*index,0);
		for(idx = beginn;idx < ending;idx++){
			if(str->data[idx] == '$')
				contains = 1;
		}

		return NULL;
	}
	TCL_String *outStr = malloc(sizeof(TCL_String) +
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
		if(baseStr[strIdx] == '\\' && state == 0)
			state = 1;
		else state = 0;
		//
		if(baseStr[strIdx] == '[' && baseStr[strIdx + 1] == ']'){
			TCL_String *stStr = ctx->parseStack[*stack];
			(*stack)++;
			tcl_string_cp(&outStr,stStr);
			continue;
		}
		if(baseStr[strIdx] == '$' && state == 0){
			int32_t ofVar = strIdx;
			TCL_String *varStr = tclr_get_var_slice(baseStr,&ofVar);
			// TODO indexing with ( and )
			TCL_String *fetch = tcl_get_from_scope(&ctx,varStr);
			free(varStr);
			free(fetch);
			tcl_string_cp(&outStr,fetch);
			continue;
		}
		if(baseStr[strIdx] == '$' && state == 1)
			outStr->length--;
		outStr->data[outStr->length] = baseStr[strIdx];
		outStr->length++;
	}
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
	struct _TCLF_KV *fnIdx = tclf_get_function(scope,str);

	if(fnIdx == TCLF_FN_NATIVE){
	}
}

#endif
