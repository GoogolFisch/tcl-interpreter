#ifndef TCL_STRING_H_
#define TCL_STRING_H_

#include<stdint.h>
#include<stdlib.h>
#include"tcl_struct.h"
#include"tcl_type.h"


TCL_String *tcls_string_from_array(uint8_t str[],int32_t length,int32_t *index);
void tcls_insert_command(TCL_Commands *cmd,TCL_String *str,
		int32_t *strIdx,TCLS_CMD_FLAGS flags);

// until where
int32_t _tcls_string_get_length_array(uint8_t str[],int32_t length,int32_t index,
		int32_t preFlags){
	int32_t stringOffsets[TCLS_STRING_DEPTH];
	int32_t stackIdx = 0;
	int32_t beginning = index;
	int32_t ending = index;
	char state = 0;
	if(str[index] != '"' && str[index] != '{' && str[index] != '['){
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
			if(str[ending] == ']' && *(stringOffsets[stackIdx]) == '['){
				stackIdx--;
			}
			if(str[ending] == '}' && *(stringOffsets[stackIdx]) == '{'){
				stackIdx--;
			}
			if(str[ending] == '{'){
				stringOffsets[stackIdx] = *index;
				stackIdx++;
			}
			// also use preFlags
			if(str[ending] == '[' && *(stringOffsets[0]) == '"'){
				stringOffsets[stackIdx] = *index;
				stackIdx++;
			}
			if(str[ending] == '\\'){
				state = 1;
			}
		}
		else if(state == 1)state = 0;
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
			strOut->data[strOut->length++] = str[idx];
			if(str[idx] == '\\'){
				state = 1;
				continue;
			}
		}
		else{
			state = 0;
			//strOut->data[strOut->length++] = str[idx];
			//   /*
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
			else if(str[idx] == '$'){
				strOut->data[strOut->length++] = '$';
			}
			else
				strOut->data[strOut->length++] = str[idx];
			////   */
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
		TCL_String *cc = malloc(sizeof(TCL_String) +
				sizeof(char) * TCL_MIN_CAPACITY);
		cc->capacity = TCL_MIN_CAPACITY;
		cc->tags = TCL_ST_None;
		cc->refs = 1;
		cc->length = 0;
		//TCL_String *cc = tcls_string_from_array(
		//		str->data,str->length,index);
		int strIdx = 0;
		while(strIdx < str->length){
			//int32_t _tcls_string_get_length_array(
			//	uint8_t str[],int32_t length,int32_t index,

			if(strIdx->data[strIdx] == '{'){
			}
			if(strIdx->data[strIdx] == '['){
				// this is the best part!

			}



			strIdx++;
		}
		cmd->arguments[cmd->length] = cc;
		cmd->length++;
	}
}

void _tcls_sub_expr_cmd(TCL_Commands **cmd,
		TCL_String *outStr,int32_t *strIdx){
	char extraState;
	int32_t beg;
	// TODO add single [ ] multi run
	while(beg){
		beg = 0;
		outStr->length--;
		tcls_insert_command(cmd,outStr,&beg,TCLS_CMD_PUSH);
		outStr->length++;
		if(outStr->data[beg] == ';'){
			beg = 1;
			cmd->commands[cmd->length - 1]->flags = TCLS_CMD_NORMAL;
		}
	}
}

TCL_String *_tcls_cmd_get_string(TCL_Commands **cmd,TCL_String *str,
		int32_t *strIdx){
	int32_t start = *strIdx;
	int32_t ending = _tcls_string_get_length_array(&(str->data),
			str->length - start,start);
	int32_t idx = start;
	TCL_String *outStr;

	int32_t idx = start;
	//(*strIdx) = *idx;

	if(str->data[*start] == '{'){
		outStr = tcls_string_from_array(&(str->data),ending - start,&idx);
		(*strIdx) = idx;
		return outStr;
	}
	//
	outStr = malloc(sizeof(TCL_String) + sizeof(char) * (ending - start));
	int32_t stringOffsets[TCLS_STRING_DEPTH];
	int32_t stackIdx = 0;
	char state = 0;
	for(idx = start;idx < ending;idx++){
		if(state & 1 == 0){
			if(str[ending] == '"' && *(stringOffsets[stackIdx]) == '"'){
				stackIdx--;
			}
			if(str[ending] == ']' && *(stringOffsets[stackIdx]) == '['){
				stackIdx--;
				if(stackIdx == 0){
					state &= ~2;
					//
					beg = strinOffsets[stackIdx + 1] + 1;
					extraState = 1;
					_tcls_sub_expr_cmd(cmd,outStr,strIdx);
				}
			}
			if(str[ending] == '}' && *(stringOffsets[stackIdx]) == '{'){
				stackIdx--;
			}
			if(str[ending] == '{'){
				stringOffsets[stackIdx] = *index;
				stackIdx++;
			}
			// also use preFlags
			if(str[ending] == '['){
				stringOffsets[stackIdx] = *index;
				stackIdx++;
			}
			if(str[ending] == '\\'){
				state = 1;
				continue;
			}
			// exclude everything in [ ]
			if(state & 2 == 0){
				outStr->data[outStr->length] = str[ending];
				outStr->length++;
				if(str[ending] == '[' && stackIdx == 1)
					state |= 4;
			}
		}
		else if(state & 1 == 1)state &= (~1);
		if(stackIdx == 0)break;
	}



	return outStr;
}

void tcls_insert_command(TCL_Commands **cmd,TCL_String *str,
		int32_t *strIdx,TCLS_CMD_FLAGS flags){
	struct _TCLS_Cmd *icmd = malloc(sizeof(struct _TCLS_Cmd) +
			sizeof(TCL_String) * 0);
	icmd->length = 0;
	icmd->capacity = 0;
	icmd->flags = flags;
	icmd->stackDepth = 0;
	icmd->commands = NULL;
	icmd->moreData = NULL;

	icmd->commands = _tcls_cmd_get_string(cmd,str,strIdx);

	while(1){
		if(str->data[*strIdx] == '\n') break;
		if(str->data[*strIdx] == '\r') break;
		if(str->data[*strIdx] == ';') break;
		// normal way
		if(str->data[*strIdx] == ' ') (*strIdx)++;
		else{} // TODO: sould error?
		// regrow
		if(icmd->capacity <= icmd->length){
			icmd->capacity *= 2;
			if(icmd->capacity == 0)icmd->capacity = TCL_MIN_CAPACITY;
			icmd = realloc(sizeof(struct _TCLS_Cmd) +
					sizeof(TCL_String) * icmd->capacity);
		}
		//
		icmd->arguments[icmd->length] = _tcl_cmd_get_string(cmd,str,strIdx);
		icmd->length++;
	}

	if((*cmd)->length >= (*cmd)->capacity){
		(*cmd)->capacity *= 2;
		(*cmd) = realloc(sizeof(TCLS_Commands) +
			sizeof(struct _TCLS_Cmd*) * (*cmd)->capacity);
	}
	(*cmd)->commands[cmd->length] = icmd;
	(*cmd)->length++;
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
