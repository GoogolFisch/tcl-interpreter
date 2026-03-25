
#ifndef TCL_FUNCTION_H_
#define TCL_FUNCTION_H_

#include"tcl_struct.h"
#include"tcl_type.h"

struct TCLF_KV *tclf_get_function(TCLF_Scope *scope,TCL_String *str){
	size_t hash = tcl_hash_string(str);
	for(int32_t idx = 0;idx < scope->length;idx++){
		if(scope->kv[idx].kHash != hash)
			continue;
		if(tcl_string_eq(scope->kv[idx].key,str))
			return &(scope->kv[idx]);
	}
	return NULL;
}

void tclf_insert_natFunction(TCLF_Scope **ptr_scope,TCL_String *str,TCLF_NAT_FN fn){
	TCLF_Scope *scope = *ptr_scope;
	if(scope->capacity <= scope->length){
		scope->capacity *= 2;
		*ptr_scope = realloc(scope,sizeof(struct TCLF_Scope) +
			sizeof(TCLF_KV) * scope->capacity);
		scope = *ptr_scope;
	}
	size_t hash = tcl_hash_string(str);
	scope->kv[scope->length].kHash = hash;
	scope->kv[scope->length].key = str;
	scope->kv[scope->length].cmds = NULL;
	scope->kv[scope->length].natFn = fn;
	scope->kv[scope->length].flags = TCLF_FN_NATIVE;
	scope->length++;
}

TCLF_Scope *tclf_make_function_scope(){
	TCLF_Scope *fnScope = malloc(sizeof(struct TCLF_Scope) +
			sizeof(TCLF_KV) * TCL_MIN_CAPACITY);
	fnScope->length = 0;
	fnScope->capacity = TCL_MIN_CAPACITY;

	return fnScope;
}

#endif
