
#ifndef TCL_STRUCT_H_
#define TCL_STRUCT_H_

#include<stdlib.h>
#include<stdint.h>
#include<gmp.h>

// ======== DEFINES
#define TCL_MIN_CAPACITY 128
#define TCLS_STRING_DEPTH 64
#define TCLR_STACK_SIZE 64

// ======== structs
enum TCL_NumTypes{
	NUMBERT_None    =   0,
	NUMBERT_Float   =   1,
	NUMBERT_Gmpz    =   2,
	NUMBERT_Gmpq    =   3,
	NUMBERT_Gmpf    =   4,
	NUMBERT_Mask    =  63,
	NUMBERT_DO_FREE = 128,
};
union TCL_NumCombine{
	mpz_t gmpz;
	mpq_t gmpq;
	mpf_t gmpf;
	float flt;
};
struct TCL_Number{
	union TCL_NumCombine var;
	enum TCL_NumTypes typ;
};

enum TCL_String_Tags{
	TCL_ST_None,
	TCL_ST_Variable,
	TCL_ST_LookUp,
	TCL_ST_Object,

	//TCL_ST_Defer = 32,
	TCL_ST_Int = 64,
	TCL_ST_Quot = 128,
	TCL_ST_Float = 192,
};

// should be immutable when refs > 1?
struct TCL_String{
	int32_t refs;
	int32_t tags;
	int32_t length;
	struct TCL_Number var;
	void *deferCallback;
	void *freeCallback;
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
	void *deferFree;
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
	TCLF_FN_PROC,
	TCLF_FN_NATIVE,
	TCLF_FN_RAW,
};
struct TCLF_KV{
	size_t kHash;
	struct TCL_String *key;
	enum TCLF_FN_FLAGS flags;
	void *(natFn);
	void *(freeFn);
	struct TCL_String *arguments;
	//struct TCL_String *body;
	struct TCLS_Commands *body;
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
enum TCLRG_VARIABLE{
	TCLRG_SHOW_AST = 1,
	TCLRG_SHOW_GC = 2,
	TCLRG_VERBOSE_EXEC = 4,
};
struct TCLR_Context{
	//
	int32_t instruction;
	int32_t parseStackIdx;
	enum TCLR_FLAGS flags;
	enum TCLRG_VARIABLE globFlags;
	struct TCLS_Commands *program;
	struct TCL_String *parseStack[TCLR_STACK_SIZE];
	struct TCL_Scope *scope;
	struct TCLF_Scope *fnScope;
	struct TCLR_Context *parent;
	struct TCLR_Context *vparent;
	struct TCL_StringArena *arena;
};

// ========== typedefs
typedef enum TCL_NumTypes TCL_NumTypes;
typedef union TCL_NumCombine TCL_NumCombine;
typedef struct TCL_Number TCL_Number;
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
typedef void(*TCL_DEFER_CBack)(TCL_String **str); // also for free


#endif
