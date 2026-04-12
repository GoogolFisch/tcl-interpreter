
#include"load.h"

TCL_String *putsFunction(TCLR_Context **ctx, TCLS_Cmd *cmd){
	// puts {value} ;# for now
	if((*ctx)->globFlags & TCLRG_VERBOSE_EXEC){
		ctx = ctx;
		return NULL;
	}
	if(cmd->length == 1){
		printf("%.*s\n",cmd->arguments[0]->length,cmd->arguments[0]->data);
	}
	return NULL;
}
TCL_String *setFunction(TCLR_Context **ctx,TCLS_Cmd *cmd){
	// set {name} {value}
	if(cmd->length != 2){
		printf("Err with set (fa1889db-c6a3-4a1b-b927-96d77805c985)\n");
		return NULL;
	}
	tcl_set_into_scope(&((*ctx)->scope),cmd->arguments[0],cmd->arguments[1]);
	return cmd->arguments[1];
}
TCL_String *incrFunction(TCLR_Context **ctx,TCLS_Cmd *cmd){
	// incr {name} {value?}
	if(cmd->length > 2 || cmd->length == 0){
		printf("Err with set (59c22b64-11e7-458d-b5a8-ba0c7911fb52)\n");
		return NULL;
	}
	TCL_String *setTo;
	if(cmd->length == 2)
		setTo = cmd->arguments[1];
	else{
		setTo = tcl_create_cstring("1");
		tcl_set_string_arena(&((*ctx)->arena),setTo);
	}
	tcl_set_into_scope(&((*ctx)->scope),cmd->arguments[0],setTo);
	return setTo;
}
TCL_String *procFunction(TCLR_Context **ctx,TCLS_Cmd *cmd){
	// proc {name} {args} {body}
	ctx = ctx;
	if(cmd->length != 3){
		printf("Err with set (a274c31d-5058-436c-8d6c-7da31b615a8f)\n");
		return NULL;
	}
	TCLS_Commands *tcmd = tcls_parse_commands(&((*ctx)->arena),cmd->arguments[2]);
	tcmd->refs++;
	tclf_insert_procFunction(&((*ctx)->fnScope),cmd->arguments[0], 
		cmd->arguments[1],tcmd);
	return NULL;
}


typedef struct TCLCORE_LIST_Expr{
	int32_t flags;
	TCL_Slice *str;
	struct TCLCORE_KV_Expr *left;
	struct TCLCORE_KV_Expr *right;
} TCLCORE_LIST_Expr;
typedef struct TCLCORE_Expr{
	int32_t capacity;
	int32_t length;
	TCLCORE_LIST_Expr expr[];
} TCLCORE_Expr;
TCL_String *exprFunctionFree(TCLR_Context **ctx,TCLS_Cmd *cmd){
	ctx = ctx;
	// proc {name} {args} {body}
	printf("free %.*s\n",cmd->command->length,cmd->command->data);
	for(int32_t i = 0;i < cmd->length;i++){
		printf("- %.*s\n",cmd->arguments[i]->length,cmd->arguments[i]->data);
	}
	if(cmd->length != 1){
		printf("Err with set (d0378443-5e77-4361-886d-45d58a2691da)\n");
		return NULL;
	}
	/*
	TCLCORE_Expr *exprPtr = (TCLCORE_Expr*)(cmd->moreData);
	for(int32_t ovExpr = 0;ovExpr < exprPtr->length;ovExpr++){
		free(exprPtr->);
	}
	*/
	if(cmd->moreData != NULL){
		// TODO also free potential memory in the LIST
		free(cmd->moreData);
	}
	return NULL;
}
TCL_String *exprFunction(TCLR_Context **ctx,TCLS_Cmd *cmd){
	ctx = ctx;
	// proc {name} {args} {body}
	printf("fn %.*s\n",cmd->command->length,cmd->command->data);
	for(int32_t i = 0;i < cmd->length;i++){
		printf("- %.*s\n",cmd->arguments[i]->length,cmd->arguments[i]->data);
	}
	if(cmd->length != 1){
		printf("Err with set (62bf270b-ba0d-44ca-8bd2-5498044247ed)\n");
		return NULL;
	}
	TCLCORE_Expr *exprPtr = malloc(sizeof(TCLCORE_Expr) +
			sizeof(TCLCORE_LIST_Expr) * 128);
	exprPtr->capacity = 128;
	exprPtr->length = 0;
	return NULL;
}

#ifdef FN_REDEF_INIT
#define load_init fn_load_core_init
#endif
void load_init(TCLR_Context **ctx){
	insert_natFunction(ctx,"puts",putsFunction,NULL);
	insert_natFunction(ctx,"set" , setFunction,NULL);
	insert_natFunction(ctx,"incr",incrFunction,NULL);
	insert_natFunction(ctx,"proc",procFunction,NULL);
	insert_natFunction(ctx,"expr",exprFunction,exprFunctionFree);
}
#ifdef FN_REDEF_INIT
#undef load_init
#endif



