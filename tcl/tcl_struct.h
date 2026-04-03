
#ifndef TCL_STRUCT_H_
#define TCL_STRUCT_H_

#include<stdlib.h>
#include<stdint.h>

// ======== DEFINES
#define TCL_MIN_CAPACITY 128
#define TCLS_STRING_DEPTH 64
#define TCLR_STACK_SIZE 64

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
	uint8_t data[];
};
struct TCL_Slice{
	int32_t refs;
	int32_t tags;
	int32_t offset;
	int32_t length;
	struct TCL_String *string;
};
struct _TCL_KV{
	size_t kHash;
	struct TCL_String *key;
	struct TCL_String *value;
};
struct TCL_Scope{
	int32_t length;
	int32_t capacity;
	struct _TCL_KV kv[];
};
struct TCL_StringArena{
	int32_t length;
	int32_t capacity;
	struct TCL_String *(string[]);
};
struct TCL_SliceArena{
	int32_t length;
	int32_t capacity;
	struct TCL_Slice *(string[]);
};

enum TCLS_CMD_FLAGS {
	TCLS_CMD_NORMAL = 0,
	TCLS_CMD_PUSH = 1
};
struct TCLS_Cmd{
	int32_t length;
	int32_t capacity;
	enum TCLS_CMD_FLAGS flags;
	int32_t stackDepth;
	struct TCL_String *command;
	void *moreData;
	struct TCL_String *(arguments[]);
};
struct TCLS_Commands{
	int32_t refs;
	int32_t tags;
	int32_t length;
	int32_t capacity;
	struct TCLS_Cmd *(commands[]);
};
//
enum TCLF_FN_FLAGS{
	TCLF_FN_CONVERT, 
	TCLF_FN_NATIVE, 
	TCLF_FN_RAW, 
};
struct TCLF_KV{
	size_t kHash;
	struct TCL_String *key;
	enum TCLF_FN_FLAGS flags;
	struct TCLS_Commands *cmds;
	void *(natFn);
};
struct TCLF_Scope{
	int32_t length;
	int32_t capacity;
	struct TCLF_KV kv[];
};
//
enum TCLR_FLAGS{
	TCLR_NONE_LAYER = 0,
	TCLR_FULL_LAYER = 1,
	TCLR_NEGATIVE_LAYER = 2,
};
struct TCLR_Context{
	//
	int32_t instruction;
	int32_t parseStackIdx;
	enum TCLR_FLAGS flags;
	struct TCLS_Commands *program;
	struct TCL_String *parseStack[TCLR_STACK_SIZE];
	struct TCL_Scope *scope;
	struct TCLF_Scope *fnScope;
	struct TCLR_Context *parent;
	struct TCLR_Context *vparent;
	struct TCL_StringArena *arena;
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
typedef struct TCLS_Cmd TCLS_Cmd;
typedef struct TCLS_Commands TCLS_Commands;
//
typedef struct TCLF_KV TCLF_KV;
typedef struct TCLF_Scope TCLF_Scope;
//
typedef enum TCLR_FLAGS TCLR_FLAGS;
typedef struct TCLR_Context TCLR_Context;
// ======= fn typdef
typedef TCL_String*(*TCLF_NAT_Fn)(TCLR_Context **ctx,TCLS_Cmd *cmd);

#endif
