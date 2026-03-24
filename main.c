
#include<stdlib.h>
#include<stdint.h>
#include<stdio.h>
#include"tcl_struct.h"
#include"tcl_type.h"
#include"tcl_string.h"
#include"tcl_function.h"
#include"tcl_run.h"

// bla bla bla bla

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
	return 0;
}
int32_t print_error(int32_t argc,char **argv,char **env){
	argc = argc;
	argv = argv;
	env = env;
	return 0;
}
int32_t set_variable(int32_t argc,char **argv,char **env){
	argc = argc;
	argv = argv;
	env = env;
	return 0;
}

int32_t make_and_run(int32_t length,char *fData){
	// TODO
	TCL_StringArena *ar = tcl_create_string_arena();
	TCL_String *str = tcl_create_string(length,fData);
	TCLS_Commands *tcmd = tcls_parse_commands(str);
	TCLR_Context *ctx = tclr_make_context(NULL,TCLR_FULL_LAYER);
	ctx->program = tcmd;
	ctx->arena = ar;
	while(ctx != NULL){
		tclr_step_instruction(&ctx);
	}
	return 0;
}

int32_t main(int32_t argc,char **argv,char **env){
	if(argc <= 1){
		return print_help(argc,argv,env);
	}
	FILE *fptr;
	char *fileName = NULL;
	while(fileName == NULL){
		argc--;
		argv = &(argv[1]);
		if(argc < 1)
			return print_error(argc,argv,env);
		if(argv[0][0] == '-'){
			if(set_variable(argc,argv,env))
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
	fread(fileData,length,sizeof(char),fptr);
	
	//printf("File: %s\n%s\n",fileName,fileData);
	make_and_run(length,fileData);

	fclose(fptr);
}
