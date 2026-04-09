
#include"load.h"

TCL_String *putsFunction(TCLR_Context **ctx, TCLS_Cmd *cmd){
	// puts {value} ;# for now
	ctx = ctx;
	printf("fn %.*s\n",cmd->command->length,cmd->command->data);
	for(int32_t i = 0;i < cmd->length;i++){
		printf("- %.*s\n",cmd->arguments[i]->length,cmd->arguments[i]->data);
	}
	return NULL;
}
TCL_String *setFunction(TCLR_Context **ctx,TCLS_Cmd *cmd){
	// set {name} {value}
	printf("fn %.*s\n",cmd->command->length,cmd->command->data);
	for(int32_t i = 0;i < cmd->length;i++){
		printf("- %.*s\n",cmd->arguments[i]->length,cmd->arguments[i]->data);
	}
	if(cmd->length != 2){
		printf("Err with set (fa1889db-c6a3-4a1b-b927-96d77805c985)\n");
		return NULL;
	}
	tcl_set_into_scope(&((*ctx)->scope),cmd->arguments[0],cmd->arguments[1]);
	return cmd->arguments[1];
}
TCL_String *incrFunction(TCLR_Context **ctx,TCLS_Cmd *cmd){
	// incr {name} {value?}
	printf("fn %.*s\n",cmd->command->length,cmd->command->data);
	for(int32_t i = 0;i < cmd->length;i++){
		printf("- %.*s\n",cmd->arguments[i]->length,cmd->arguments[i]->data);
	}
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
	ctx = ctx;
	// proc {name} {args} {body}
	printf("fn %.*s\n",cmd->command->length,cmd->command->data);
	for(int32_t i = 0;i < cmd->length;i++){
		printf("- %.*s\n",cmd->arguments[i]->length,cmd->arguments[i]->data);
	}
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

#ifdef FN_REDEF_INIT
#define load_init fn_load_core_init
#endif
void load_init(TCLR_Context **ctx){
	insert_natFunction(ctx,"puts",putsFunction);
	insert_natFunction(ctx,"set" , setFunction);
	insert_natFunction(ctx,"incr",incrFunction);
	insert_natFunction(ctx,"proc",procFunction);
}
#ifdef FN_REDEF_INIT
#undef load_init
#endif



