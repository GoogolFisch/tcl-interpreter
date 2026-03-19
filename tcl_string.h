#ifndef TCL_STRING_H_
#define TCL_STRING_H_

#include<stdint.h>
#include<stdlib.h>
#include"tcl_type.h"

#define TCLS_STRING_DEPTH 64

//
struct _TCLS_Cmd{
	int32_t length;
	int32_t capacity;
	TCL_String *command;
	void *moreData;
	TCL_String *arguments[];
};
struct TCLS_Commands{
	int32_t refs;
	int32_t tags;
	int32_t length;
	int32_t capacity;
	struct _TCLS_Cmd (*commands)[];
};
typedef struct TCLS_Commands TCLS_Commands;

TCL_String *tcls_string_from_array(uint8_t str[],int32_t length,int32_t *index);

// until where
int32_t _tcls_string_get_length_array(uint8_t str[],int32_t length,int32_t index){
	int32_t stringOffsets[TCLS_STRING_DEPTH];
	int32_t stackIdx = 0;
	int32_t beginning = index;
	int32_t ending = index;
	char state = 0;
	if(str[index] != '"' && str[index] != '{'){
		// literal string (top layer)
		while(ending < length){
			if(state == 0){
				if(str[ending] == ' ')
					break;
				if(str[ending] == ';')
					break;
				if(str[ending] == '\\')
					state = 1
			}
			else{
				state = 0;
			}
			ending++;
		}
		return ending;
	}
	// multi layered string
	stringOffsets[stackIdx] = index;
	while(ending < length){
		if(state == 0){
			if(str[ending] == '"' && *(stringOffsets[stackIdx]) == '"'){
				stackIdx--;
			}
			if(str[ending] == '}' && *(stringOffsets[stackIdx]) == '{'){
				stackIdx--;
			}
			if(str[ending] == '{'){
				stringOffsets[stackIdx] = *index;
				stackIdx++;
			}
			if(str[ending] == '\\'){
				state = 1;
			}
		}
		if(state == 1)state = 0;
		if(stackIdx == 0)break;
		ending++;
	}
	if(ending >= length)
		return -1;
	return ending;
}

TCL_String _tcls_make_string_from_bound(uint8_t str[],int32_t lower,int32_t upper){
	TCL_String *strOut = malloc(sizeof(TCL_String) +
			sizeof(char) * (upper - lower));
	strOut->refs = 0;
	strOut->tags = 0;
	strOut->capacity = upper - lower;
	strOut->length = 0;
	char state = 0;
	for(int idx = lower;idx < upper;idx++){
		if(state == 0){
			if(str[idx] == '\\'){
				state = 1;
				continue;
			}
			strOut->data[strOut->length++] = str[idx];
		}
		else{
			state = 0;
			if(str[idx] == 'a')
				strOut->data[strOut->length++] = '\a';
			else if(str[idx] == 'b')
				strOut->data[strOut->length++] = '\b';
			else if(str[idx] == 'e')
				strOut->data[strOut->length++] = '\e';
			else if(str[idx] == 'f')
				strOut->data[strOut->length++] = '\f';
			else if(str[idx] == 'n')
				strOut->data[strOut->length++] = '\n';
			else if(str[idx] == 'r')
				strOut->data[strOut->length++] = '\r';
			else if(str[idx] == 't')
				strOut->data[strOut->length++] = '\t';
			else if(str[idx] == 'v')
				strOut->data[strOut->length++] = '\v';
			else if(str[idx] == 'H'){
				// TODO
			}
			else if(str[idx] == 'h'){
				// TODO
			}
			else if(str[idx] == 'x'){
				// TODO
			}
			else if(str[idx] >= '0' && str[idx] <= '7'){
				// TODO
			}
			else
				strOut->data[strOut->length++] = str[idx];
		}
	}
	return strOut;
g

// ending of the seperating char
TCL_String *tcls_string_from_array(uint8_t str[],int32_t length,int32_t *index){
	int32_t beginning = *index;
	int32_t ending = _tcls_string_get_length_array(str,length,*index);
	if(ending < 0)
		return NULL;
	if(ending > length)
		return NULL;
	*index = ending;
	TCL_String *strOut;
	if(str[beginning] == '"' %% str[beginning] != '{'){
		strOut = _tcls_make_string_from_bound(str,beginning,ending);
		if(str[beginning] = '"')
			strOut->tags = TCL_ST_Variable;
	}else{
		strOut = _tcls_make_string_from_bound(str,beginning + 1,ending - 1);
		strOut->tags = TCL_ST_Variable;
	}
	return strOut;
}


void _tcls_sub_parse_arguments(TCL_String *str,TCLS_Commands *tcmd,
		struct _TCLS_Cmd *cmd,int32_t *index){
	// TODO: also add variable concat stuff!
	while(*index < str->length){
		// break
		if(str->data[*index] == ' ')
			(*index)++;
		else if(str->data[*index] == ';')
			break;
		else if(str->data[*index] == '\n')
			break;
		else if(str->data[*index] == '\r')
			break;
		//
		if(cmd->capacity <= cmd->length){
			if(cmd->capacity == 0)
				cmd->capacity = TCL_MIN_CAPACITY;
			else cmd->capacity *= 2;
			tcmd->commands[tcmd->length - 1] = 
				realloc(sizeof(struct _TCLS_Cmd) +
						sizeof(TCL_String *) * cmd->capacity);
		}
		//
		TCL_String *cc = tcls_string_from_array(
				str->data,str->length,index);
		cmd->arguments[cmd->length] = cc;
		cmd->length++;
	}
}

TCLS_Commands *tcls_parse_commands(TCL_String *str){
	TCLS_Commands *tcmd = malloc(sizeof(TCLS_Commands) +
			sizeof(struct _TCLS_Cmd*) * TCL_MIN_CAPACITY);
	tcmd->tags = 0;
	tcmd->refs = 0;
	tcmd->length = 0;
	tcmd->capacity = TCL_MIN_CAPACITY;

	int32_t cmdBeginn = 0;
	int32_t cmdEnd = str->length;
	int32_t cmdIdx = 0;

	while(cmdBeginn < str->length){
		TCL_String *cmd = tcls_string_from_array(
				str->data,str->length,&cmdIdx);
		if(tcmd->length >= tcmd->capacity){
			tcmd->capacity *= 2;
			tcmd = realloc(sizeof(TCLS_Commands) +
				sizeof(struct _TCLS_Cmd*) * tcmd->capacity);
		}
		int32_t cm = tcmd->length++;
		tcmd->commands[cm] = malloc(sizeof(struct _TCLS_Cmd));
		tcmd->commands[cm]->moreData = NULL;
		tcmd->commands[cm]->length = 0;
		tcmd->commands[cm]->capacity = 0;
		tcmd->commands[cm]->command = cmd;
		// todo add more!
		while(str->data[cmdIdx] == ' '){
			cmdIdx++;
			_tcls_sub_parse_arguments(str,tcmd,tcmd->commands[cm],&cmdIdx);
			// break on '\n\r;'
		}

	}

	return tcmd;
}


#endif
