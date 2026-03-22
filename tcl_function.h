
#ifndef TCL_FUNCTION_H_
#define TCL_FUNCTION_H_

#include"tcl_type.h"

struct _TCLF_KV{
	size_t kHash;
	TCL_String *key;
	TCL_String *value;
};
struct TCLF_Scope{
	int32_t length;
	int32_t capacity;
	struct _TCLF_KV kv[];
};

typedef struct TCLF_Scope TCLF_Scope;

#endif
