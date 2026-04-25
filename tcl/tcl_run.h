
#ifndef TCL_RUN_H_
#define TCL_RUN_H_

#include"tcl_struct.h"
#include"tcl_type.h"
#include"tcl_string.h"
#include"tcl_debug.h"


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
	cOut->globFlags = (ctx == NULL) ? 0 : ctx->globFlags;
	//
	if(flag & TCLR_NEGATIVE_LAYER)
		cOut->vparent = ctx;

	return cOut;
}
void tclr_free_context(TCLR_Context *ctx){
	tcl_garbage_collect_arena(&(ctx->arena));
	tcl_drop_scope(&(ctx->scope));
	if(ctx->program->gc.refs == 0)
		tcls_free_commands(&(ctx->program));
	/* / ?????
	for(int i = 0;i < ctx->parseStackIdx;i++){
		free(ctx->parseStack[i]);
	} //  */
	if(ctx->parent == NULL){
		if(ctx->globFlags & TCLRG_SHOW_GC){
			db_print_arena(ctx->arena);
		}
		tclf_free_function_scope(&(ctx->fnScope));
		while(tcl_garbage_collect_arena(&(ctx->arena)));
		if(ctx->globFlags & TCLRG_SHOW_GC){
			// raise here for fun!
			db_raise_arena(ctx->arena);
		}
		free(ctx->arena);
		// TODO
	}
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
		outStr = _tcls_make_string_from_bound(str->data,begin + 1,ending - 1,0);
		*index = ending;
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
	outStr->var = 0;
	outStr->capacity = TCL_MIN_CAPACITY;
	outStr->length = 0;
	outStr->deferCallback = NULL;
	outStr->replaceWith = NULL;
	outStr->gc.freeCallback = (void*)tcl_free_string;
	outStr->gc.refs = 0;
	outStr->gc.tags = TCL_ST_None;

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
			outStr = realloc(outStr,sizeof(TCL_String) +
			sizeof(char) * TCL_MIN_CAPACITY);
		}
		outStr->data[outStr->length] = str->data[idx];
		outStr->length++;
	}
	*index = idx - 1;

	return outStr;
}
TCL_String *tclr_compile_str(TCLR_Context *ctx,int32_t *stack,TCL_String *base){
	TCL_String *outStr = malloc(sizeof(TCL_String) + sizeof(char) * base->length);
	outStr->length = 0;
	outStr->var = NULL;
	outStr->capacity = base->length;
	outStr->deferCallback = NULL;
	outStr->replaceWith = NULL;
	outStr->gc.freeCallback = (void*)tcl_free_string;
	outStr->gc.refs = 0;
	outStr->gc.tags = 0;
	char state = 0;
	for(int32_t strIdx = 0;strIdx < base->length;strIdx++){
		if(base->data[strIdx] == '\\' && state == 0)
			state = 1;
		else state = 0;
		//
		if(base->data[strIdx] == '[' && base->data[strIdx + 1] == ']'){
			TCL_String *stStr = ctx->parseStack[*stack];
			(*stack)++;
			strIdx++;
			if(stStr == NULL)
				continue;
			if(base->length == 2){
				// early exit for set
				free(outStr);
				return ctx->parseStack[(*stack) - 1];
			}
			if(stStr->deferCallback != NULL){
				//TCL_String *old = fetch;
				//old->gc.refs--;
				stStr->gc.refs--;
				((TCL_DEFER_CBack)(stStr->deferCallback))(&stStr);
				tcl_set_garbage_arena(
						&(ctx->arena),
						(TCL_Disposable*)stStr);
				stStr->gc.refs++;
			}
			tcl_string_cp(&outStr,stStr);
			stStr->gc.refs--;
			continue;
		}
		if(base->data[strIdx] == '$' && state == 0){
			int32_t ofVar = strIdx + 1;
			TCL_String *varStr = tclr_get_var_slice(base,&ofVar);
			// TODO indexing with ( and )
			//ofVar += varStr->length - 1;
			TCL_String *fetch = tcl_get_from_scope(&(ctx->scope),varStr);
			if(fetch == NULL){
				printf("Variable not found %.*s!"
				"(475b4b5c-bc52-4517-82ac-a82fec7f7b25)\n",
				varStr->length,varStr->data);
				continue;
			}
			while(fetch->replaceWith != NULL){
				//fetch->refs--;
				fetch = fetch->replaceWith;
				tcl_set_into_scope(&(ctx->scope),varStr,fetch);
			}
			if(fetch->deferCallback != NULL){
				//TCL_String *old = fetch;
				//old->gc.refs--;
				fetch->gc.refs--;
				((TCL_DEFER_CBack)(fetch->deferCallback))(&fetch);
				tcl_set_garbage_arena(
						&(ctx->arena),
						(TCL_Disposable*)fetch);
				tcl_set_garbage_arena(
						&(ctx->arena),
						(TCL_Disposable*)varStr);
				//fetch->refs++;
				tcl_set_into_scope(&(ctx->scope),varStr,fetch);
			}
			else free(varStr);
			tcl_string_cp(&outStr,fetch);
			//free(fetch);
			strIdx = ofVar;
			continue;
		}
		if(base->data[strIdx] == '$' && state == 1)
			outStr->length--;
		if(outStr->length >= outStr->capacity){
			outStr->capacity *= 2;
			outStr = realloc(outStr,sizeof(TCL_String) +
					sizeof(char) * outStr->capacity);
		}
		outStr->data[outStr->length] = base->data[strIdx];
		outStr->length++;
	}
	return outStr;
}
void _tclr_fill_scope(TCL_GarbageArena **arena,TCL_Scope **scope,TCL_String *str,TCLS_Cmd *cmd){
	TCL_Slice *slc = malloc(sizeof(TCL_Slice));
	slc->length = str->length;
	slc->offset = 0;
	slc->string = str;
	for(int32_t idx = 0;slc != NULL && idx < cmd->length;idx++){
		TCL_String *key = tcls_list_iter(&slc);
		tcl_set_into_scope(scope,key,cmd->arguments[idx]);
		tcl_set_garbage_arena(arena,(TCL_Disposable*)key);
	}
}

void tclr_step_instruction(TCLR_Context **ctx_ptr){
	TCLR_Context *ctx = *ctx_ptr;
	if(ctx == NULL)return;
	if((*ctx_ptr)->program->length <= ctx->instruction){
		ctx->program->gc.refs--;
		TCLR_Context *freeing = ctx;
		*ctx_ptr = (*ctx_ptr)->parent;
		tclr_free_context(freeing);
		return;
	}
	struct TCLS_Cmd *curCmd = ctx->program->commands[ctx->instruction];
	if(curCmd->command == NULL){
		printf("(15f3ecff-6441-443f-b8c7-ea7b1656697e)\n");
	}
	struct TCLS_Cmd *execCmd = malloc(sizeof(TCLS_Cmd) + 
			sizeof(TCL_String) * curCmd->length);
	execCmd->length = curCmd->length;
	execCmd->capacity = curCmd->length;
	execCmd->stackDepth = curCmd->stackDepth;
	execCmd->flags = curCmd->flags;
	execCmd->moreData = curCmd->moreData;
	execCmd->deferFree = NULL;

	int32_t stOff = execCmd->stackDepth;
	execCmd->command = curCmd->command;
	if((curCmd->command->gc.tags & TCL_ST_Mask) == TCL_ST_Variable){
		execCmd->command = tclr_compile_str(ctx,&stOff,curCmd->command);
	}
	execCmd->command->gc.refs++;
	tcl_set_garbage_arena(&(ctx->arena),(TCL_Disposable*)execCmd->command);
	ctx->parseStackIdx -= execCmd->stackDepth;
	struct TCLF_KV *fnIdx = tclf_get_function(ctx->fnScope,execCmd->command);
	if(fnIdx == NULL){
		printf("Not finding %.*s function.\n"
				,curCmd->command->length,curCmd->command->data);
		db_print_cmd(curCmd);

		ctx->instruction++;
		if(curCmd->flags == TCLS_CMD_PUSH)
			ctx->parseStack[curCmd->stackDepth] = NULL;
		execCmd->command->gc.refs--;
		free(execCmd); /// 
		// TODO if fnIdx->flags == TCLF_FN_PUSH
		return;
	}

	if(fnIdx->flags & TCLF_FN_RAW){
		for(int32_t i = 0;i < curCmd->length;i++){
			execCmd->arguments[i] = curCmd->arguments[i];
			execCmd->arguments[i]->gc.refs++;
		}
	}
	else{
		for(int32_t i = 0;i < curCmd->length;i++){
			if(curCmd->arguments[i]->gc.tags & TCL_ST_Variable){
				execCmd->arguments[i] = tclr_compile_str(
						ctx,&stOff,curCmd->arguments[i]);
				tcl_set_garbage_arena(
						&(ctx->arena),
						(TCL_Disposable*)execCmd->arguments[i]);
			}else{
				execCmd->arguments[i] = curCmd->arguments[i];
			}
			execCmd->arguments[i]->gc.refs++;
		}
	}
	TCL_String *returned = NULL;
	// TODO
	if(fnIdx->flags == TCLF_FN_NATIVE){
		if(ctx->globFlags & TCLRG_VERBOSE_EXEC){
			printf("fn %.*s\n",   execCmd->command->length,
					      execCmd->command->data);
			for(int32_t i = 0;i < execCmd->length;i++){
				printf("- %.*s\n",  execCmd->arguments[i]->length,
						    execCmd->arguments[i]->data);
			}
		}
		if(curCmd->deferFree != NULL && fnIdx->natFn != curCmd->deferFree){
			((TCLF_NAT_Fn)(fnIdx->natFn))(ctx_ptr,execCmd);
		}
		returned = ((TCLF_NAT_Fn)(fnIdx->natFn))(ctx_ptr,execCmd);
		curCmd->moreData = execCmd->moreData;
		curCmd->deferFree = fnIdx->freeFn;
		if((curCmd->command->gc.tags & TCL_ST_Mask) == TCL_ST_Variable &&
				fnIdx->freeFn != NULL){
			if(ctx->globFlags & TCLRG_VERBOSE_EXEC){
				printf("free %.*s\n",  execCmd->command->length,
						      execCmd->command->data);
			}
			((TCLF_NAT_Fn)(fnIdx->freeFn))(ctx_ptr,execCmd);
			curCmd->moreData = NULL;
		}
	}else if(fnIdx->flags == TCLF_FN_PROC){
		TCLR_Context *lowCtx = tclr_make_context(ctx,TCLR_FULL_LAYER);
		// TODO add argument parsing!
		_tclr_fill_scope(&(lowCtx->arena),&(lowCtx->scope),
				fnIdx->arguments,execCmd);
		lowCtx->program = fnIdx->body;
		lowCtx->program->gc.refs++;
		(*ctx_ptr) = lowCtx;
		// TODO also think about return values!
	}
	if(curCmd->flags == TCLS_CMD_PUSH){
		ctx->parseStack[curCmd->stackDepth] = returned;
		if(returned != NULL)returned->gc.refs++;
	}
	if(returned != NULL){
		tcl_set_garbage_arena(&(ctx->arena),(TCL_Disposable*)returned);
	}
	returned = returned;
	tcls_free_cmd(&execCmd);
	ctx->instruction++;
}

#endif
