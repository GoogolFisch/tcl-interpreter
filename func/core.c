
#include"load.h"

TCL_String *putsFunction(TCLR_Context **ctx, TCLS_Cmd *cmd){
	ctx = ctx;
	printf("fn %.*s\n",cmd->command->length,cmd->command->data);
	for(int32_t i = 0;i < cmd->length;i++){
		printf("- %.*s\n",cmd->arguments[i]->length,cmd->arguments[i]->data);
	}
	return NULL;
}
TCL_String *setFunction(TCLR_Context **ctx,TCLS_Cmd *cmd){
	printf("fn %.*s\n",cmd->command->length,cmd->command->data);
	for(int32_t i = 0;i < cmd->length;i++){
		printf("- %.*s\n",cmd->arguments[i]->length,cmd->arguments[i]->data);
	}
	if(cmd->length != 2){
		printf("Err with set (fa1889db-c6a3-4a1b-b927-96d77805c985)\n");
		return;
	}
	tcl_set_into_scope(&((*ctx)->scope),cmd->arguments[0],cmd->arguments[1]);
	return NULL;
}
TCL_String *incrFunction(TCLR_Context **ctx,TCLS_Cmd *cmd){
	printf("fn %.*s\n",cmd->command->length,cmd->command->data);
	for(int32_t i = 0;i < cmd->length;i++){
		printf("- %.*s\n",cmd->arguments[i]->length,cmd->arguments[i]->data);
	}
	if(cmd->length > 2 && cmd->length == 0){
		printf("Err with set (59c22b64-11e7-458d-b5a8-ba0c7911fb52)\n");
		return;
	}
	if(cmd->length == 2)
		tcl_set_into_scope(&((*ctx)->scope),cmd->arguments[0],cmd->arguments[1]);
	else{
		TCL_String *setTo = tcl_create_cstring("1");
		tcl_set_string_arena(&((*ctx)->arena),setTo);
		tcl_set_into_scope(&((*ctx)->scope),cmd->arguments[0],setTo);
	}
	return NULL;
}

#ifdef FN_REDEF_INIT
#define load_init fn_load_core_init
#endif
void load_init(TCL_Context **ctx){
	insert_natFunction(ctx,"puts",putsFunction);
	insert_natFunction(ctx,"set" , setFunction);
	insert_natFunction(ctx,"incr",incrFunction);
}
#ifdef FN_REDEF_INIT
#undef load_init
#endif



