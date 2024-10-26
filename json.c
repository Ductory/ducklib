#include "json.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <stdint.h>
#include <limits.h>
#include <setjmp.h>
#include <stdarg.h>


/////////////////
// declaration //
/////////////////

static void free_value(jval_t val);


////////////////
// allocation //
////////////////

#define alloc(obj,n)  malloc(sizeof(*(obj)) * (n))
#define allocr(obj,n) realloc((obj), sizeof(*(obj)) * (n))
#define elemset(obj,val,n) memset(obj, val, (n) * sizeof(*(obj)))

#define DICT_INIT_BITS 3
#define DICT_INIT_SIZE ((size_t)1 << DICT_INIT_BITS)
#define BUF_INIT_SIZE 256
#define ARR_INIT_SIZE  4

static jobj_t new_object(void)
{
	jobj_t obj = alloc(obj, 1);
	size_t size = DICT_INIT_SIZE;
	obj->size_bits = DICT_INIT_BITS;
	obj->entries   = alloc(obj->entries, size);
	obj->indices   = alloc(obj->indices, size);
	obj->count     = 0;
	elemset(obj->indices, -1, size);
	return obj;
}

static jarr_t new_array(void)
{
	jarr_t arr = alloc(arr, 1);
	arr->size = 0;
	arr->cap = ARR_INIT_SIZE;
	arr->data = NULL;
	return arr;
}


////////////////
// dictionary //
////////////////

#define DICT_SIZE(dict) ((size_t)1 << (dict)->size_bits)
#define DICT_MASK(dict) (DICT_SIZE(dict) - 1)
#define NEXT_INDEX(dict,i) ((((i) * 5) + 1) & DICT_MASK(dict))
#define IS_ENTRY(ent,h,k) ((ent)->hash == h && strcmp((ent)->key, k) == 0)

#define IDX_EMPTY ((size_t)-1)
#define IDX_DUMMY ((size_t)-2)
#define IDX_MAX   IDX_DUMMY
#define IDX_INVALID ((size_t)-1)

static size_t hash_key(const char *key)
{
	size_t h = 1;
	const uint8_t *k = (uint8_t*)key;
	while (*k)
		h += (h << 5) + (h >> (sizeof(size_t) * CHAR_BIT - 5)) + *k++;
	return h;
}

static size_t query_index(jobj_t obj, const char *key)
{
	size_t hash = hash_key(key);
	size_t i = hash & DICT_MASK(obj), e;
	while ((e = obj->indices[i]) != IDX_EMPTY) {
		if (e != IDX_DUMMY && IS_ENTRY(obj->entries[e], hash, key))
			return i;
		i = NEXT_INDEX(obj, i);
	}
	return IDX_INVALID;
}

static void expand_dict(jobj_t obj)
{
	size_t size = (size_t)1 << ++obj->size_bits, mask = size - 1;
	obj->entries = allocr(obj->entries, size);
	obj->indices = allocr(obj->indices, size);
	elemset(obj->indices, -1, size);
	size_t cnt = 0;
	for (size_t e = 0; e < obj->count; ++e) {
		jitem_t item = obj->entries[e];
		if (!item)
			continue;
		size_t i = item->hash & mask;
		while (obj->indices[i] != IDX_EMPTY)
			i = NEXT_INDEX(obj, i);
		obj->indices[i] = cnt;
		obj->entries[cnt] = item;
		++cnt;
	}
	obj->count = cnt;
}

/**
 * add an entry to dictionary
 * @param  obj 
 * @param  key 
 * @param  val 
 * @return     1 if success
 *
 * @note won't copy key and value. caller should do the work.
 */
static bool dict_add(jobj_t obj, const char *key, jval_t val)
{
	size_t hash = hash_key(key);
	size_t i = hash & DICT_MASK(obj), e;
	while ((e = obj->indices[i]) < IDX_MAX) {
		jitem_t item = obj->entries[e];
		if (IS_ENTRY(item, hash, key)) {
			free_value((jval_t)item);
			*(jval_t)item = *val;
			return true;
		}
		i = NEXT_INDEX(obj, i);
	}
	jitem_t item = alloc(item, 1);
	item->hash = hash;
	item->key = key;
	*(jval_t)item = *val;
	obj->indices[i] = obj->count;
	obj->entries[obj->count] = item;
	++obj->count;
	if (obj->count > DICT_SIZE(obj) * 2 / 3)
		expand_dict(obj);
	return true;
}


////////////////////////
// exception handling //
////////////////////////

static jmp_buf _buf;

static void __attribute__((noreturn)) error(const char *s, const char *msg)
{
	fprintf(stderr, "json error(%s):\nnear %s\n", msg, s);
	/* memory leak */
	longjmp(_buf, 1);
}


/////////////
// parsing //
/////////////

/* character handle */

/* hex to dec */
static inline int htod(int x) { return (x & 0xF) + (x >> 6) * 9; }
/* is oct */
static int isodigit(int x) { return (x & 0xF8) == '0'; }
/* is bin */
static int isbdigit(int x) { return (x & 0xFE) == '0'; }


/* scan */

typedef const char **ps_t;

#define enter_ps const char *s = *ps;
#define leave_ps *ps = s;

static inline void skip_space(ps_t ps)
{
	enter_ps
	while (isspace(*s) && ++s);
	leave_ps
}

/**
 * match a character
 * @param  ps     
 * @param  ch     
 * @param  strict strict mode (throw an exception if unmatched)
 * @return        1 if matched
 */
static bool _match(ps_t ps, char ch, bool strict)
{
	skip_space(ps);
	if (**ps != ch) {
		if (strict)
			error(*ps, "unmatch");
		else
			return false;
	}
	++*ps;
	return true;
}

/**
 * match a character (strict)
 * @param  ps 
 * @param  ch 
 * @return    
 * @see `_match`
 */
static inline bool match(ps_t ps, char ch) { return _match(ps, ch, true); }

/**
 * match a character (not strict)
 * @param  ps 
 * @param  ch 
 * @return    
 * @see `_match`
 */
static inline bool try(ps_t ps, char ch) { return _match(ps, ch, false); }


/* parse */

static jobj_t parse_object(ps_t ps);
static jarr_t parse_array(ps_t ps);

static const char *parse_key(ps_t ps)
{
	match(ps, '"');
	enter_ps
	const char *head = s;
	while (*s != '"' && ++s);
	// alloc key
	size_t len = s - head;
	char *key = malloc(len + 1);
	memcpy(key, head, len);
	key[len] = '\0';
	++s; // skip '"'
	leave_ps
	return key;
}

static void parse_number(ps_t ps, jval_t val)
{
	enter_ps
	int sign = 1, e_sign = 1, base = 10;
	jlong_t ji;
	jreal_t jr, e = 1;
	int (*in_radix)(int) = isdigit; // check validity of the character (decimal in default)
begin_match:
	switch (*s) {
		case '-': // minus
			sign = -sign;
		case '+':
			++s;
			goto begin_match;
		case '0': // change radix
			switch (s[1]) {
				case 'b': case 'B':
					base = 2;
					in_radix = isbdigit;
					s += 2;
					break;
				case '0' ... '7':
					base = 8;
					in_radix = isodigit;
					++s;
					break;
				case 'x': case 'X':
					base = 16;
					in_radix = isxdigit;
					s += 2;
					break;
			}
		case '1' ... '9':
			// integer part
			for (ji = 0; in_radix(*s); ++s)
				ji = ji * base + htod(*s);
		case '.': // allow no leading-zero
			switch (*s) {
				case '.': // decimal part
					val->t = JT_REAL;
					++s;
					for (jr = ji; in_radix(*s); ++s)
						jr += (e /= base) * htod(*s);
					val->r = sign * jr;
					break;
				case 'e': case 'E': // scientific notation
					val->t = JT_REAL;
					switch (*++s) {
						case '-':
							e_sign = -1;
						case '+':
							++s;
							break;
					}
					jr = ji;
					for (ji = 0; isdigit(*s); ++s)
						ji = ji * 10 + *s - '0';
					jr *= exp(log(base) * e_sign * ji);
					val->r = sign * jr;
					break;
				default:
					val->t = JT_LONG;
					val->l = sign * ji;
					break;
			}
			break;
	}
	leave_ps
}

static inline jstr_t parse_string(ps_t ps)
{
	const uint8_t *s = (uint8_t*)*ps;
	++s;
	size_t len = 0, buf_size = BUF_INIT_SIZE;
	jchar_t *buf = alloc(buf, BUF_INIT_SIZE);
	while (*s != '"') {
		if (*s == '\\') { // escape sequence
			s += 2;
			switch (s[-1]) {
				case '0':  buf[len++] = '\0'; break;
				case 'a':  buf[len++] = '\a'; break;
				case 'b':  buf[len++] = '\b'; break;
				case 't':  buf[len++] = '\t'; break;
				case 'n':  buf[len++] = '\n'; break;
				case 'v':  buf[len++] = '\v'; break;
				case 'f':  buf[len++] = '\f'; break;
				case 'r':  buf[len++] = '\r'; break;
				case '/':  buf[len++] = '/';  break;
				case '\\': buf[len++] = '\\'; break;
				case '"':  buf[len++] = '"';  break;
				case 'u': { // utf-16
					jchar_t ch = 0;
					for (int i = 0; i < 4; ++i, ++s)
						ch = (ch << 4) | htod(*s);
					buf[len++] = ch;
					break;
				}
				default:
					error((char*)s, "unknown escape sequence");
					break;
			}
		} else if (*s >= 0x80) { // assuming utf-8
			int cont, c = *s, n;
			// check leading byte
			if (c < 0xC2) {
				goto invalid_utf8;
			} else if (c <= 0xDF) {
				cont = 1; n = c & 0x1F;
			} else if (c <= 0xEF) {
				cont = 2; n = c & 0xF;
			} else if (c <= 0xF4) {
				cont = 3; n = c & 0x7;
			} else {
				goto invalid_utf8;
			}
			// decode
			for (int i = 1; i <= cont; i++) {
				int l = 0x80, h = 0xBF;
				// second byte
				if (i == 1) {
					switch (c) {
						case 0xE0: l = 0xA0; break;
						case 0xED: h = 0x9F; break;
						case 0xF0: l = 0x90; break;
						case 0xF4: h = 0x8F; break;
					}
				}
				if (s[i] < l || s[i] > h)
					goto invalid_utf8;
				n = (n << 6) | (s[i] & 0x3F);
			}

			s += 1 + cont;
			// change to utf-16
			if (n < 0x10000) {
				buf[len++] = n;
			} else {
				n -= 0x10000;
				buf[len++] = 0xD800 | (n >> 10);
				buf[len++] = 0xDC00 | (n & 0x3FF);
			}
		} else { // ascii
			buf[len++] = *s++;
		}
		if (len + 2 > buf_size)
			buf = allocr(buf, buf_size <<= 1);
	}
	++s;
	buf[len++] = '\0';
	*ps = (const char*)s;
	return allocr(buf, len);
	/* error handling */
invalid_utf8:
	error((char*)s, "ill-formed UTF-8 subsequence");
}

static void parse_value(ps_t ps, jval_t val)
{
	skip_space(ps);
	enter_ps
	switch (*s) {
		case '-':
		case '+':
		case '0' ... '9':
		case '.':
			parse_number(&s, val);
			break;
		case '"':
			val->t = JT_STRING;
			val->s = parse_string(&s);
			break;
		case 't':
			if (s[1] == 'r' && s[2] == 'u' && s[3] == 'e') {
				val->t = JT_BOOL;
				val->b = true;
				s += 4;
			} else {
				error(s, "invalid value");
			}
			break;
		case 'f':
			if (s[1] == 'a' && s[2] == 'l' && s[3] == 's' && s[4] == 'e') {
				val->t = JT_BOOL;
				val->b = false;
				s += 5;
			} else {
				error(s, "invalid value");
			}
			break;
		case 'n':
			if (s[1] == 'u' && s[2] == 'l' && s[3] == 'l') {
				val->t = JT_NULL;
				s += 4;
			} else {
				error(s, "invalid value");
			}
			break;
		case '{':
			val->t = JT_OBJECT;
			val->o = parse_object(&s);
			break;
		case '[':
			val->t = JT_ARRAY;
			val->a = parse_array(&s);
			break;
		default:
			error(s, "unexpected char");
			break;
	}
	leave_ps
}

static jobj_t parse_object(ps_t ps)
{
	match(ps, '{');
	jobj_t obj = new_object();
	// empty object
	if (try(ps, '}'))
		return obj;
	struct _jsonval val;
	const char *key;
	do {
		key = parse_key(ps);
		match(ps, ':');
		parse_value(ps, &val);
	} while (dict_add(obj, key, &val)
		&& (try(ps, ',') || !match(ps, '}')) && !try(ps, '}'));
	return obj;
}

static inline void check_arr_expand(jarr_t arr)
{
	if (arr->size >= arr->cap)
		arr->data = allocr(arr->data, arr->cap <<= 1);
}

static jarr_t parse_array(ps_t ps)
{
	match(ps, '[');
	jarr_t arr = new_array();
	// empty array
	if (try(ps, ']'))
		return arr;
	
	arr->data = alloc(arr->data, ARR_INIT_SIZE);
	do {
		check_arr_expand(arr);
		parse_value(ps, &arr->data[arr->size++]);
	} while ((try(ps, ',') || !match(ps, ']')) && !try(ps, ']'));
	return arr;
}


//////////
// load //
//////////

/**
 * load json from string
 * @param  str 
 * @return     
 */
json_t json_load(const char *str)
{
	if (!str || setjmp(_buf))
		return NULL;
	skip_space(&str);
	return *str == '{' ? (void*)parse_object(&str) : (void*)parse_array(&str);
}

/**
 * load json from file
 * @param  name 
 * @return      
 */
json_t json_loadf(const char *name)
{
	if (!name)
		return NULL;
	FILE *fp = fopen(name, "r");
	if (!fp)
		return NULL;
	fseek(fp, 0, SEEK_END);
	size_t size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	char *str = malloc(size + 1);
	str[size] = '\0';
	fread(str, 1, size, fp);
	fclose(fp);
	json_t json = json_load(str);
	free(str);
	return json;
}


/////////
//free //
/////////

static void free_value(jval_t val)
{
	switch (val->t) {
		case JT_STRING: free(val->s); break;
		case JT_OBJECT: json_freeobj(val->o); break;
		case JT_ARRAY:  json_freearr(val->a); break;
		default: break;
	}
}

static void free_item(jitem_t item)
{
	free((void*)item->key);
	free_value((jval_t)item);
	free(item);
}

void json_freeobj(jobj_t obj)
{
	for (size_t e = 0; e < obj->count; ++e) {
		jitem_t item = obj->entries[e];
		if (!item)
			continue;
		free_item(item);
	}
	free(obj->entries);
	free(obj->indices);
	free(obj);
}

void json_freearr(jarr_t arr)
{
	for (size_t i = 0; i < arr->size; ++i)
		free_value(&arr->data[i]);
	free(arr->data);
	free(arr);
}


//////////////
// get item //
//////////////

jitem_t json_get(jobj_t obj, const char *key)
{
	if (!obj)
		return NULL;
	size_t i = query_index(obj, key);
	if (i == IDX_INVALID)
		return NULL;
	return obj->entries[obj->indices[i]];
}

jitem_t json_getl(jobj_t obj, ...)
{
	if (!obj)
		return NULL;
	va_list ap;
	va_start(ap, obj);
	const char *key;
	jitem_t item;
	while ((key = va_arg(ap, const char *))) {
		size_t i = query_index(obj, key);
		if (i == IDX_INVALID)
			return NULL;
		item = obj->entries[obj->indices[i]];
		obj = item->o;
	}
	va_end(ap);
	return item;
}


////////////////
// comparison //
////////////////

bool json_cmpval(jval_t val1, jval_t val2)
{
	if (val1->t != val2->t)
		return false;
	switch (val1->t) {
		case JT_EMPTY:  
		case JT_NULL:   return true;
		case JT_INT:    return val1->i == val2->i;
		case JT_LONG:   return val1->l == val2->l;
		case JT_REAL:   return val1->r == val2->r;
		case JT_BOOL:   return val1->b == val2->b;
		case JT_STRING: return !wcscmp(val1->s, val2->s);
		case JT_ARRAY:  return json_cmparr(val1->a, val2->a);
		case JT_OBJECT: return json_cmpobj(val1->o, val2->o);
		default:        return false;
	}
}

bool json_cmpobj(jobj_t obj1, jobj_t obj2)
{
	for (size_t i = 0; i < obj2->count; ++i) {
		jitem_t itm2 = obj2->entries[i];
		if (!itm2)
			continue;
		size_t j = query_index(obj1, itm2->key);
		if (j == IDX_INVALID)
			return false;
		if (!json_cmpval((jval_t)obj1->entries[obj1->indices[j]], (jval_t)itm2))
			return false;
	}
	return true;
}

bool json_cmparr(jarr_t arr1, jarr_t arr2)
{
	if (arr1->size != arr2->size)
		return false;
	for (size_t i = 0; i < arr1->size; ++i)
		if (!json_cmpval(&arr1->data[i], &arr2->data[i]))
			return false;
	return true;
}


///////////////
// duplicate //
///////////////

static void dup_value(jval_t dest, jval_t src)
{
	switch ((dest->t = src->t)) {
		case JT_EMPTY:
		case JT_NULL:   break;
		case JT_INT:    dest->i = src->i; break;
		case JT_LONG:   dest->l = src->l; break;
		case JT_REAL:   dest->r = src->r; break;
		case JT_BOOL:   dest->b = src->b; break;
		case JT_STRING: dest->s = wcsdup(src->s); break;
		case JT_ARRAY:  dest->a = json_duparr(src->a); break;
		case JT_OBJECT: dest->o = json_dupobj(src->o); break;
	}
}

static jitem_t dup_item(jitem_t item)
{
	jitem_t i = alloc(i, 1);
	i->hash = item->hash;
	i->key = strdup(item->key);
	dup_value((jval_t)i, (jval_t)item);
	return i;
}

jobj_t json_dupobj(jobj_t obj)
{
	jobj_t o = new_object();
	size_t size = (size_t)1 << (o->size_bits = obj->size_bits), mask = size - 1;
	o->entries = allocr(o->entries, size);
	o->indices = allocr(o->indices, size);
	elemset(o->indices, -1, size);
	size_t cnt = 0;
	for (size_t e = 0; e < obj->count; ++e) {
		jitem_t item = obj->entries[e];
		if (!item)
			continue;
		size_t i = item->hash & mask;
		while (o->indices[i] != IDX_EMPTY)
			i = NEXT_INDEX(o, i);
		o->indices[i] = cnt;
		o->entries[cnt] = dup_item(item);
		++cnt;
	}
	o->count = cnt;
	return o;
}

jarr_t json_duparr(jarr_t arr)
{
	jarr_t a = new_array();
	a->size = arr->size;
	a->cap = arr->cap;
	a->data = alloc(a->data, a->cap);
	for (size_t i = 0; i < a->size; ++i)
		dup_value(&a->data[i], &arr->data[i]);
	return a;
}


////////////////
// add/remove //
////////////////

bool json_objadd(jobj_t obj, const char *key, jval_t val)
{
	const char *new_key = strdup(key);
	struct _jsonval new_val;
	dup_value(&new_val, val);
	return dict_add(obj, new_key, &new_val);
}

void json_objrmv(jobj_t obj, const char *key)
{
	size_t i = query_index(obj, key);
	if (i == IDX_INVALID)
		return;
	size_t e = obj->indices[i];
	jitem_t item = obj->entries[e];
	free_item(item);
	obj->indices[i] = IDX_DUMMY;
	obj->entries[e] = NULL;
}

bool json_arradd(jarr_t arr, jval_t val)
{
	check_arr_expand(arr);
	dup_value(&arr->data[arr->size++], val);
	return true;
}

void json_arrrmv(jarr_t arr, size_t idx)
{
	memcpy(&arr->data[idx], &arr->data[idx + 1],
		(--arr->size - idx) * sizeof(struct _jsonval));
}


////////////
// buffer //
////////////

struct _buf {
	size_t size;
	size_t cap;
	char *buf;
	bool utf8;
};
typedef struct _buf *buf_t;

static buf_t new_buf(bool utf8)
{
	size_t size = BUF_INIT_SIZE;
	buf_t buf = alloc(buf, 1);
	buf->cap = size;
	buf->size = 0;
	buf->buf = malloc(size);
	buf->utf8 = utf8;
	return buf;
}

static inline void check_buf_expand(buf_t buf)
{
	while (buf->size >= buf->cap)
		buf->cap <<= 1;
	buf->buf = allocr(buf->buf, buf->cap);
}

static int buf_ccat(buf_t buf, int ch)
{
	check_buf_expand(buf);
	return buf->buf[buf->size++] = ch;
}

static int buf_cat(buf_t buf, const char *s)
{
	size_t len = strlen(s);
	size_t tmp = buf->size;
	buf->size += len;
	check_buf_expand(buf);
	memcpy(buf->buf + tmp, s, len);
	return len;
}

static int buf_wcat(buf_t buf, const wchar_t *s)
{
	size_t len = wcslen(s) * 2;
	size_t tmp = buf->size;
	buf->size += len;
	check_buf_expand(buf);
	memcpy(buf->buf + tmp, s, len);
	return len;
}

static int buf_catf(buf_t buf, const char *fmt, ...)
{
	char b[2048];
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(b, sizeof(buf), fmt, ap);
	return buf_cat(buf, b);
}

static void buf_unget(buf_t buf)
{
	--buf->size;
}

#define _ccat(x) buf_ccat(buf, (x))
#define _cat(x)  buf_cat(buf, (x))
#define _catf(x...) buf_catf(buf, x)
#define _unget() buf_unget(buf)


//////////
// save //
//////////

static void save_object(buf_t buf, jobj_t obj);
static void save_array(buf_t buf, jarr_t arr);

static void save_string(buf_t buf, jstr_t s)
{
	_ccat('"');
	size_t len = 0;
	char *p = buf->buf + buf->size, *end = buf->buf + buf->cap;
	while (*s) {
		int n = *s;
		switch (n) {
			case '\0': _cat("\\0"); goto set_ptr;
			case '\a': _cat("\\a"); goto set_ptr;
			case '\b': _cat("\\b"); goto set_ptr;
			case '\t': _cat("\\t"); goto set_ptr;
			case '\n': _cat("\\n"); goto set_ptr;
			case '\v': _cat("\\v"); goto set_ptr;
			case '\f': _cat("\\f"); goto set_ptr;
			case '\r': _cat("\\r"); goto set_ptr;
			case '"': _cat("\\\""); goto set_ptr;
			case '\\': _cat("\\\\"); goto set_ptr;
			case 0x1 ... 0x6: // control
			case 0xE ... 0x1F:
				_catf("\\u%04x", n);
				goto set_ptr;
			case 0x20 ... '"' - 1: // printable
			case '"' + 1 ... '\\' - 1:
			case '\\' + 1 ... 0x7F:
				_ccat(n); goto set_ptr;
			default:
				if (buf->utf8) {
					if (n >> 10 == 0x36) {
						++s;
						if (*s >> 10 != 0x37)
							goto invalid_utf16;
						n = 0x10000 + (((n & 0x3FF) << 10) | (*s & 0x3FF));
					}
					// 转utf-8
					if (n <= 0x7F) {
						*p++ = n;
					} else if (n <= 0x7FF) {
						*p++ = 0xC0 | (n >> 6);
						*p++ = 0x80 | (n & 0x3F);
					} else if (n <= 0xFFFF) {
						*p++ = 0xE0 | (n >> 12);
						*p++ = 0x80 | (n >> 6 & 0x3F);
						*p++ = 0x80 | (n      & 0x3F);
					} else {
						*p++ = 0xF0 | (n >> 18);
						*p++ = 0x80 | (n >> 12 & 0x3F);
						*p++ = 0x80 | (n >> 6  & 0x3F);
						*p++ = 0x80 | (n       & 0x3F);
					}
					buf->size = p - buf->buf;
				} else {
					_catf("\\u%04x", n); goto set_ptr;
				}
				break;
			set_ptr:
				p = buf->buf + buf->size;
		}
		++s;
		if (p + 6 > end) {
			size_t len = p - buf->buf;
			buf->buf = allocr(buf->buf, buf->cap <<= 1);
			p = buf->buf + len;
			end = buf->buf + buf->cap;
		}
	}
	buf->size = p - buf->buf;
	_ccat('"');
	return;
	/* error handling */
invalid_utf16:
	error("", "ill-formed UTF-16 subsequence");
}

static void save_value(buf_t buf, jval_t val)
{
	switch (val->t) {
		case JT_INT:    _catf("%d", val->i); break;
		case JT_LONG:   _catf("%lld", val->l); break;
		case JT_REAL:   _catf("%f", val->r); break;
		case JT_BOOL:   _cat(val->b ? "true" : "false"); break;
		case JT_NULL:   _cat("null"); break;
		case JT_STRING: save_string(buf, val->s); break;
		case JT_ARRAY:  save_array(buf, val->a); break;
		case JT_OBJECT: save_object(buf, val->o); break;
		default: break;
	}
}

static void save_item(buf_t buf, jitem_t item)
{
	_ccat('"'); _cat(item->key); _ccat('"');
	_ccat(':');
	save_value(buf, (jval_t)item);
}

static void save_object(buf_t buf, jobj_t obj)
{
	_ccat('{');
	if (obj->count) {
		jitem_t *item = obj->entries;
		jitem_t *end = obj->entries + obj->count;
		do
			if (*item)
				save_item(buf, *item);
		while (++item < end && _ccat(','));
	}
	_ccat('}');
}

static void save_array(buf_t buf, jarr_t arr)
{
	_ccat('[');
	if (arr->size) {
		jval_t val = arr->data;
		jval_t end = arr->data + arr->size;
		do
			save_value(buf, val);
		while (++val < end && _ccat(','));
	}
	_ccat(']');
}

char *json_saveobj(jobj_t obj, bool utf8)
{
	buf_t buf = new_buf(utf8);
	save_object(buf, obj);
	_ccat('\0');
	char *s = buf->buf;
	free(buf);
	return s;
}

char *json_savearr(jarr_t arr, bool utf8)
{
	buf_t buf = new_buf(utf8);
	save_array(buf, arr);
	_ccat('\0');
	char *s = buf->buf;
	free(buf);
	return s;
}

size_t json_objcount(jobj_t obj)
{
	return obj->count;
}

size_t json_arrcount(jarr_t arr)
{
	return arr->size;
}
