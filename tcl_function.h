
#ifndef TCL_FUNCTION_H_
#define TCL_FUNCTION_H_

#include"tcl_struct.h"
#include"tcl_type.h"

struct TCLF_KV *tclf_get_function(TCLF_Scope *scope,TCL_String *str){
	size_t hash = tcl_hash_string(str);
	for(int32_t idx = 0;idx < scope->length;idx++){
		if(scope->kv[idx].kHash != hash)
			continue;
		if(!tcl_string_eq(scope->kv[idx].key,str))
			return &(scope->kv[idx]);
	}
	return NULL;
}

#endif
