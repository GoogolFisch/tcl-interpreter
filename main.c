
#include<stdlib.h>
#include <unistd.h>
#include<stdint.h>
#include<stdio.h>
#define TCL_IMPLEMENTATION
#include"tcl/include.h"
#include"tcl/tcl_debug.h"
#define FN_REDEF_INIT
#include"func/core.c"

// bla bla bla bla

//char g_ShowAst = 0;

int32_t print_error(int32_t argc,char **argv,char **env){
	argc = argc;
	argv = argv;
	env = env;
	return 0;
}
struct densify{
	char *name;
	enum TCLRG_VARIABLE val;
};
int32_t denC = 3;
const struct densify den[] = {
	{.name = "show-ast",.val=TCLRG_SHOW_AST},
	{.name = "show-gc",.val=TCLRG_SHOW_GC},
	{.name = "verb-exec",.val=TCLRG_VERBOSE_EXEC},
	{.name = NULL,.val = 0},
};
int32_t print_help(int32_t argc,char **argv,char **env){
	printf("Usage of %s\n",argv[0]);
	printf("Arg count:%i\n",argc);
	int32_t i = 0;
	for(i = 0;i < argc;i++){
		printf("- %s\n",argv[i]);
	}
	printf("Env:\n");
	for(i = 0;env[i] != NULL;i++){
		printf("- %s\n",env[i]);
	}
	printf("Flags:\n");
	for(i = 0;i < denC && den[i].name != NULL;i++){
		printf("-%s\n",den[i].name);
	}
	return 0;
}
int32_t set_variable(int32_t argc,char **argv,char **env,enum TCLRG_VARIABLE *flag){
	argc = argc;
	argv = argv;
	for(int32_t j = 0;j < denC && den[j].name != NULL;j++){
		for(int32_t idx = 1;argv[0][idx] != 0;idx++){
			if(argv[0][idx] != den[j].name[idx - 1])
				goto _set_var_continue;
		}
		*flag |= den[j].val;

_set_var_continue:
	}
	env = env;
	return 0;
}

void testFn(TCLR_Context **ctx,TCLS_Cmd *cmd){
	ctx = ctx;
	printf("fn %.*s\n",cmd->command->length,cmd->command->data);
	for(int32_t i = 0;i < cmd->length;i++){
		printf("- %.*s\n",cmd->arguments[i]->length,cmd->arguments[i]->data);
	}
}
void setFn(TCLR_Context **ctx,TCLS_Cmd *cmd){
	printf("fn %.*s\n",cmd->command->length,cmd->command->data);
	for(int32_t i = 0;i < cmd->length;i++){
		printf("- %.*s\n",cmd->arguments[i]->length,cmd->arguments[i]->data);
	}
	if(cmd->length != 2){
		printf("Err with set (fa1889db-c6a3-4a1b-b927-96d77805c985)\n");
		return;
	}
	tcl_set_into_scope(&((*ctx)->scope),cmd->arguments[0],cmd->arguments[1]);
}
#define makeStr(var,msg) TCL_String *var;                                      \
 var = malloc(sizeof(TCL_String) + sizeof(char) * sizeof(msg));                \
 var->var.typ = 0;                                                             \
 var->deferCallback = NULL;                                                    \
 var->replaceWith = NULL;                                                      \
 var->freeCallback = NULL;                                                     \
 var->capacity = sizeof(msg);                                                  \
 var->length = sizeof(msg) - 1;                                                \
 for(int32_t __var_o = 0;__var_o < (int32_t)sizeof(msg);__var_o++){            \
  var->data[__var_o] = msg[__var_o];                                           \
 }

TCLR_Context *make_ctx(int32_t length,char *fData){
	// TODO
	TCLF_Scope *fnScope = tclf_make_function_scope();
	/* /
	makeStr(testFunc,"puts");
	tclf_insert_natFunction(&fnScope,testFunc,(void(*)())testFn);
	makeStr(setFunc,"set");
	tclf_insert_natFunction(&fnScope,setFunc,(void(*)())setFn);
	//  */
	TCL_StringArena *ar = tcl_create_string_arena();
	TCL_String *str = tcl_create_string(length,fData);
	TCLS_Commands *tcmd = tcls_parse_commands(&ar,str);
	tcmd->refs++;
	TCLR_Context *ctx = tclr_make_context(NULL,TCLR_FULL_LAYER);
	ctx->program = tcmd;
	ctx->arena = ar;
	ctx->fnScope = fnScope;
#ifdef FN_REDEF_INIT
	fn_load_core_init(&ctx);
#endif
	free(str);
	return ctx;
}
void run_ctx(TCLR_Context **ctx){
	while(*ctx != NULL){
		tclr_step_instruction(ctx);
	}
}
void free_ctx(TCLR_Context *ctx){
	TCLR_Context *low_ctx;
	while(ctx != NULL){
		tcl_garbage_collect_string_arena(&(ctx->arena));
		low_ctx = ctx->parent;
		tclr_free_context(ctx);
		ctx = low_ctx;
	}
}

void oom_adj_set(void){
#ifndef OOM_NO_ADJ
	char spaceStr[256];
	snprintf(spaceStr,255,"/proc/%i/oom_score_adj",getpid());
	spaceStr[255] = 0;
	FILE *oom_ptr = fopen(spaceStr,"w");
	if(oom_ptr == NULL){
		printf("ERROR not able to open %s\n",spaceStr);
		return;
	}
	char oom_value[] = "500";
	fwrite(oom_value,sizeof(char),sizeof(oom_value),oom_ptr);
	fclose(oom_ptr);
#endif
}

int32_t main(int32_t argc,char **argv,char **env){
	if(argc <= 1){
		return print_help(argc,argv,env);
	}
	enum TCLRG_VARIABLE globFlag = 0;
	oom_adj_set();
	FILE *fptr;
	char *fileName = NULL;
	while(fileName == NULL){
		argc--;
		argv = &(argv[1]);
		if(argc < 1)
			return print_error(argc,argv,env);
		if(argv[0][0] == '-'){
			if(set_variable(argc,argv,env,&globFlag))
				return print_error(argc,argv,env);
			continue;
		}
		fileName = argv[0];
	}
	fptr = fopen(fileName,"r");
	fseek(fptr,0,SEEK_END);
	int32_t length = ftell(fptr);
	fseek(fptr,0,SEEK_SET);
	char *fileData = malloc(sizeof(char) * (length + 3));
	fileData[length] = 0;
	fileData[length + 1] = 0;
	int32_t newLength = fread(fileData,sizeof(char),length,fptr);
	fclose(fptr);
	if(newLength != length){
		printf("Error with reading file!\nExpecting %i but got %i\n",length,newLength);
		return 1;
	}
	
	//printf("File: %s\n%s\n",fileName,fileData);
	TCLR_Context *ctx = make_ctx(length,fileData);
	ctx->globFlags = globFlag;
	if(globFlag & TCLRG_SHOW_AST){
		db_print_commands(ctx->program);
	}
	run_ctx(&ctx);
	/*if(!g_ShowAst)
		run_ctx(&ctx);
	else
		db_print_commands(ctx->program);
	*/


	free_ctx(ctx);
	free(fileData);
	//tcl_garbage_collect_string_arena(&(ctx->arena));

}
