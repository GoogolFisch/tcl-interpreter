
#ifndef TCL_TYPE_H_
#define TCL_TYPE_H_

#include<stdint.h>
#include<stdlib.h>

#include"tcl_struct.h"

// ========== declerations

size_t tcl_hash_string(TCL_String *string);
size_t tcl_hash_array(char *arr,int32_t length);
char tcl_string_eq(TCL_String *cmp,TCL_String *str);
void tcl_string_cp(TCL_String *into,TCL_String *from);

TCL_Slice *tcl_get_slice_of(TCL_SliceArena **sliceArena,
		TCL_String *string,int32_t start,int32_t end);

// scope stuff
void tcl_set_into_scope(TCL_Scope **stringScope,
		TCL_String *key,TCL_String *value);
void tcl_insert_into_scope(TCL_Scope **stringScope,
		TCL_String *key,TCL_String *value);
TCL_String *tcl_get_from_scope(TCL_Scope **stringScope,TCL_String *key);
TCL_String *tcl_get_from_scope_slice(TCL_Scope **stringScope,TCL_Slice *key);
void tcl_drop_scope(TCL_Scope **stringScope);
TCL_Scope *tcl_create_scope(void);

// stringArena
TCL_StringArena *tcl_create_string_arena(void);
void TCL_set_string_arena(TCL_StringArena **stringArena,TCL_String *string);
void TCL_garbage_collect_string_arena(TCL_StringArena **stringArena);

// ========== functions
size_t tcl_hash_string(TCL_String *string){
	return tcl_hash_array(&(string->data),string->length);
}
size_t tcl_hash_array(char *arr,int32_t length){
	size_t hash = 0;
	/*
	switch(length & 3){
		case 3:hash += string->data[length - 3] * 555;
		case 2:hash += string->data[length - 2] * 177;
		case 1:hash += string->data[length - 1];
		default:break;
	}
	int32_t *changed = (void*)(arr);
	for(int32_t idx = 0;idx < (length >> 2);idx++){
		hash += (hash << 17) ^ (hash << 13);
		hash += changed[idx];
		hash ^= hash >> 11;
	} */
	for(int32_t idx = 0;idx < length;idx++){
		hash += (hash << (sizeof(size_t) * 4 + 3)) ^
			(hash << (sizeof(size_t) * 2 + 1));
		hash += arr[idx];
		hash ^= (hash >> 11) + (hash >> (sizeof(size_t) * 3 + 2));
	}
	return hash;
}
char tcl_string_eq(TCL_String *cmp,TCL_String *str){
	if(str->length != cmp->length)
		return 0;
	// this is very unlikely
	if(str == cmp)
		return 1;
	for(int over = 0;over < str->length;over++){
		if(cmp->data[over] != str->data[over]){
			return 0;
		}
	}
	return 1;
}
void tcl_string_cp(TCL_String **into,TCL_String *from){
	char sizeChange = 0;
	while((*into)->capacity <= (*into)->length + from->length){
		(*into)->capacity *= 2;
		sizeChange = 1;
	}
	if(sizeChange){
		(*into) = realloc(sizeof(TCL_String) +
				sizeof(char) * (*into)->capacity);
	}
	for(int idx = 0;idx < from->length;idx++){
		(*into)->data[(*into)->length] = from->data[idx];
		(*into)->length++;
	}
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
		if(!tcl_string_eq(scope->kv[idx].key,key))
			continue;
		oldValue = scope->kv[idx].value;
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
		if(hash != scope->kv[idx].kHash)continue;
		if(!tcl_string_eq(scope->kv[idx].value,key))
			continue;
		return scope->kv[idx].value;
	}
	return NULL;
}
TCL_String *tcl_get_from_scope_slice(TCL_Scope **stringScope,TCL_Slice *key){
	TCL_Scope *scope = *stringScope;
	size_t hash = tcl_hash_array(&(key->string->data[key->offset]),key->length);
	for(int idx = 0;idx < scope->length;idx++){
		if(hash != scope->kv[idx].kHash)continue;
		if(!tcl_string_eq(scope->kv[idx].value,key))
			continue;
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
