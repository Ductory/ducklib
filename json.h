#ifndef _JSON_H
#define _JSON_H

#if __STDC_VERSION__ < 202000
#include <stdbool.h>
#endif
#include <stdint.h>
#include <wchar.h>

enum JSON_TYPE {
	JT_EMPTY,
	JT_INT,
	JT_LONG,
	JT_REAL,
	JT_STRING,
	JT_BOOL,
	JT_ARRAY,
	JT_OBJECT,
	JT_NULL
};

typedef enum JSON_TYPE jtype_t;
typedef int jint_t;
typedef long long jlong_t;
typedef double jreal_t;
typedef bool jbool_t;
typedef wchar_t jchar_t, *jstr_t;
typedef void *json_t;
typedef struct _jsonval *jval_t;
typedef struct _jsonitem *jitem_t;
typedef struct _jsonobj *jobj_t;
typedef struct _jsonarr *jarr_t;

struct _jsonval {
	jtype_t t;
	union {
		jint_t  i;
		jlong_t l;
		jreal_t r;
		jbool_t b;
		jstr_t  s;
		jobj_t  o;
		jarr_t  a;
	};
};

struct _jsonitem {
	struct _jsonval;
	const char *key;
	size_t hash;
};

struct _jsonobj {
	uint8_t  size_bits;
	jitem_t *entries;
	size_t  *indices;
	size_t   count;
};

struct _jsonarr {
	size_t  size, cap;
	jval_t  data;
};

json_t json_load(const char *str);
json_t json_loadf(const char *name);
void json_freeobj(jobj_t obj);
void json_freearr(jarr_t arr);
jitem_t json_get(jobj_t obj, const char *key);
jitem_t json_getl(jobj_t obj, ...);

bool json_cmpval(jval_t val1, jval_t val2);
bool json_cmpobj(jobj_t obj1, jobj_t obj2);
bool json_cmparr(jarr_t arr1, jarr_t arr2);
jobj_t json_dupobj(jobj_t obj);
jarr_t json_duparr(jarr_t arr);
bool json_objadd(jobj_t obj, const char *key, jval_t val);
void json_objrmv(jobj_t obj, const char *key);
bool json_arradd(jarr_t arr, jval_t val);
void json_arrrmv(jarr_t arr, size_t idx);
char *json_saveobj(jobj_t obj, bool utf8);
char *json_savearr(jarr_t arr, bool utf8);
size_t json_objcount(jobj_t obj);
size_t json_arrcount(jarr_t arr);

#define json_free(a) _Generic(a, \
		jobj_t: json_freeobj, \
		jarr_t: json_freearr  \
	)(a)
#define json_cmp(a,b) _Generic(a, \
		jval_t: json_cmpval, \
		jobj_t: json_cmpobj, \
		jarr_t: json_cmparr  \
	)(a, b)
#define json_dup(a) _Generic(a, \
		jobj_t: json_dupobj, \
		jarr_t: json_duparr  \
	)(a)
#define json_add(a,args...) _Generic(a, \
		jobj_t: json_objadd, \
		jarr_t: json_arradd  \
	)(a, args)
#define json_rmv(a,args) _Generic(a, \
		jobj_t: json_objrmv, \
		jarr_t: json_arrrmv  \
	)(a, args)
#define json_save(a,args) _Generic(a, \
		jobj_t: json_saveobj, \
		jarr_t: json_savearr  \
	)(a, args)
#define json_count(a) _Generic(a, \
		jobj_t: json_objcount, \
		jarr_t: json_arrcount  \
	)(a)


#define JSON_GET_INT(obj,x)  json_get(obj, # x)->i
#define JSON_GET_LONG(obj,x) json_get(obj, # x)->l
#define JSON_GET_REAL(obj,x) json_get(obj, # x)->r
#define JSON_GET_BOOL(obj,x) json_get(obj, # x)->b
#define JSON_GET_STR(obj,x)  json_get(obj, # x)->s
#define JSON_GET_OBJ(obj,x)  json_get(obj, # x)->o
#define JSON_GET_ARR(obj,x)  json_get(obj, # x)->a

#define JSON_FOR_OBJ(o,item) \
	for (jitem_t item, *_ent = (o)->entries; ({while (!*_ent && ++_ent);}), \
		item = *_ent, _ent < (o)->entries + (o)->count; ++_ent)

#define JSON_FOR_ARR(a,val) \
	for (jval_t val = (a)->arr; val < (a)->arr + (a)->size; ++val)

#endif // _JSON_H
