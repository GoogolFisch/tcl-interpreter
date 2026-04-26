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
TCLS_Cmd *tcls_cmd_parse(TCL_GarbageArena **garbageArena,
		TCLS_Commands **tcmd,TCL_String *str,
		int32_t *index,int32_t upper,int32_t stackOffset);
TCLS_Commands *tcls_parse_commands(TCL_GarbageArena **garbageArena,TCL_String *str);
void tcls_free_cmd(TCLS_Cmd **cmd);
void tcls_free_commands(TCLS_Commands **tcmd);

TCL_String *tcls_list_iter(TCL_Slice **slice);


// until where
static int32_t _tcls_string_skip_white(uint8_t str[],int32_t length,int32_t index);
static int32_t _tcls_string_length_square(uint8_t str[],int32_t length,int32_t index);
//
static int32_t _tcls_string_skip_white(uint8_t str[],int32_t length,int32_t index){
	for(;index < length;index++){
		if(str[index] == ' ')continue;
		if(str[index] == '\t')continue;
		if(str[index] == '\r')continue;
		if(str[index] == '\n')continue;
		if(str[index] == ';')continue;
		break;
	}
	return index;
}
static int32_t _tcls_string_length_word(uint8_t str[],int32_t length,int32_t index,char brk){
	// from " this untill this "
	int32_t ending = index;
	char state = 0;
	ending++;
	while(ending < length){
		if(state == 0){
			if(str[ending] == '[')
				ending = _tcls_string_length_square(str,length,ending);
			if(str[ending] == '\t') break;
			if(str[ending] == '\r') break;
			if(str[ending] == '\n') break;
			if(str[ending] == ' ') break;
			if(str[ending] == ';') break;
			if(str[ending] == brk) break;
			if(str[ending] == '\\') state = 1;
		}
		else{
			state = 0;
		}
		ending++;
	}
	if(ending > length)
		return -1;
	return ending;
}
static int32_t _tcls_string_length_quote(uint8_t str[],int32_t length,int32_t index){
	// from " this untill this "
	int32_t ending = index;
	char state = 0;
	if(str[index] != '"')return ending;
	ending++;
	while(ending < length){
		if(state == 0){
			if(str[ending] == '"'){
				ending++;
				break;
			}
			if(str[ending] == '\\') state = 1;
		}
		else state = 0;
		ending++;
	}
	if(ending > length)
		return -1;
	return ending;
}
static int32_t _tcls_string_length_curly(uint8_t str[],int32_t length,int32_t index){
	int32_t stackIdx = 0;
	//int32_t beginning = index;
	int32_t ending = index;
	char state = 0;
	if(str[index] != '{')return ending;
	ending++;
	while(ending < length){
		if(state == 0){
			// MARK
			if(str[ending] == '}')
				stackIdx--;
			if(str[ending] == '{')
				stackIdx++;
			if(str[ending] == '\\')
				state = 1;
		}
		else if(state == 1)state = 0;
		ending++;
		if(stackIdx < 0)break;
	}
	if(ending > length)
		return -1;
	return ending;
}
static int32_t _tcls_string_length_square(uint8_t str[],int32_t length,int32_t index){
	//int32_t beginning = index;
	int32_t ending = index;
	char state = 0;
	if(str[index] != '[')return ending;
	ending++;
	while(ending < length){
		if(state == 0){
			// MARK
			if(str[ending] == ']'){
				ending++;
				break;
			}
			if(str[ending] == '{')
				ending = _tcls_string_length_curly(str,length,ending);
			else if(str[ending] == '"')
				ending = _tcls_string_length_quote(str,length,ending);
			else
				ending = _tcls_string_length_word(str,length,ending,']');
			/* if(str[ending] == '\\')
				state = 1;
			// */
		}
		else if(state == 1)state = 0;
		//ending++;
	}
	if(ending > length)
		return -1;
	return ending;
}
static int32_t _tcls_string_get_length_array(uint8_t str[],int32_t length,int32_t index,
		int32_t preFlags){
	preFlags = preFlags;
	if(length <= index)return -1;
	if(str[index] == '[')return _tcls_string_length_square(str,length,index);
	if(str[index] == '{')return _tcls_string_length_curly(str,length,index);
	if(str[index] == '"')return _tcls_string_length_quote(str,length,index);
	if(str[index] != '"' && str[index] != '{' && str[index] != '['){
		return _tcls_string_length_word(str,length,index,';');
	}
	// multi layered string
	return -1;
}

static TCL_String *_tcls_make_string_from_bound(uint8_t str[],
		int32_t lower,int32_t upper,char doChange){
	if(upper < lower)
		*(size_t**)NULL = NULL;
	TCL_String *strOut = malloc(sizeof(TCL_String) +
			sizeof(char) * (upper - lower));
	strOut->var = NULL;
	strOut->deferCallback = NULL;
	strOut->replaceWith = NULL;
	strOut->gc.freeCallback = (void*)tcl_free_string;
	strOut->gc.refs = 0;
	strOut->gc.tags = TCL_ST_None;
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
			if(str[idx] == '$'){
				strOut->gc.tags = TCL_ST_Variable;
			}
		}
		else{
			state = 0;
			if(!doChange){
				strOut->data[strOut->length++] = '\\';
				strOut->data[strOut->length++] = str[idx];
			}
			//strOut->data[strOut->length++] = str[idx];
			//   /*
			else if(str[idx] == 'a')
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
		strOut = _tcls_make_string_from_bound(str,beginning,ending,1);
		//strOut->tags = TCL_ST_Variable;
	}else if(str[beginning] == '"'){
		strOut = _tcls_make_string_from_bound(str,beginning + 1,ending - 1,1);
		//strOut->tags = TCL_ST_Variable;
	}else if(str[beginning] == '{'){
		strOut = _tcls_make_string_from_bound(str,beginning + 1,ending - 1,0);
		strOut->gc.tags = TCL_ST_None;
	}else{
		// RAISE ERROR
		strOut = _tcls_make_string_from_bound(str,beginning + 1,ending - 1,0);
		strOut->gc.tags = TCL_ST_None;
		if(str[beginning] == '"')
			strOut->gc.tags = TCL_ST_Variable;
	}
	strOut->gc.refs++;
	return strOut;
}

static TCL_String *_tcls_var_mkString(TCL_GarbageArena **garbageArena,
		TCL_String *str,TCLS_Commands **tcmd,
		TCLS_Cmd **ptr_cmd,int32_t *index,
		int32_t ending,int32_t *cStack){
	tcmd = tcmd;
	ptr_cmd = ptr_cmd;
	//
	TCL_String *cc = malloc(sizeof(TCL_String) +
			sizeof(char) * (ending - *index));
	cc->capacity = ending - *index;
	cc->var = NULL;
	cc->deferCallback = NULL;
	cc->replaceWith = NULL;
	cc->gc.freeCallback = (void*)tcl_free_string;
	cc->gc.tags = TCL_ST_None;
	cc->gc.refs = 0;
	cc->length = 0;
	//TCL_String *cc = tcls_string_from_array(
	//		str->data,str->length,index);
	int32_t strIdx = 0;
	if(str->data[*index] == '"'){
		// remove " and " from strings
		strIdx++;
		ending--;
	}
	for(strIdx += *index;strIdx < ending;strIdx++){
		// TODO TODOOO
		if(str->data[strIdx] == '['){
			cc->gc.tags = TCL_ST_Variable;
			cc->data[cc->length] = str->data[strIdx];
			cc->length++;
			int32_t lower = strIdx + 1;
			int32_t upper = _tcls_string_length_square(
					str->data,str->length,strIdx) - 1;
			TCLS_Cmd *cmd = NULL;
			while(lower < upper){
				TCLS_Cmd *lowCmd = tcls_cmd_parse(garbageArena,
						tcmd,str,&lower,upper,*cStack);
				if(lowCmd == NULL)break;
				cmd = lowCmd;
			}
			(*cStack)++;
			if(cmd != NULL){
				cmd->flags = TCLS_CMD_PUSH;
			}
			// todo
			lower = lower;
			strIdx = upper + 1;
			cc->data[cc->length] = str->data[strIdx - 1];
			cc->length++;
			// this is the best part!
			continue;
		}
		else if(str->data[strIdx] == '$'){
			cc->gc.tags = TCL_ST_Variable;
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


static void _tcls_sub_parse_arguments(TCL_GarbageArena **garbageArena,
		TCL_String *str,TCLS_Commands **tcmd,
		TCLS_Cmd **ptr_cmd,int32_t *index,
		int32_t upper,int32_t *cStack){
	TCLS_Cmd *cmd = *ptr_cmd;
	tcmd = tcmd;
	// TODO: also add variable concat stuff!
	while(*index < upper){
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
				str->data,upper,*index,0);
		if(ending < 0)break;
		TCL_String *cc;
		if(str->data[*index] == '{'){
			cc = _tcls_make_string_from_bound(
					str->data,*index + 1,ending - 1,0);
			cc->gc.tags = TCL_ST_None;
		} else{
			cc = _tcls_var_mkString(garbageArena,str,tcmd,
					ptr_cmd,index,ending,cStack);
			cmd = *ptr_cmd;
		}
		tcl_set_garbage_arena(garbageArena,(TCL_Disposable*)cc);
		cc->gc.refs++;
		*index = ending;
		//
		if(cmd->capacity <= cmd->length){
			if(cmd->capacity == 0)
				cmd->capacity = TCL_MIN_CAPACITY;
			else cmd->capacity *= 2;
			cmd = realloc(cmd,sizeof(TCLS_Cmd) +
						sizeof(TCL_String *) * cmd->capacity);
			*ptr_cmd = cmd;
		}
		cmd->arguments[cmd->length] = cc;
		cmd->length++;
		//*index = strIdx;
	}
}

static void _tcls_sub_expr_cmd(TCLS_Commands **cmd,
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

static TCL_String *_tcls_cmd_get_string(TCLS_Commands **cmd,TCL_String *str,
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
		// this is double work!
		outStr = tcls_string_from_array(str->data,ending,&idx);
		(*strIdx) = idx;
		return outStr;
	}
	//
	outStr = malloc(sizeof(TCL_String) + sizeof(char) * (ending - start));
	outStr->var = NULL;
	outStr->gc.freeCallback = (void*)tcl_free_string;
	outStr->deferCallback = NULL;
	outStr->replaceWith = NULL;
	char startChar = str->data[*strIdx];
	if(startChar == '"'){start++;ending--;}
	int32_t beg;
	char state = 0;
	for(idx = start;idx < ending;idx++){
		if(state == 0 && startChar == '"' && str->data[ending] == '"'){
			break;
		}
		if(state == 0 && str->data[ending] == '['){
			outStr->data[outStr->length] = '[';
			outStr->length++;
			// TODO ???
			beg = ending;
			_tcls_sub_expr_cmd(cmd,outStr,&beg);
			//
			outStr->data[outStr->length] = ']';
			outStr->length++;
		}
		if(str->data[ending] == '\\'){
			state ^= 1;
			continue;
		}
		// exclude everything in [ ]
		if(state == 0 || startChar == '{'){
			outStr->data[outStr->length] = str->data[ending];
			outStr->length++;
		}
	}


	return outStr;
}

void tcls_insert_command(TCLS_Commands **cmd,TCL_String *str,
		int32_t *strIdx,TCLS_CMD_FLAGS flags){
	TCLS_Cmd *icmd = malloc(sizeof(TCLS_Cmd) +
			sizeof(TCL_String) * 0);
	icmd->length = 0;
	icmd->capacity = 0;
	icmd->flags = flags;
	icmd->stackDepth = 0;
	icmd->command = NULL;
	icmd->deferFree = NULL;

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
			icmd = realloc(icmd,sizeof(TCLS_Cmd) +
					sizeof(TCL_String) * icmd->capacity);
		}
		//
		icmd->arguments[icmd->length] = _tcls_cmd_get_string(cmd,str,strIdx);
		icmd->length++;
	}

	if((*cmd)->length >= (*cmd)->capacity){
		(*cmd)->capacity *= 2;
		(*cmd) = realloc(*cmd,sizeof(TCLS_Commands) +
			sizeof(TCLS_Cmd*) * (*cmd)->capacity);
	}
	(*cmd)->commands[(*cmd)->length] = icmd;
	(*cmd)->length++;
}
TCLS_Cmd *tcls_cmd_parse(TCL_GarbageArena **garbageArena,
		TCLS_Commands **tcmd,TCL_String *str,
		int32_t *index,int32_t upper,int32_t stackOffset){
	for(;*index < str->length;(*index)++){
		if(str->data[*index] == ' ')continue;
		if(str->data[*index] == '\n')continue;
		if(str->data[*index] == '\r')continue;
		if(str->data[*index] == '\t')continue;
		break;
	}
	TCL_String *cmd = tcls_string_from_array(
			str->data,str->length,index);//,&stackOffset);
	if(cmd == NULL)return NULL;
	tcl_set_garbage_arena(garbageArena,(TCL_Disposable*)cmd);

	TCLS_Cmd *lowCmd = malloc(sizeof(TCLS_Cmd));
	lowCmd->moreData = NULL;
	lowCmd->length = 0;
	lowCmd->capacity = 0;
	lowCmd->flags = TCLS_CMD_NORMAL;
	lowCmd->stackDepth = stackOffset;
	lowCmd->command = cmd;
	lowCmd->deferFree = NULL;
	// todo add more!
	while(*index < upper && str->data[*index] == ' '){
		// run only once!?
		(*index)++;
		_tcls_sub_parse_arguments(garbageArena,str,tcmd,
				&lowCmd,index,upper,&stackOffset);
		// break on '\n\r;'
	}
	if((*tcmd)->capacity <= (*tcmd)->length){
		(*tcmd)->capacity *= 2;
		(*tcmd) = realloc((*tcmd),sizeof(TCLS_Commands) +
			sizeof(TCLS_Cmd*) * (*tcmd)->capacity);
	}
	int32_t cm = (*tcmd)->length++;
	(*tcmd)->commands[cm] = lowCmd;
	return lowCmd;
}

TCLS_Commands *tcls_parse_commands(TCL_GarbageArena **garbageArena,TCL_String *str){
	TCLS_Commands *tcmd = malloc(sizeof(TCLS_Commands) +
			sizeof(TCLS_Cmd*) * TCL_MIN_CAPACITY);
	tcmd->gc.tags = 0;
	tcmd->gc.refs = 0;
	tcmd->length = 0;
	tcmd->capacity = TCL_MIN_CAPACITY;

	int32_t cmdBeginn = 0;
	//int32_t cmdEnd = str->length;
	int32_t cmdIdx = 0;

	while(cmdBeginn < str->length){
		TCLS_Cmd *lowCmd = tcls_cmd_parse(garbageArena,
				&tcmd,str,&cmdIdx,str->length,0);
		if(lowCmd == NULL)break;
		cmdBeginn = cmdIdx;
	}

	return tcmd;
}
void tcls_free_cmd(TCLS_Cmd **cmd){
	TCLS_Cmd *c = *cmd;
	if(c->deferFree != NULL)
		((TCLF_NAT_Fn)(c->deferFree))(NULL,c);
	c->command->gc.refs--;
	for(int32_t subIdx = 0;subIdx < c->length;subIdx++){
		c->arguments[subIdx]->gc.refs--;
	}
	free(c);
}

void tcls_free_commands(TCLS_Commands **tcmd){
	TCLS_Commands *cmds = *tcmd;
	for(int32_t idx = 0;idx < cmds->length;idx++){
		TCLS_Cmd *c = cmds->commands[idx];
		if(c->deferFree != NULL){
			((TCLF_NAT_Fn)(c->deferFree))(NULL,c);
		}
		c->command->gc.refs--;
		for(int32_t subIdx = 0;subIdx < c->length;subIdx++){
			c->arguments[subIdx]->gc.refs--;
		}
		free(c);
	}
	free(cmds);
}

// NOTE you have to free the String
TCL_String *tcls_list_iter(TCL_Slice **slice){
	TCL_Slice *slc = *slice;
	int32_t lower = slc->offset;
	int32_t upper = _tcls_string_get_length_array(
			slc->string->data,slc->length + lower,lower,0);
	TCL_String *str = _tcls_make_string_from_bound(
			slc->string->data,lower,upper,1);
	upper = _tcls_string_skip_white(slc->string->data,slc->length,upper);
	slc->length -= upper - lower;
	slc->offset += upper - lower;
	if(slc->length <= 0){
		free(slc);
		*slice = NULL;
	}
	return str;
}

#endif
