
#ifndef TCL_STRUCT_H_
#define TCL_STRUCT_H_

#include<stdlib.h>
#include<stdint.h>

// ======== DEFINES
#define TCL_MIN_CAPACITY 128
#define TCLS_STRING_DEPTH 64

// ======== structs
union TCL_Combined{
	int32_t nInt;
	float nFloat;
};

enum TCL_String_Tags{
	TCL_ST_None,
	TCL_ST_Variable,
	TCL_ST_LookUp,
	TCL_ST_Object,
};

// should be immutable when refs > 1?
struct TCL_String{
	int32_t refs;
	int32_t tags;
	int32_t length;
	int32_t capacity;
	char data[];
};
struct TCL_Slice{
	int32_t refs;
	int32_t tags;
	int32_t offset;
	int32_t length;
	TCL_String *string;
}
struct _TCL_KV{
	size_t kHash;
	TCL_String *key;
	TCL_String *value;
};
struct TCL_Scope{
	int32_t length;
	int32_t capacity;
	struct _TCL_KV kv[];
};
struct TCL_StringArena{
	int32_t length;
	int32_t capacity;
	struct TCL_String (*string)[];
};
struct TCL_SliceArena{
	int32_t length;
	int32_t capacity;
	struct TCL_String (*string)[];
};

enum TCLS_CMD_FLAGS {
	TCLS_CMD_NORMAL = 0,
	TCLS_CMD_PUSH = 1
};
struct _TCLS_Cmd{
	int32_t length;
	int32_t capacity;
	TCLS_Cmd_Flags flags;
	int32_t stackDepth;
	TCL_String *command;
	void *moreData;
	TCL_String (*arguments)[];
};
struct TCLS_Commands{
	int32_t refs;
	int32_t tags;
	int32_t length;
	int32_t capacity;
	struct _TCLS_Cmd (*commands)[];
};
//
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
//
enum TCLR_FLAGS{
	TCLR_NONE_LAYER = 0,
	TCLR_FULL_LAYER = 1,
	TCLR_NEGATIVE_LAYER = 2,
}
struct TCLR_Context{
	//
	int32_t instruction;
	int32_t parseStackIdx;
	TCLR_FLAGS flags;
	TCLS_Commands *program;
	TCL_String (*parseStack)[TCLR_STACK_SIZE];
	struct TCL_Scope *scope;
	struct TCLR_Context *parent;
	struct TCLR_Context *vparent;
};

// ========== typedefs
// 
typedef struct TCL_String TCL_String;
typedef struct TCL_Slice TCL_Slice;
typedef struct TCL_Scope TCL_Scope;
typedef struct TCL_StringArena TCL_StringArena;
typedef struct TCL_SliceArena TCL_SliceArena;
//
typedef enum TCLS_CMD_FLAGS TCLS_CMD_FLAGS;
typedef struct TCLS_Commands TCLS_Commands;
//
typedef struct TCLF_Scope TCLF_Scope;
//
typedef struct TCLR_Context TCLR_Context;

#endif
