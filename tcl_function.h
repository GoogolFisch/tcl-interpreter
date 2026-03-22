
#ifndef TCL_FUNCTION_H_
#define TCL_FUNCTION_H_

#include"tcl_struct.h"
#include"tcl_type.h"

struct _TCLF_KV *tclf_get_function(TCLF_Scope *scope,TCL_String *str){
	size_t h1 = tcl_hash_string(str);
	for(idx = 0;idx < scope->length;idx++){
		if(!tcl_string_eq(scope->kv[idx].key,str))
			return scope->kv[idx].value;
	}
}

#endif
