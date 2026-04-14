
#ifndef FUNC_CORE_EXPR_H_
#define FUNC_CORE_EXPR_H_

enum EXPR_LIST_Flags{
	EXPR_LISTF_FREE = 0,
	EXPR_LISTF_USED = 1,
};
typedef struct TCLCORE_LIST_Expr{
	enum EXPR_LIST_Flags flags;
	TCL_Slice str;
	int32_t left;
	int32_t right;
	/* /
	struct TCLCORE_KV_Expr *left;
	struct TCLCORE_KV_Expr *right;
	//  */
} TCLCORE_LIST_Expr;
typedef struct TCLCORE_Expr{
	int32_t capacity;
	int32_t length;
	TCLCORE_LIST_Expr expr[];
} TCLCORE_Expr;

void exprTokenise(TCLCORE_Expr **exprList,TCL_String *str){
	int32_t idx, exprOff;
	exprOff = 0;
	TCLCORE_Expr *expr = *exprList;
	if(expr->capacity == 0){
		expr->capacity = 8;
		(*exprList) = realloc(expr,sizeof(TCLCORE_Expr) +
				sizeof(TCLCORE_LIST_Expr) * expr->capacity);
		expr = *exprList;
	}
	expr->expr[0] = (TCLCORE_LIST_Expr){0};
	expr->expr[0].str = (TCL_Slice){   .refs = 1, .tags = 0,
		.length = 0, .offset = 0,  .string = str,
	};
	//
	char state = 0;
	char move;
	idx = 0;
	goto expr_token_append;
	for(;idx < str->length;idx++){
		move = str->data[idx];
		if((uint8_t)      move >= 128) move = ':';
		if(move >= 'A' && move <= 'Z') move = ':';
		if(move >= 'a' && move <= 'z') move = ':';
		if(               move == '_') move = ':';
		if(move >= '0' && move <= '9') move = '0';
		//
		if(state == '$' && move == ':') goto expr_token_append;
		if(state == '0' && move == '0') goto expr_token_append;
		if(state == ':' && move == ':') goto expr_token_append;
		if(state == '0' && move == '.'){
			state = '.';
			goto expr_token_append;
		}
		if(move == ' ' || move == '\t' ||
				move == '\r' || move == '\n'){
			state = 0;
			continue;
		}
		
		if(expr->length >= expr->capacity){
			expr->capacity *= 2;
			size_t sz = sizeof(TCLCORE_Expr);
			sz += sizeof(TCLCORE_LIST_Expr) * expr->capacity;
			(*exprList) = realloc(expr,sz);
			expr = *exprList;
		}
		expr->expr[exprOff] = (TCLCORE_LIST_Expr){0};
		expr->expr[exprOff].str = (TCL_Slice){ .refs = 1, .tags = 0,
			.length = 0, .offset = idx,  .string = str,
		};
		//
expr_token_append:
		expr->expr[exprOff].str.length++;
		if(state == 0){
			state = str->data[idx];
			if((uint8_t)       state >= 128) state = ':';
			if(state >= 'A' && state <= 'Z') state = ':';
			if(state >= 'a' && state <= 'z') state = ':';
			if(                state == '_') state = ':';
			if(state >= '0' && state <= '9') state = '0';
			//if(state == '.') state = '.';
		}
	}
}
void exprTokensFree(TCLCORE_Expr *exprList){
	/*
	for(int32_t idx = 0:idx < expr->length;idx++){
	}
	//   */
	free(exprList);
}

int32_t exprTokenOperationTree(TCLCORE_Expr *exprList,int32_t home,
		int32_t lower,int32_t upper,
		char inFlags){

	int32_t left  = home - 1;
	int32_t right = home + 1;
	if(inFlags == 0)inFlags |= 3;
	if(inFlags & 1){
		while(left  >= lower){
			if(exprList->expr[left ].flags == EXPR_LISTF_FREE)
				break;
			left --;
		}
		if(left  >= lower)return 1;
		exprList->expr[left ].flags = EXPR_LISTF_USED;
		exprList->expr[home ].left  = left;
	}
	if(inFlags & 2){
		while(right < upper){
			if(exprList->expr[right].flags == EXPR_LISTF_FREE)
				break;
			right++;
		}
		if(right < upper)return 1;
		exprList->expr[right].flags = EXPR_LISTF_USED;
		exprList->expr[home ].right = right;
	}
	return 0;
}




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

#endif
