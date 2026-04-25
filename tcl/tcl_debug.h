
#ifndef TCL_DEBUG_H_
#define TCL_DEBUG_H_

#include"tcl_struct.h"

void db_print_string(TCL_String *str){
	printf("String:%i/%i %i -> %.*s\n",
			str->length,str->capacity,str->gc.tags,
			str->length,str->data);
}
void db_print_cmd(TCLS_Cmd *cmd){
	printf("Cmd:%i/%i %i %iStack\n",
			cmd->length,cmd->capacity,
			(int32_t)cmd->flags,cmd->stackDepth);
	if(cmd->flags == TCLS_CMD_PUSH) printf("< ");
	else if(cmd->flags == TCLS_CMD_NORMAL) printf("> ");
	else printf("| ");
	TCL_String *str = cmd->command;
	printf("String:%i/%i %i -> %.*s\n",
			str->length,str->capacity,str->gc.tags,
			str->length,str->data);
	for(int32_t i = 0;i < cmd->length;i++){
		str = cmd->arguments[i];
		printf("- String:%i/%i %i -> %.*s\n",
			str->length,str->capacity,str->gc.tags,
			str->length,str->data);
	}
}
void db_print_commands(TCLS_Commands *com){
	printf("Commands:%i/%i %i\n",
			com->length,com->capacity,
			(int32_t)com->gc.tags);
	for(int32_t j = 0;j < com->length;j++){
		TCLS_Cmd *cmd = com->commands[j];
		printf("  Cmd:%i/%i %i %iStack\n",
				cmd->length,cmd->capacity,
				(int32_t)cmd->flags,cmd->stackDepth);
		if(cmd->flags == TCLS_CMD_PUSH) printf("  < ");
		else if(cmd->flags == TCLS_CMD_NORMAL) printf("  > ");
		else printf("  | ");
		TCL_String *str = cmd->command;
		printf("String:%i/%i %i -> %.*s\n",
				str->length,str->capacity,str->gc.tags,
				str->length,str->data);
		for(int32_t i = 0;i < cmd->length;i++){
			str = cmd->arguments[i];
			printf("  - String:%i/%i %i -> %.*s\n",
				str->length,str->capacity,str->gc.tags,
				str->length,str->data);
		}
	}
}
void db_print_arena(TCL_GarbageArena *arena){
	printf("Arena: %i/%i\n",arena->length,arena->capacity);
	for(int32_t i = 0;i < arena->length;i++){
		TCL_Disposable *place = arena->list[i];
		printf("%i #%i\n",
				place->tags,place->refs);
		/*printf("String:%i/%i %i #%i -> %.*s\n",
				str->length,str->capacity,str->gc.tags,str->refs,
				str->length,str->data);*/
	}
}
void db_raise_arena(TCL_GarbageArena *arena){
	for(int32_t i = 0;i < arena->length;i++){
		TCL_Disposable *place = arena->list[i];
		if(place->refs & 0xffff){
			free(place);
		}
		free(place);
	}
}

#endif
