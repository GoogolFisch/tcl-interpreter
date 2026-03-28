#ifndef TCL_STRING_H_
#define TCL_STRING_H_

#include<stdint.h>
#include<stdlib.h>
#include"tcl_struct.h"
#include"tcl_type.h"


TCL_String *tcls_string_from_array(uint8_t *str,int32_t length,int32_t *index);
void tcls_insert_command(TCLS_Commands **cmd,TCL_String *str,
		int32_t *strIdx,TCLS_CMD_FLAGS flags);
void tcls_insert_command(TCLS_Commands **cmd,TCL_String *str,
		int32_t *strIdx,TCLS_CMD_FLAGS flags);
TCLS_Commands *tcls_parse_commands(TCL_String *str);

// until where
int32_t _tcls_string_get_length_array(uint8_t str[],int32_t length,int32_t index,
		int32_t preFlags){
	preFlags ^= preFlags;
	if(length <= index)return -1;
	int32_t stringOffsets[TCLS_STRING_DEPTH];
	int32_t stackIdx = 0 + preFlags;
	//int32_t beginning = index;
	int32_t ending = index;
	char state = 0;
	if(str[index] != '"' && str[index] != '{' && str[index] != '['){
		// literal string (top layer)
		while(ending < length){
			if(state == 0){
				if(str[ending] == '\t') break;
				if(str[ending] == '\r') break;
				if(str[ending] == '\n') break;
				if(str[ending] == ' ') break;
				if(str[ending] == ';') break;
				if(str[ending] == '\\') state = 1;
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
	ending++;
	while(ending < length){
		if(state == 0){
			// MARK
			if(str[ending] == '"' && str[stringOffsets[stackIdx]] == '"'){
				stackIdx--;
			}
			if(str[ending] == ']' && str[stringOffsets[stackIdx]] == '['){
				stackIdx--;
			}
			if(str[ending] == '}' && str[stringOffsets[stackIdx]] == '{'){
				stackIdx--;
			}
			if(str[ending] == '{'){
				stringOffsets[stackIdx] = index;
				stackIdx++;
			}
			// also use preFlags
			if(str[ending] == '[' && str[stringOffsets[0]] == '"'){
				stringOffsets[stackIdx] = index;
				stackIdx++;
			}
			if(str[ending] == '\\'){
				state = 1;
			}
		}
		else if(state == 1)state = 0;
		if(stackIdx < 0)break;
		ending++;
	}
	if(ending >= length)
		return -1;
	return ending;
}

TCL_String *_tcls_make_string_from_bound(uint8_t str[],int32_t lower,int32_t upper){
	if(upper < lower)
		*(size_t**)NULL = NULL;
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
}

// ending of the seperating char
TCL_String *tcls_string_from_array(uint8_t *str,int32_t length,int32_t *index){
	int32_t beginning = *index;
	int32_t ending = _tcls_string_get_length_array(str,length,*index,0);
	if(ending < 0)
		return NULL;
	if(ending > length)
		return NULL;
	*index = ending;
	TCL_String *strOut;
	if(length >= beginning && str[beginning] != '"' && str[beginning] != '{'){
		strOut = _tcls_make_string_from_bound(str,beginning,ending);
		strOut->tags = TCL_ST_Variable;
	}else{
		strOut = _tcls_make_string_from_bound(str,beginning + 1,ending - 1);
		if(str[beginning] == '"')
			strOut->tags = TCL_ST_Variable;
	}
	return strOut;
}

TCL_String *_tcls_var_mkString(TCL_String *str,TCLS_Commands *tcmd,
		struct TCLS_Cmd **ptr_cmd,int32_t *index,int32_t ending){
	tcmd = tcmd;
	ptr_cmd = ptr_cmd;
	//
	TCL_String *cc = malloc(sizeof(TCL_String) +
			sizeof(char) * (ending - *index));
	cc->capacity = ending - *index;
	cc->tags = TCL_ST_None;
	cc->refs = 1;
	cc->length = 0;
	//TCL_String *cc = tcls_string_from_array(
	//		str->data,str->length,index);
	int32_t strIdx;
	for(strIdx = *index;strIdx < ending;strIdx++){
		// TODO TODOOO
		if(str->data[strIdx] == '['){
			// this is the best part!
		}
		if(cc->capacity <= cc->length){
			cc->capacity *= 2;
			cc = realloc(cc,sizeof(TCL_String) +
					sizeof(char) * cc->capacity);
		}
		cc->data[cc->length] = str->data[strIdx];
		cc->length++;
	}
	return cc;
}


void _tcls_sub_parse_arguments(TCL_String *str,TCLS_Commands *tcmd,
		struct TCLS_Cmd **ptr_cmd,int32_t *index){
	struct TCLS_Cmd *cmd = *ptr_cmd;
	tcmd = tcmd;
	// TODO: also add variable concat stuff!
	while(*index < str->length){
		// break
		while(str->data[*index] == ' ')
			(*index)++;
		if(str->data[*index] == ';')
			break;
		else if(str->data[*index] == '\n')
			break;
		else if(str->data[*index] == '\r')
			break;
		//
		int32_t ending = _tcls_string_get_length_array(
				str->data,str->length,*index,0);
		if(ending < 0)break;
		TCL_String *cc;
		if(str->data[*index] == '{'){
			cc = _tcls_make_string_from_bound(
					str->data,*index + 1,ending - 1);
		} else{
			cc = _tcls_var_mkString(str,tcmd,ptr_cmd,index,ending);
			cmd = *ptr_cmd;
		}
		*index = ending;
		//
		if(cmd->capacity <= cmd->length){
			if(cmd->capacity == 0)
				cmd->capacity = TCL_MIN_CAPACITY;
			else cmd->capacity *= 2;
			cmd = realloc(cmd,sizeof(struct TCLS_Cmd) +
						sizeof(TCL_String *) * cmd->capacity);
			*ptr_cmd = cmd;
		}
		cmd->arguments[cmd->length] = cc;
		cmd->length++;
		//*index = strIdx;
	}
}

void _tcls_sub_expr_cmd(TCLS_Commands **cmd,
		TCL_String *outStr,int32_t *strIdx){
	//char extraState;
	int32_t beg;
	strIdx = strIdx;
	// TODO add single [ ] multi run
	while(beg){
		beg = 0;
		outStr->length--;
		tcls_insert_command(cmd,outStr,&beg,TCLS_CMD_PUSH);
		outStr->length++;
		if(outStr->data[beg] == ';'){
			beg = 1;
			(*cmd)->commands[(*cmd)->length - 1]->flags = TCLS_CMD_NORMAL;
		}
	}
}

TCL_String *_tcls_cmd_get_string(TCLS_Commands **cmd,TCL_String *str,
		int32_t *strIdx){
	int32_t start = *strIdx;
	// ???
	int32_t ending = _tcls_string_get_length_array(str->data,
			str->length - start,start,0);
	int32_t idx = start;
	TCL_String *outStr;

	//int32_t idx = start;
	//(*strIdx) = *idx;

	if(str->data[start] == '{'){
		outStr = tcls_string_from_array(str->data,ending - start,&idx);
		(*strIdx) = idx;
		return outStr;
	}
	//
	outStr = malloc(sizeof(TCL_String) + sizeof(char) * (ending - start));
	int32_t stringOffsets[TCLS_STRING_DEPTH];
	int32_t stackIdx = 0;
	int32_t beg;
	char state = 0;
	for(idx = start;idx < ending;idx++){
		if((state & 1) == 0){
			if(str->data[ending] == '"' && str->data[stringOffsets[stackIdx]] == '"'){
				stackIdx--;
			}
			if(str->data[ending] == ']' && str->data[stringOffsets[stackIdx]] == '['){
				stackIdx--;
				if(stackIdx == 0){
					state &= ~2;
					// TODO ???
					beg = stringOffsets[stackIdx + 1] + 1;
					//extraState = 1;
					_tcls_sub_expr_cmd(cmd,outStr,&beg);
				}
			}
			if(str->data[ending] == '}' && str->data[stringOffsets[stackIdx]] == '{'){
				stackIdx--;
			}
			if(str->data[ending] == '{'){
				stringOffsets[stackIdx] = idx;
				stackIdx++;
			}
			// also use preFlags
			if(str->data[ending] == '['){
				stringOffsets[stackIdx] = idx;
				stackIdx++;
			}
			if(str->data[ending] == '\\'){
				state = 1;
				continue;
			}
			// exclude everything in [ ]
			if((state & 2) == 0){
				outStr->data[outStr->length] = str->data[ending];
				outStr->length++;
				if(str->data[ending] == '[' && stackIdx == 1)
					state |= 4;
			}
		}
		else if((state & 1) == 1)state &= (~1);
		if(stackIdx == 0)break;
	}



	return outStr;
}

void tcls_insert_command(TCLS_Commands **cmd,TCL_String *str,
		int32_t *strIdx,TCLS_CMD_FLAGS flags){
	struct TCLS_Cmd *icmd = malloc(sizeof(struct TCLS_Cmd) +
			sizeof(TCL_String) * 0);
	icmd->length = 0;
	icmd->capacity = 0;
	icmd->flags = flags;
	icmd->stackDepth = 0;
	icmd->command = NULL;
	icmd->moreData = NULL;

	icmd->command = _tcls_cmd_get_string(cmd,str,strIdx);

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
			icmd = realloc(icmd,sizeof(struct TCLS_Cmd) +
					sizeof(TCL_String) * icmd->capacity);
		}
		//
		icmd->arguments[icmd->length] = _tcls_cmd_get_string(cmd,str,strIdx);
		icmd->length++;
	}

	if((*cmd)->length >= (*cmd)->capacity){
		(*cmd)->capacity *= 2;
		(*cmd) = realloc(*cmd,sizeof(TCLS_Commands) +
			sizeof(struct TCLS_Cmd*) * (*cmd)->capacity);
	}
	(*cmd)->commands[(*cmd)->length] = icmd;
	(*cmd)->length++;
}

TCLS_Commands *tcls_parse_commands(TCL_String *str){
	TCLS_Commands *tcmd = malloc(sizeof(TCLS_Commands) +
			sizeof(struct TCLS_Cmd*) * TCL_MIN_CAPACITY);
	tcmd->tags = 0;
	tcmd->refs = 0;
	tcmd->length = 0;
	tcmd->capacity = TCL_MIN_CAPACITY;

	int32_t cmdBeginn = 0;
	//int32_t cmdEnd = str->length;
	int32_t cmdIdx = 0;

	while(cmdBeginn < str->length){
		if(str->data[cmdIdx] == '\n') cmdIdx++;
		TCL_String *cmd = tcls_string_from_array(
				str->data,str->length,&cmdIdx);
		if(cmd == NULL)break;
		struct TCLS_Cmd *lowCmd = malloc(sizeof(struct TCLS_Cmd));
		lowCmd->moreData = NULL;
		lowCmd->length = 0;
		lowCmd->capacity = 0;
		lowCmd->command = cmd;
		// todo add more!
		while(cmdIdx < str->length && str->data[cmdIdx] == ' '){
			cmdIdx++;
			_tcls_sub_parse_arguments(str,tcmd,&lowCmd,&cmdIdx);
			// break on '\n\r;'
		}
		if(tcmd->capacity <= tcmd->length){
			tcmd->capacity *= 2;
			tcmd = realloc(tcmd,sizeof(TCLS_Commands) +
				sizeof(struct TCLS_Cmd*) * tcmd->capacity);
		}
		int32_t cm = tcmd->length++;
		tcmd->commands[cm] = lowCmd;
		cmdBeginn = cmdIdx;
	}

	return tcmd;
}


#endif
