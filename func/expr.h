
#ifndef FUNC_CORE_EXPR_H_
#define FUNC_CORE_EXPR_H_

#include<gmp.h>
#include<math.h>
#include<stdint.h>

enum EXPR_LIST_Flags{
	EXPR_LISTF_FREE = 0,
	EXPR_LISTF_USED = 1,
	EXPR_LISTF_CALL = 2,
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
	expr->expr[0].left = -1;
	expr->expr[0].right = -1;
	//
	char state = 0;
	char move;
	idx = 0;
	expr->length++;
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
		
		//
		exprOff++;
		expr->length++;
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
			if(state == '$'){
				expr->expr[exprOff].str.tags = TCL_ST_Variable;
				expr->expr[exprOff].str.length--;
				expr->expr[exprOff].str.offset++;
			}
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
		if(left  <  lower)return 1 + home;
		exprList->expr[left ].flags = EXPR_LISTF_USED;
		exprList->expr[home ].left  = left;
	}
	if(inFlags & 2){
		while(right < upper){
			if(exprList->expr[right].flags == EXPR_LISTF_FREE)
				break;
			right++;
		}
		if(right >= upper)return 1 + home;
		exprList->expr[right].flags = EXPR_LISTF_USED;
		exprList->expr[home ].right = right;
	}
	return 0;
}
int32_t exprTokenOverList(TCLCORE_Expr *exprList,
		int32_t lower,int32_t upper,
		char inFlags){

	int32_t stack = 0;
	int32_t stackBack;
	int32_t idx;
	for(idx = lower;idx < upper;idx++){
		TCL_Slice *slc = &(exprList->expr[idx].str);
		if(slc->length == 0)continue;
		// never run vvvv
		if(exprList->expr[idx].flags == EXPR_LISTF_USED)continue;
		//
		if(slc->string->data[slc->offset] == '('){
			if(stack == 0)stackBack = idx;
			stack++;
		}
		if(slc->string->data[slc->offset] == ')'){
			stack--;
			if(stack == 0){
				exprTokenOverList(exprList,
						stackBack + 1,idx - 1,inFlags);
				exprList->expr[stackBack].flags = EXPR_LISTF_CALL;
				exprList->expr[idx      ].flags = EXPR_LISTF_CALL;
				// TODO
			}
		}
	}
	if(stack)
		return idx;
	/// combine * % /
	for(idx = lower;idx < upper;idx++){
		TCL_Slice *slc = &(exprList->expr[idx].str);
		if(slc->length == 0)continue;
		if(exprList->expr[idx].flags == EXPR_LISTF_USED)continue;
		//
		if(slc->string->data[slc->offset] == '*')
			exprTokenOperationTree(exprList,idx,lower,upper,0);
		if(slc->string->data[slc->offset] == '/')
			exprTokenOperationTree(exprList,idx,lower,upper,0);
		if(slc->string->data[slc->offset] == '%')
			exprTokenOperationTree(exprList,idx,lower,upper,0);
	}
	/// combine + -
	for(idx = lower;idx < upper;idx++){
		TCL_Slice *slc = &(exprList->expr[idx].str);
		if(slc->length == 0)continue;
		if(exprList->expr[idx].flags == EXPR_LISTF_USED)continue;
		//
		if(slc->string->data[slc->offset] == '+')
			exprTokenOperationTree(exprList,idx,lower,upper,0);
		if(slc->string->data[slc->offset] == '-')
			exprTokenOperationTree(exprList,idx,lower,upper,0);
	}
	
	return 0;
}
void _exprNumberMayFree(TCL_Number *num){
	if(!(num->typ & NUMBERT_DO_FREE))
		return;
	if((num->typ & NUMBERT_Mask) == NUMBERT_Gmpz)
		mpz_clear(num->var.gmpz);
	if((num->typ & NUMBERT_Mask) == NUMBERT_Gmpq)
		mpq_clear(num->var.gmpq);
	if((num->typ & NUMBERT_Mask) == NUMBERT_Gmpf)
		mpf_clear(num->var.gmpf);
}
void _exprFreeDefer(TCL_String **str){
	(*str)->var.typ &= ~NUMBERT_DO_FREE;
	_exprNumberMayFree(&((*str)->var));
}
//flag == 0 => normal expr, flag == 1 => gmp
int32_t _exprParseString(TCL_Slice slc,TCL_Number *num,int32_t flag){
	TCL_String *str = slc.string;
	char *strStart = (char*)&(slc.string->data[slc.offset]);
	char state = '0';
	int offset = 0;
	if(str->length <= offset)
		return 1;
	if(strStart[offset] == '-'){
		offset++;
	}
	if(slc.length <= offset)
		return 1;
	while(slc.length > offset){
		// don't care about whitespaces
		if(((uint8_t)strStart[offset]) <= ' ')continue;
		else if(state == '.' && strStart[offset] == '.')
			return 1;
		else if(state == '0' && strStart[offset] == '.')
			state = '.';
		else if(state == '0' && strStart[offset] == '/' && flag > 0)
			state = '/';
		else if(state == '.' && strStart[offset] >= '0' &&
					strStart[offset] <= '9')
			state = '.';
		else if(state == '/' && strStart[offset] >= '0' &&
					strStart[offset] <= '9')
			state = '/';
		else if(state == '0' && strStart[offset] >= '0' &&
					strStart[offset] <= '9')
			state = '0';
		else return 1;
		offset++;
	}
	num = num;
	flag = flag;
	// TODO
	char upper = strStart[offset];
	strStart[offset] = 0; // this is a bad thing!
	if(flag == 0){
		if(state == '0'){
			mpz_init_set_str(num->var.gmpz,strStart,10);
			num->typ = NUMBERT_Gmpz;
		} else if(state == '.'){ // this is funny
			char getFlt[64];
			snprintf(getFlt,sizeof(getFlt) - 1,"%%%df",offset);
			sscanf(strStart,getFlt,offset,&(num->var.flt));
			num->typ = NUMBERT_Float;
		}
	}else if(flag == 1){
		if(state == '0'){
			mpz_init_set_str(num->var.gmpz,strStart,10);
			num->typ = NUMBERT_Gmpz;
		} else if(state == '.'){
			mpf_init_set_str(num->var.gmpf,strStart,10);
			num->typ = NUMBERT_Gmpf;
		} else if(state == '/'){
			mpq_init(num->var.gmpq);
			mpq_set_str(num->var.gmpq,strStart,10);
			num->typ = NUMBERT_Gmpq;
		}
	}
	strStart[offset] = upper;

	return 0;
}
TCL_Number exprTokenInterpret(TCLR_Context *ctx,TCLCORE_Expr *exprList,
		int32_t idx){
	TCL_Number outNum = (TCL_Number){0};
	outNum.typ = NUMBERT_DO_FREE;
	if(idx == -1)
		return outNum;
	TCL_Slice *slc = &(exprList->expr[idx].str);
	if(slc->tags == TCL_ST_Variable){
		/*
		TCL_Slice vslice = (TCL_Slice){
			.refs = 1, .flags = 0,
			.length = slc->length - 1,
			.offset = slc->offset + 1,
			.string = slc->string,
		};
		outStr = tcl_get_from_scope_slice(&(ctx->scope),vslice);
		//  */
		TCL_String *outStr = tcl_get_from_scope_slice(&(ctx->scope),slc);
		if((outStr->var.typ & NUMBERT_Mask) != NUMBERT_None)
			return outStr->var;
		TCL_Slice slc = (TCL_Slice){
			.refs = -1, .tags = 0,
			.offset = 0, .length = outStr->length,
			.string = outStr
		};
		_exprParseString(slc,&(outStr->var),0);
		outStr->freeCallback = (void*)_exprFreeDefer;
		return outStr->var;
	}
	TCL_Number left  = exprTokenInterpret(ctx,exprList,exprList->expr[idx].left );
	TCL_Number right = exprTokenInterpret(ctx,exprList,exprList->expr[idx].right);
	if((left.typ & NUMBERT_Mask) == NUMBERT_Gmpz &&
			(right.typ & NUMBERT_Mask) == NUMBERT_Float){
		left.typ = (left.typ & ~NUMBERT_Mask) | NUMBERT_Float;
		float lft = (float)mpz_get_d(left.var.gmpz);
		mpz_clear(left.var.gmpz); // wtf, how do I do this?
		left.var.flt = lft;
		//left->
	}
	if((left.typ & NUMBERT_Mask) == NUMBERT_Float &&
			(right.typ & NUMBERT_Mask) == NUMBERT_Gmpz){
		//right->
		right.typ = (right.typ & ~NUMBERT_Mask) | NUMBERT_Float;
		float rft = (float)mpz_get_d(right.var.gmpz);
		mpz_clear(right.var.gmpz);
		right.var.flt = rft;
	}
	//
	if(slc->string->data[slc->offset] == '+'){
		if(left.typ & NUMBERT_Gmpz && right.typ & NUMBERT_Gmpz){
			mpz_add(outNum.var.gmpz,left.var.gmpz,right.var.gmpz);
			outNum.typ = NUMBERT_Gmpz | NUMBERT_DO_FREE;
		}
		if(left.typ & NUMBERT_Float && right.typ & NUMBERT_Float){
			outNum.var.flt = left.var.flt + right.var.flt;
			outNum.typ = NUMBERT_Float;
		}
	}
	else if(slc->string->data[slc->offset] == '-'){
		if(left.typ & NUMBERT_Gmpz && right.typ & NUMBERT_Gmpz){
			mpz_sub(outNum.var.gmpz,left.var.gmpz,right.var.gmpz);
			outNum.typ = NUMBERT_Gmpz | NUMBERT_DO_FREE;
		}
		if(left.typ & NUMBERT_Float && right.typ & NUMBERT_Float){
			outNum.var.flt = left.var.flt - right.var.flt;
			outNum.typ = NUMBERT_Float;
		}
	}
	else if(slc->string->data[slc->offset] == '*'){
		if(left.typ & NUMBERT_Gmpz && right.typ & NUMBERT_Gmpz){
			mpz_mul(outNum.var.gmpz,left.var.gmpz,right.var.gmpz);
			outNum.typ = NUMBERT_Gmpz | NUMBERT_DO_FREE;
		}
		if(left.typ & NUMBERT_Float && right.typ & NUMBERT_Float){
			outNum.var.flt = left.var.flt * right.var.flt;
			outNum.typ = NUMBERT_Float;
		}
	}
	else if(slc->string->data[slc->offset] == '/'){
		if(left.typ & NUMBERT_Gmpz && right.typ & NUMBERT_Gmpz){
			mpz_divexact(outNum.var.gmpz,left.var.gmpz,right.var.gmpz);
			outNum.typ = NUMBERT_Gmpz | NUMBERT_DO_FREE;
		}
		if(left.typ & NUMBERT_Float && right.typ & NUMBERT_Float){
			outNum.var.flt = left.var.flt / right.var.flt;
			outNum.typ = NUMBERT_Float;
		}
	}
	else if(slc->string->data[slc->offset] == '%'){
		if(left.typ & NUMBERT_Gmpz && right.typ & NUMBERT_Gmpz){
			mpz_mod(outNum.var.gmpz,left.var.gmpz,right.var.gmpz);
			outNum.typ = NUMBERT_Gmpz | NUMBERT_DO_FREE;
		}
		if(left.typ & NUMBERT_Float && right.typ & NUMBERT_Float){
			outNum.var.flt = fmodf(left.var.flt,right.var.flt);
			outNum.typ = NUMBERT_Float;
		}
	}

	//
	_exprNumberMayFree(&left);
	_exprNumberMayFree(&right);

	return outNum;
}
void exprResolveDefer(TCL_String **strptr){
	//TCL_String mem = **strptr;
	int32_t refs = (*strptr)->refs;
	int32_t tags = (*strptr)->tags;
	//tags &= ~TCL_ST_Defer;
	struct TCL_Number var = (*strptr)->var;
	int32_t vcap = -1;
	// why gcc why is it an int32_t larger?
	int32_t sz = (int32_t)(sizeof(TCL_String) - sizeof(int32_t));
	if(((*strptr)->var.typ & NUMBERT_Mask) == NUMBERT_Float){
		vcap = asprintf((char**)strptr,"%0*i%f",
				(int)sz,0,
				var.var.flt);
	}
	else if(((*strptr)->var.typ & NUMBERT_Mask) == NUMBERT_Gmpz){
		vcap = gmp_asprintf((char**)strptr,"%0*i%Zd",
				(int)sz,0,
				var.var.gmpz);
	}
	else if(((*strptr)->var.typ & NUMBERT_Mask) == NUMBERT_Gmpq){
		vcap = gmp_asprintf((char**)strptr,"%0*i%Qx",
				(int)sz,0,
				var.var.gmpq);
	}
	else if(((*strptr)->var.typ & NUMBERT_Mask) == NUMBERT_Gmpf){
		vcap = gmp_asprintf((char**)strptr,"%0*i%Ff",
				(int)sz,0,
				var.var.gmpf);
	}
	(*strptr)->deferCallback = NULL;
	if(var.typ & NUMBERT_DO_FREE){
		//free((*strptr)->var.
	}
	if(vcap == -1){
		return;
	}
	(*strptr)->freeCallback = _exprFreeDefer;
	//(*strptr)->freeCallback = NULL;
	(*strptr)->capacity = vcap - sz + 1;
	(*strptr)->length = vcap - sz;
	(*strptr)->refs = refs & ~0xffffff;
	(*strptr)->tags = tags;
	(*strptr)->var = var;
}

TCL_String *exprGetString(TCL_Number *num){
	TCL_String *strOut = NULL;
	strOut = malloc(sizeof(TCL_String));
	strOut->var = *num;
	strOut->length = 0;
	strOut->capacity = 0;
	strOut->refs = 0;
	strOut->deferCallback = (void*)exprResolveDefer;
	strOut->freeCallback = (void*)_exprFreeDefer;
	strOut->tags = 0;//TLC_ST_Defer;
	/*
	if((num->typ & NUMBERT_Mask) == NUMBERT_Gmpz);
	if((num->typ & NUMBERT_Mask) == NUMBERT_Gmpq);
	if((num->typ & NUMBERT_Mask) == NUMBERT_Gmpf);
	*/

	return strOut;
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
		exprTokensFree((TCLCORE_Expr*)cmd->moreData);
		cmd->moreData = NULL;
	}
	return NULL;
}
TCL_String *exprFunction(TCLR_Context **ctx,TCLS_Cmd *cmd){
	ctx = ctx;
	// proc {name} {args} {body}
	printf("free %.*s\n",cmd->command->length,cmd->command->data);
	for(int32_t i = 0;i < cmd->length;i++){
		printf("- %.*s\n",cmd->arguments[i]->length,cmd->arguments[i]->data);
	}
	if(cmd->length != 1){
		printf("Err with set (62bf270b-ba0d-44ca-8bd2-5498044247ed)\n");
		return NULL;
	}
	TCLCORE_Expr *exprPtr = cmd->moreData;
	int32_t error = 0;
	if(cmd->moreData == NULL){
		exprPtr = malloc(sizeof(TCLCORE_Expr) +
				sizeof(TCLCORE_LIST_Expr) * 128);
		exprPtr->capacity = 128;
		exprPtr->length = 0;
		exprTokenise(&exprPtr,cmd->arguments[0]);
		error = exprTokenOverList(exprPtr,0,exprPtr->length,0);
	}
	TCL_String *outStr = NULL;
	// TODO
	if(!error){
		int32_t idx = 0;
		for(idx = 0;idx < exprPtr->length;idx++){
			if(exprPtr->expr[idx].flags == EXPR_LISTF_FREE)break;
		}
		if(idx < exprPtr->length){
			TCL_Number num = exprTokenInterpret(*ctx,exprPtr,idx);
			num.typ &= ~NUMBERT_DO_FREE;
			outStr = exprGetString(&num);
			tcl_set_string_arena(&((*ctx)->arena),outStr);
		}
	}
	// make this compute stuff!
	if(
			cmd->arguments[0]->tags == TCL_ST_None &&
			cmd->command->tags == TCL_ST_None &&
			!error
	)
		cmd->moreData = exprPtr;
	else{
		cmd->moreData = NULL;
		exprTokensFree(exprPtr);
	}
	return outStr;
}

#endif
