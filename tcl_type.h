
#ifndef TCL_TYPE_H_
#define TCL_TYPE_H_

#include<stdint.h>
#include<stdlib.h>


// ======== DEFINES
#define TCL_MIN_CAPACITY 128

#define TCL_REALLOC_LIST(list,baseType,elementType)                  \
do{                                                                  \
 if((*list)->length >= (*list)->capacity){                           \
  (*list)->capacity *= 2;                                            \
  *list = realloc(sizeof(baseType) +                                 \
    sizeof(elementType) * (*list)->capacity;                         \
 }                                                                   \
}while(false)

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

// ========== typedefs
typedef struct TCL_String TCL_String;
typedef struct TCL_Slice TCL_Slice;
typedef struct TCL_Scope TCL_Scope;
typedef struct TCL_StringArena TCL_StringArena;
typedef struct TCL_SliceArena TCL_SliceArena;

// ========== declerations

size_t tcl_hash_string(TCL_String *string);

TCL_Slice *tcl_get_slice_of(TCL_SliceArena **sliceArena,
		TCL_String *string,int32_t start,int32_t end);

// scope stuff
void tcl_set_into_scope(TCL_Scope **stringScope,
		TCL_String *key,TCL_String *value);
void tcl_insert_into_scope(TCL_Scope **stringScope,
		TCL_String *key,TCL_String *value);
TCL_String *tcl_get_from_scope(TCL_Scope **stringScope,TCL_String *key);
void tcl_drop_scope(TCL_Scope **stringScope);
TCL_Scope *tcl_create_scope(void);

// stringArena
TCL_StringArena *tcl_create_string_arena(void);
void TCL_set_string_arena(TCL_StringArena **stringArena,TCL_String *string);
void TCL_garbage_collect_string_arena(TCL_StringArena **stringArena);

// ========== functions
size_t tcl_hash_string(TCL_String *string){
	size_t hash = 0;
	int32_t length = string->length;
	switch(length & 3){
		case 3:hash += string->data[length - 3] * 555;
		case 2:hash += string->data[length - 2] * 177;
		case 1:hash += string->data[length - 1];
		default:break;
	}
	int32_t *changed = (void*)(string->data);
	for(int32_t idx = 0;idx < (length >> 2);idx++){
		hash += (hash << 17) ^ (hash << 13);
		hash += changed[idx];
		hash ^= hash >> 11;
	}
	return hash;
}

TCL_Slice *tcl_get_slice_of(TCL_SliceArena **sliceArena,
		TCL_String *string,int32_t start,int32_t end){

	if((*sliceArena)->length >= (*sliceArena)->capacity){
		(*sliceArena)->capacity *= 2;
		*sliceArena = realloc(sizeof(TCL_SliceArena) +
				sizeof(TCL_String) * (*sliceArena)->capacity;
	}
	TCL_SliceArena *arena = *sliceArena;

	TCL_Slice *slice = malloc(sizeof(TCL_Slice));
	arena->string[arena->length++] = slice;
	slice->offset = start;
	slice->tags = 0;
	slice->length = end - start;
	slice->string = string;
	string->refs++;
	slice->refs++; // ???
	return slice;
}

void tcl_set_into_scope(TCL_Scope **stringScope,
		TCL_String *key,TCL_String *value){
	// test if it's there
	TCL_Scope *scope = *stringScope;
	TCL_String *oldValue = NULL;
	int idx;
	size_t hash = tcl_hash_string(key);
	for(idx = 0;idx < scope->length;idx++){
		if(scope->kv[idx].kHash != hash)continue;
		if(scope->kv[idx].key->length != key->length)continue;
		// this is very unlikely
		if(scope->kv[idx].key == key){
			oldValue = scope->kv[idx].value;
			break;
		}

		char *fetch = scope->kv[idx].key->data;
		char matches = 1;
		for(int over = 0;over < key->length;over++){
			if(fetch[over] != key->data[over]){
				matches = 0;
				break;
			}
		}
		if(matches){
			oldValue = scope->kv[idx].value;
			break;
		}
	}
	// swap values
	if(oldValue != NULL){
		oldValue->refs--;
		scope->kv[idx].value = value;
		value->refs++;
		return;
	}
	// insert
	tcl_insert_into_scope(stringScope,key,value);
}
void tcl_insert_into_scope(TCL_Scope **stringScope,
		TCL_String *key,TCL_String *value){
	if((*stringScope)->length >= (*stringScope)->capacity){
		(*stringScope)->capacity *= 2;
		*stringScope = realloc(sizeof(TCL_Scope) +
				sizeof(struct _TCL_KV) * (*stringScope)->capacity;
	}
	TCL_Scope *scope = *stringScope;

	struct _TCK_KV *kv = &(scope->kv[scope->length++]);
	size_t hash = tcl_hash_string(key);
	kv->kHash = hash;
	kv->key = key;
	kv->value = value;
	key->refs++;
	value->refs++;
	return;
}
TCL_String *tcl_get_from_scope(TCL_Scope **stringScope,TCL_String *key){
	TCL_Scope *scope = *stringScope;
	size_t hash = tcl_hash_string(key);
	for(int idx = 0;idx < scope->length;idx++){
		if(scope->kv[idx].kHash != hash)continue;
		if(scope->kv[idx].key->length != key->length)continue;
		// this is very unlikely
		if(scope->kv[idx].key == key)return scope->kv[idx].value;

		char *fetch = scope->kv[idx].key->data;
		char matches = 1;
		for(int over = 0;over < key->length;over++){
			if(fetch[over] != key->data[over]){
				matches = 0;
				break;
			}
		}
		if(matches)
			return scope->kv[idx].value;
	}
	return NULL;
}
void tcl_drop_scope(TCL_Scope **stringScope){
	TCL_Scope *scope = *stringScope;
	for(int idx = 0;idx < scope->length;idx++){
		scope->kv[idx].key->refs--;
		scope->kv[idx].value->refs--;
	}
	free(scope);
	return;
}
TCL_Scope *tcl_create_scope(void){
	TCL_Scope *scope = malloc(sizeof(TCL_Scope) +
			sizeof(struct _TCL_KV) * TCL_MIN_CAPACITY);

	scope->length = 0;
	scope->capacity = TCL_MIN_CAPACITY;
	return scope;
}

// stringArena
TCL_StringArena *tcl_create_string_arena(void){
	TCL_StringArena *arena = malloc(sizeof(TCL_StringArena) +
			sizeof(*TCL_String) * TCL_MIN_CAPACITY);

	arena->length = 0;
	arena->capacity = TCL_MIN_CAPACITY;
	return scope;
}
void TCL_set_string_arena(TCL_StringArena **stringArena,TCL_String *string){
	if((*stringArena)->length >= (*stringArena)->capacity){
		(*stringArena)->capacity *= 2;
		*stringArena = realloc(sizeof(TCL_Scope) +
				sizeof(struct _TCL_KV) * (*stringArena)->capacity;
	}
	TCL_Scope *arena = *stringArena;

	arena->string[arena->length++] = string;
}
void TCL_garbage_collect_string_arena(TCL_StringArena **stringArena){
	TCL_Scope *arena = *stringArena;
	for(int idx = 0;idx < arena->length;idx++){
		TCL_String *string = *(arena->string[idx]);
		if(string->refs)continue;
		arena->string[idx] = arena->string[arena->length - 1];
		arena->length--;
		free(string);
		idx--;
	}
}


#endif
