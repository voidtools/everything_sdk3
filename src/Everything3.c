
//
// Copyright (C) 2025 voidtools / David Carpenter
// 
// Permission is hereby granted, free of charge, 
// to any person obtaining a copy of this software 
// and associated documentation files (the "Software"), 
// to deal in the Software without restriction, 
// including without limitation the rights to use, 
// copy, modify, merge, publish, distribute, sublicense, 
// and/or sell copies of the Software, and to permit 
// persons to whom the Software is furnished to do so, 
// subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be 
// included in all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES 
// OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, 
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, 
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE 
// SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
//
//
//
// Notes:
// A WouldBlock() API would not work because we are not syncronized.
// -We might block after the call returns.
// We wait for the DB to load before returning results. no need to call Everything3_IsDBLoaded.
//
//
//
// TODO:
// PropVariant support.
// EnumResults() for faster ES results. -don't use a result_list 
// Convert properties, eg: value type is PROPERTY_VALUE_TYPE_BYTE, which we should be able to get from GetPropertyDWORD() -or should we be strict because casting may lose data
// match_pinyin search flag.
// Index Journal API
// api to get a property from a filename?
// a flag to load properties in the background and post result changes.
//
//
//
// Changes:
// A cleaner way to call Everything3_Search() after Everything3_IsResultListChange returns true. -call Everything3_GetResults
// Everything3_Search() needs a force search flag. -If we search for dm:last5mins and research, the search hasn't changed, so the last5mins search will not update. the same results are returned. -added a flag, but to keep things simple, caller can add no-search-current-results: to their search.
// FindFirstFile() API
// GetFileAttributes() API or FileExists() API
// 64bit dll will break if using 32bit Everything. (Viewport count overflows)
// a way to get the content property -currently the content property id changes. -made content id static.
// is Network Index online API -just search for the network index root and check the online property.
// static lib

// dll export
#ifdef EVERYTHING3_LIB
#define EVERYTHING3_USERAPI
#else
#define EVERYTHING3_USERAPI __declspec(dllexport)
#endif

#define WIN32_LEAN_AND_MEAN

// includes
#include <windows.h>
#include <limits.h>	// SIZE_MAX

#ifdef _DEBUG
#define _EVERYTHING3_DEBUG
// track memory leaks.
SIZE_T _everything3_mem_allocated = 0;
#endif

#ifdef _EVERYTHING3_DEBUG
#include <assert.h>	// assert()
#else
#define assert(exp)
#endif

// include
#include "../include/Everything3.h"

// stack buffer sizes
#define _EVERYTHING3_WCHAR_BUF_STACK_SIZE					MAX_PATH
#define _EVERYTHING3_UTF8_BUF_STACK_SIZE					MAX_PATH

// IPC pipe commands.
#define _EVERYTHING3_COMMAND_GET_IPC_PIPE_VERSION			0
#define _EVERYTHING3_COMMAND_GET_MAJOR_VERSION				1
#define _EVERYTHING3_COMMAND_GET_MINOR_VERSION				2
#define _EVERYTHING3_COMMAND_GET_REVISION					3
#define _EVERYTHING3_COMMAND_GET_BUILD_NUMBER				4
#define _EVERYTHING3_COMMAND_GET_TARGET_MACHINE				5
#define _EVERYTHING3_COMMAND_FIND_PROPERTY_FROM_NAME		6
#define _EVERYTHING3_COMMAND_SEARCH							7
#define _EVERYTHING3_COMMAND_IS_DB_LOADED					8
#define _EVERYTHING3_COMMAND_IS_PROPERTY_INDEXED			9
#define _EVERYTHING3_COMMAND_IS_PROPERTY_FAST_SORT			10
#define _EVERYTHING3_COMMAND_GET_PROPERTY_NAME				11
#define _EVERYTHING3_COMMAND_GET_PROPERTY_CANONICAL_NAME	12
#define _EVERYTHING3_COMMAND_GET_PROPERTY_TYPE				13
#define _EVERYTHING3_COMMAND_IS_RESULT_CHANGE				14
#define _EVERYTHING3_COMMAND_GET_RUN_COUNT					15
#define _EVERYTHING3_COMMAND_SET_RUN_COUNT					16
#define _EVERYTHING3_COMMAND_INC_RUN_COUNT					17
#define _EVERYTHING3_COMMAND_GET_FOLDER_SIZE				18
#define _EVERYTHING3_COMMAND_GET_FILE_ATTRIBUTES			19
#define _EVERYTHING3_COMMAND_GET_FILE_ATTRIBUTES_EX			20
#define _EVERYTHING3_COMMAND_GET_FIND_FIRST_FILE			21
#define _EVERYTHING3_COMMAND_GET_RESULTS					22
#define _EVERYTHING3_COMMAND_SORT							23
#define _EVERYTHING3_COMMAND_WAIT_FOR_RESULT_CHANGE			24
#define _EVERYTHING3_COMMAND_CANCEL							25

// IPC pipe responses
#define _EVERYTHING3_RESPONSE_OK_MORE_DATA					100 // expect another repsonse.
#define _EVERYTHING3_RESPONSE_OK							200 // reply data depending on request.
#define _EVERYTHING3_RESPONSE_ERROR_BAD_REQUEST				400
#define _EVERYTHING3_RESPONSE_ERROR_CANCELLED				401 // another requested was made while processing...
#define _EVERYTHING3_RESPONSE_ERROR_NOT_FOUND				404
#define _EVERYTHING3_RESPONSE_ERROR_OUT_OF_MEMORY			500
#define _EVERYTHING3_RESPONSE_ERROR_INVALID_COMMAND			501

#define _EVERYTHING3_MESSAGE_DATA(msg)						((void *)(((_everything3_message_t *)(msg)) + 1))

// minimum buffer sizes before growing.
#define _EVERYTHING3_MIN_PROPERTY_REQUEST_COUNT				32
#define _EVERYTHING3_MIN_SORT_COUNT							8

// search flags.
#define _EVERYTHING3_SEARCH_FLAG_MATCH_CASE					0x00000001	// match case
#define _EVERYTHING3_SEARCH_FLAG_MATCH_WHOLEWORD			0x00000002	// match whole word
#define _EVERYTHING3_SEARCH_FLAG_MATCH_PATH					0x00000004	// include paths in search
#define _EVERYTHING3_SEARCH_FLAG_REGEX						0x00000008	// enable regex
#define _EVERYTHING3_SEARCH_FLAG_MATCH_DIACRITICS			0x00000010	// match diacritic marks
#define _EVERYTHING3_SEARCH_FLAG_MATCH_PREFIX				0x00000020	// match prefix (Everything 1.5)
#define _EVERYTHING3_SEARCH_FLAG_MATCH_SUFFIX				0x00000040	// match suffix (Everything 1.5)
#define _EVERYTHING3_SEARCH_FLAG_IGNORE_PUNCTUATION			0x00000080	// ignore punctuation (Everything 1.5)
#define _EVERYTHING3_SEARCH_FLAG_IGNORE_WHITESPACE			0x00000100	// ignore white-space (Everything 1.5)
#define _EVERYTHING3_SEARCH_FLAG_FOLDERS_FIRST_ASCENDING	0x00000000	// folders first when sort ascending
#define _EVERYTHING3_SEARCH_FLAG_FOLDERS_FIRST_ALWAYS		0x00000200	// folders first
#define _EVERYTHING3_SEARCH_FLAG_FOLDERS_FIRST_NEVER		0x00000400	// folders last 
#define _EVERYTHING3_SEARCH_FLAG_FOLDERS_FIRST_DESCENDING	0x00000600	// folders first when sort descending
#define _EVERYTHING3_SEARCH_FLAG_TOTAL_SIZE					0x00000800	// calculate total size
#define _EVERYTHING3_SEARCH_FLAG_HIDE_RESULT_OMISSIONS		0x00001000	// hide omitted results
#define _EVERYTHING3_SEARCH_FLAG_SORT_MIX					0x00002000	// mix file and folder results
#define _EVERYTHING3_SEARCH_FLAG_64BIT						0x00004000	// SIZE_T is 64bits. Otherwise, 32bits.
#define _EVERYTHING3_SEARCH_FLAG_FORCE						0x00008000	// Force a research, even when the search state doesn't change.

#define _EVERYTHING3_SEARCH_SORT_FLAG_DESCENDING			0x00000001

#define _EVERYTHING3_SEARCH_PROPERTY_REQUEST_FLAG_FORMAT	0x00000001
#define _EVERYTHING3_SEARCH_PROPERTY_REQUEST_FLAG_HIGHLIGHT	0x00000002

#define _EVERYTHING3_RESULT_LIST_ITEM_FLAG_FOLDER			0x01
#define _EVERYTHING3_RESULT_LIST_ITEM_FLAG_ROOT				0x02

#define _EVERYTHING3_POOL_MIN_CHUNK_SIZE					65536
#define _EVERYTHING3_POOL_CHUNK_DATA(chunk)					((void *)(((_everything3_pool_chunk_t *)(chunk)) + 1))

#define _EVERYTHING3_BLOB8_DATA(blob)						((void *)(((_everything3_blob8_t *)(blob)) + 1))
#define _EVERYTHING3_BLOB16_DATA(blob)						((void *)(((_everything3_blob16_t *)(blob)) + 1))

#define _EVERYTHING3_FIND_HANDLE_CHUNK_DATA(chunk)			((BYTE *)(((_everything3_find_handle_chunk_t *)(chunk)) + 1))

#ifdef _EVERYTHING3_DEBUG
#include <stdio.h>
#define _everything3_debug_printf		printf
#else
#define _everything3_debug_printf(...)
#endif

// a dyanimcally sized wchar string
// has some stack space to avoid memory allocations.
typedef struct _everything3_wchar_buf_s
{
	// pointer to wchar string data.
	// buf is NULL terminated.
	EVERYTHING3_WCHAR *buf;
	
	// length of buf in wide chars.
	// does not include the null terminator.
	SIZE_T length_in_wchars;
	
	// size of the buffer in wide chars
	// includes room for the null terminator.
	SIZE_T size_in_wchars;
	
	// some stack for us before we need to allocate memory from the system.
	EVERYTHING3_WCHAR stack_buf[_EVERYTHING3_WCHAR_BUF_STACK_SIZE];
	
}_everything3_wchar_buf_t;

// a dyanimcally sized UTF-8 string
// has some stack space to avoid memory allocations.
typedef struct _everything3_utf8_buf_s
{
	// pointer to UTF-8 string data.
	// buf is NULL terminated.
	EVERYTHING3_UTF8 *buf;

	// length of buf in bytes.
	// does not include the null terminator.
	SIZE_T length_in_bytes;

	// size of the buffer in bytes
	// includes room for the null terminator.
	SIZE_T size_in_bytes;

	// some stack for us before we need to allocate memory from the system.
	EVERYTHING3_UTF8 stack_buf[_EVERYTHING3_UTF8_BUF_STACK_SIZE];
	
}_everything3_utf8_buf_t;

// IPC pipe message
typedef struct _everything3_message_s
{
	DWORD code; // _EVERYTHING3_COMMAND_* or _EVERYTHING3_RESPONSE_*
	DWORD size; // excludes header size.
	
	// data follows
	// BYTE data[size];
	
}_everything3_message_t;

// IPC pipe client
typedef struct _everything3_client_s
{
	// critical section
	CRITICAL_SECTION cs;

	// handle to the IPC pipe.
	// INVALID_HANDLE_VALUE if not connected.
	HANDLE pipe_handle;
	
	// events.
	HANDLE send_event;
	HANDLE recv_event;
	HANDLE shutdown_event;
	
}_everything3_client_t;

// recv stream.
typedef struct _everything3_stream_s
{
	_everything3_client_t *client;
	BYTE *buf;
	BYTE *p;
	SIZE_T avail;
	int error_code;
	int got_last;
	int is_64bit;
		
}_everything3_stream_t;

// property request item
typedef struct _everything3_search_property_request_s
{
	DWORD property_id;
	DWORD flags;
	
}_everything3_search_property_request_t;

// sort item
typedef struct _everything3_search_sort_s
{
	DWORD property_id;
	DWORD flags;
	
}_everything3_search_sort_t;

// search state
typedef struct _everything3_search_state_s
{
	CRITICAL_SECTION cs;

	// the search text
	// can be NULL
	EVERYTHING3_UTF8 *search_text;
	
	_everything3_search_property_request_t *property_request_array;
	_everything3_search_sort_t *sort_array;

	SIZE_T search_len;
	SIZE_T viewport_offset;
	SIZE_T viewport_count;
	SIZE_T property_request_count;
	SIZE_T property_request_allocated;
	SIZE_T sort_count;
	SIZE_T sort_allocated;

	DWORD search_flags;
		
}_everything3_search_state_t;

typedef struct _everything3_result_list_property_request_s
{
	// byte offset for this property in an item.
	SIZE_T offset;

	DWORD property_id;
	
	// one or more of _EVERYTHING3_SEARCH_PROPERTY_REQUEST_FLAG_*
	DWORD flags;
	DWORD value_type;
	
}_everything3_result_list_property_request_t;

// sort item
typedef struct _everything3_result_list_sort_s
{
	DWORD property_id;
	DWORD flags;
	
}_everything3_result_list_sort_t;

typedef struct _everything3_result_list_item_s
{
	// pointer to item data:
	// BYTE item_flags
	// property 0
	// property 1
	// property 2
	// ...
	void *property_data;
	
}_everything3_result_list_item_t;

// a data pool chunk
typedef struct _everything3_pool_chunk_s
{
	struct _everything3_pool_chunk_s *next;
	SIZE_T size;
	
	// data follows.

}_everything3_pool_chunk_t;

// a data pool that only grows
// used to build our result list
typedef struct _everything3_pool_s
{
	// a list of full chunks.
	_everything3_pool_chunk_t *chunk_start;
	_everything3_pool_chunk_t *chunk_last;
	
	// current position
	BYTE *p;
	
	// available bytes.
	SIZE_T avail;

}_everything3_pool_t;

// result list
typedef struct _everything3_result_list_s
{
	EVERYTHING3_UINT64 total_result_size;

	// total items.
	SIZE_T folder_result_count;
	SIZE_T file_result_count;
	
	// view port
	SIZE_T viewport_offset;
	SIZE_T viewport_count;
	
	// valid flags.
	DWORD valid_flags;

	// the sort order.
	_everything3_result_list_sort_t *sort_array;
	SIZE_T sort_count;

	// list of requested properties.
	_everything3_result_list_property_request_t *property_request_array;
	SIZE_T property_request_count;

	// property ID sorted property request array.
	_everything3_result_list_property_request_t **sorted_property_request_array;
	
	// a list of items. 
	// the total number of items in this array is viewport_count 
	_everything3_result_list_item_t *item_array;

	// item and property data pool.
	_everything3_pool_t pool;
	
}_everything3_result_list_t;

// Pascal string
// A NULL pointer to a pstring is an empty string.
// avoid allocating empty strings.
typedef struct _everything3_utf8_pstring_s
{
	// if len == 255
	// then a SIZE_T follows with the real length.
	BYTE len;

	// NOT NULL terminated EVERYTHING3_UTF8 *text follows.

}_everything3_utf8_pstring_t;

// a blob of memory.
// maximum size is 255.
// typically used for crc32, sha256, etc..
typedef struct _everything3_blob8_s
{
	BYTE len;

	// data follows.
	
}_everything3_blob8_t;

// a blob of memory.
// maximum size is 65535.
// typically used for first 256 bytes, first 512 bytes (first sector)
typedef struct _everything3_blob16_s
{
	WORD len;

	// data follows.
	
}_everything3_blob16_t;

#pragma pack (push,1)

// a WIN32_FIND_DATA struct for basic file information
// values might be -1 if they are not indexed.
typedef struct _everything3_win32_find_data_s
{
    EVERYTHING3_UINT64 date_created;
    EVERYTHING3_UINT64 date_accessed;
    EVERYTHING3_UINT64 date_modified;
    EVERYTHING3_UINT64 size;
    
    // attributes will be 0 if not indexed.
    // the FILE_ATTRIBUTE_DIRECTORY bit is always valid.
    DWORD attributes;
    
    // NOT NULL terminated filename follows.
    // EVERYTHING3_UTF8 filename[];
	
}_everything3_win32_find_data_t;

#pragma pack (pop)

// a FindFirstFile find handle.
typedef struct _everything3_find_handle_chunk_s
{
	struct _everything3_find_handle_chunk_s *next;
	
	uintptr_t size;
	
	// data follows.
	
}_everything3_find_handle_chunk_t;

// a FindFirstFile find handle.
typedef struct _everything3_find_handle_s
{
	// current position
	BYTE *p;
	
	// available bytes.
	SIZE_T avail;
	
	_everything3_find_handle_chunk_t *chunk_start;
	_everything3_find_handle_chunk_t *chunk_cur;
	DWORD error_code;
	
}_everything3_find_handle_t;

// static functions
static void _everything3_Lock(_everything3_client_t *client);
static void _everything3_Unlock(_everything3_client_t *client);
static SIZE_T _everything3_utf8_string_get_length_in_bytes(const EVERYTHING3_UTF8 *s);
static void *_everything3_mem_alloc(SIZE_T size);
static void *_everything3_mem_calloc(SIZE_T size);
static void _everything3_mem_free(void *ptr);
static SIZE_T _everything3_safe_size_add(SIZE_T a,SIZE_T b);
static SIZE_T _everything3_safe_size_mul_size_of_pointer(SIZE_T a);
static EVERYTHING3_WCHAR *_everything3_wchar_string_cat_wchar_string_no_null_terminate(EVERYTHING3_WCHAR *buf,EVERYTHING3_WCHAR *current_d,const EVERYTHING3_WCHAR *s);
static void _everything3_wchar_buf_init(_everything3_wchar_buf_t *wcbuf);
static void _everything3_wchar_buf_kill(_everything3_wchar_buf_t *wcbuf);
static void _everything3_wchar_buf_empty(_everything3_wchar_buf_t *wcbuf);
static BOOL _everything3_wchar_buf_grow_size(_everything3_wchar_buf_t *wcbuf,SIZE_T length_in_wchars);
static BOOL _everything3_wchar_buf_grow_length(_everything3_wchar_buf_t *wcbuf,SIZE_T length_in_wchars);
static BOOL _everything3_wchar_buf_get_pipe_name(_everything3_wchar_buf_t *wcbuf,const EVERYTHING3_WCHAR *instance_name);
static BOOL _everything3_wchar_buf_copy_ansi_string(_everything3_wchar_buf_t *wcbuf,const EVERYTHING3_CHAR *s);
static BOOL _everything3_wchar_buf_copy_utf8_string_n(_everything3_wchar_buf_t *wcbuf,const EVERYTHING3_UTF8 *s,SIZE_T slen);
static BOOL _everything3_send(_everything3_client_t *client,DWORD code,const void *in_data,SIZE_T in_size);
static BOOL _everything3_recv_header(_everything3_client_t *client,_everything3_message_t *recv_header);
static BOOL _everything3_recv_data(_everything3_client_t *client,void *buf,SIZE_T buf_size);
static BOOL _everything3_recv_skip(_everything3_client_t *client,SIZE_T size);
static BOOL _everything3_ioctrl(_everything3_client_t *client,DWORD code,const void *in_data,SIZE_T in_size,void *out_data,SIZE_T out_size,SIZE_T *out_numread);
static BOOL _everything3_ioctrl_get_string(_everything3_client_t *client,DWORD code,const void *in_data,SIZE_T in_size,_everything3_utf8_buf_t *cbuf);
static void _everything3_utf8_buf_init(_everything3_utf8_buf_t *wcbuf);
static void _everything3_utf8_buf_kill(_everything3_utf8_buf_t *wcbuf);
static void _everything3_utf8_buf_empty(_everything3_utf8_buf_t *wcbuf);
static BOOL _everything3_utf8_buf_grow_size(_everything3_utf8_buf_t *wcbuf,SIZE_T size_in_bytes);
static BOOL _everything3_utf8_buf_grow_length(_everything3_utf8_buf_t *wcbuf,SIZE_T length_in_bytes);
static BOOL _everything3_utf8_buf_copy_wchar_string(_everything3_utf8_buf_t *cbuf,const EVERYTHING3_WCHAR *ws);
static BOOL _everything3_utf8_buf_copy_ansi_string(_everything3_utf8_buf_t *cbuf,const EVERYTHING3_CHAR *as);
static EVERYTHING3_UTF8 *_everything3_utf8_string_copy_wchar_string(EVERYTHING3_UTF8 *buf,const EVERYTHING3_WCHAR *ws);
static DWORD _everything3_find_property(EVERYTHING3_CLIENT *client,const EVERYTHING3_UTF8 *name,SIZE_T name_length_in_bytes);
static void _everything3_stream_read_data(_everything3_stream_t *stream,void *data,SIZE_T size);
static BYTE _everything3_stream_read_byte(_everything3_stream_t *stream);
static EVERYTHING3_WORD _everything3_stream_read_word(_everything3_stream_t *stream);
static DWORD _everything3_stream_read_dword(_everything3_stream_t *stream);
static EVERYTHING3_UINT64 _everything3_stream_read_uint64(_everything3_stream_t *stream);
static SIZE_T _everything3_stream_read_size_t(_everything3_stream_t *stream);
static SIZE_T _everything3_stream_read_len_vlq(_everything3_stream_t *stream);
static BYTE *_everything3_copy_len_vlq(BYTE *buf,SIZE_T value);
static BYTE *_everything3_copy_dword(BYTE *buf,DWORD value);
static BYTE *_everything3_copy_uint64(BYTE *buf,EVERYTHING3_UINT64 value);
static BYTE *_everything3_copy_size_t(BYTE *buf,SIZE_T value);
static BOOL _everything3_change_search_flags(EVERYTHING3_SEARCH_STATE *search_state,DWORD remove_flags,DWORD add_flags);
static BOOL _everything3_set_search_flag(EVERYTHING3_SEARCH_STATE *search_state,DWORD flag,BOOL set_flag);
static DWORD _everything3_get_search_flags(EVERYTHING3_SEARCH_STATE *search_state);
static BOOL _everything3_is_search_flag_set(EVERYTHING3_SEARCH_STATE *search_state,DWORD flag);
static void _everything3_pool_init(_everything3_pool_t *pool);
static void _everything3_pool_kill(_everything3_pool_t *pool);
static void *_everything3_pool_alloc(_everything3_pool_t *pool,SIZE_T size);
static void *_everything3_copy_memory(void *dst,const void *src,SIZE_T size);
static void *_everything3_zero_memory(void *dst,SIZE_T size);
static SIZE_T _everything3_utf8_pstring_calculate_size(SIZE_T len);
static EVERYTHING3_UTF8 *_everything3_utf8_pstring_init_len(_everything3_utf8_pstring_t *pstring,SIZE_T len);
static const EVERYTHING3_UTF8 *_everything3_utf8_pstring_get_text(const _everything3_utf8_pstring_t *pstring);
static SIZE_T _everything3_utf8_pstring_get_len(const _everything3_utf8_pstring_t *pstring);
static _everything3_result_list_property_request_t *_everything3_find_property_request_from_property_id(const EVERYTHING3_RESULT_LIST *result_list,DWORD property_id,BOOL highlight,BOOL format);
static void _everything3_sort_merge(void **dst,void **left,SIZE_T left_count,void **right,SIZE_T right_count,int (*comp)(const void *,const void *));
static void _everything3_sort_split(void **dst,void **src,SIZE_T count,int (*comp)(const void *,const void *));
static BOOL _everything3_sort(void **base,SIZE_T count,int (*comp)(const void *,const void *));
static int _everything3_result_list_property_request_compare(const _everything3_result_list_property_request_t *a,const _everything3_result_list_property_request_t *b);
static void *_everything3_binary_search(void **array_base,SIZE_T count,void *compare_data,int (*compare_proc)(const void *a,const void *b));
static SIZE_T _everything3_safe_wchar_string_copy_utf8_string_n(EVERYTHING3_WCHAR *wbuf,SIZE_T wbuf_size_in_wchars,const EVERYTHING3_UTF8 *s,SIZE_T len_in_bytes);
static SIZE_T _everything3_safe_utf8_string_copy_utf8_string_n(EVERYTHING3_UTF8 *buf,SIZE_T bufsize,const EVERYTHING3_UTF8 *s,SIZE_T len_in_bytes);
static SIZE_T _everything3_safe_ansi_string_copy_utf8_string_n(EVERYTHING3_CHAR *buf,SIZE_T bufsize,const EVERYTHING3_UTF8 *s,SIZE_T len_in_bytes);
static BOOL _everything3_get_property_name(EVERYTHING3_CLIENT *client,DWORD property_id,_everything3_utf8_buf_t *cbuf);
static BOOL _everything3_get_property_canonical_name(EVERYTHING3_CLIENT *client,DWORD property_id,_everything3_utf8_buf_t *cbuf);
static BOOL _everything3_get_item_property_text(const EVERYTHING3_RESULT_LIST *result_list,SIZE_T result_index,DWORD property_id,BOOL highlight,BOOL format,_everything3_utf8_pstring_t **pstring);
static SIZE_T _everything3_get_item_property_text_utf8(const EVERYTHING3_RESULT_LIST *result_list,SIZE_T result_index,DWORD property_id,BOOL highlight,BOOL format,EVERYTHING3_UTF8 *buf,EVERYTHING3_SIZE_T bufsize);
static SIZE_T _everything3_get_item_property_text_wchar(const EVERYTHING3_RESULT_LIST *result_list,SIZE_T result_index,DWORD property_id,BOOL highlight,BOOL format,EVERYTHING3_WCHAR *wbuf,SIZE_T wbuf_size_in_wchars);
static SIZE_T _everything3_get_item_property_text_ansi(const EVERYTHING3_RESULT_LIST *result_list,SIZE_T result_index,DWORD property_id,BOOL highlight,BOOL format,EVERYTHING3_CHAR *buf,EVERYTHING3_SIZE_T bufsize);
static DWORD _everything3_get_run_count(EVERYTHING3_CLIENT *client,const EVERYTHING3_UTF8 *filename,SIZE_T filename_length_in_bytes);
static BOOL _everything3_set_run_count(EVERYTHING3_CLIENT *client,const EVERYTHING3_UTF8 *filename,SIZE_T filename_length_in_bytes,DWORD run_count);
static DWORD _everything3_inc_run_count(EVERYTHING3_CLIENT *client,const EVERYTHING3_UTF8 *filename,SIZE_T filename_length_in_bytes);
static BOOL _everything3_add_search_property_request(EVERYTHING3_SEARCH_STATE *search_state,DWORD property_id,BOOL format,BOOL highlight);
static EVERYTHING3_DWORD _everything3_get_file_attributes(EVERYTHING3_CLIENT *client,const EVERYTHING3_UTF8 *filename,SIZE_T filename_length_in_bytes);
static BOOL _everything3_get_file_attributes_ex(EVERYTHING3_CLIENT *client,const EVERYTHING3_UTF8 *filename,SIZE_T filename_length_in_bytes,_everything3_utf8_buf_t *data_cbuf);
static EVERYTHING3_FIND_HANDLE *_everything3_find_first_file(EVERYTHING3_CLIENT *client,const EVERYTHING3_UTF8 *filename,SIZE_T filename_length_in_bytes);
static void _everything3_find_handle_chunk_free(_everything3_find_handle_chunk_t *chunk);
static void _everything3_find_handle_chunk_read_data(EVERYTHING3_FIND_HANDLE *find_handle,void *buf,uintptr_t size);
static BYTE _everything3_find_handle_chunk_read_byte(EVERYTHING3_FIND_HANDLE *find_handle);
static WORD _everything3_find_handle_chunk_read_word(EVERYTHING3_FIND_HANDLE *find_handle);
static DWORD _everything3_find_handle_chunk_read_dword(EVERYTHING3_FIND_HANDLE *find_handle);
static EVERYTHING3_UINT64 _everything3_find_handle_chunk_read_uint64(EVERYTHING3_FIND_HANDLE *find_handle);
static EVERYTHING3_RESULT_LIST *_everything3_search_with_extra_flags(EVERYTHING3_CLIENT *client,EVERYTHING3_SEARCH_STATE *search_state,DWORD command,DWORD extra_flags);

// just a wrapper for GetLastError();
EVERYTHING3_USERAPI DWORD EVERYTHING3_API Everything3_GetLastError(void)
{
	return GetLastError();
}

// cat a wide string to another wide string.
// call with a NULL buf to calculate the size.
// returns required length in wide chars. (not bytes)
static EVERYTHING3_WCHAR *_everything3_wchar_string_cat_wchar_string_no_null_terminate(EVERYTHING3_WCHAR *buf,EVERYTHING3_WCHAR *current_d,const EVERYTHING3_WCHAR *s)
{
	const EVERYTHING3_WCHAR *p;
	EVERYTHING3_WCHAR *d;

	p = s;
	d = current_d;
	
	while(*p)
	{
		if (buf)
		{
			*d++ = *p;
		}
		else
		{
			d = (void *)_everything3_safe_size_add((SIZE_T)d,1);
		}
	
		p++;
	}
	
	return d;
}

// get pipe name.
// instance_name can be NULL.
static EVERYTHING3_WCHAR *_Everything3_get_pipe_name(EVERYTHING3_WCHAR *buf,const EVERYTHING3_WCHAR *instance_name)
{
	EVERYTHING3_WCHAR *d;
	
	d = buf;
	
	d = _everything3_wchar_string_cat_wchar_string_no_null_terminate(buf,d,L"\\\\.\\PIPE\\Everything IPC");

	if ((instance_name) && (*instance_name))
	{
	// TODO: we need to escape special characters (URL escape)
		d = _everything3_wchar_string_cat_wchar_string_no_null_terminate(buf,d,L" (");
		d = _everything3_wchar_string_cat_wchar_string_no_null_terminate(buf,d,instance_name);
		d = _everything3_wchar_string_cat_wchar_string_no_null_terminate(buf,d,L")");
	}

	if (buf)	
	{
		*d = 0;
	}
	
	return d;
}

// instance_name must be non-NULL.
static BOOL _everything3_wchar_buf_get_pipe_name(_everything3_wchar_buf_t *wcbuf,const EVERYTHING3_WCHAR *instance_name)
{
	if (_everything3_wchar_buf_grow_length(wcbuf,(SIZE_T)_Everything3_get_pipe_name(NULL,instance_name)))
	{
		_Everything3_get_pipe_name(wcbuf->buf,instance_name);
		
		return TRUE;
	}
	
	return FALSE;
}

// Connect to Everything
// connects to the named pipe "\\\\.\\PIPE\\Everything IPC"
// Everything will try to host a few pipe servers so we should be able to connect immediately.
// keep polling for a connection if this fails with EVERYTHING3_ERROR_IPC_PIPE_NOT_FOUND.
// instance_name can BE NULL.
// a NULL instance_name or an empty instance_name will connect to the unnamed instance.
// The Everything 1.5a release will use an "1.5a" instance.
EVERYTHING3_USERAPI EVERYTHING3_CLIENT *EVERYTHING3_API Everything3_ConnectUTF8(const EVERYTHING3_UTF8 *instance_name)
{
	EVERYTHING3_CLIENT *ret;
	
	ret = NULL;
	
	if (instance_name)
	{
		_everything3_wchar_buf_t wcbuf;

		_everything3_wchar_buf_init(&wcbuf);

		if (_everything3_wchar_buf_copy_utf8_string_n(&wcbuf,instance_name,_everything3_utf8_string_get_length_in_bytes(instance_name)))
		{
			ret = Everything3_ConnectW(wcbuf.buf);
		}

		_everything3_wchar_buf_kill(&wcbuf);
	}
	else
	{
		ret = Everything3_ConnectW(NULL);
	}
	
	return ret;
}

// Connect to Everything
// connects to the named pipe "\\\\.\\PIPE\\Everything IPC (instance-name)"
// connects to the named pipe "\\\\.\\PIPE\\Everything IPC" when no instance name is supplied.
// Everything will try to host a few pipe servers so we should be able to connect immediately.
// keep polling for a connection if this fails with EVERYTHING3_ERROR_IPC_PIPE_NOT_FOUND.
// instance_name can BE NULL.
// a NULL instance_name or an empty instance_name will connect to the unnamed instance.
// The Everything 1.5a release will use an "1.5a" instance.
EVERYTHING3_USERAPI _everything3_client_t *EVERYTHING3_API Everything3_ConnectW(const EVERYTHING3_WCHAR *instance_name)
{
	_everything3_client_t *ret;
	_everything3_wchar_buf_t pipe_name_wcbuf;

	ret = NULL;
	_everything3_wchar_buf_init(&pipe_name_wcbuf);

	if (_everything3_wchar_buf_get_pipe_name(&pipe_name_wcbuf,instance_name))
	{
		HANDLE pipe_handle;
		
		pipe_handle = CreateFileW(pipe_name_wcbuf.buf,GENERIC_READ|GENERIC_WRITE,0,0,OPEN_EXISTING,FILE_FLAG_OVERLAPPED,0);
		if (pipe_handle != INVALID_HANDLE_VALUE)
		{
			// connected..
			_everything3_client_t *client;
			
			client = _everything3_mem_calloc(sizeof(_everything3_client_t));
			
			if (client)
			{
				InitializeCriticalSection(&client->cs);

				client->pipe_handle = pipe_handle;
								
				// client owns handler now.
				pipe_handle = INVALID_HANDLE_VALUE;
				
				client->shutdown_event = CreateEvent(0,TRUE,FALSE,0);
				if (client->shutdown_event)
				{
					client->send_event = CreateEvent(0,TRUE,FALSE,0);
					if (client->send_event)
					{
						client->recv_event = CreateEvent(0,TRUE,FALSE,0);
						if (client->recv_event)
						{
							ret = client;
							client = NULL;
						}
						else
						{
							SetLastError(EVERYTHING3_ERROR_OUT_OF_MEMORY);
						}
					}
					else
					{
						SetLastError(EVERYTHING3_ERROR_OUT_OF_MEMORY);
					}
				}
				else
				{
					SetLastError(EVERYTHING3_ERROR_OUT_OF_MEMORY);
				}
				
				if (client)
				{
					Everything3_DestroyClient(client);
				}
			}
			else
			{
				SetLastError(EVERYTHING3_ERROR_OUT_OF_MEMORY);
			}
			
			if (pipe_handle != INVALID_HANDLE_VALUE)
			{
				CloseHandle(pipe_handle);
			}
		
		}
		else
		{
			_everything3_debug_printf("CreateFile failed %u\n",GetLastError());
		
			SetLastError(EVERYTHING3_ERROR_IPC_PIPE_NOT_FOUND);
		}
	}
	else
	{
		SetLastError(EVERYTHING3_ERROR_OUT_OF_MEMORY);
	}
	
	_everything3_wchar_buf_kill(&pipe_name_wcbuf);

	return ret;
}

// copy an ANSI string into a wchar buffer.
static BOOL _everything3_wchar_buf_copy_ansi_string(_everything3_wchar_buf_t *wcbuf,const EVERYTHING3_CHAR *s)
{
	SIZE_T s_length_in_bytes;
	int wlen;
	
	s_length_in_bytes = _everything3_utf8_string_get_length_in_bytes(s);
	
	if (s_length_in_bytes <= EVERYTHING3_DWORD_MAX)
	{
		wlen = MultiByteToWideChar(CP_ACP,0,s,(DWORD)s_length_in_bytes,NULL,0);
		if (wlen >= 0)
		{
			if (_everything3_wchar_buf_grow_length(wcbuf,wlen))
			{
				MultiByteToWideChar(CP_ACP,0,s,(DWORD)s_length_in_bytes,wcbuf->buf,wlen);
				
				wcbuf->buf[wcbuf->length_in_wchars] = 0;
				
				return TRUE;
			}
		}
	}
	else
	{
		SetLastError(EVERYTHING3_ERROR_OUT_OF_MEMORY);
	}
	
	return FALSE;
}

// copy a utf8 string into a wchar buffer.
static BOOL _everything3_wchar_buf_copy_utf8_string_n(_everything3_wchar_buf_t *wcbuf,const EVERYTHING3_UTF8 *s,SIZE_T slen)
{
	SIZE_T size_in_wchars;
	
	size_in_wchars = _everything3_safe_wchar_string_copy_utf8_string_n(NULL,0,s,slen);
	
	if (_everything3_wchar_buf_grow_size(wcbuf,size_in_wchars))
	{
		wcbuf->length_in_wchars = _everything3_safe_wchar_string_copy_utf8_string_n(wcbuf->buf,wcbuf->size_in_wchars,s,slen);
		
		return TRUE;
	}
	
	return FALSE;
}


// Connect to Everything
// connects to the named pipe "\\\\.\\PIPE\\Everything IPC"
// Everything will try to host a few pipe servers so we should be able to connect immediately.
// keep polling for a connection if this fails with EVERYTHING3_ERROR_IPC_PIPE_NOT_FOUND.
// instance_name can BE NULL.
// a NULL instance_name or an empty instance_name will connect to the unnamed instance.
// The Everything 1.5a release will use an "1.5a" instance.
EVERYTHING3_USERAPI _everything3_client_t *EVERYTHING3_API Everything3_ConnectA(const EVERYTHING3_CHAR *instance_name)
{
	_everything3_client_t *ret;
	
	ret = NULL;
	
	if (instance_name)
	{
		_everything3_client_t *ret;
		_everything3_wchar_buf_t instance_name_wcbuf;
		
		ret = NULL;
		_everything3_wchar_buf_init(&instance_name_wcbuf);

		if (_everything3_wchar_buf_copy_ansi_string(&instance_name_wcbuf,instance_name))
		{
			ret = Everything3_ConnectW(instance_name_wcbuf.buf);
		}
		
		_everything3_wchar_buf_kill(&instance_name_wcbuf);
	}
	else
	{
		ret = Everything3_ConnectW(NULL);
	}
		
	return ret;
}

// can be called from any thread.
// Cancels any pending search.
// Any future searches will fail.
EVERYTHING3_USERAPI BOOL EVERYTHING3_API Everything3_ShutdownClient(_everything3_client_t *client)
{
	if (client)
	{
		// no need to enter CS.
		SetEvent(client->shutdown_event);
		
		return TRUE;
	}

	SetLastError(EVERYTHING3_ERROR_INVALID_PARAMETER);
	
	return FALSE;
}

// Disconnect from the IPC pipe.
// Return resources to the system.
EVERYTHING3_USERAPI BOOL EVERYTHING3_API Everything3_DestroyClient(_everything3_client_t *client)
{
	if (client)
	{
		Everything3_ShutdownClient(client);
		
		if (client->recv_event)
		{	
			CloseHandle(client->recv_event);
		}
		
		if (client->send_event)
		{	
			CloseHandle(client->send_event);
		}
		
		if (client->shutdown_event)
		{	
			CloseHandle(client->shutdown_event);
		}
		
		if (client->pipe_handle != INVALID_HANDLE_VALUE)
		{
			CloseHandle(client->pipe_handle);
			
			client->pipe_handle = INVALID_HANDLE_VALUE;
		}
		
		DeleteCriticalSection(&client->cs);
		
		_everything3_mem_free(client);
		
		return TRUE;
	}
	else
	{
		SetLastError(EVERYTHING3_ERROR_INVALID_PARAMETER);
		
		return FALSE;
	}
}

// Allocate some memory.
// can return NULL.
static void *_everything3_mem_alloc(SIZE_T size)
{
	void *p;
	
	// ensure SIZE_MAX allocations fail.
	if (size == SIZE_MAX)
	{
		return NULL;
	}
	
#ifdef _EVERYTHING3_DEBUG
{
	BYTE *p;
	SIZE_T allocsize;
	
	allocsize = 4096 + ((size + (4096-1)) & ~(uintptr_t)(4096-1)) + 4096;

	p = VirtualAlloc(0,allocsize,MEM_RESERVE,PAGE_NOACCESS);
	
	if (!p)
	{
		DebugBreak();
	}

	p += 4096;
	
	if (!VirtualAlloc(p,size,MEM_COMMIT,PAGE_READWRITE))
	{
		DebugBreak();
	}

	// move forward a little bit, so our last byte is the last byte in a page.
	// this gap will be UNPROTECTED, (use MEM_DEBUG 4)
	if ((size & (4096-1)))
	{
		p += (4096 - (size & (4096-1)));
	}
	
	{
		MEMORY_BASIC_INFORMATION mbi;
		
		// dont free
		if (!VirtualQuery(p,&mbi,sizeof(MEMORY_BASIC_INFORMATION)))
		{
			DebugBreak();		
		}
		
		_everything3_mem_allocated += mbi.RegionSize;
	}
			
	return p;
}
#endif

	p = HeapAlloc(GetProcessHeap(),0,size);

	if (p)
	{

	}
	else
	{
		SetLastError(EVERYTHING3_ERROR_OUT_OF_MEMORY);
	}
	
	return p;
}

// alloc some memory and clear the memory.
// can return NULL.
static void *_everything3_mem_calloc(SIZE_T size)
{
	void *p;
	
	p = _everything3_mem_alloc(size);
	if (p)
	{
		_everything3_zero_memory(p,size);
	}
	
	return p;
}

// Free some memory back to the system.
static void _everything3_mem_free(void *ptr)
{
#ifdef _EVERYTHING3_DEBUG
{
	MEMORY_BASIC_INFORMATION mbi;
	DWORD oldProtect;
	
	if (!ptr)
	{
		DebugBreak();		
	}
	
	// dont free
	if (!VirtualQuery(ptr,&mbi,sizeof(MEMORY_BASIC_INFORMATION)))
	{
		DebugBreak();		
	}

	_everything3_mem_allocated -= mbi.RegionSize;
		
	if (!VirtualProtect(mbi.BaseAddress,mbi.RegionSize,PAGE_NOACCESS,&oldProtect))
	{
		DebugBreak();		
	}
	
	return;
}
#endif

	HeapFree(GetProcessHeap(),0,ptr);
}

// safely add some values.
// SIZE_MAX is used as an invalid value.
// you will be unable to allocate SIZE_MAX bytes.
// always use _everything3_safe_size when allocating memory.
static SIZE_T _everything3_safe_size_add(SIZE_T a,SIZE_T b)
{
	SIZE_T c;
	
	c = a + b;
	
	if (c < a) 
	{
		return SIZE_MAX;
	}
	
	return c;
}

// safely multiple a value by the size of a pointer.
// SIZE_MAX is used as an invalid value.
static SIZE_T _everything3_safe_size_mul_size_of_pointer(SIZE_T a)
{
	SIZE_T c;
	
	c = _everything3_safe_size_add(a,a); // x2
	c = _everything3_safe_size_add(c,c); // x4

#if SIZE_MAX == 0xFFFFFFFFFFFFFFFFUI64

	c = _everything3_safe_size_add(c,c); // x8

#elif SIZE_MAX == 0xFFFFFFFF

#else

	#error unknown SIZE_MAX
		
#endif

	return c;	
}

// lock the client
// this will prevent reentry calls from other threads.
static void _everything3_Lock(_everything3_client_t *client)
{
	EnterCriticalSection(&client->cs);
}

// unlock the client
static void _everything3_Unlock(_everything3_client_t *client)
{
	LeaveCriticalSection(&client->cs);
}

// avoid other libs
static SIZE_T _everything3_utf8_string_get_length_in_bytes(const EVERYTHING3_UTF8 *s)
{
	const EVERYTHING3_UTF8 *p;
	
	p = s;
	
	while(*p)
	{
		p++;
	}
	
	return p - s;
}

// initialize a wchar buffer.
// the default value is an empty string.
static void _everything3_wchar_buf_init(_everything3_wchar_buf_t *wcbuf)
{
	wcbuf->buf = wcbuf->stack_buf;
	wcbuf->length_in_wchars = 0;
	wcbuf->size_in_wchars = _EVERYTHING3_WCHAR_BUF_STACK_SIZE;
	wcbuf->buf[0] = 0;
}

// kill a wchar buffer.
// returns any allocated memory back to the system.
static void _everything3_wchar_buf_kill(_everything3_wchar_buf_t *wcbuf)
{
	if (wcbuf->buf != wcbuf->stack_buf)
	{
		_everything3_mem_free(wcbuf->buf);
	}
}

// empty a wchar buffer.
// the wchar buffer is set to an empty string.
static void _everything3_wchar_buf_empty(_everything3_wchar_buf_t *wcbuf)
{
	_everything3_wchar_buf_kill(wcbuf);
	_everything3_wchar_buf_init(wcbuf);
}

// doesn't keep the existing text.
// doesn't set the text.
// doesn't set length.
// caller should set the text.
// returns FALSE on error. Call GetLastError() for more information.
// returns TRUE if successful.
static BOOL _everything3_wchar_buf_grow_size(_everything3_wchar_buf_t *wcbuf,SIZE_T size_in_wchars)
{
	EVERYTHING3_WCHAR *new_buf;
	
	if (size_in_wchars <= wcbuf->size_in_wchars)
	{
		return TRUE;
	}
	
	_everything3_wchar_buf_empty(wcbuf);

	new_buf = _everything3_mem_alloc(_everything3_safe_size_add(size_in_wchars,size_in_wchars));
	
	if (new_buf)
	{
		wcbuf->buf = new_buf;
		wcbuf->size_in_wchars = size_in_wchars;
		
		return TRUE;
	}
	
	return FALSE;
}

// doesn't keep the existing text.
// doesn't set the text, only sets the length.
// DOES set the length.
// caller should set the text.
// returns FALSE on error. Call GetLastError() for more information.
// returns TRUE if successful.
static BOOL _everything3_wchar_buf_grow_length(_everything3_wchar_buf_t *wcbuf,SIZE_T length_in_wchars)
{
	if (_everything3_wchar_buf_grow_size(wcbuf,_everything3_safe_size_add(length_in_wchars,1)))
	{
		wcbuf->length_in_wchars = length_in_wchars;

		return TRUE;
	}
	
	return FALSE;
}

// send some data to the IPC pipe.
// returns TRUE if successful.
// returns FALSE on error. Call GetLastError() for more information.
static BOOL _everything3_send(_everything3_client_t *client,DWORD code,const void *in_data,SIZE_T in_size)
{
	// make sure the in_size is sane.
	if (in_size <= EVERYTHING3_DWORD_MAX)
	{
		_everything3_message_t send_message;
		DWORD numwritten;
		OVERLAPPED send_overlapped;
		BYTE *send_p;
		DWORD send_run;
		
		send_message.code = code;
		send_message.size = (DWORD)in_size;
		
		send_overlapped.hEvent = client->send_event;
		send_overlapped.Offset = 0;
		send_overlapped.OffsetHigh = 0;
		
		send_p = (BYTE *)&send_message;
		send_run = sizeof(send_message);

		while(send_run)
		{
			if (WriteFile(client->pipe_handle,send_p,send_run,&numwritten,&send_overlapped))
			{
				if (numwritten)
				{
					send_p += numwritten;
					send_run -= numwritten;

					continue;
				}
				else
				{
					// eof
					SetLastError(EVERYTHING3_ERROR_DISCONNECTED);

					return FALSE;
				}
			}
			else
			{
				DWORD last_error;
				
				last_error = GetLastError();
				
				if ((last_error == ERROR_IO_INCOMPLETE) || (last_error == ERROR_IO_PENDING))
				{
					HANDLE wait_handles[2];
					
					wait_handles[0] = client->shutdown_event;
					wait_handles[1] = client->send_event;
					
					if (WaitForMultipleObjects(2,wait_handles,FALSE,INFINITE) == WAIT_OBJECT_0)
					{
						SetLastError(EVERYTHING3_ERROR_SHUTDOWN);
						
						return FALSE;
					}
					
					// still writing..
					if (GetOverlappedResult(client->pipe_handle,&send_overlapped,&numwritten,FALSE))
					{
						if (numwritten)
						{
							send_p += numwritten;
							send_run -= numwritten;

							continue;
						}
						else
						{
							// eof
							SetLastError(EVERYTHING3_ERROR_DISCONNECTED);

							return FALSE;
						}
					}
					else
					{
						_everything3_debug_printf("send GetOverlappedResult error: %p: %u\n",client->pipe_handle,GetLastError());

						SetLastError(EVERYTHING3_ERROR_DISCONNECTED);
						
						return FALSE;
					}
				}
				else
				{
					_everything3_debug_printf("ipc pipe overlapped send error %p: %u\n",client->pipe_handle,last_error);
					
					SetLastError(EVERYTHING3_ERROR_DISCONNECTED);

					return FALSE;
				}
			}
		}
		
		send_p = (BYTE *)in_data;
		send_run = (DWORD)in_size;

		while(send_run)
		{
			if (WriteFile(client->pipe_handle,send_p,send_run,&numwritten,&send_overlapped))
			{
				send_p += numwritten;
				send_run -= numwritten;
				
				// continue..
				continue;
			}
			else
			{
				DWORD last_error;
				
				last_error = GetLastError();
				
				if ((last_error == ERROR_IO_INCOMPLETE) || (last_error == ERROR_IO_PENDING))
				{	
					HANDLE wait_handles[2];
					
					wait_handles[0] = client->shutdown_event;
					wait_handles[1] = client->send_event;
					
					if (WaitForMultipleObjects(2,wait_handles,FALSE,INFINITE) == WAIT_OBJECT_0)
					{
						SetLastError(EVERYTHING3_ERROR_SHUTDOWN);
						
						return FALSE;
					}
					
					// still writing..
					if (GetOverlappedResult(client->pipe_handle,&send_overlapped,&numwritten,FALSE))
					{
						send_p += numwritten;
						send_run -= numwritten;
				
						continue;
					}
					else
					{
						_everything3_debug_printf("send GetOverlappedResult error: %p: %u\n",client->pipe_handle,GetLastError());

						SetLastError(EVERYTHING3_ERROR_DISCONNECTED);
						
						return FALSE;
					}
				}
				else
				{
					_everything3_debug_printf("ipc pipe overlapped send error %p: %u\n",client->pipe_handle,last_error);
					
					SetLastError(EVERYTHING3_ERROR_DISCONNECTED);

					return FALSE;
				}
			}
		}
		
		return TRUE;
	}
	else
	{
		SetLastError(EVERYTHING3_ERROR_OUT_OF_MEMORY);

		return FALSE;
	}
}

// Receive the message header from the IPC pipe.
// returns TRUE if successful.
// returns FALSE on error. Call GetLastError() for more information.
static BOOL _everything3_recv_header(_everything3_client_t *client,_everything3_message_t *recv_header)
{
	BOOL ret;
	
	ret = FALSE;
	
	if (_everything3_recv_data(client,recv_header,sizeof(_everything3_message_t)))
	{
		if ((recv_header->code == _EVERYTHING3_RESPONSE_OK) || (recv_header->code == _EVERYTHING3_RESPONSE_OK_MORE_DATA))
		{
			ret = TRUE;
		}
		else
		{
			// skip data
			if (_everything3_recv_skip(client,recv_header->size))
			{
				if (recv_header->code == _EVERYTHING3_RESPONSE_ERROR_BAD_REQUEST)
				{
					SetLastError(EVERYTHING3_ERROR_BAD_REQUEST);
				}
				else
				if (recv_header->code == _EVERYTHING3_RESPONSE_ERROR_CANCELLED)
				{
					SetLastError(EVERYTHING3_ERROR_CANCELLED);
				}
				else
				if (recv_header->code == _EVERYTHING3_RESPONSE_ERROR_NOT_FOUND)
				{
					SetLastError(EVERYTHING3_ERROR_IPC_PIPE_NOT_FOUND);
				}
				else
				if (recv_header->code == _EVERYTHING3_RESPONSE_ERROR_OUT_OF_MEMORY)
				{
					SetLastError(EVERYTHING3_ERROR_SERVER);
				}
				else
				if (recv_header->code == _EVERYTHING3_RESPONSE_ERROR_INVALID_COMMAND)
				{
					SetLastError(EVERYTHING3_ERROR_INVALID_COMMAND);
				}
				else
				{
					SetLastError(EVERYTHING3_ERROR_BAD_RESPONSE);
				}
			}
		}
	}
	
	return ret;
}

// Receive some data from the IPC pipe.
// returns TRUE if successful.
// returns FALSE on error. Call GetLastError() for more information.
static BOOL _everything3_recv_data(_everything3_client_t *client,void *buf,SIZE_T buf_size)
{
	BOOL ret;
	DWORD numread;
	OVERLAPPED recv_overlapped;
	BYTE *recv_p;
	SIZE_T recv_run;
	
	ret = FALSE;
	recv_overlapped.hEvent = client->recv_event;
	recv_overlapped.Offset = 0;
	recv_overlapped.OffsetHigh = 0;
	
	recv_p = (BYTE *)buf;
	recv_run = buf_size;

	for(;;)
	{
		DWORD chunk_size;
		
		if (!recv_run)
		{
			ret = TRUE;

			break;
		}
		
		if (recv_run <= 65536)
		{
			chunk_size = (DWORD)recv_run;
		}
		else
		{
			chunk_size = 65536;
		}
		
		if (ReadFile(client->pipe_handle,recv_p,chunk_size,&numread,&recv_overlapped))
		{
			if (numread)
			{
				recv_p += numread;
				recv_run -= numread;
				
				// continue..
			}
			else
			{
				// eof
				SetLastError(EVERYTHING3_ERROR_DISCONNECTED);

				break;
			}
		}
		else
		{
			DWORD last_error;
			
			last_error = GetLastError();
			
			if ((last_error == ERROR_IO_INCOMPLETE) || (last_error == ERROR_IO_PENDING))
			{
				HANDLE wait_handles[2];
				
				wait_handles[0] = client->shutdown_event;
				wait_handles[1] = client->recv_event;
				
				if (WaitForMultipleObjects(2,wait_handles,FALSE,INFINITE) == WAIT_OBJECT_0)
				{
					SetLastError(EVERYTHING3_ERROR_SHUTDOWN);
					
					break;
				}
				
				// still reading..
				if (GetOverlappedResult(client->pipe_handle,&recv_overlapped,&numread,FALSE))
				{
					if (numread)
					{
						recv_p += numread;
						recv_run -= numread;
						
						// continue...
					}
					else
					{
						// eof
						SetLastError(EVERYTHING3_ERROR_DISCONNECTED);

						break;
					}
				}
				else
				{
					_everything3_debug_printf("recv GetOverlappedResult error: %p: %u\n",client->pipe_handle,GetLastError());

					SetLastError(EVERYTHING3_ERROR_DISCONNECTED);
					
					break;
				}
			}
			else
			{
				_everything3_debug_printf("ipc pipe overlapped recv error %p: %u\n",client->pipe_handle,last_error);
				
				SetLastError(EVERYTHING3_ERROR_DISCONNECTED);

				break;
			}
		}
	}
	
	return ret;
}

// Receive some data from the IPC pipe.
// returns TRUE if successful.
// returns FALSE on error. Call GetLastError() for more information.
static BOOL _everything3_recv_skip(_everything3_client_t *client,SIZE_T size)
{
	BOOL ret;
	BYTE buf[256];
	SIZE_T run;
	
	ret = FALSE;
	run = size;
	
	for(;;)
	{
		SIZE_T recv_size;
		
		if (!run)
		{
			ret = TRUE;
			break;
		}
		
		recv_size = run;
		if (recv_size > 256)
		{
			recv_size = 256;
		}
		
		if (!_everything3_recv_data(client,buf,recv_size))
		{
			break;
		}
		
		run -= recv_size;
	}
	
	return ret;
}


// send the IPC pipe a command and wait for a response.
// returns TRUE if successful.
// returns FALSE on error. Call GetLastError() for more information.
static BOOL _everything3_ioctrl(_everything3_client_t *client,DWORD code,const void *in_data,SIZE_T in_size,void *out_data,SIZE_T out_size,SIZE_T *out_numread)
{
	BOOL ret;
	
	ret = FALSE;

	if (client)
	{
		_everything3_Lock(client);
		
		if (_everything3_send(client,code,in_data,in_size))
		{
			_everything3_message_t recv_header;
			
			if (_everything3_recv_header(client,&recv_header))
			{
				if (recv_header.size <= out_size)
				{
					if (_everything3_recv_data(client,out_data,recv_header.size))
					{
						if (out_numread)
						{
							*out_numread = recv_header.size;
						}
						
						ret = TRUE;
					}
				}
				else
				{
					if (_everything3_recv_skip(client,recv_header.size))
					{
						// not enough room to store response data.
						SetLastError(EVERYTHING3_ERROR_INSUFFICIENT_BUFFER);
					}
				}
			}
		}		
		
		_everything3_Unlock(client);
	}
	else
	{
		SetLastError(EVERYTHING3_ERROR_INVALID_PARAMETER);
	}

	return ret;
}

// send the IPC pipe a command and wait for a string response.
// returns TRUE if successful.
// returns FALSE on error. Call GetLastError() for more information.
static BOOL _everything3_ioctrl_get_string(_everything3_client_t *client,DWORD code,const void *in_data,SIZE_T in_size,_everything3_utf8_buf_t *cbuf)
{
	BOOL ret;
	
	ret = FALSE;

	if (client)
	{
		_everything3_Lock(client);
		
		if (_everything3_send(client,code,in_data,in_size))
		{
			_everything3_message_t recv_header;
			
			if (_everything3_recv_header(client,&recv_header))
			{
				if (_everything3_utf8_buf_grow_length(cbuf,recv_header.size))
				{
					if (_everything3_recv_data(client,cbuf->buf,recv_header.size))
					{
						// NULL terminate.
						cbuf->buf[recv_header.size] = 0;
						
						ret = TRUE;
					}
				}
				else
				{
					if (_everything3_recv_skip(client,recv_header.size))
					{
						// not enough room to store response data.
						SetLastError(EVERYTHING3_ERROR_OUT_OF_MEMORY);
					}
				}
			}
		}		
		
		_everything3_Unlock(client);
	}
	else
	{
		SetLastError(EVERYTHING3_ERROR_INVALID_PARAMETER);
	}

	return ret;
}

// returns non-zero if successful.
// returns 0 on error. Call GetLastError() for more information.
EVERYTHING3_USERAPI DWORD EVERYTHING3_API Everything3_GetIPCPipeVersion(_everything3_client_t *client)
{
	DWORD ret;
	DWORD value;
	SIZE_T numread;
			
	ret = 0;
		
	if (_everything3_ioctrl(client,_EVERYTHING3_COMMAND_GET_IPC_PIPE_VERSION,NULL,0,&value,sizeof(DWORD),&numread))
	{
		if (numread == sizeof(DWORD))
		{
			ret = value;

			if (!ret)
			{
				SetLastError(EVERYTHING3_OK);
			}
		}
		else
		{
			SetLastError(EVERYTHING3_ERROR_BAD_RESPONSE);
		}
	}

	return ret;
}

// returns the Everything major.minor.revision.build major-version
// returns non-zero if successful.
// returns 0 on error. Call GetLastError() for more information.
EVERYTHING3_USERAPI DWORD EVERYTHING3_API Everything3_GetMajorVersion(EVERYTHING3_CLIENT *client)
{
	DWORD ret;
	DWORD value;
	SIZE_T numread;
	
	ret = 0;
		
	if (_everything3_ioctrl(client,_EVERYTHING3_COMMAND_GET_MAJOR_VERSION,NULL,0,&value,sizeof(DWORD),&numread))
	{
		if (numread == sizeof(DWORD))
		{
			ret = value;

			if (!ret)
			{
				SetLastError(EVERYTHING3_OK);
			}
		}
		else
		{
			SetLastError(EVERYTHING3_ERROR_BAD_RESPONSE);
		}
	}

	return ret;
}

// returns the Everything major.minor.revision.build minor-version
// returns non-zero if successful.
// returns 0 on error. Call GetLastError() for more information.
EVERYTHING3_USERAPI DWORD EVERYTHING3_API Everything3_GetMinorVersion(EVERYTHING3_CLIENT *client)
{
	DWORD ret;
	DWORD value;
	SIZE_T numread;
	
	ret = 0;
		
	if (_everything3_ioctrl(client,_EVERYTHING3_COMMAND_GET_MINOR_VERSION,NULL,0,&value,sizeof(DWORD),&numread))
	{
		if (numread == sizeof(DWORD))
		{
			ret = value;

			if (!ret)
			{
				SetLastError(EVERYTHING3_OK);
			}
		}
		else
		{
			SetLastError(EVERYTHING3_ERROR_BAD_RESPONSE);
		}
	}

	return ret;
}

// returns the Everything major.minor.revision.build revision-version
// returns non-zero if successful.
// returns 0 on error. Call GetLastError() for more information.
EVERYTHING3_USERAPI DWORD EVERYTHING3_API Everything3_GetRevision(EVERYTHING3_CLIENT *client)
{
	DWORD ret;
	DWORD value;
	SIZE_T numread;
	
	ret = 0;
		
	if (_everything3_ioctrl(client,_EVERYTHING3_COMMAND_GET_REVISION,NULL,0,&value,sizeof(DWORD),&numread))
	{
		if (numread == sizeof(DWORD))
		{
			ret = value;

			if (!ret)
			{
				SetLastError(EVERYTHING3_OK);
			}
		}
		else
		{
			SetLastError(EVERYTHING3_ERROR_BAD_RESPONSE);
		}
	}
	
	return ret;
}

// returns the Everything major.minor.revision.build build-version
// returns non-zero if successful.
// returns 0 on error. Call GetLastError() for more information.
EVERYTHING3_USERAPI DWORD EVERYTHING3_API Everything3_GetBuildNumber(EVERYTHING3_CLIENT *client)
{
	DWORD ret;
	DWORD value;
	SIZE_T numread;
	
	ret = 0;
		
	if (_everything3_ioctrl(client,_EVERYTHING3_COMMAND_GET_BUILD_NUMBER,NULL,0,&value,sizeof(DWORD),&numread))
	{
		if (numread == sizeof(DWORD))
		{
			ret = value;
							
			if (!ret)
			{
				SetLastError(EVERYTHING3_OK);
			}
		}
		else
		{
			SetLastError(EVERYTHING3_ERROR_BAD_RESPONSE);
		}
	}

	return ret;
}

// returns one of the EVERYTHING3_TARGET_MACHINE_* values.
// returns non-zero if successful.
// returns EVERYTHING3_TARGET_MACHINE_UNKNOWN on error. Call GetLastError() for more information.
EVERYTHING3_USERAPI DWORD EVERYTHING3_API Everything_GetTargetMachine(EVERYTHING3_CLIENT *client)
{
	DWORD ret;
	DWORD value;
	SIZE_T numread;
	
	ret = EVERYTHING3_TARGET_MACHINE_UNKNOWN;
		
	if (_everything3_ioctrl(client,_EVERYTHING3_COMMAND_GET_TARGET_MACHINE,NULL,0,&value,sizeof(DWORD),&numread))
	{
		if (numread == sizeof(DWORD))
		{
			ret = value;
			
			if (!ret)
			{
				SetLastError(EVERYTHING3_OK);
			}
		}
		else
		{
			SetLastError(EVERYTHING3_ERROR_BAD_RESPONSE);
		}
	}
	
	return ret;
}

// returns TRUE if the database is loaded. Otherwise, returns FALSE.
// returns FALSE on error. Call GetLastError() for more information.
EVERYTHING3_USERAPI BOOL EVERYTHING3_API Everything3_IsDBLoaded(EVERYTHING3_CLIENT *client)
{
	BOOL ret;
	DWORD value;
	SIZE_T numread;
	
	ret = FALSE;
		
	if (_everything3_ioctrl(client,_EVERYTHING3_COMMAND_IS_DB_LOADED,NULL,0,&value,sizeof(DWORD),&numread))
	{
		if (numread == sizeof(DWORD))
		{
			ret = (BOOL)value;
			
			if (!ret)
			{
				SetLastError(EVERYTHING3_OK);
			}
		}
		else
		{
			SetLastError(EVERYTHING3_ERROR_BAD_RESPONSE);
		}
	}

	return ret;
}

// Returns TRUE if the specified property is indexed. Otherwise, returns FALSE.
// Searching will be instant if the property is indexed.
// Retrieving the property value will be instant if the property is indexed.
// Everything3_Search will take a long time to retrieve unindexed property values.
// Returns FALSE on error. Call GetLastError() for more information.
EVERYTHING3_USERAPI BOOL EVERYTHING3_API Everything3_IsPropertyIndexed(EVERYTHING3_CLIENT *client,DWORD property_id)
{
	BOOL ret;
	DWORD value;
	SIZE_T numread;
	
	ret = FALSE;
		
	if (_everything3_ioctrl(client,_EVERYTHING3_COMMAND_IS_PROPERTY_INDEXED,&property_id,sizeof(DWORD),&value,sizeof(DWORD),&numread))
	{
		if (numread == sizeof(DWORD))
		{
			ret = (BOOL)value;
			
			if (!ret)
			{
				SetLastError(EVERYTHING3_OK);
			}
		}
		else
		{
			SetLastError(EVERYTHING3_ERROR_BAD_RESPONSE);
		}
	}

	return ret;
}

// Returns TRUE if the specified property is fast sorted. Otherwise, returns FALSE.
// Sorting will be instant if the property is fast sorted.
// Everything3_Search will take a long time if sorting by a property that is not fast sorted.
// Returns FALSE on error. Call GetLastError() for more information.
EVERYTHING3_USERAPI BOOL EVERYTHING3_API Everything3_IsPropertyFastSort(EVERYTHING3_CLIENT *client,DWORD property_id)
{
	BOOL ret;
	DWORD value;
	SIZE_T numread;
	
	ret = FALSE;
		
	if (_everything3_ioctrl(client,_EVERYTHING3_COMMAND_IS_PROPERTY_FAST_SORT,&property_id,sizeof(DWORD),&value,sizeof(DWORD),&numread))
	{
		if (numread == sizeof(DWORD))
		{
			ret = (BOOL)value;
			
			if (!ret)
			{
				SetLastError(EVERYTHING3_OK);
			}
		}
		else
		{
			SetLastError(EVERYTHING3_ERROR_BAD_RESPONSE);
		}
	}

	return ret;
}

// creates a search state with the default settings.
EVERYTHING3_USERAPI EVERYTHING3_SEARCH_STATE *EVERYTHING3_API Everything3_CreateSearchState(void)
{
	EVERYTHING3_SEARCH_STATE *ret;
	EVERYTHING3_SEARCH_STATE *search_state;
	
	ret = NULL;
	
	search_state = _everything3_mem_calloc(sizeof(EVERYTHING3_SEARCH_STATE));
	if (search_state)
	{
		InitializeCriticalSection(&search_state->cs);
		
		search_state->viewport_count = SIZE_MAX;
					
#if SIZE_MAX == 0xFFFFFFFFFFFFFFFFUI64

		search_state->search_flags |= _EVERYTHING3_SEARCH_FLAG_64BIT;

#elif SIZE_MAX == 0xFFFFFFFF

#else

		#error unknown SIZE_MAX
		
#endif

		ret = search_state;
	}
	
	return ret;
}

// Destroy a search state.
// returns resources back to the system.
// The search_state cannot be used after it is destroyed.
EVERYTHING3_USERAPI BOOL EVERYTHING3_API Everything3_DestroySearchState(EVERYTHING3_SEARCH_STATE *search_state)
{
	BOOL ret;
	
	ret = FALSE;
	
	if (search_state)
	{
		DeleteCriticalSection(&search_state->cs);
		
		if (search_state->sort_array)
		{
			_everything3_mem_free(search_state->sort_array);
		}
		
		if (search_state->property_request_array)
		{
			_everything3_mem_free(search_state->property_request_array);
		}
		
		if (search_state->search_text)
		{
			_everything3_mem_free(search_state->search_text);
		}
		
		_everything3_mem_free(search_state);

		ret = TRUE;
	}
	else
	{
		SetLastError(EVERYTHING3_ERROR_INVALID_PARAMETER);
	}
	
	return ret;
}

// remove and add flags to the search state.
static BOOL _everything3_change_search_flags(EVERYTHING3_SEARCH_STATE *search_state,DWORD remove_flags,DWORD add_flags)
{
	BOOL ret;
	
	ret = FALSE;
	
	if (search_state)
	{
		EnterCriticalSection(&search_state->cs);
		
		search_state->search_flags &= (~remove_flags);
		search_state->search_flags |= add_flags;
		
		LeaveCriticalSection(&search_state->cs);
		
		ret = TRUE;
	}
	else
	{
		SetLastError(EVERYTHING3_ERROR_INVALID_PARAMETER);
	}
	
	return ret;
}

// set or unset a search flag.
static BOOL _everything3_set_search_flag(EVERYTHING3_SEARCH_STATE *search_state,DWORD flag,BOOL set_flag)
{
	return _everything3_change_search_flags(search_state,flag,set_flag ? flag : 0);
}

// get the current search flags.
// returns one of more of _EVERYTHING3_SEARCH_FLAG_*
static DWORD _everything3_get_search_flags(EVERYTHING3_SEARCH_STATE *search_state)
{
	DWORD ret;
	
	ret = 0;
	
	if (search_state)
	{
		EnterCriticalSection(&search_state->cs);
		
		ret = search_state->search_flags;

		LeaveCriticalSection(&search_state->cs);

		SetLastError(EVERYTHING3_OK);
	}
	else
	{
		SetLastError(EVERYTHING3_ERROR_INVALID_PARAMETER);
	}
	
	return ret;
}

// returns TRUE if the flag is set. Otherwise, returns FALSE.
static BOOL _everything3_is_search_flag_set(EVERYTHING3_SEARCH_STATE *search_state,DWORD flag)
{
	DWORD search_flags;
	
	search_flags = _everything3_get_search_flags(search_state);
	
	return (search_flags & flag) ? TRUE : FALSE;
}

EVERYTHING3_USERAPI BOOL EVERYTHING3_API Everything3_SetSearchMatchCase(EVERYTHING3_SEARCH_STATE *search_state,BOOL match_case)
{
	return _everything3_set_search_flag(search_state,_EVERYTHING3_SEARCH_FLAG_MATCH_CASE,match_case);
}

EVERYTHING3_USERAPI BOOL EVERYTHING3_API Everything3_GetSearchMatchCase(EVERYTHING3_SEARCH_STATE *search_state)
{
	return _everything3_is_search_flag_set(search_state,_EVERYTHING3_SEARCH_FLAG_MATCH_CASE);
}

EVERYTHING3_USERAPI BOOL EVERYTHING3_API Everything3_SetSearchMatchDiacritics(EVERYTHING3_SEARCH_STATE *search_state,BOOL match_diacritics)
{
	return _everything3_set_search_flag(search_state,_EVERYTHING3_SEARCH_FLAG_MATCH_DIACRITICS,match_diacritics);
}

EVERYTHING3_USERAPI BOOL EVERYTHING3_API Everything3_GetSearchMatchDiacritics(EVERYTHING3_SEARCH_STATE *search_state)
{
	return _everything3_is_search_flag_set(search_state,_EVERYTHING3_SEARCH_FLAG_MATCH_DIACRITICS);
}

EVERYTHING3_USERAPI BOOL EVERYTHING3_API Everything3_SetSearchMatchWholeWords(EVERYTHING3_SEARCH_STATE *search_state,BOOL match_whole_words)
{
	return _everything3_set_search_flag(search_state,_EVERYTHING3_SEARCH_FLAG_MATCH_WHOLEWORD,match_whole_words);
}

EVERYTHING3_USERAPI BOOL EVERYTHING3_API Everything3_GetSearchMatchWholeWords(EVERYTHING3_SEARCH_STATE *search_state)
{
	return _everything3_is_search_flag_set(search_state,_EVERYTHING3_SEARCH_FLAG_MATCH_WHOLEWORD);
}

EVERYTHING3_USERAPI BOOL EVERYTHING3_API Everything3_SetSearchMatchPath(EVERYTHING3_SEARCH_STATE *search_state,BOOL match_path)
{
	return _everything3_set_search_flag(search_state,_EVERYTHING3_SEARCH_FLAG_MATCH_PATH,match_path);
}

EVERYTHING3_USERAPI BOOL EVERYTHING3_API Everything3_GetSearchMatchPath(EVERYTHING3_SEARCH_STATE *search_state)
{
	return _everything3_is_search_flag_set(search_state,_EVERYTHING3_SEARCH_FLAG_MATCH_PATH);
}

EVERYTHING3_USERAPI BOOL EVERYTHING3_API Everything3_SetSearchMatchPrefix(EVERYTHING3_SEARCH_STATE *search_state,BOOL match_prefix)
{
	return _everything3_set_search_flag(search_state,_EVERYTHING3_SEARCH_FLAG_MATCH_PREFIX,match_prefix);
}

EVERYTHING3_USERAPI BOOL EVERYTHING3_API Everything3_GetSearchMatchPrefix(EVERYTHING3_SEARCH_STATE *search_state)
{
	return _everything3_is_search_flag_set(search_state,_EVERYTHING3_SEARCH_FLAG_MATCH_PREFIX);
}

EVERYTHING3_USERAPI BOOL EVERYTHING3_API Everything3_SetSearchMatchSuffix(EVERYTHING3_SEARCH_STATE *search_state,BOOL match_suffix)
{
	return _everything3_set_search_flag(search_state,_EVERYTHING3_SEARCH_FLAG_MATCH_SUFFIX,match_suffix);
}

EVERYTHING3_USERAPI BOOL EVERYTHING3_API Everything3_GetSearchMatchSuffix(EVERYTHING3_SEARCH_STATE *search_state)
{
	return _everything3_is_search_flag_set(search_state,_EVERYTHING3_SEARCH_FLAG_MATCH_SUFFIX);
}

EVERYTHING3_USERAPI BOOL EVERYTHING3_API Everything3_SetSearchIgnorePunctuation(EVERYTHING3_SEARCH_STATE *search_state,BOOL ignore_punctuation)
{
	return _everything3_set_search_flag(search_state,_EVERYTHING3_SEARCH_FLAG_IGNORE_PUNCTUATION,ignore_punctuation);
}

EVERYTHING3_USERAPI BOOL EVERYTHING3_API Everything3_GetSearchIgnorePunctuation(EVERYTHING3_SEARCH_STATE *search_state)
{
	return _everything3_is_search_flag_set(search_state,_EVERYTHING3_SEARCH_FLAG_IGNORE_PUNCTUATION);
}

EVERYTHING3_USERAPI BOOL EVERYTHING3_API Everything3_SetSearchWhitespace(EVERYTHING3_SEARCH_STATE *search_state,BOOL ignore_whitespace)
{
	return _everything3_set_search_flag(search_state,_EVERYTHING3_SEARCH_FLAG_IGNORE_WHITESPACE,ignore_whitespace);
}

EVERYTHING3_USERAPI BOOL EVERYTHING3_API Everything3_GetSearchWhitespace(EVERYTHING3_SEARCH_STATE *search_state)
{
	return _everything3_is_search_flag_set(search_state,_EVERYTHING3_SEARCH_FLAG_IGNORE_WHITESPACE);
}

EVERYTHING3_USERAPI BOOL EVERYTHING3_API Everything3_SetSearchRegex(EVERYTHING3_SEARCH_STATE *search_state,BOOL match_regex)
{
	return _everything3_set_search_flag(search_state,_EVERYTHING3_SEARCH_FLAG_REGEX,match_regex);
}

EVERYTHING3_USERAPI BOOL EVERYTHING3_API Everything3_GetSearchRegex(EVERYTHING3_SEARCH_STATE *search_state)
{
	return _everything3_is_search_flag_set(search_state,_EVERYTHING3_SEARCH_FLAG_REGEX);
}

EVERYTHING3_USERAPI BOOL EVERYTHING3_API Everything3_SetSearchFoldersFirst(EVERYTHING3_SEARCH_STATE *search_state,DWORD folders_first_type)
{
	DWORD folders_first_flags;
	
	switch(folders_first_type)
	{
		case EVERYTHING3_SEARCH_FOLDERS_FIRST_ASCENDING:
			folders_first_flags = _EVERYTHING3_SEARCH_FLAG_FOLDERS_FIRST_ASCENDING;
			break;

		case EVERYTHING3_SEARCH_FOLDERS_FIRST_ALWAYS:
			folders_first_flags = _EVERYTHING3_SEARCH_FLAG_FOLDERS_FIRST_ALWAYS;
			break;

		case EVERYTHING3_SEARCH_FOLDERS_FIRST_NEVER:
			folders_first_flags = _EVERYTHING3_SEARCH_FLAG_FOLDERS_FIRST_NEVER;
			break;

		case EVERYTHING3_SEARCH_FOLDERS_FIRST_DESCENDING:
			folders_first_flags = _EVERYTHING3_SEARCH_FLAG_FOLDERS_FIRST_DESCENDING;
			break;
			
		default:
			SetLastError(EVERYTHING3_ERROR_INVALID_PARAMETER);
			return FALSE;
	}

	return _everything3_change_search_flags(search_state,_EVERYTHING3_SEARCH_FLAG_FOLDERS_FIRST_ALWAYS | _EVERYTHING3_SEARCH_FLAG_FOLDERS_FIRST_NEVER,folders_first_flags);
}

EVERYTHING3_USERAPI DWORD EVERYTHING3_API Everything3_GetSearchFoldersFirst(EVERYTHING3_SEARCH_STATE *search_state)
{
	DWORD search_flags;

	search_flags = _everything3_get_search_flags(search_state) & (_EVERYTHING3_SEARCH_FLAG_FOLDERS_FIRST_ALWAYS | _EVERYTHING3_SEARCH_FLAG_FOLDERS_FIRST_NEVER);
	
	switch(search_flags)
	{
		default:
		case _EVERYTHING3_SEARCH_FLAG_FOLDERS_FIRST_ASCENDING:
			return EVERYTHING3_SEARCH_FOLDERS_FIRST_ASCENDING;

		case _EVERYTHING3_SEARCH_FLAG_FOLDERS_FIRST_ALWAYS:
			return EVERYTHING3_SEARCH_FOLDERS_FIRST_ALWAYS;

		case _EVERYTHING3_SEARCH_FLAG_FOLDERS_FIRST_NEVER:
			return EVERYTHING3_SEARCH_FOLDERS_FIRST_NEVER;

		case _EVERYTHING3_SEARCH_FLAG_FOLDERS_FIRST_DESCENDING:
			return EVERYTHING3_SEARCH_FOLDERS_FIRST_DESCENDING;
	}
}

// Request the total result size 
// It can take a millisecond or two to calculate the total size.
// If the request is not made, Everything3_GetResultListTotalSize will return EVERYTHING3_UINT64_MAX
EVERYTHING3_USERAPI BOOL EVERYTHING3_API Everything3_SetSearchRequestTotalSize(EVERYTHING3_SEARCH_STATE *search_state,BOOL request_total_size)
{
	return _everything3_set_search_flag(search_state,_EVERYTHING3_SEARCH_FLAG_TOTAL_SIZE,request_total_size);
}

EVERYTHING3_USERAPI BOOL EVERYTHING3_API Everything3_GetSearchRequestTotalSize(EVERYTHING3_SEARCH_STATE *search_state)
{
	return _everything3_is_search_flag_set(search_state,_EVERYTHING3_SEARCH_FLAG_TOTAL_SIZE);
}

EVERYTHING3_USERAPI BOOL EVERYTHING3_API Everything3_SetSearchHideResultOmissions(EVERYTHING3_SEARCH_STATE *search_state,BOOL hide_result_omissions)
{
	return _everything3_set_search_flag(search_state,_EVERYTHING3_SEARCH_FLAG_HIDE_RESULT_OMISSIONS,hide_result_omissions);
}

EVERYTHING3_USERAPI BOOL EVERYTHING3_API Everything3_GetSearchHideResultOmissions(EVERYTHING3_SEARCH_STATE *search_state)
{
	return _everything3_is_search_flag_set(search_state,_EVERYTHING3_SEARCH_FLAG_HIDE_RESULT_OMISSIONS);
}

EVERYTHING3_USERAPI BOOL EVERYTHING3_API Everything3_SetSearchSortMix(EVERYTHING3_SEARCH_STATE *search_state,BOOL sort_mix)
{
	return _everything3_set_search_flag(search_state,_EVERYTHING3_SEARCH_FLAG_SORT_MIX,sort_mix);
}

EVERYTHING3_USERAPI BOOL EVERYTHING3_API Everything3_GetSearchSortMix(EVERYTHING3_SEARCH_STATE *search_state)
{
	return _everything3_is_search_flag_set(search_state,_EVERYTHING3_SEARCH_FLAG_SORT_MIX);
}

// Set the search text.
EVERYTHING3_USERAPI BOOL EVERYTHING3_API Everything3_SetSearchTextUTF8(EVERYTHING3_SEARCH_STATE *search_state,const EVERYTHING3_UTF8 *search)
{
	BOOL ret;
	
	ret = FALSE;
	
	if ((search_state) && (search))
	{
		SIZE_T new_search_len;
		EVERYTHING3_UTF8 *new_search_text;
		
//TODO: avoid the alloc, we should prealloc some room in the search state.		
		new_search_len = _everything3_utf8_string_get_length_in_bytes(search);
		new_search_text = _everything3_mem_alloc(new_search_len);
		
		if (new_search_text)
		{
			EVERYTHING3_UTF8 *old_search_text;
			
			CopyMemory(new_search_text,search,new_search_len);
		
			EnterCriticalSection(&search_state->cs);
			
			old_search_text = search_state->search_text;
			
			search_state->search_text = new_search_text;
			search_state->search_len = new_search_len;
			
			LeaveCriticalSection(&search_state->cs);

			if (old_search_text)
			{
				_everything3_mem_free(old_search_text);
			}
			
			ret = TRUE;
		}
	}
	else
	{
		SetLastError(EVERYTHING3_ERROR_INVALID_PARAMETER);
	}
	
	return ret;
}

EVERYTHING3_USERAPI BOOL EVERYTHING3_API Everything3_SetSearchTextW(EVERYTHING3_SEARCH_STATE *search_state,const EVERYTHING3_WCHAR *search)
{
	BOOL ret;
	_everything3_utf8_buf_t search_cbuf;

	ret = FALSE;
	
	_everything3_utf8_buf_init(&search_cbuf);

	if (_everything3_utf8_buf_copy_wchar_string(&search_cbuf,search))
	{
		ret = Everything3_SetSearchTextUTF8(search_state,search_cbuf.buf);
	}
	
	_everything3_utf8_buf_kill(&search_cbuf);
	
	return ret;
}	

EVERYTHING3_USERAPI BOOL EVERYTHING3_API Everything3_SetSearchTextA(EVERYTHING3_SEARCH_STATE *search_state,const EVERYTHING3_CHAR *search)
{
	BOOL ret;
	_everything3_utf8_buf_t search_cbuf;

	ret = FALSE;
	
	_everything3_utf8_buf_init(&search_cbuf);

	if (_everything3_utf8_buf_copy_ansi_string(&search_cbuf,search))
	{
		ret = Everything3_SetSearchTextUTF8(search_state,search_cbuf.buf);
	}
	
	_everything3_utf8_buf_kill(&search_cbuf);
	
	return ret;
}	

EVERYTHING3_USERAPI SIZE_T EVERYTHING3_API Everything3_GetSearchTextUTF8(EVERYTHING3_SEARCH_STATE *search_state,EVERYTHING3_UTF8 *buf,SIZE_T bufsize)
{
	return _everything3_safe_utf8_string_copy_utf8_string_n(buf,bufsize,search_state->search_text,search_state->search_len);
}

EVERYTHING3_USERAPI SIZE_T EVERYTHING3_API Everything3_GetSearchTextW(EVERYTHING3_SEARCH_STATE *search_state,EVERYTHING3_WCHAR *wbuf,SIZE_T wbuf_size_in_wchars)
{
	return _everything3_safe_wchar_string_copy_utf8_string_n(wbuf,wbuf_size_in_wchars,search_state->search_text,search_state->search_len);
}

EVERYTHING3_USERAPI SIZE_T EVERYTHING3_API Everything3_GetSearchTextA(EVERYTHING3_SEARCH_STATE *search_state,CHAR *buf,SIZE_T bufsize)
{
	return _everything3_safe_ansi_string_copy_utf8_string_n(buf,bufsize,search_state->search_text,search_state->search_len);
}

// Specify the sort
// the default is no sort request.
// Everything will fall back to Name ascending if no sort is specified.
// There's no limit on the number of sorts you can add.
// However, Everything 1.5 currently only supports 3 sorts.
// You can specify the sort in your search text with the sort: function.
// sort: will override any added sorts with Everything3_AddSearchSort.
// sort: supports up to 8 sorts.
// Sorts can be cleared with Everything3_ClearSearchSorts().
EVERYTHING3_USERAPI BOOL EVERYTHING3_API Everything3_AddSearchSort(EVERYTHING3_SEARCH_STATE *search_state,DWORD property_id,BOOL ascending)
{
	BOOL ret;
	
	ret = FALSE;
	
	if ((search_state) && (property_id != EVERYTHING3_INVALID_PROPERTY_ID))
	{
		DWORD sort_flags;
		
		sort_flags = 0;

		if (!ascending)
		{
			sort_flags |= _EVERYTHING3_SEARCH_SORT_FLAG_DESCENDING;
		}
		
		EnterCriticalSection(&search_state->cs);
				
		if (search_state->sort_count < search_state->sort_allocated)
		{
			search_state->sort_array[search_state->sort_count].property_id = property_id;
			search_state->sort_array[search_state->sort_count].flags = sort_flags;
			
			search_state->sort_count++;
			
			ret = TRUE;
		}
		else
		{
			SIZE_T new_sort_allocated;
			_everything3_search_sort_t *new_sort_array;
			SIZE_T array_size;
			
			new_sort_allocated = _everything3_safe_size_add(search_state->sort_allocated,search_state->sort_allocated);
			if (!new_sort_allocated)
			{
				new_sort_allocated = _EVERYTHING3_MIN_SORT_COUNT;
			}
			
			assert(sizeof(_everything3_search_sort_t) == 8);
					
			// * (sizeof(DWORD) + sizeof(DWORD))
			array_size = _everything3_safe_size_add(new_sort_allocated,new_sort_allocated); // x2
			array_size = _everything3_safe_size_add(array_size,array_size); // x4
			array_size = _everything3_safe_size_add(array_size,array_size); // 8
			
			new_sort_array = _everything3_mem_alloc(array_size);
			if (new_sort_array)
			{
				CopyMemory(new_sort_array,search_state->sort_array,search_state->sort_count * sizeof(DWORD));
				
				if (search_state->sort_array)
				{
					_everything3_mem_free(search_state->sort_array);
				}
				
				search_state->sort_array = new_sort_array;
				search_state->sort_allocated = new_sort_allocated;
				
				new_sort_array[search_state->sort_count].property_id = property_id;
				new_sort_array[search_state->sort_count].flags = sort_flags;
			
				search_state->sort_count++;
			
				ret = TRUE;
			}
		}
				
		LeaveCriticalSection(&search_state->cs);
	}
	else
	{
		SetLastError(EVERYTHING3_ERROR_INVALID_PARAMETER);
	}
	
	return ret;
}

// Remove all sorts.
// Everything will fall back to Name ascending if no sort is specified.
// Sorts can be added with Everything3_AddSearchSort().
EVERYTHING3_USERAPI BOOL EVERYTHING3_API Everything3_ClearSearchSorts(EVERYTHING3_SEARCH_STATE *search_state)
{
	BOOL ret;
	
	ret = FALSE;
	
	if (search_state)
	{
		EnterCriticalSection(&search_state->cs);
		
		if (search_state->sort_array)
		{
			_everything3_mem_free(search_state->sort_array);
			
			search_state->sort_array = NULL;
		}
				
		search_state->sort_count = 0;
		search_state->sort_allocated = 0;
		
		LeaveCriticalSection(&search_state->cs);
	}
	else
	{
		SetLastError(EVERYTHING3_ERROR_INVALID_PARAMETER);
	}
	
	return ret;
}

// return the number of sorts added with Everything3_AddSearchSort().
EVERYTHING3_USERAPI SIZE_T EVERYTHING3_API Everything3_GetSearchSortCount(EVERYTHING3_SEARCH_STATE *search_state)
{
	SIZE_T ret;
	
	ret = 0;
	
	if (search_state)
	{
		EnterCriticalSection(&search_state->cs);
		
		ret = search_state->sort_count;

		LeaveCriticalSection(&search_state->cs);

		SetLastError(EVERYTHING3_OK);
	}
	else
	{
		SetLastError(EVERYTHING3_ERROR_INVALID_PARAMETER);
	}
	
	return ret;
}

// Get the property ID from the specified sort index.
EVERYTHING3_USERAPI DWORD EVERYTHING3_API Everything3_GetSearchSortPropertyId(EVERYTHING3_SEARCH_STATE *search_state,SIZE_T sort_index)
{
	DWORD ret;
	
	ret = EVERYTHING3_INVALID_PROPERTY_ID;
	
	if (search_state)
	{
		EnterCriticalSection(&search_state->cs);

		if (sort_index < search_state->sort_count)
		{
			ret = search_state->sort_array[sort_index].property_id;
		}
		else
		{
			SetLastError(EVERYTHING3_ERROR_INVALID_PARAMETER);
		}

		LeaveCriticalSection(&search_state->cs);
	}
	else
	{
		SetLastError(EVERYTHING3_ERROR_INVALID_PARAMETER);
	}
	
	return ret;
}

// returns TRUE if the sort is ascending from the specified sort index.
EVERYTHING3_USERAPI BOOL EVERYTHING3_API Everything3_GetSearchSortAscending(EVERYTHING3_SEARCH_STATE *search_state,SIZE_T sort_index)
{
	BOOL ret;
	
	ret = FALSE;
	
	if (search_state)
	{
		EnterCriticalSection(&search_state->cs);

		if (sort_index < search_state->sort_count)
		{
			if (!(search_state->sort_array[sort_index].flags & _EVERYTHING3_SEARCH_SORT_FLAG_DESCENDING))
			{
				ret = TRUE;
			}

			SetLastError(EVERYTHING3_OK);
		}
		else
		{
			SetLastError(EVERYTHING3_ERROR_INVALID_PARAMETER);
		}

		LeaveCriticalSection(&search_state->cs);
	}
	else
	{
		SetLastError(EVERYTHING3_ERROR_INVALID_PARAMETER);
	}
	
	return ret;
}

// add a property request.
// See Everything3_AddSearchPropertyRequest() for more information
static BOOL _everything3_add_search_property_request(EVERYTHING3_SEARCH_STATE *search_state,DWORD property_id,BOOL format,BOOL highlight)
{
	BOOL ret;
	
	ret = FALSE;
	
	if ((search_state) && (property_id != EVERYTHING3_INVALID_PROPERTY_ID))
	{
		DWORD property_request_flags;
		
		property_request_flags = 0;
		
		if (highlight)
		{
			property_request_flags |= _EVERYTHING3_SEARCH_PROPERTY_REQUEST_FLAG_HIGHLIGHT;
		}
		
		if (format)
		{
			property_request_flags |= _EVERYTHING3_SEARCH_PROPERTY_REQUEST_FLAG_FORMAT;
		}
		
		EnterCriticalSection(&search_state->cs);
				
		if (search_state->property_request_count < search_state->property_request_allocated)
		{
			search_state->property_request_array[search_state->property_request_count].property_id = property_id;
			search_state->property_request_array[search_state->property_request_count].flags = property_request_flags;
			
			search_state->property_request_count++;
			
			ret = TRUE;
		}
		else
		{
			SIZE_T new_property_request_allocated;
			_everything3_search_property_request_t *new_property_request_array;
			SIZE_T array_size;
			
			new_property_request_allocated = _everything3_safe_size_add(search_state->property_request_allocated,search_state->property_request_allocated);
			if (!new_property_request_allocated)
			{
				new_property_request_allocated = _EVERYTHING3_MIN_PROPERTY_REQUEST_COUNT;
			}
			
			assert(sizeof(_everything3_search_property_request_t) == 8);
			
			// * sizeof(_everything3_search_property_request_t)
			array_size = _everything3_safe_size_add(new_property_request_allocated,new_property_request_allocated); // x2
			array_size = _everything3_safe_size_add(array_size,array_size); // x4
			array_size = _everything3_safe_size_add(array_size,array_size); // x8
			
			new_property_request_array = _everything3_mem_alloc(array_size);
			if (new_property_request_array)
			{
				CopyMemory(new_property_request_array,search_state->property_request_array,search_state->property_request_count * sizeof(DWORD));
				
				if (search_state->property_request_array)
				{
					_everything3_mem_free(search_state->property_request_array);
				}
				
				search_state->property_request_array = new_property_request_array;
				search_state->property_request_allocated = new_property_request_allocated;
				
				new_property_request_array[search_state->property_request_count].property_id = property_id;
				new_property_request_array[search_state->property_request_count].flags = property_request_flags;
			
				search_state->property_request_count++;
			
				ret = TRUE;
			}
		}
				
		LeaveCriticalSection(&search_state->cs);
	}
	else
	{
		SetLastError(EVERYTHING3_ERROR_INVALID_PARAMETER);
	}
	
	return ret;
}

// Specify which properties to gather.
// the default is no property requests.
// Everything will fall back to EVERYTHING3_PROPERTY_ID_FULL_PATH property if no properties are specified.
// There's no limit on the number of properties you can add.
// A search that requests indexed properties will return immediately.
// A search that requests properties that have not been indexed will be gathered before the search returns.
// the IPC pipe client will cache these properties. Future searches will return immediately.
// Properties can be cleared with Everything3_ClearSearchPropertyRequests().
EVERYTHING3_USERAPI BOOL EVERYTHING3_API Everything3_AddSearchPropertyRequest(EVERYTHING3_SEARCH_STATE *search_state,DWORD property_id)
{
	return _everything3_add_search_property_request(search_state,property_id,FALSE,FALSE);
}

EVERYTHING3_USERAPI BOOL EVERYTHING3_API Everything3_AddSearchPropertyRequestFormatted(EVERYTHING3_SEARCH_STATE *search_state,DWORD property_id)
{
	return _everything3_add_search_property_request(search_state,property_id,TRUE,FALSE);
}

EVERYTHING3_USERAPI BOOL EVERYTHING3_API Everything3_AddSearchPropertyRequestHighlighted(EVERYTHING3_SEARCH_STATE *search_state,DWORD property_id)
{
	return _everything3_add_search_property_request(search_state,property_id,TRUE,TRUE);
}

// Clear all property requests.
// Everything will fall back to EVERYTHING3_PROPERTY_ID_FULL_PATH property if no properties are specified.
// Properties can be requested with Everything3_AddSearchPropertyRequest.
EVERYTHING3_USERAPI BOOL EVERYTHING3_API Everything3_ClearSearchPropertyRequests(EVERYTHING3_SEARCH_STATE *search_state)
{
	BOOL ret;
	
	ret = FALSE;
	
	if (search_state)
	{
		EnterCriticalSection(&search_state->cs);
		
		if (search_state->property_request_array)
		{
			_everything3_mem_free(search_state->property_request_array);
			
			search_state->property_request_array = NULL;
		}
				
		search_state->property_request_count = 0;
		search_state->property_request_allocated = 0;
		
		LeaveCriticalSection(&search_state->cs);
	}
	else
	{
		SetLastError(EVERYTHING3_ERROR_INVALID_PARAMETER);
	}
	
	return ret;
}

// return the number of requested properties.
// returns 0 on error. Call Everything3_GetLastError() for more information.
EVERYTHING3_USERAPI SIZE_T EVERYTHING3_API Everything3_GetSearchPropertyRequestCount(EVERYTHING3_SEARCH_STATE *search_state)
{
	SIZE_T ret;
	
	ret = 0;
	
	if (search_state)
	{
		EnterCriticalSection(&search_state->cs);
		
		ret = search_state->property_request_count;

		LeaveCriticalSection(&search_state->cs);

		SetLastError(EVERYTHING3_OK);
	}
	else
	{
		SetLastError(EVERYTHING3_ERROR_INVALID_PARAMETER);
	}
	
	return ret;
}

// Get the property ID from the specified property request index.
// returns EVERYTHING3_INVALID_PROPERTY_ID on error. Call Everything3_GetLastError() for more information.
EVERYTHING3_USERAPI DWORD EVERYTHING3_API Everything3_GetSearchPropertyRequestPropertyId(EVERYTHING3_SEARCH_STATE *search_state,SIZE_T index)
{
	DWORD ret;
	
	ret = EVERYTHING3_INVALID_PROPERTY_ID;
	
	if (search_state)
	{
		EnterCriticalSection(&search_state->cs);

		if (index < search_state->property_request_count)
		{
			ret = search_state->property_request_array[index].property_id;
		}
		else
		{
			SetLastError(EVERYTHING3_ERROR_INVALID_PARAMETER);
		}

		LeaveCriticalSection(&search_state->cs);
	}
	else
	{
		SetLastError(EVERYTHING3_ERROR_INVALID_PARAMETER);
	}
	
	return ret;
}

// Returns TRUE if the request property should be highlighted.
// Otherwise, returns FALSE.
// Returns FALSE on error. Call Everything3_GetLastError() for more information.
EVERYTHING3_USERAPI BOOL EVERYTHING3_API Everything3_GetSearchPropertyRequestHighlight(EVERYTHING3_SEARCH_STATE *search_state,SIZE_T index)
{
	BOOL ret;
	
	ret = FALSE;
	
	if (search_state)
	{
		EnterCriticalSection(&search_state->cs);

		if (index < search_state->property_request_count)
		{
			if (search_state->property_request_array[index].flags & _EVERYTHING3_SEARCH_PROPERTY_REQUEST_FLAG_HIGHLIGHT)
			{
				ret = 1;
			}

			SetLastError(EVERYTHING3_OK);
		}
		else
		{
			SetLastError(EVERYTHING3_ERROR_INVALID_PARAMETER);
		}

		LeaveCriticalSection(&search_state->cs);
	}
	else
	{
		SetLastError(EVERYTHING3_ERROR_INVALID_PARAMETER);
	}
	
	return ret;
}

// Returns TRUE if the request property should be highlighted.
// Otherwise, returns FALSE.
// Returns FALSE on error. Call Everything3_GetLastError() for more information.
EVERYTHING3_USERAPI BOOL EVERYTHING3_API Everything3_GetSearchPropertyRequestFormat(EVERYTHING3_SEARCH_STATE *search_state,SIZE_T index)
{
	BOOL ret;
	
	ret = FALSE;
	
	if (search_state)
	{
		EnterCriticalSection(&search_state->cs);

		if (index < search_state->property_request_count)
		{
			if (search_state->property_request_array[index].flags & _EVERYTHING3_SEARCH_PROPERTY_REQUEST_FLAG_FORMAT)
			{
				ret = 1;
			}

			SetLastError(EVERYTHING3_OK);
		}
		else
		{
			SetLastError(EVERYTHING3_ERROR_INVALID_PARAMETER);
		}

		LeaveCriticalSection(&search_state->cs);
	}
	else
	{
		SetLastError(EVERYTHING3_ERROR_INVALID_PARAMETER);
	}
	
	return ret;
}

// Sets the Viewport offset.
// Useful for creating a small window of results.
EVERYTHING3_USERAPI BOOL EVERYTHING3_API Everything3_SetSearchViewportOffset(EVERYTHING3_SEARCH_STATE *search_state,SIZE_T offset)
{
	BOOL ret;
	
	ret = FALSE;
	
	if (search_state)
	{
		EnterCriticalSection(&search_state->cs);
		
		search_state->viewport_offset = offset;
		
		LeaveCriticalSection(&search_state->cs);
	}
	else
	{
		SetLastError(EVERYTHING3_ERROR_INVALID_PARAMETER);
	}
	
	return ret;
}

EVERYTHING3_USERAPI SIZE_T EVERYTHING3_API Everything3_GetSearchViewportOffset(EVERYTHING3_SEARCH_STATE *search_state)
{
	SIZE_T ret;
	
	ret = 0;
	
	if (search_state)
	{
		EnterCriticalSection(&search_state->cs);
		
		ret = search_state->viewport_offset;
		
		LeaveCriticalSection(&search_state->cs);

		SetLastError(EVERYTHING3_OK);
	}
	else
	{
		SetLastError(EVERYTHING3_ERROR_INVALID_PARAMETER);
	}
	
	return ret;
}

// Sets the Viewport Count.
// The default is to show all results.
// Useful for creating a small window of results.
EVERYTHING3_USERAPI BOOL EVERYTHING3_API Everything3_SetSearchViewportCount(EVERYTHING3_SEARCH_STATE *search_state,SIZE_T count)
{
	BOOL ret;
	
	ret = FALSE;
	
	if (search_state)
	{
		EnterCriticalSection(&search_state->cs);
		
		search_state->viewport_count = count;
		
		LeaveCriticalSection(&search_state->cs);
	}
	else
	{
		SetLastError(EVERYTHING3_ERROR_INVALID_PARAMETER);
	}
	
	return ret;
}

EVERYTHING3_USERAPI SIZE_T EVERYTHING3_API Everything3_GetSearchViewportCount(EVERYTHING3_SEARCH_STATE *search_state)
{
	SIZE_T ret;
	
	ret = 0;
	
	if (search_state)
	{
		EnterCriticalSection(&search_state->cs);
		
		ret = search_state->viewport_count;
		
		LeaveCriticalSection(&search_state->cs);

		SetLastError(EVERYTHING3_OK);
	}
	else
	{
		SetLastError(EVERYTHING3_ERROR_INVALID_PARAMETER);
	}
	
	return ret;
}

// execute a search or get the current result list.
static EVERYTHING3_RESULT_LIST *_everything3_search_with_extra_flags(EVERYTHING3_CLIENT *client,EVERYTHING3_SEARCH_STATE *search_state,DWORD command,DWORD extra_flags)
{
	EVERYTHING3_RESULT_LIST *ret;
	
	ret = NULL;
	
	if ((client) && (search_state))
	{
		BYTE *packet_data;
		SIZE_T packet_size;
		
		_everything3_Lock(client);
		
		EnterCriticalSection(&search_state->cs);
		
		// search_flags
		packet_size = sizeof(DWORD);
		
		// search_len
		packet_size = _everything3_safe_size_add(packet_size,(SIZE_T)_everything3_copy_len_vlq(NULL,search_state->search_len));
		
		// search_text
		packet_size = _everything3_safe_size_add(packet_size,search_state->search_len);

		// view port offset
		packet_size = _everything3_safe_size_add(packet_size,sizeof(SIZE_T));
		packet_size = _everything3_safe_size_add(packet_size,sizeof(SIZE_T));

		// sort
		packet_size = _everything3_safe_size_add(packet_size,(SIZE_T)_everything3_copy_len_vlq(NULL,search_state->sort_count));
		packet_size = _everything3_safe_size_add(packet_size,search_state->sort_count * sizeof(_everything3_search_sort_t));

		// property request 
		packet_size = _everything3_safe_size_add(packet_size,(SIZE_T)_everything3_copy_len_vlq(NULL,search_state->property_request_count));
		packet_size = _everything3_safe_size_add(packet_size,search_state->property_request_count * sizeof(_everything3_search_property_request_t));
		
		packet_data = _everything3_mem_alloc(packet_size);
		if (packet_data)
		{
			BYTE *packet_d;
			
			packet_d = packet_data;
			
			// search flags
			packet_d = _everything3_copy_dword(packet_d,search_state->search_flags);
			
			// search text
			packet_d = _everything3_copy_len_vlq(packet_d,search_state->search_len);
			packet_d = _everything3_copy_memory(packet_d,search_state->search_text,search_state->search_len);

			// viewport
			packet_d = _everything3_copy_size_t(packet_d,search_state->viewport_offset);
			packet_d = _everything3_copy_size_t(packet_d,search_state->viewport_count);

			// sort
			
			packet_d = _everything3_copy_len_vlq(packet_d,search_state->sort_count);

			{
				_everything3_search_sort_t *sort_p;
				SIZE_T sort_run;
				
				sort_p = search_state->sort_array;
				sort_run = search_state->sort_count;
				
				while(sort_run)
				{
					packet_d = _everything3_copy_dword(packet_d,sort_p->property_id);
					packet_d = _everything3_copy_dword(packet_d,sort_p->flags);
					
					sort_p++;
					sort_run--;
				}
			}

			// property requests

			packet_d = _everything3_copy_len_vlq(packet_d,search_state->property_request_count);

			{
				_everything3_search_property_request_t *property_request_p;
				SIZE_T property_request_run;
				
				property_request_p = search_state->property_request_array;
				property_request_run = search_state->property_request_count;
				
				while(property_request_run)
				{
					packet_d = _everything3_copy_dword(packet_d,property_request_p->property_id);
					packet_d = _everything3_copy_dword(packet_d,property_request_p->flags);
					
					property_request_p++;
					property_request_run--;
				}
			}

			assert((packet_d - packet_data) == packet_size);
			
			if (_everything3_send(client,command,packet_data,packet_size))
			{
				EVERYTHING3_RESULT_LIST *result_list;
				
				result_list = _everything3_mem_calloc(sizeof(EVERYTHING3_RESULT_LIST));
				
				if (result_list)
				{
					_everything3_stream_t stream;
					SIZE_T item_total_property_size;
					SIZE_T size_t_size;
					
					_everything3_pool_init(&result_list->pool);
					item_total_property_size = 1; // item_flags;
					
					result_list->total_result_size = EVERYTHING3_UINT64_MAX;
					
					stream.client = client;
					stream.buf = NULL;
					stream.p = NULL;
					stream.avail = 0;
					stream.error_code = 0;
					stream.got_last = 0;
					stream.is_64bit = 0;
					size_t_size = sizeof(DWORD);
					
					result_list->valid_flags = _everything3_stream_read_dword(&stream);

					if (result_list->valid_flags & _EVERYTHING3_SEARCH_FLAG_64BIT)
					{
						stream.is_64bit = 1;
						size_t_size = sizeof(EVERYTHING3_UINT64);
					}

					// result counts
					result_list->folder_result_count = _everything3_stream_read_size_t(&stream);
					result_list->file_result_count = _everything3_stream_read_size_t(&stream);
					
					// total size.
					if (result_list->valid_flags & _EVERYTHING3_SEARCH_FLAG_TOTAL_SIZE)
					{
						result_list->total_result_size = _everything3_stream_read_uint64(&stream);
					}
					
					// viewport
					result_list->viewport_offset = _everything3_stream_read_size_t(&stream);
					result_list->viewport_count = _everything3_stream_read_size_t(&stream);
					
					// sort
					result_list->sort_count = _everything3_stream_read_len_vlq(&stream);
					
					assert(sizeof(_everything3_result_list_sort_t) == 8);
							
					if (result_list->sort_count)
					{
						SIZE_T sort_size;
						
						sort_size = result_list->sort_count;

						assert(sizeof(_everything3_result_list_sort_t) == 8);
									
						sort_size = _everything3_safe_size_add(sort_size,sort_size); // x2
						sort_size = _everything3_safe_size_add(sort_size,sort_size); // x4
						sort_size = _everything3_safe_size_add(sort_size,sort_size); // x8
						
						result_list->sort_array = _everything3_mem_alloc(sort_size);

						if (!result_list->sort_array)
						{
							SetLastError(EVERYTHING3_ERROR_OUT_OF_MEMORY);
							
							goto got_error;
						}
						
						{
							SIZE_T sort_run;
							_everything3_result_list_sort_t *sort_d;
							
							sort_run = result_list->sort_count;
							sort_d = result_list->sort_array;
							
							while(sort_run)
							{
								sort_d->property_id = _everything3_stream_read_dword(&stream);
								sort_d->flags = _everything3_stream_read_dword(&stream);
								
								sort_d++;
								sort_run--;
							}
						}
					}

					// property requests
					result_list->property_request_count = _everything3_stream_read_len_vlq(&stream);
					
					if (result_list->property_request_count)
					{
						SIZE_T property_request_size;
						
						property_request_size = result_list->property_request_count;
						
						assert(sizeof(_everything3_result_list_property_request_t) <= 24);

						property_request_size = _everything3_safe_size_add(property_request_size,property_request_size); // x2
						property_request_size = _everything3_safe_size_add(property_request_size,property_request_size); // x4
						property_request_size = _everything3_safe_size_add(property_request_size,property_request_size); // x8
						property_request_size = _everything3_safe_size_add(property_request_size,property_request_size); // x16
						property_request_size = _everything3_safe_size_add(property_request_size,property_request_size); // x32 (over allocate)
						
						result_list->property_request_array = _everything3_mem_alloc(property_request_size);
						
						if (!result_list->property_request_array)
						{
							SetLastError(EVERYTHING3_ERROR_OUT_OF_MEMORY);
							
							goto got_error;
						}
						
						result_list->sorted_property_request_array = _everything3_mem_alloc(_everything3_safe_size_mul_size_of_pointer(result_list->property_request_count));
						
						if (!result_list->sorted_property_request_array)
						{
							SetLastError(EVERYTHING3_ERROR_OUT_OF_MEMORY);
							
							goto got_error;
						}
						
						{
							SIZE_T property_request_run;
							_everything3_result_list_property_request_t *property_request_d;
							_everything3_result_list_property_request_t **sorted_property_request_d;
							
							property_request_run = result_list->property_request_count;
							property_request_d = result_list->property_request_array;
							sorted_property_request_d = result_list->sorted_property_request_array;
							
							while(property_request_run)
							{
								property_request_d->offset = item_total_property_size;
								property_request_d->property_id = _everything3_stream_read_dword(&stream);
								property_request_d->flags = _everything3_stream_read_dword(&stream);
								property_request_d->value_type = _everything3_stream_read_byte(&stream);
								
								if (property_request_d->flags & (_EVERYTHING3_SEARCH_PROPERTY_REQUEST_FLAG_FORMAT|_EVERYTHING3_SEARCH_PROPERTY_REQUEST_FLAG_HIGHLIGHT))
								{
									// pstring.
									item_total_property_size = _everything3_safe_size_add(item_total_property_size,sizeof(void *));
								}
								else
								{
									// add to total item size.
									switch(property_request_d->value_type)
									{
										case EVERYTHING3_PROPERTY_VALUE_TYPE_PSTRING: 
										case EVERYTHING3_PROPERTY_VALUE_TYPE_PSTRING_MULTISTRING: 
										case EVERYTHING3_PROPERTY_VALUE_TYPE_PSTRING_STRING_REFERENCE:
										case EVERYTHING3_PROPERTY_VALUE_TYPE_PSTRING_FOLDER_REFERENCE:
										case EVERYTHING3_PROPERTY_VALUE_TYPE_PSTRING_FILE_OR_FOLDER_REFERENCE:
											item_total_property_size = _everything3_safe_size_add(item_total_property_size,sizeof(void *));
											break;
											
										case EVERYTHING3_PROPERTY_VALUE_TYPE_BYTE:
										case EVERYTHING3_PROPERTY_VALUE_TYPE_BYTE_GET_TEXT:
											item_total_property_size = _everything3_safe_size_add(item_total_property_size,sizeof(BYTE));
											break;

										case EVERYTHING3_PROPERTY_VALUE_TYPE_WORD:
										case EVERYTHING3_PROPERTY_VALUE_TYPE_WORD_GET_TEXT:
											item_total_property_size = _everything3_safe_size_add(item_total_property_size,sizeof(WORD));
											break;

										case EVERYTHING3_PROPERTY_VALUE_TYPE_DWORD: 
										case EVERYTHING3_PROPERTY_VALUE_TYPE_DWORD_FIXED_Q1K: 
										case EVERYTHING3_PROPERTY_VALUE_TYPE_DWORD_GET_TEXT: 
											item_total_property_size = _everything3_safe_size_add(item_total_property_size,sizeof(DWORD));
											break;
											
										case EVERYTHING3_PROPERTY_VALUE_TYPE_UINT64:
											item_total_property_size = _everything3_safe_size_add(item_total_property_size,sizeof(unsigned __int64));
											break;

										case EVERYTHING3_PROPERTY_VALUE_TYPE_UINT128:
											item_total_property_size = _everything3_safe_size_add(item_total_property_size,sizeof(EVERYTHING3_UINT128));
											break;

										case EVERYTHING3_PROPERTY_VALUE_TYPE_DIMENSIONS:
											item_total_property_size = _everything3_safe_size_add(item_total_property_size,sizeof(EVERYTHING3_DIMENSIONS));
											break;

										case EVERYTHING3_PROPERTY_VALUE_TYPE_SIZE_T:
											item_total_property_size = _everything3_safe_size_add(item_total_property_size,size_t_size);
											break;

										case EVERYTHING3_PROPERTY_VALUE_TYPE_INT32_FIXED_Q1K:
										case EVERYTHING3_PROPERTY_VALUE_TYPE_INT32_FIXED_Q1M:
											item_total_property_size = _everything3_safe_size_add(item_total_property_size,sizeof(EVERYTHING3_INT32));
											break;

										case EVERYTHING3_PROPERTY_VALUE_TYPE_BLOB8:
										case EVERYTHING3_PROPERTY_VALUE_TYPE_BLOB16:
											item_total_property_size = _everything3_safe_size_add(item_total_property_size,sizeof(void *));
											break;
									}
								}
								
								*sorted_property_request_d++ = property_request_d;
								
								property_request_d++;
								property_request_run--;
							}
						}
						
						// sort properties..
						if (!_everything3_sort(result_list->sorted_property_request_array,result_list->property_request_count,_everything3_result_list_property_request_compare))
						{
							SetLastError(EVERYTHING3_ERROR_OUT_OF_MEMORY);
							
							goto got_error;
						}
						
						{
							SIZE_T property_request_run;
							_everything3_result_list_property_request_t **sorted_property_request_p;
							
							property_request_run = result_list->property_request_count;
							sorted_property_request_p = result_list->sorted_property_request_array;
							
							while(property_request_run)
							{
								_everything3_debug_printf("PROPERTY %u %08x %u %p\n",(*sorted_property_request_p)->property_id,(*sorted_property_request_p)->flags,(*sorted_property_request_p)->value_type,(*sorted_property_request_p)->offset);
								
								sorted_property_request_p++;
								property_request_run--;
							}
						}

						_everything3_debug_printf("total item property size: %p\n",item_total_property_size);
					}
					
					// read items
					
					if (result_list->viewport_count)
					{
						SIZE_T item_array_size;
						SIZE_T viewport_run;
						_everything3_result_list_item_t *item_d;
						
						item_array_size = _everything3_safe_size_mul_size_of_pointer(result_list->viewport_count);
						
						result_list->item_array = _everything3_mem_alloc(item_array_size);
						
						if (!result_list->item_array)
						{
							SetLastError(EVERYTHING3_ERROR_OUT_OF_MEMORY);
							
							goto got_error;
						}
						
						item_d = result_list->item_array;
						
						viewport_run = result_list->viewport_count;
						
						while(viewport_run)
						{
							BYTE *property_d;
							
							item_d->property_data = _everything3_pool_alloc(&result_list->pool,item_total_property_size);
							
							property_d = item_d->property_data;
							
							*property_d++ = _everything3_stream_read_byte(&stream);
							
							// read properties..
							{
								SIZE_T property_request_run;
								const _everything3_result_list_property_request_t *property_request_p;
								
								property_request_run = result_list->property_request_count;
								property_request_p = result_list->property_request_array;
								
								while(property_request_run)
								{
									if (property_request_p->flags & (_EVERYTHING3_SEARCH_PROPERTY_REQUEST_FLAG_FORMAT|_EVERYTHING3_SEARCH_PROPERTY_REQUEST_FLAG_HIGHLIGHT))
									{
										SIZE_T len;
										SIZE_T pstring_size;
										_everything3_utf8_pstring_t *pstring;
										BYTE *pstring_text;
										
										len = _everything3_stream_read_len_vlq(&stream);
										
										if (len)
										{
											// _everything3_debug_printf("LEN %p\n",len);
											pstring_size = _everything3_utf8_pstring_calculate_size(len);
											
											pstring = _everything3_pool_alloc(&result_list->pool,pstring_size);
											
											pstring_text = _everything3_utf8_pstring_init_len(pstring,len);
											
											_everything3_stream_read_data(&stream,pstring_text,len);
											// _everything3_debug_printf("%s\n",pstring_text);
										}
										else
										{
											pstring = NULL;
										}

										property_d = _everything3_copy_memory(property_d,&pstring,sizeof(_everything3_utf8_pstring_t *));
									}
									else
									{
										// add to total item size.
										switch(property_request_p->value_type)
										{
											case EVERYTHING3_PROPERTY_VALUE_TYPE_PSTRING: 
											case EVERYTHING3_PROPERTY_VALUE_TYPE_PSTRING_MULTISTRING: 
											case EVERYTHING3_PROPERTY_VALUE_TYPE_PSTRING_STRING_REFERENCE:
											case EVERYTHING3_PROPERTY_VALUE_TYPE_PSTRING_FOLDER_REFERENCE:
											case EVERYTHING3_PROPERTY_VALUE_TYPE_PSTRING_FILE_OR_FOLDER_REFERENCE:

												{
													SIZE_T len;
													SIZE_T pstring_size;
													_everything3_utf8_pstring_t *pstring;
													BYTE *pstring_text;
													
													len = _everything3_stream_read_len_vlq(&stream);
													
													if (len)
													{
														// _everything3_debug_printf("LEN %p\n",len);
														pstring_size = _everything3_utf8_pstring_calculate_size(len);
														
														pstring = _everything3_pool_alloc(&result_list->pool,pstring_size);
														
														pstring_text = _everything3_utf8_pstring_init_len(pstring,len);
														
														_everything3_stream_read_data(&stream,pstring_text,len);
														// _everything3_debug_printf("%s\n",pstring_text);
													}
													else
													{
														pstring = NULL;
													}

													property_d = _everything3_copy_memory(property_d,&pstring,sizeof(_everything3_utf8_pstring_t *));
												}

												break;

											case EVERYTHING3_PROPERTY_VALUE_TYPE_BYTE:
											case EVERYTHING3_PROPERTY_VALUE_TYPE_BYTE_GET_TEXT:

												_everything3_stream_read_data(&stream,property_d,sizeof(BYTE));
												property_d += sizeof(BYTE);

												break;

											case EVERYTHING3_PROPERTY_VALUE_TYPE_WORD:
											case EVERYTHING3_PROPERTY_VALUE_TYPE_WORD_GET_TEXT:

												_everything3_stream_read_data(&stream,property_d,sizeof(WORD));
												property_d += sizeof(WORD);

												break;

											case EVERYTHING3_PROPERTY_VALUE_TYPE_DWORD: 
											case EVERYTHING3_PROPERTY_VALUE_TYPE_DWORD_FIXED_Q1K: 
											case EVERYTHING3_PROPERTY_VALUE_TYPE_DWORD_GET_TEXT: 

												_everything3_stream_read_data(&stream,property_d,sizeof(DWORD));
												property_d += sizeof(DWORD);

												break;
												
											case EVERYTHING3_PROPERTY_VALUE_TYPE_UINT64: 

												_everything3_stream_read_data(&stream,property_d,sizeof(EVERYTHING3_UINT64));
												property_d += sizeof(EVERYTHING3_UINT64);

												break;
												
											case EVERYTHING3_PROPERTY_VALUE_TYPE_UINT128: 

												_everything3_stream_read_data(&stream,property_d,sizeof(EVERYTHING3_UINT128));
												property_d += sizeof(EVERYTHING3_UINT128);

												break;
												
											case EVERYTHING3_PROPERTY_VALUE_TYPE_DIMENSIONS: 

												_everything3_stream_read_data(&stream,property_d,sizeof(EVERYTHING3_DIMENSIONS));
												property_d += sizeof(EVERYTHING3_DIMENSIONS);

												break;
												
											case EVERYTHING3_PROPERTY_VALUE_TYPE_SIZE_T: 
												_everything3_stream_read_data(&stream,property_d,size_t_size);
												property_d += size_t_size;
												break;
												
											case EVERYTHING3_PROPERTY_VALUE_TYPE_INT32_FIXED_Q1K: 
											case EVERYTHING3_PROPERTY_VALUE_TYPE_INT32_FIXED_Q1M: 
												_everything3_stream_read_data(&stream,property_d,sizeof(EVERYTHING3_INT32));
												property_d += sizeof(EVERYTHING3_INT32);
												break;

											case EVERYTHING3_PROPERTY_VALUE_TYPE_BLOB8:

												{
													BYTE len;
													_everything3_blob8_t *blob;
													
													len = _everything3_stream_read_byte(&stream);
													
													if (len)
													{	
														SIZE_T blob_size;
														
														// _everything3_debug_printf("LEN %p\n",len);
														blob_size = _everything3_safe_size_add(len,1);
														
														blob = _everything3_pool_alloc(&result_list->pool,blob_size);
														
														blob->len = len;
														
														_everything3_stream_read_data(&stream,_EVERYTHING3_BLOB8_DATA(blob),len);
													}
													else
													{
														blob = NULL;
													}

													property_d = _everything3_copy_memory(property_d,&blob,sizeof(_everything3_blob8_t *));
												}

												break;

											case EVERYTHING3_PROPERTY_VALUE_TYPE_BLOB16:

												{
													WORD len;
													_everything3_blob16_t *blob;
													
													len = _everything3_stream_read_word(&stream);
													
													if (len)
													{	
														SIZE_T blob_size;
														
														// _everything3_debug_printf("LEN %p\n",len);
														blob_size = _everything3_safe_size_add(len,1);
														
														blob = _everything3_pool_alloc(&result_list->pool,blob_size);
														
														blob->len = len;
														
														_everything3_stream_read_data(&stream,_EVERYTHING3_BLOB16_DATA(blob),len);
													}
													else
													{
														blob = NULL;
													}

													property_d = _everything3_copy_memory(property_d,&blob,sizeof(_everything3_blob16_t *));
												}

												break;

										}
									}
									
									property_request_p++;
									property_request_run--;
								}
							}
							
							item_d++;
							viewport_run--;
						}
					}
					
					if (stream.error_code)
					{
						SetLastError(stream.error_code);
						
						goto got_error;
					}

					ret = result_list;
					result_list = NULL;
					
got_error:
					
					if (stream.buf)
					{
						_everything3_mem_free(stream.buf);
					}
					
					if (result_list)
					{
						Everything3_DestroyResultList(result_list);
					}
				}
			}
			
			_everything3_mem_free(packet_data);
		}
		
		LeaveCriticalSection(&search_state->cs);
	
		_everything3_Unlock(client);
	}
	else
	{
		SetLastError(EVERYTHING3_ERROR_INVALID_PARAMETER);
	}
	
	return ret;
}

// returns FALSE on error.
// Call will block until search completes and all properties requests have been gathered.
// Call Everything3_ShutdownClient or Everything3_DestroyClient to cancel the search.
// The returned result list should only be used by the same thread. This can be different to the thread used to call Everything3_Search.
EVERYTHING3_USERAPI EVERYTHING3_RESULT_LIST *EVERYTHING3_API Everything3_Search(EVERYTHING3_CLIENT *client,EVERYTHING3_SEARCH_STATE *search_state)
{
	return _everything3_search_with_extra_flags(client,search_state,_EVERYTHING3_COMMAND_SEARCH,0);
}

// Same as Everything3_Search, except no new search or sort is performed.
EVERYTHING3_USERAPI EVERYTHING3_RESULT_LIST *EVERYTHING3_API Everything3_GetResults(EVERYTHING3_CLIENT *client,EVERYTHING3_SEARCH_STATE *search_state)
{
	return _everything3_search_with_extra_flags(client,search_state,_EVERYTHING3_COMMAND_GET_RESULTS,0);
}

// Same as Everything3_Search, except no new search is performed.
// Use Everything3_ClearSearchSorts and Everything3_AddSearchSort to specify the sort.
EVERYTHING3_USERAPI EVERYTHING3_RESULT_LIST *EVERYTHING3_API Everything3_Sort(EVERYTHING3_CLIENT *client,EVERYTHING3_SEARCH_STATE *search_state)
{
	return _everything3_search_with_extra_flags(client,search_state,_EVERYTHING3_COMMAND_SORT,0);
}

// Destroy the result list.
// Returns resources back to the system.
// Do not use the result list after it has been destroyed.
EVERYTHING3_USERAPI BOOL EVERYTHING3_API Everything3_DestroyResultList(EVERYTHING3_RESULT_LIST *result_list)
{
	BOOL ret;
	
	ret = FALSE;
	
	// we don't need to lock
	if (result_list)
	{
		_everything3_pool_kill(&result_list->pool);
		
		if (result_list->item_array)
		{
			_everything3_mem_free(result_list->item_array);
		}
		
		if (result_list->sorted_property_request_array)
		{
			_everything3_mem_free(result_list->sorted_property_request_array);
		}
		
		if (result_list->property_request_array)
		{
			_everything3_mem_free(result_list->property_request_array);
		}

		if (result_list->sort_array)
		{
			_everything3_mem_free(result_list->sort_array);
		}
		
		_everything3_mem_free(result_list);
		
		ret = TRUE;
	}
	else
	{
		SetLastError(EVERYTHING3_ERROR_INVALID_PARAMETER);
	}
	
	return ret;
}

// a result or a property has changed.
// call Everything3_GetResults to get new results.
// this call returns immediately. It does not block.
// Poll for changes every second.
// Everything3_Search, Everything3_Sort and Everything3_GetResults will clear the internal 'is_changed' flag on the server.
EVERYTHING3_USERAPI BOOL EVERYTHING3_API Everything3_IsResultListChange(EVERYTHING3_CLIENT *client)
{
	BOOL ret;
	DWORD value;
	SIZE_T numread;

	ret = FALSE;
	
	if (_everything3_ioctrl(client,_EVERYTHING3_COMMAND_IS_RESULT_CHANGE,NULL,0,&value,sizeof(DWORD),&numread))
	{
		if (numread == sizeof(DWORD))
		{
			if (value)
			{
				ret = TRUE;
			}

			if (!ret)
			{
				SetLastError(EVERYTHING3_OK);
			}
		}
		else
		{
			SetLastError(EVERYTHING3_ERROR_BAD_RESPONSE);
		}
	}
	
	return ret;
}

// Similar to Everything3_IsResultListChange.
// Except, this function waits indefinately until the result list changes.
// No need to poll for changes, which will be more efficient.
// The wait can be cancelled with Everything3_ShutdownClient.
EVERYTHING3_USERAPI BOOL EVERYTHING3_API Everything3_WaitForResultListChange(EVERYTHING3_CLIENT *client)
{
	BOOL ret;
	DWORD value;
	SIZE_T numread;

	ret = FALSE;
	
	if (_everything3_ioctrl(client,_EVERYTHING3_COMMAND_WAIT_FOR_RESULT_CHANGE,NULL,0,&value,sizeof(DWORD),&numread))
	{
		if (numread == sizeof(DWORD))
		{
			if (value)
			{
				ret = TRUE;
			}

			if (!ret)
			{
				SetLastError(EVERYTHING3_OK);
			}
		}
		else
		{
			SetLastError(EVERYTHING3_ERROR_BAD_RESPONSE);
		}
	}
	
	return ret;
}

// returns EVERYTHING3_INVALID_PROPERTY_ID on error. Call Everything3_GetLastError() for more information.
static DWORD _everything3_find_property(EVERYTHING3_CLIENT *client,const EVERYTHING3_UTF8 *canonical_name,SIZE_T canonical_name_length_in_bytes)
{
	DWORD ret;
	DWORD value;
	SIZE_T numread;

	ret = EVERYTHING3_INVALID_PROPERTY_ID;
	
	if (_everything3_ioctrl(client,_EVERYTHING3_COMMAND_FIND_PROPERTY_FROM_NAME,canonical_name,canonical_name_length_in_bytes,&value,sizeof(DWORD),&numread))
	{
		if (numread == sizeof(DWORD))
		{
			ret = value;

			if (ret == EVERYTHING3_INVALID_PROPERTY_ID)
			{
				SetLastError(EVERYTHING3_ERROR_PROPERTY_NOT_FOUND);
			}
		}
		else
		{
			SetLastError(EVERYTHING3_ERROR_BAD_RESPONSE);
		}
	}
	
	return ret;
}

// Find a property by canonical name.
// If no property is found, tries a localized name lookup.
// If no property is found, tries a English (US) name lookup.
// returns EVERYTHING3_INVALID_PROPERTY_ID on error. Call Everything3_GetLastError() for more information.
// Can find Windows Property System properties.
// Windows Property System Property IDs will change between sessions.
EVERYTHING3_USERAPI DWORD EVERYTHING3_API Everything3_FindPropertyUTF8(EVERYTHING3_CLIENT *client,const EVERYTHING3_UTF8 *canonical_name)
{
	DWORD ret;

	ret = _everything3_find_property(client,canonical_name,_everything3_utf8_string_get_length_in_bytes(canonical_name));
	
	return ret;
}

// Find a property by canonical name.
// If no property is found, tries a localized name lookup.
// If no property is found, tries a English (US) name lookup.
// returns EVERYTHING3_INVALID_PROPERTY_ID on error. Call Everything3_GetLastError() for more information.
// Can find Windows Property System properties.
// Windows Property System Property IDs will change between sessions.
EVERYTHING3_USERAPI DWORD EVERYTHING3_API Everything3_FindPropertyW(EVERYTHING3_CLIENT *client,const EVERYTHING3_WCHAR *canonical_name)
{
	DWORD ret;
	_everything3_utf8_buf_t property_name_cbuf;

	ret = EVERYTHING3_INVALID_PROPERTY_ID;
	_everything3_utf8_buf_init(&property_name_cbuf);

	if (_everything3_utf8_buf_copy_wchar_string(&property_name_cbuf,canonical_name))
	{
		ret = _everything3_find_property(client,property_name_cbuf.buf,property_name_cbuf.length_in_bytes);
	}

	_everything3_utf8_buf_kill(&property_name_cbuf);
	
	return ret;
}

// Find a property by canonical name.
// If no property is found, tries a localized name lookup.
// If no property is found, tries a English (US) name lookup.
// returns EVERYTHING3_INVALID_PROPERTY_ID on error. Call Everything3_GetLastError() for more information.
// Can find Windows Property System properties.
// Windows Property System Property IDs will change between sessions.
EVERYTHING3_USERAPI DWORD EVERYTHING3_API Everything3_FindPropertyA(EVERYTHING3_CLIENT *client,const EVERYTHING3_CHAR *canonical_name)
{
	DWORD ret;
	_everything3_utf8_buf_t property_name_cbuf;

	ret = EVERYTHING3_INVALID_PROPERTY_ID;
	_everything3_utf8_buf_init(&property_name_cbuf);

	if (_everything3_utf8_buf_copy_ansi_string(&property_name_cbuf,canonical_name))
	{
		ret = _everything3_find_property(client,property_name_cbuf.buf,property_name_cbuf.length_in_bytes);
	}

	_everything3_utf8_buf_kill(&property_name_cbuf);
	
	return ret;
}

// Get the property name from the property ID.
static BOOL _everything3_get_property_name(EVERYTHING3_CLIENT *client,DWORD property_id,_everything3_utf8_buf_t *cbuf)
{
	return _everything3_ioctrl_get_string(client,_EVERYTHING3_COMMAND_GET_PROPERTY_NAME,&property_id,sizeof(DWORD),cbuf);
}

// Get the localized property name from the specified property ID.
EVERYTHING3_USERAPI SIZE_T EVERYTHING3_API Everything3_GetPropertyNameUTF8(EVERYTHING3_CLIENT *client,DWORD property_id,EVERYTHING3_UTF8 *buf,EVERYTHING3_SIZE_T bufsize)
{
	EVERYTHING3_SIZE_T ret;
	_everything3_utf8_buf_t cbuf;
	
	ret = 0;
	_everything3_utf8_buf_init(&cbuf);
	
	if (_everything3_get_property_name(client,property_id,&cbuf))
	{
		ret = _everything3_safe_utf8_string_copy_utf8_string_n(buf,bufsize,cbuf.buf,cbuf.length_in_bytes);
	}

	_everything3_utf8_buf_kill(&cbuf);

	return ret;
}

// Get the localized property name from the specified property ID.
EVERYTHING3_USERAPI EVERYTHING3_SIZE_T EVERYTHING3_API Everything3_GetPropertyNameW(EVERYTHING3_CLIENT *client,DWORD property_id,EVERYTHING3_WCHAR *wbuf,EVERYTHING3_SIZE_T wbuf_size_in_wchars)
{
	EVERYTHING3_SIZE_T ret;
	_everything3_utf8_buf_t cbuf;
	
	ret = 0;
	_everything3_utf8_buf_init(&cbuf);
	
	if (_everything3_get_property_name(client,property_id,&cbuf))
	{
		ret = _everything3_safe_wchar_string_copy_utf8_string_n(wbuf,wbuf_size_in_wchars,cbuf.buf,cbuf.length_in_bytes);
	}

	_everything3_utf8_buf_kill(&cbuf);

	return ret;
}

// Get the localized property name from the specified property ID.
EVERYTHING3_USERAPI EVERYTHING3_SIZE_T EVERYTHING3_API Everything3_GetPropertyNameA(EVERYTHING3_CLIENT *client,DWORD property_id,EVERYTHING3_CHAR *buf,EVERYTHING3_SIZE_T bufsize)
{
	EVERYTHING3_SIZE_T ret;
	_everything3_utf8_buf_t cbuf;
	
	ret = 0;
	_everything3_utf8_buf_init(&cbuf);
	
	if (_everything3_get_property_name(client,property_id,&cbuf))
	{
		ret = _everything3_safe_ansi_string_copy_utf8_string_n(buf,bufsize,cbuf.buf,cbuf.length_in_bytes);
	}

	_everything3_utf8_buf_kill(&cbuf);

	return ret;
}

// Get the canonical property name from the specified property ID.
// For example: Property-system:System.Size
static BOOL _everything3_get_property_canonical_name(EVERYTHING3_CLIENT *client,DWORD property_id,_everything3_utf8_buf_t *cbuf)
{
	return _everything3_ioctrl_get_string(client,_EVERYTHING3_COMMAND_GET_PROPERTY_CANONICAL_NAME,&property_id,sizeof(DWORD),cbuf);
}

// Get the canonical property name from the specified property ID.
EVERYTHING3_USERAPI SIZE_T EVERYTHING3_API Everything3_GetPropertyCanonicalNameUTF8(EVERYTHING3_CLIENT *client,DWORD property_id,EVERYTHING3_UTF8 *buf,EVERYTHING3_SIZE_T bufsize)
{
	EVERYTHING3_SIZE_T ret;
	_everything3_utf8_buf_t cbuf;
	
	ret = 0;
	_everything3_utf8_buf_init(&cbuf);
	
	if (_everything3_get_property_canonical_name(client,property_id,&cbuf))
	{
		ret = _everything3_safe_utf8_string_copy_utf8_string_n(buf,bufsize,cbuf.buf,cbuf.length_in_bytes);
	}

	_everything3_utf8_buf_kill(&cbuf);

	return ret;
}

// Get the canonical property name from the specified property ID.
EVERYTHING3_USERAPI EVERYTHING3_SIZE_T EVERYTHING3_API Everything3_GetPropertyCanonicalNameW(EVERYTHING3_CLIENT *client,DWORD property_id,EVERYTHING3_WCHAR *wbuf,EVERYTHING3_SIZE_T wbuf_size_in_wchars)
{
	EVERYTHING3_SIZE_T ret;
	_everything3_utf8_buf_t cbuf;
	
	ret = 0;
	_everything3_utf8_buf_init(&cbuf);
	
	if (_everything3_get_property_canonical_name(client,property_id,&cbuf))
	{
		ret = _everything3_safe_wchar_string_copy_utf8_string_n(wbuf,wbuf_size_in_wchars,cbuf.buf,cbuf.length_in_bytes);
	}

	_everything3_utf8_buf_kill(&cbuf);

	return ret;
}

// Get the canonical property name from the specified property ID.
EVERYTHING3_USERAPI EVERYTHING3_SIZE_T EVERYTHING3_API Everything3_GetPropertyCanonicalNameA(EVERYTHING3_CLIENT *client,DWORD property_id,EVERYTHING3_CHAR *buf,EVERYTHING3_SIZE_T bufsize)
{
	EVERYTHING3_SIZE_T ret;
	_everything3_utf8_buf_t cbuf;
	
	ret = 0;
	_everything3_utf8_buf_init(&cbuf);
	
	if (_everything3_get_property_canonical_name(client,property_id,&cbuf))
	{
		ret = _everything3_safe_ansi_string_copy_utf8_string_n(buf,bufsize,cbuf.buf,cbuf.length_in_bytes);
	}

	_everything3_utf8_buf_kill(&cbuf);

	return ret;
}

// returns one of the EVERYTHING3_PROPERTY_TYPE_* types.
EVERYTHING3_USERAPI DWORD EVERYTHING3_API Everything3_GetPropertyType(EVERYTHING3_CLIENT *client,DWORD property_id)
{
	DWORD ret;
	DWORD value;
	SIZE_T numread;
			
	ret = 0;
		
	if (_everything3_ioctrl(client,_EVERYTHING3_COMMAND_GET_PROPERTY_TYPE,&property_id,sizeof(DWORD),&value,sizeof(DWORD),&numread))
	{
		if (numread == sizeof(DWORD))
		{
			ret = value;

			if (!ret)
			{
				SetLastError(EVERYTHING3_OK);
			}
		}
		else
		{
			SetLastError(EVERYTHING3_ERROR_BAD_RESPONSE);
		}
	}

	return ret;
}

// Init a utf8 buffer to an empty string.
static void _everything3_utf8_buf_init(_everything3_utf8_buf_t *cbuf)
{
	cbuf->buf = cbuf->stack_buf;
	cbuf->length_in_bytes = 0;
	cbuf->size_in_bytes = _EVERYTHING3_UTF8_BUF_STACK_SIZE;
	cbuf->buf[0] = 0;
}

// Kill the UTF-8 buffer, releasing any allocated memory back to the system.
static void _everything3_utf8_buf_kill(_everything3_utf8_buf_t *cbuf)
{
	if (cbuf->buf != cbuf->stack_buf)
	{
		_everything3_mem_free(cbuf->buf);
	}
}

// Empty the UTF-8 buffer, the buffer will be set to an empty string.
static void _everything3_utf8_buf_empty(_everything3_utf8_buf_t *cbuf)
{
	_everything3_utf8_buf_kill(cbuf);
	_everything3_utf8_buf_init(cbuf);
}

// doesn't keep the existing text.
// doesn't set the text, only sets the length.
static BOOL _everything3_utf8_buf_grow_size(_everything3_utf8_buf_t *cbuf,SIZE_T size_in_bytes)
{
	BYTE *new_buf;
	
	if (size_in_bytes <= cbuf->size_in_bytes)
	{
		return TRUE;
	}
	
	_everything3_utf8_buf_empty(cbuf);
	
	new_buf = _everything3_mem_alloc(size_in_bytes);
	
	if (new_buf)
	{
		cbuf->buf = new_buf;
		cbuf->size_in_bytes = size_in_bytes;
		
		return TRUE;
	}
	
	return FALSE;
}

// doesn't keep the existing text.
// doesn't set the text, only sets the length.
static BOOL _everything3_utf8_buf_grow_length(_everything3_utf8_buf_t *cbuf,SIZE_T length_in_bytes)
{
	if (_everything3_utf8_buf_grow_size(cbuf,_everything3_safe_size_add(length_in_bytes,1)))
	{
		cbuf->length_in_bytes = length_in_bytes;
		
		return TRUE;
	}
	
	return FALSE;
}

// Copy a wide char string into a UTF-8 buffer.
// Use a NULL buffer to calculate the size.
// Handles surrogates correctly.
static EVERYTHING3_UTF8 *_everything3_utf8_string_copy_wchar_string(EVERYTHING3_UTF8 *buf,const EVERYTHING3_WCHAR *ws)
{
	const EVERYTHING3_WCHAR *p;
	EVERYTHING3_UTF8 *d;
	int c;
	
	p = ws;
	d = buf;
	
	while(*p)
	{
		c = *p++;
		
		// surrogates
		if ((c >= 0xD800) && (c < 0xDC00))
		{
			if ((*p >= 0xDC00) && (*p < 0xE000))
			{
				c = 0x10000 + ((c - 0xD800) << 10) + (*p - 0xDC00);
				
				p++;
			}
		}
		
		if (c > 0xffff)
		{
			// 4 bytes
			if (buf)
			{
				*d++ = ((c >> 18) & 0x07) | 0xF0; // 11110xxx
				*d++ = ((c >> 12) & 0x3f) | 0x80; // 10xxxxxx
				*d++ = ((c >> 6) & 0x3f) | 0x80; // 10xxxxxx
				*d++ = (c & 0x3f) | 0x80; // 10xxxxxx
			}
			else
			{
				d = (void *)_everything3_safe_size_add((SIZE_T)d,4);
			}
		}
		else
		if (c > 0x7ff)
		{
			// 3 bytes
			if (buf)
			{
				*d++ = ((c >> 12) & 0x0f) | 0xE0; // 1110xxxx
				*d++ = ((c >> 6) & 0x3f) | 0x80; // 10xxxxxx
				*d++ = (c & 0x3f) | 0x80; // 10xxxxxx
			}
			else
			{
				d = (void *)_everything3_safe_size_add((SIZE_T)d,3);
			}
		}
		else
		if (c > 0x7f)
		{
			// 2 bytes
			if (buf)
			{
				*d++ = ((c >> 6) & 0x1f) | 0xC0; // 110xxxxx
				*d++ = (c & 0x3f) | 0x80; // 10xxxxxx
			}
			else
			{
				d = (void *)_everything3_safe_size_add((SIZE_T)d,2);
			}
		}
		else
		{	
			// ascii
			if (buf)
			{
				*d++ = c;
			}
			else
			{
				d = (void *)_everything3_safe_size_add((SIZE_T)d,1);
			}
		}
	}
	
	if (buf)
	{
		*d = 0;
	}
	
	return d;
}

// copy a wide string into a UTF-8 buffer.
// returns TRUE on success. Otherwise FALSE if there's not enough memory.
static BOOL _everything3_utf8_buf_copy_wchar_string(_everything3_utf8_buf_t *cbuf,const EVERYTHING3_WCHAR *ws)
{
	if (_everything3_utf8_buf_grow_length(cbuf,(SIZE_T)_everything3_utf8_string_copy_wchar_string(NULL,ws)))
	{
		_everything3_utf8_string_copy_wchar_string(cbuf->buf,ws);
		
		return TRUE;
	}
	
	return FALSE;
}

// copy a ANSI string into a UTF-8 buffer.
// returns TRUE on success. Otherwise FALSE if there's not enough memory.
static BOOL _everything3_utf8_buf_copy_ansi_string(_everything3_utf8_buf_t *cbuf,const EVERYTHING3_CHAR *as)
{
	BOOL ret;
	_everything3_wchar_buf_t wcbuf;

	ret = FALSE;
	_everything3_wchar_buf_init(&wcbuf);

	if (_everything3_wchar_buf_copy_ansi_string(&wcbuf,as))
	{
		if (_everything3_utf8_buf_copy_wchar_string(cbuf,wcbuf.buf))
		{
			ret = TRUE;
		}
	}
	
	_everything3_wchar_buf_kill(&wcbuf);
	
	return ret;
}

// return the number of folder results in a result list.
// Adding the folder count and file count returns the total number of results.
// This number can be more than the viewport count.
EVERYTHING3_USERAPI SIZE_T EVERYTHING3_API Everything3_GetResultListFolderCount(const EVERYTHING3_RESULT_LIST *result_list)
{
	return result_list->folder_result_count;
}

// return the number of file results in a result list.
// Adding the folder count and file count returns the total number of results.
// This number can be more than the viewport count.
EVERYTHING3_USERAPI SIZE_T EVERYTHING3_API Everything3_GetResultListFileCount(const EVERYTHING3_RESULT_LIST *result_list)
{
	return result_list->file_result_count;
}

// return the total number of file and folder results in a result list.
// This number can be more than the viewport count.
EVERYTHING3_USERAPI SIZE_T EVERYTHING3_API Everything3_GetResultListCount(const EVERYTHING3_RESULT_LIST *result_list)
{
	return result_list->folder_result_count + result_list->file_result_count;
}

// returns the total result size in bytes.
// returns EVERYTHING3_UINT64_MAX if the size is unknown or not requested.
// use Everything3_SetSearchRequestTotalSize() to request and calculate the total size.
EVERYTHING3_USERAPI EVERYTHING3_UINT64 EVERYTHING3_API Everything3_GetResultListTotalSize(const EVERYTHING3_RESULT_LIST *result_list)
{
	return result_list->total_result_size;
}

// get the result list viewport offset.
// useful for scrolling a window of results.
EVERYTHING3_USERAPI SIZE_T EVERYTHING3_API Everything3_GetResultListViewportOffset(const EVERYTHING3_RESULT_LIST *result_list)
{
	return result_list->viewport_offset;
}

// get the result list viewport count.
// useful for scrolling a window of results or limiting the number of results to send over IPC.
EVERYTHING3_USERAPI SIZE_T EVERYTHING3_API Everything3_GetResultListViewportCount(const EVERYTHING3_RESULT_LIST *result_list)
{
	return result_list->viewport_count;
}

// get the number of sorts
// we only allow a maximum of 3 sorts to be requested.
// However, searching for sort: can support up to 8 sorts.
// sort: in the search text will override any requested sorts.
// this can return up to a maximum of 8 sorts.
EVERYTHING3_USERAPI SIZE_T EVERYTHING3_API Everything3_GetResultListSortCount(const EVERYTHING3_RESULT_LIST *result_list)
{
	return result_list->sort_count;
}

// get the sort property ID for the specified sort index.
// sort_index MUST be less than Everything3_GetResultListSortCount()
// sort_index is 0 based.
EVERYTHING3_USERAPI DWORD EVERYTHING3_API Everything3_GetResultListSortPropertyId(const EVERYTHING3_RESULT_LIST *result_list,EVERYTHING3_SIZE_T sort_index)
{
	DWORD ret;
	
	ret = EVERYTHING3_INVALID_PROPERTY_ID;
	
	if (sort_index < result_list->sort_count)
	{
		ret = result_list->sort_array[sort_index].property_id;
	}
	else
	{
		SetLastError(EVERYTHING3_ERROR_INVALID_PARAMETER);
	}

	return ret;
}

// get the sort direction for the specified sort index.
// returns TRUE if sort ascending. Otherwise, sort descending.
// sort_index MUST be less than Everything3_GetResultListSortCount()
// sort_index is 0 based.
EVERYTHING3_USERAPI BOOL EVERYTHING3_API Everything3_GetResultListSortAscending(const EVERYTHING3_RESULT_LIST *result_list,EVERYTHING3_SIZE_T sort_index)
{
	BOOL ret;
	
	ret = FALSE;
	
	if (sort_index < result_list->sort_count)
	{
		if (result_list->sort_array[sort_index].flags & _EVERYTHING3_SEARCH_SORT_FLAG_DESCENDING)
		{
			SetLastError(EVERYTHING3_OK);
		}
		else
		{
			ret = TRUE;
		}
	}
	else
	{
		SetLastError(EVERYTHING3_ERROR_INVALID_PARAMETER);
	}

	return ret;
}

EVERYTHING3_USERAPI SIZE_T EVERYTHING3_API Everything3_GetResultListPropertyRequestCount(const EVERYTHING3_RESULT_LIST *result_list)
{
	return result_list->property_request_count;
}

EVERYTHING3_USERAPI DWORD EVERYTHING3_API Everything3_GetResultListPropertyRequestPropertyId(const EVERYTHING3_RESULT_LIST *result_list,EVERYTHING3_SIZE_T property_index)
{
	DWORD ret;
	
	ret = EVERYTHING3_INVALID_PROPERTY_ID;
	
	if (property_index < result_list->property_request_count)
	{
		ret = result_list->property_request_array[property_index].property_id;
	}
	else
	{
		SetLastError(EVERYTHING3_ERROR_INVALID_PARAMETER);
	}

	return ret;
}

EVERYTHING3_USERAPI DWORD EVERYTHING3_API Everything3_GetResultListPropertyRequestValueType(const EVERYTHING3_RESULT_LIST *result_list,EVERYTHING3_SIZE_T property_index)
{
	DWORD ret;
	
	ret = EVERYTHING3_INVALID_PROPERTY_ID;
	
	if (property_index < result_list->property_request_count)
	{
		ret = result_list->property_request_array[property_index].value_type;
	}
	else
	{
		SetLastError(EVERYTHING3_ERROR_INVALID_PARAMETER);
	}

	return ret;
}

// returns SIZE_MAX on error.
EVERYTHING3_USERAPI SIZE_T EVERYTHING3_API Everything3_GetResultListPropertyRequestOffset(const EVERYTHING3_RESULT_LIST *result_list,EVERYTHING3_SIZE_T property_index)
{
	SIZE_T ret;
	
	ret = SIZE_MAX;
	
	if (property_index < result_list->property_request_count)
	{
		ret = result_list->property_request_array[property_index].offset;
	}
	else
	{
		SetLastError(EVERYTHING3_ERROR_INVALID_PARAMETER);
	}

	return ret;
}

// Read some data from the IPC Pipe.
static void _everything3_stream_read_data(_everything3_stream_t *stream,void *data,SIZE_T size)
{
	BYTE *d;
	SIZE_T run;
	
	d = (BYTE *)data;
	run = size;
	
	while(run)
	{
		SIZE_T chunk_size;
		
		if (!stream->avail)
		{
			_everything3_message_t recv_header;
			
			if (stream->got_last)
			{
				// read passed EOF
				_everything3_zero_memory(d,run);
				
				stream->error_code = EVERYTHING3_ERROR_BAD_RESPONSE;
			
				return;
			}
			
			if (!_everything3_recv_header(stream->client,&recv_header))
			{
				// read header failed.
				_everything3_zero_memory(d,run);
				
				stream->error_code = GetLastError();
				
				break;
			}

			// last chunk?
			if (recv_header.code == _EVERYTHING3_RESPONSE_OK)
			{
				stream->got_last = 1;
			}
			
			if (recv_header.size)			
			{
//TODO: don't reallocate
				if (stream->buf)
				{
					_everything3_mem_free(stream->buf);
				}
			
				stream->buf = _everything3_mem_alloc(recv_header.size);
				if (!stream->buf)
				{
					_everything3_zero_memory(d,run);
					
					stream->error_code = EVERYTHING3_ERROR_OUT_OF_MEMORY;
					
					break;
				}
						
				if (!_everything3_recv_data(stream->client,stream->buf,recv_header.size))
				{
					_everything3_zero_memory(d,run);

					stream->error_code = GetLastError();
					
					break;
				}
				
				stream->p = stream->buf;
				stream->avail = recv_header.size;
			}
		}
		
		// stream->avail can be zero if we received a zero-sized data message.
		
		chunk_size = run;
		if (chunk_size > stream->avail)
		{
			chunk_size = stream->avail;
		}
		
		CopyMemory(d,stream->p,chunk_size);
		
		stream->p += chunk_size;
		stream->avail -= chunk_size;
		
		d += chunk_size;
		run -= chunk_size;
	}
}

static BYTE _everything3_stream_read_byte(_everything3_stream_t *stream)
{
	BYTE value;
	
	_everything3_stream_read_data(stream,&value,sizeof(BYTE));	
	
	return value;
}

static EVERYTHING3_WORD _everything3_stream_read_word(_everything3_stream_t *stream)
{
	EVERYTHING3_WORD value;
	
	_everything3_stream_read_data(stream,&value,sizeof(EVERYTHING3_WORD));	
	
	return value;
}

static DWORD _everything3_stream_read_dword(_everything3_stream_t *stream)
{
	DWORD value;
	
	_everything3_stream_read_data(stream,&value,sizeof(DWORD));	
	
	return value;
}

static EVERYTHING3_UINT64 _everything3_stream_read_uint64(_everything3_stream_t *stream)
{
	EVERYTHING3_UINT64 value;
	
	_everything3_stream_read_data(stream,&value,sizeof(EVERYTHING3_UINT64));	
	
	return value;
}

// get a SIZE_T, where the size can differ to Everything.
// we set the error code if the value would overflow. (we are 32bit and Everything is 64bit and the value is > 0xffffffff)
static SIZE_T _everything3_stream_read_size_t(_everything3_stream_t *stream)
{
	SIZE_T ret;
	
	ret = SIZE_MAX;
	
	if (stream->is_64bit)
	{
		EVERYTHING3_UINT64 uint64_value;
		
		uint64_value = _everything3_stream_read_uint64(stream);	
					
#if SIZE_MAX == 0xFFFFFFFFFFFFFFFFUI64

		ret = uint64_value;

#elif SIZE_MAX == 0xFFFFFFFF

		if (uint64_value <= SIZE_MAX)
		{
			ret = (SIZE_T)uint64_value;
		}
		else
		{
			stream->error_code = EVERYTHING3_ERROR_OUT_OF_MEMORY;
		}
		
#else

		#error unknown SIZE_MAX
		
#endif		
	}
	else
	{
		ret = _everything3_stream_read_dword(stream);	
	}

	return ret;
}

// read a variable length quantity.
// Doesn't have to be too efficient as the data will follow immediately.
// Sets the error code if the length would overflow (32bit dll, 64bit Everything, len > 0xffffffff )
static SIZE_T _everything3_stream_read_len_vlq(_everything3_stream_t *stream)
{
	BYTE byte_value;
	WORD word_value;
	DWORD dword_value;
	SIZE_T start;
	
	start = 0;

	// BYTE
	byte_value = _everything3_stream_read_byte(stream);
	
	if (byte_value < 0xff)
	{
		return byte_value;
	}

	// WORD
	start = _everything3_safe_size_add(start,0xff);
	
	word_value = _everything3_stream_read_word(stream);
	
	if (word_value < 0xffff)
	{
		return _everything3_safe_size_add(start,word_value);
	}	
	
	// DWORD
	start = _everything3_safe_size_add(start,0xffff);

	dword_value = _everything3_stream_read_dword(stream);
	
	if (dword_value < 0xffffffff)
	{
		return _everything3_safe_size_add(start,dword_value);
	}

#if SIZE_MAX == 0xFFFFFFFFFFFFFFFFUI64

	{
		EVERYTHING3_UINT64 uint64_value;
		
		// UINT64
		start = _everything3_safe_size_add(start,0xffffffff);

		uint64_value = _everything3_stream_read_uint64(stream);

		if (uint64_value < 0xFFFFFFFFFFFFFFFFUI64)
		{
			return _everything3_safe_size_add(start,uint64_value);
		}

		stream->error_code = EVERYTHING3_ERROR_OUT_OF_MEMORY;
		return EVERYTHING3_UINT64_MAX;
	}
	
#elif SIZE_MAX == 0xffffffffui32
		
	stream->error_code = EVERYTHING3_ERROR_OUT_OF_MEMORY;
	return EVERYTHING3_DWORD_MAX;

#else

	#error unknown UINTPTR_MAX

#endif
}

// fill in a buffer with a VLQ value and progress the buffer pointer.
static BYTE *_everything3_copy_len_vlq(BYTE *buf,SIZE_T value)
{
	BYTE *d;
	
	d = buf;
	
	if (value < 0xff)
	{
		if (buf)
		{
			*d++ = (BYTE)value;
		}
		else
		{
			d++;
		}
		
		return d;
	}
	
	value -= 0xff;

	if (buf)
	{
		*d++ = 0xff;
	}
	else
	{
		d++;
	}
	
	if (value < 0xffff)
	{
		if (buf)
		{
			*d++ = (BYTE)(value >> 8);
			*d++ = (BYTE)(value);
		}
		else
		{
			d += 2;
		}
		
		return d;
	}
	
	value -= 0xffff;

	if (buf)
	{
		*d++ = 0xff;
		*d++ = 0xff;
	}
	else
	{
		d += 2;
	}
	
	if (value < 0xffffffff)
	{
		if (buf)
		{
			*d++ = (BYTE)(value >> 24);
			*d++ = (BYTE)(value >> 16);
			*d++ = (BYTE)(value >> 8);
			*d++ = (BYTE)(value);
		}
		else
		{
			d += 4;
		}
		
		return d;
	}
	
	value -= 0xffffffff;

	if (buf)
	{
		*d++ = 0xff;
		*d++ = 0xff;
		*d++ = 0xff;
		*d++ = 0xff;
	}
	else
	{
		d += 4;
	}
	
	// value cannot be larger than or equal to 0xffffffffffffffff
#if SIZE_MAX == 0xFFFFFFFFFFFFFFFFUI64

	if (buf)
	{
		*d++ = (BYTE)(value >> 56);
		*d++ = (BYTE)(value >> 48);
		*d++ = (BYTE)(value >> 40);
		*d++ = (BYTE)(value >> 32);
		*d++ = (BYTE)(value >> 24);
		*d++ = (BYTE)(value >> 16);
		*d++ = (BYTE)(value >> 8);
		*d++ = (BYTE)(value);
	}
	else
	{
		d += 8;
	}
	

#elif SIZE_MAX == 0xFFFFFFFF

	if (buf)
	{
		*d++ = 0x00;
		*d++ = 0x00;
		*d++ = 0x00;
		*d++ = 0x00;
		*d++ = 0xff;
		*d++ = 0xff;
		*d++ = 0xff;
		*d++ = 0xff;
	}
	else
	{
		d += 8;
	}
	
#else
	#error unknown SIZE_MAX
#endif

	return d;
}

static BYTE *_everything3_copy_dword(BYTE *buf,DWORD value)
{
	return _everything3_copy_memory(buf,&value,sizeof(DWORD));
}

static BYTE *_everything3_copy_uint64(BYTE *buf,EVERYTHING3_UINT64 value)
{
	return _everything3_copy_memory(buf,&value,sizeof(EVERYTHING3_UINT64));
}

static BYTE *_everything3_copy_size_t(BYTE *buf,SIZE_T value)
{
#if SIZE_MAX == 0xFFFFFFFFFFFFFFFFUI64
	
	return _everything3_copy_uint64(buf,(EVERYTHING3_UINT64)value);
	
#elif SIZE_MAX == 0xFFFFFFFF

	return _everything3_copy_dword(buf,(DWORD)value);
	
#else
	#error unknown SIZE_MAX
#endif

}

// Is the result list item a folder or a file?
EVERYTHING3_USERAPI BOOL EVERYTHING3_API Everything3_IsFolderResult(const EVERYTHING3_RESULT_LIST *result_list,SIZE_T result_index)
{
	BOOL ret;
	
	ret = FALSE;
	
	if ((result_list) && (result_index < result_list->viewport_count))
	{
		BYTE *property_p;
		
		property_p = result_list->item_array[result_index].property_data;
		
		if (*property_p & _EVERYTHING3_RESULT_LIST_ITEM_FLAG_FOLDER)
		{
			return TRUE;
		}
	}
	else
	{
		SetLastError(EVERYTHING3_ERROR_INVALID_PARAMETER);
	}
		
	return ret;
}

// is the result list item a root item (has no parent)
EVERYTHING3_USERAPI BOOL EVERYTHING3_API Everything3_IsRootResult(const EVERYTHING3_RESULT_LIST *result_list,SIZE_T result_index)
{
	BOOL ret;
	
	ret = FALSE;
	
	if ((result_list) && (result_index < result_list->viewport_count))
	{
		BYTE *property_p;
		
		property_p = result_list->item_array[result_index].property_data;
		
		if (*property_p & _EVERYTHING3_RESULT_LIST_ITEM_FLAG_ROOT)
		{
			return TRUE;
		}
	}
	else
	{
		SetLastError(EVERYTHING3_ERROR_INVALID_PARAMETER);
	}
		
	return ret;
}

// binary search indexes for the specified compare data.
static void *_everything3_binary_search(void **array_base,SIZE_T count,void *compare_data,int (*compare_proc)(const void *a,const void *b))
{
	SIZE_T blo;
	SIZE_T bhi;
	
	// binary search
	blo = 0;
	bhi = count;

	while(blo < bhi)
	{
		SIZE_T bpos;
		int i;
		
		bpos = blo + ((bhi - blo) / 2);

		i = compare_proc(array_base[bpos],compare_data);
		
		if (i > 0)
		{
			bhi = bpos;
		}
		else
		if (!i)
		{
			// already in the list!
			return array_base[bpos];
		}
		else
		{
			blo = bpos + 1;
		}
	}
	
	return NULL;
}

// find a property request from a property ID.
static _everything3_result_list_property_request_t *_everything3_find_property_request_from_property_id(const EVERYTHING3_RESULT_LIST *result_list,DWORD property_id,BOOL highlight,BOOL format)
{
	_everything3_result_list_property_request_t compare_data;
	
	compare_data.property_id = property_id;
	compare_data.flags = 0;
	
	if (highlight)
	{
		compare_data.flags |=_EVERYTHING3_SEARCH_PROPERTY_REQUEST_FLAG_HIGHLIGHT;
	}

	if (format)
	{
		compare_data.flags |=_EVERYTHING3_SEARCH_PROPERTY_REQUEST_FLAG_FORMAT;
	}

	return _everything3_binary_search(result_list->sorted_property_request_array,result_list->property_request_count,&compare_data,_everything3_result_list_property_request_compare);
}

// sets last error.
// can return a NULL pstring.
static BOOL _everything3_get_item_property_text(const EVERYTHING3_RESULT_LIST *result_list,SIZE_T result_index,DWORD property_id,BOOL highlight,BOOL format,_everything3_utf8_pstring_t **pstring)
{
	BOOL ret;
	
	ret = FALSE;
	
	if ((result_list) && (result_index < result_list->viewport_count))
	{
		_everything3_result_list_property_request_t *property_request;

		property_request = _everything3_find_property_request_from_property_id(result_list,property_id,highlight,format);
		
		if (property_request)
		{
			BYTE *property_p;
			
			property_p = ((BYTE *)(result_list->item_array[result_index].property_data)) + property_request->offset;
			
			switch(property_request->value_type)
			{
				case EVERYTHING3_PROPERTY_VALUE_TYPE_PSTRING: 
				case EVERYTHING3_PROPERTY_VALUE_TYPE_PSTRING_MULTISTRING: 
				case EVERYTHING3_PROPERTY_VALUE_TYPE_PSTRING_STRING_REFERENCE:
				case EVERYTHING3_PROPERTY_VALUE_TYPE_PSTRING_FOLDER_REFERENCE:
				case EVERYTHING3_PROPERTY_VALUE_TYPE_PSTRING_FILE_OR_FOLDER_REFERENCE:
				
					// memory is unaligned.
					_everything3_copy_memory(pstring,property_p,sizeof(_everything3_utf8_pstring_t *));
					ret = TRUE;
					break;

				default:
					SetLastError(EVERYTHING3_ERROR_INVALID_PROPERTY_VALUE_TYPE);
					break; 
			}
		}
		else
		{
			SetLastError(EVERYTHING3_ERROR_INVALID_PARAMETER);
		}
	}
	else
	{
		SetLastError(EVERYTHING3_ERROR_PROPERTY_NOT_FOUND);
	}
		
	return ret;
}

// Get the text from a property.
static SIZE_T _everything3_get_item_property_text_utf8(const EVERYTHING3_RESULT_LIST *result_list,SIZE_T result_index,DWORD property_id,BOOL highlight,BOOL format,EVERYTHING3_UTF8 *buf,EVERYTHING3_SIZE_T bufsize)
{
	SIZE_T ret;
	_everything3_utf8_pstring_t *pstring;
	
	ret = 0;
	
	if (_everything3_get_item_property_text(result_list,result_index,property_id,highlight,format,&pstring))
	{
		ret = _everything3_safe_utf8_string_copy_utf8_string_n(buf,bufsize,_everything3_utf8_pstring_get_text(pstring),_everything3_utf8_pstring_get_len(pstring));
	}
	else
	{
		if ((buf) && (bufsize))
		{
			*buf = 0;
		}
	}
	
	return ret;
}

static SIZE_T _everything3_get_item_property_text_wchar(const EVERYTHING3_RESULT_LIST *result_list,SIZE_T result_index,DWORD property_id,BOOL highlight,BOOL format,EVERYTHING3_WCHAR *wbuf,SIZE_T wbuf_size_in_wchars)
{
	SIZE_T ret;
	_everything3_utf8_pstring_t *pstring;
	
	ret = 0;
	
	if (_everything3_get_item_property_text(result_list,result_index,property_id,highlight,format,&pstring))
	{
		ret = _everything3_safe_wchar_string_copy_utf8_string_n(wbuf,wbuf_size_in_wchars,_everything3_utf8_pstring_get_text(pstring),_everything3_utf8_pstring_get_len(pstring));
	}
	else
	{
		if ((wbuf) && (wbuf_size_in_wchars))
		{
			*wbuf = 0;
		}
	}
	
	return ret;
}

static SIZE_T _everything3_get_item_property_text_ansi(const EVERYTHING3_RESULT_LIST *result_list,SIZE_T result_index,DWORD property_id,BOOL highlight,BOOL format,EVERYTHING3_CHAR *buf,EVERYTHING3_SIZE_T bufsize)
{
	SIZE_T ret;
	_everything3_utf8_pstring_t *pstring;
	
	ret = 0;
	
	if (_everything3_get_item_property_text(result_list,result_index,property_id,highlight,format,&pstring))
	{
		ret = _everything3_safe_ansi_string_copy_utf8_string_n(buf,bufsize,_everything3_utf8_pstring_get_text(pstring),_everything3_utf8_pstring_get_len(pstring));
	}
	else
	{
		if ((buf) && (bufsize))
		{
			*buf = 0;
		}
	}
	
	return ret;
}

EVERYTHING3_USERAPI SIZE_T EVERYTHING3_API Everything3_GetResultPropertyTextUTF8(const EVERYTHING3_RESULT_LIST *result_list,SIZE_T result_index,DWORD property_id,EVERYTHING3_UTF8 *buf,EVERYTHING3_SIZE_T bufsize)
{
	return _everything3_get_item_property_text_utf8(result_list,result_index,property_id,FALSE,FALSE,buf,bufsize);
}

EVERYTHING3_USERAPI SIZE_T EVERYTHING3_API Everything3_GetResultPropertyTextW(const EVERYTHING3_RESULT_LIST *result_list,SIZE_T result_index,DWORD property_id,EVERYTHING3_WCHAR *wbuf,SIZE_T wbuf_size_in_wchars)
{
	return _everything3_get_item_property_text_wchar(result_list,result_index,property_id,FALSE,FALSE,wbuf,wbuf_size_in_wchars);
}

EVERYTHING3_USERAPI SIZE_T EVERYTHING3_API Everything3_GetResultPropertyTextA(const EVERYTHING3_RESULT_LIST *result_list,SIZE_T result_index,DWORD property_id,EVERYTHING3_CHAR *buf,EVERYTHING3_SIZE_T bufsize)
{
	return _everything3_get_item_property_text_ansi(result_list,result_index,property_id,FALSE,FALSE,buf,bufsize);
}

EVERYTHING3_USERAPI SIZE_T EVERYTHING3_API Everything3_GetResultPropertyTextFormattedUTF8(const EVERYTHING3_RESULT_LIST *result_list,SIZE_T result_index,DWORD property_id,EVERYTHING3_UTF8 *buf,EVERYTHING3_SIZE_T bufsize)
{
	return _everything3_get_item_property_text_utf8(result_list,result_index,property_id,FALSE,TRUE,buf,bufsize);
}

EVERYTHING3_USERAPI SIZE_T EVERYTHING3_API Everything3_GetResultPropertyTextFormattedW(const EVERYTHING3_RESULT_LIST *result_list,SIZE_T result_index,DWORD property_id,EVERYTHING3_WCHAR *wbuf,SIZE_T wbuf_size_in_wchars)
{
	return _everything3_get_item_property_text_wchar(result_list,result_index,property_id,FALSE,TRUE,wbuf,wbuf_size_in_wchars);
}

EVERYTHING3_USERAPI SIZE_T EVERYTHING3_API Everything3_GetResultPropertyTextFormattedA(const EVERYTHING3_RESULT_LIST *result_list,SIZE_T result_index,DWORD property_id,EVERYTHING3_CHAR *buf,EVERYTHING3_SIZE_T bufsize)
{
	return _everything3_get_item_property_text_ansi(result_list,result_index,property_id,FALSE,TRUE,buf,bufsize);
}

EVERYTHING3_USERAPI SIZE_T EVERYTHING3_API Everything3_GetResultPropertyTextHighlightedUTF8(const EVERYTHING3_RESULT_LIST *result_list,SIZE_T result_index,DWORD property_id,EVERYTHING3_UTF8 *buf,EVERYTHING3_SIZE_T bufsize)
{
	return _everything3_get_item_property_text_utf8(result_list,result_index,property_id,TRUE,TRUE,buf,bufsize);
}

EVERYTHING3_USERAPI SIZE_T EVERYTHING3_API Everything3_GetResultPropertyTextHighlightedW(const EVERYTHING3_RESULT_LIST *result_list,SIZE_T result_index,DWORD property_id,EVERYTHING3_WCHAR *wbuf,SIZE_T wbuf_size_in_wchars)
{
	return _everything3_get_item_property_text_wchar(result_list,result_index,property_id,TRUE,TRUE,wbuf,wbuf_size_in_wchars);
}

EVERYTHING3_USERAPI SIZE_T EVERYTHING3_API Everything3_GetResultPropertyTextHighlightedA(const EVERYTHING3_RESULT_LIST *result_list,SIZE_T result_index,DWORD property_id,EVERYTHING3_CHAR *buf,EVERYTHING3_SIZE_T bufsize)
{
	return _everything3_get_item_property_text_ansi(result_list,result_index,property_id,TRUE,TRUE,buf,bufsize);
}

EVERYTHING3_USERAPI BYTE EVERYTHING3_API Everything3_GetResultPropertyBYTE(const EVERYTHING3_RESULT_LIST *result_list,SIZE_T result_index,DWORD property_id)
{
	BYTE ret;
	
	ret = EVERYTHING3_BYTE_MAX;
	
	if ((result_list) && (result_index < result_list->viewport_count))
	{
		_everything3_result_list_property_request_t *property_request;

		property_request = _everything3_find_property_request_from_property_id(result_list,property_id,FALSE,FALSE);
		
		if (property_request)
		{
			BYTE *property_p;
			
			property_p = ((BYTE *)(result_list->item_array[result_index].property_data)) + property_request->offset;
			
			switch(property_request->value_type)
			{
				case EVERYTHING3_PROPERTY_VALUE_TYPE_BYTE: 
				case EVERYTHING3_PROPERTY_VALUE_TYPE_BYTE_GET_TEXT: 
					
					{
						BYTE byte_value;
						
						// memory is unaligned.
						_everything3_copy_memory(&byte_value,property_p,sizeof(BYTE));
						
						ret = byte_value;
						
						if (ret == EVERYTHING3_BYTE_MAX)
						{
							SetLastError(EVERYTHING3_OK);
						}
					}
					break;
				
				default:
					SetLastError(EVERYTHING3_ERROR_INVALID_PROPERTY_VALUE_TYPE);
					break;
			}
		}
		else
		{
			SetLastError(EVERYTHING3_ERROR_INVALID_PARAMETER);
		}
	}
	else
	{
		SetLastError(EVERYTHING3_ERROR_INVALID_PARAMETER);
	}
		
	return ret;
}

EVERYTHING3_USERAPI WORD EVERYTHING3_API Everything3_GetResultPropertyWORD(const EVERYTHING3_RESULT_LIST *result_list,SIZE_T result_index,DWORD property_id)
{
	WORD ret;
	
	ret = EVERYTHING3_WORD_MAX;
	
	if ((result_list) && (result_index < result_list->viewport_count))
	{
		_everything3_result_list_property_request_t *property_request;

		property_request = _everything3_find_property_request_from_property_id(result_list,property_id,FALSE,FALSE);
		
		if (property_request)
		{
			BYTE *property_p;
			
			property_p = ((BYTE *)(result_list->item_array[result_index].property_data)) + property_request->offset;
			
			switch(property_request->value_type)
			{
				case EVERYTHING3_PROPERTY_VALUE_TYPE_WORD: 
				case EVERYTHING3_PROPERTY_VALUE_TYPE_WORD_GET_TEXT: 
					
					{
						WORD word_value;
						
						// memory is unaligned.
						_everything3_copy_memory(&word_value,property_p,sizeof(WORD));
						
						ret = word_value;
						
						if (ret == EVERYTHING3_WORD_MAX)
						{
							SetLastError(EVERYTHING3_OK);
						}
					}
					break;
				
				default:
					SetLastError(EVERYTHING3_ERROR_INVALID_PROPERTY_VALUE_TYPE);
					break;
			}
		}
		else
		{
			SetLastError(EVERYTHING3_ERROR_INVALID_PARAMETER);
		}
	}
	else
	{
		SetLastError(EVERYTHING3_ERROR_INVALID_PARAMETER);
	}
		
	return ret;
}

EVERYTHING3_USERAPI DWORD EVERYTHING3_API Everything3_GetResultPropertyDWORD(const EVERYTHING3_RESULT_LIST *result_list,SIZE_T result_index,DWORD property_id)
{
	DWORD ret;
	
	ret = EVERYTHING3_DWORD_MAX;
	
	if ((result_list) && (result_index < result_list->viewport_count))
	{
		_everything3_result_list_property_request_t *property_request;

		property_request = _everything3_find_property_request_from_property_id(result_list,property_id,FALSE,FALSE);
		
		if (property_request)
		{
			BYTE *property_p;
			
			property_p = ((BYTE *)(result_list->item_array[result_index].property_data)) + property_request->offset;
			
			switch(property_request->value_type)
			{
				case EVERYTHING3_PROPERTY_VALUE_TYPE_DWORD: 
				case EVERYTHING3_PROPERTY_VALUE_TYPE_DWORD_FIXED_Q1K: 
				case EVERYTHING3_PROPERTY_VALUE_TYPE_DWORD_GET_TEXT: 
					{
						DWORD dword_value;
						
						// memory is unaligned.
						_everything3_copy_memory(&dword_value,property_p,sizeof(DWORD));
						
						ret = dword_value;
						
						if (ret == EVERYTHING3_DWORD_MAX)
						{
							SetLastError(EVERYTHING3_OK);
						}
					}
					break;
/*
				case EVERYTHING3_PROPERTY_VALUE_TYPE_WORD: 
				case EVERYTHING3_PROPERTY_VALUE_TYPE_WORD_GET_TEXT: 
					{
						WORD word_value;
						
						// memory is unaligned.
						_everything3_copy_memory(&word_value,property_p,sizeof(WORD));
						
						ret = word_value;
						
						if (ret == EVERYTHING3_WORD_MAX)
						{
							ret = EVERYTHING3_DWORD_MAX;
							SetLastError(EVERYTHING3_OK);
						}
					}
					break;
								
				case EVERYTHING3_PROPERTY_VALUE_TYPE_BYTE: 
				case EVERYTHING3_PROPERTY_VALUE_TYPE_BYTE_GET_TEXT: 
					{
						BYTE byte_value;
						
						// memory is unaligned.
						_everything3_copy_memory(&byte_value,property_p,sizeof(BYTE));
						
						ret = byte_value;
						
						if (ret == EVERYTHING3_BYTE_MAX)
						{
							ret = EVERYTHING3_DWORD_MAX;
							SetLastError(EVERYTHING3_OK);
						}
					}
					break;
						*/		
				default:
					SetLastError(EVERYTHING3_ERROR_INVALID_PROPERTY_VALUE_TYPE);
					break;
			}
		}
		else
		{
			SetLastError(EVERYTHING3_ERROR_INVALID_PARAMETER);
		}
	}
	else
	{
		SetLastError(EVERYTHING3_ERROR_INVALID_PARAMETER);
	}
		
	return ret;
}

EVERYTHING3_USERAPI EVERYTHING3_UINT64 EVERYTHING3_API Everything3_GetResultPropertyUINT64(const EVERYTHING3_RESULT_LIST *result_list,SIZE_T result_index,DWORD property_id)
{
	EVERYTHING3_UINT64 ret;
	
	ret = EVERYTHING3_UINT64_MAX;
	
	if ((result_list) && (result_index < result_list->viewport_count))
	{
		_everything3_result_list_property_request_t *property_request;

		property_request = _everything3_find_property_request_from_property_id(result_list,property_id,FALSE,FALSE);
		
		if (property_request)
		{
			BYTE *property_p;
			
			property_p = ((BYTE *)(result_list->item_array[result_index].property_data)) + property_request->offset;
			
			switch(property_request->value_type)
			{
				case EVERYTHING3_PROPERTY_VALUE_TYPE_UINT64: 
					{
						EVERYTHING3_UINT64 uint64_value;
						
						// memory is unaligned.
						_everything3_copy_memory(&uint64_value,property_p,sizeof(EVERYTHING3_UINT64));
						
						ret = uint64_value;
						
						if (ret == EVERYTHING3_UINT64_MAX)
						{
							SetLastError(EVERYTHING3_OK);
						}
					}
					break;
				
				default:
					SetLastError(EVERYTHING3_ERROR_INVALID_PROPERTY_VALUE_TYPE);
					break;
			}
		}
		else
		{
			SetLastError(EVERYTHING3_ERROR_INVALID_PARAMETER);
		}
	}
	else
	{
		SetLastError(EVERYTHING3_ERROR_INVALID_PARAMETER);
	}
		
	return ret;
}

EVERYTHING3_USERAPI BOOL EVERYTHING3_API Everything3_GetResultPropertyUINT128(const EVERYTHING3_RESULT_LIST *result_list,SIZE_T result_index,EVERYTHING3_DWORD property_id,EVERYTHING3_UINT128 *puint128)
{
	BOOL ret;
	
	ret = FALSE;
	
	if ((result_list) && (result_index < result_list->viewport_count) && (puint128))
	{
		_everything3_result_list_property_request_t *property_request;
		
		puint128->hi_uint64 = EVERYTHING3_UINT64_MAX;
		puint128->lo_uint64 = EVERYTHING3_UINT64_MAX;

		property_request = _everything3_find_property_request_from_property_id(result_list,property_id,FALSE,FALSE);
		
		if (property_request)
		{
			BYTE *property_p;
			
			property_p = ((BYTE *)(result_list->item_array[result_index].property_data)) + property_request->offset;
			
			switch(property_request->value_type)
			{
				case EVERYTHING3_PROPERTY_VALUE_TYPE_UINT128: 

					// memory is unaligned.
					_everything3_copy_memory(puint128,property_p,sizeof(EVERYTHING3_UINT128));
					
					ret = TRUE;

					break;
				
				default:
					SetLastError(EVERYTHING3_ERROR_INVALID_PROPERTY_VALUE_TYPE);
					break;
			}
		}
		else
		{
			SetLastError(EVERYTHING3_ERROR_INVALID_PARAMETER);
		}
	}
	else
	{
		SetLastError(EVERYTHING3_ERROR_INVALID_PARAMETER);
	}
		
	return ret;
}

EVERYTHING3_USERAPI BOOL EVERYTHING3_API Everything3_GetResultPropertyDIMENSIONS(const EVERYTHING3_RESULT_LIST *result_list,SIZE_T result_index,EVERYTHING3_DWORD property_id,EVERYTHING3_DIMENSIONS *dimensions)
{
	BOOL ret;
	
	ret = FALSE;
	
	if ((result_list) && (result_index < result_list->viewport_count) && (dimensions))
	{
		_everything3_result_list_property_request_t *property_request;
		
		dimensions->width = EVERYTHING3_DWORD_MAX;
		dimensions->height = EVERYTHING3_DWORD_MAX;

		property_request = _everything3_find_property_request_from_property_id(result_list,property_id,FALSE,FALSE);
		
		if (property_request)
		{
			BYTE *property_p;
			
			property_p = ((BYTE *)(result_list->item_array[result_index].property_data)) + property_request->offset;
			
			switch(property_request->value_type)
			{
				case EVERYTHING3_PROPERTY_VALUE_TYPE_DIMENSIONS: 

					// memory is unaligned.
					_everything3_copy_memory(dimensions,property_p,sizeof(EVERYTHING3_DIMENSIONS));
					
					ret = TRUE;

					break;
				
				default:
					_everything3_debug_printf("BAD property value type\n");
					SetLastError(EVERYTHING3_ERROR_INVALID_PROPERTY_VALUE_TYPE);
					break;
			}
		}
		else
		{
			SetLastError(EVERYTHING3_ERROR_INVALID_PARAMETER);
		}
	}
	else
	{
		SetLastError(EVERYTHING3_ERROR_INVALID_PARAMETER);
	}
		
	return ret;
}

EVERYTHING3_USERAPI SIZE_T EVERYTHING3_API Everything3_GetResultPropertySIZE_T(const EVERYTHING3_RESULT_LIST *result_list,SIZE_T result_index,DWORD property_id)
{
	SIZE_T ret;
	
	ret = SIZE_MAX;
	
	if ((result_list) && (result_index < result_list->viewport_count))
	{
		_everything3_result_list_property_request_t *property_request;

		property_request = _everything3_find_property_request_from_property_id(result_list,property_id,FALSE,FALSE);
		
		if (property_request)
		{
			BYTE *property_p;
			
			property_p = ((BYTE *)(result_list->item_array[result_index].property_data)) + property_request->offset;
			
			switch(property_request->value_type)
			{
				case EVERYTHING3_PROPERTY_VALUE_TYPE_SIZE_T: 
					if (result_list->valid_flags & _EVERYTHING3_SEARCH_FLAG_64BIT)
					{
						EVERYTHING3_UINT64 uint64_value;
						
						// memory is unaligned.
						_everything3_copy_memory(&uint64_value,property_p,sizeof(EVERYTHING3_UINT64));
						
#if SIZE_MAX == 0xFFFFFFFFFFFFFFFFUI64

						ret = uint64_value;

#elif SIZE_MAX == 0xFFFFFFFF

						if (uint64_value <= SIZE_MAX)
						{
							ret = (SIZE_T)uint64_value;
						}
						else
						{
							ret = SIZE_MAX;
						}
						
#else

		#error unknown SIZE_MAX
		
#endif
					}
					else
					{
						DWORD dword_value;
						
						// memory is unaligned.
						_everything3_copy_memory(&dword_value,property_p,sizeof(DWORD));
				
						ret = (SIZE_T)dword_value;
					}
				
					if (ret == SIZE_MAX)
					{
						SetLastError(EVERYTHING3_OK);
					}
					
					break;
				
				default:
					SetLastError(EVERYTHING3_ERROR_INVALID_PROPERTY_VALUE_TYPE);
					break;
			}
		}
		else
		{
			SetLastError(EVERYTHING3_ERROR_INVALID_PARAMETER);
		}
	}
	else
	{
		SetLastError(EVERYTHING3_ERROR_INVALID_PARAMETER);
	}
		
	return ret;
}

EVERYTHING3_USERAPI EVERYTHING3_INT32 EVERYTHING3_API Everything3_GetResultPropertyINT32(const EVERYTHING3_RESULT_LIST *result_list,SIZE_T result_index,EVERYTHING3_DWORD property_id)
{
	EVERYTHING3_INT32 ret;
	
	ret = EVERYTHING3_INT32_MIN;
	
	if ((result_list) && (result_index < result_list->viewport_count))
	{
		_everything3_result_list_property_request_t *property_request;

		property_request = _everything3_find_property_request_from_property_id(result_list,property_id,FALSE,FALSE);
		
		if (property_request)
		{
			BYTE *property_p;
			
			property_p = ((BYTE *)(result_list->item_array[result_index].property_data)) + property_request->offset;
			
			switch(property_request->value_type)
			{
				case EVERYTHING3_PROPERTY_VALUE_TYPE_INT32_FIXED_Q1K: 
				case EVERYTHING3_PROPERTY_VALUE_TYPE_INT32_FIXED_Q1M: 
					{
						EVERYTHING3_INT32 int32_value;
						
						// memory is unaligned.
						_everything3_copy_memory(&int32_value,property_p,sizeof(EVERYTHING3_INT32));
						
						ret = int32_value;
						
						if (ret == EVERYTHING3_INT32_MIN)
						{
							SetLastError(EVERYTHING3_OK);
						}
					}
					break;
				
				default:
					SetLastError(EVERYTHING3_ERROR_INVALID_PROPERTY_VALUE_TYPE);
					break;
			}
		}
		else
		{
			SetLastError(EVERYTHING3_ERROR_INVALID_PARAMETER);
		}
	}
	else
	{
		SetLastError(EVERYTHING3_ERROR_INVALID_PARAMETER);
	}
		
	return ret;
}

// if buf is NULL, bufsize will contain the required size of buffer in bytes.
// if buf is non-NULL, bufsize will contain the size copied into buffer in bytes.
// if there's not enough room, the function returns 0 and sets the last error to: EVERYTHING3_ERROR_INSUFFICIENT_BUFFER
// if there's an error, the return value is 0. Use GetLastError() to get the last error.
// same as CryptEncrypt
EVERYTHING3_USERAPI BOOL EVERYTHING3_API Everything3_GetResultPropertyBlob(const EVERYTHING3_RESULT_LIST *result_list,SIZE_T result_index,EVERYTHING3_DWORD property_id,EVERYTHING3_BYTE *buf,SIZE_T *pbufsize)
{
	BOOL ret;
	
	ret = FALSE;
	
	if ((result_list) && (result_index < result_list->viewport_count) && (pbufsize))
	{
		_everything3_result_list_property_request_t *property_request;
		SIZE_T old_buf_size;
		SIZE_T new_buf_size;

		old_buf_size = *pbufsize;
		new_buf_size = 0;
		
		property_request = _everything3_find_property_request_from_property_id(result_list,property_id,FALSE,FALSE);
		
		if (property_request)
		{
			BYTE *property_p;
			
			property_p = ((BYTE *)(result_list->item_array[result_index].property_data)) + property_request->offset;
			
			switch(property_request->value_type)
			{
				case EVERYTHING3_PROPERTY_VALUE_TYPE_BLOB8: 
					{
						_everything3_blob8_t *blob;
						
						blob = *(_everything3_blob8_t **)property_p;

						if (blob)
						{
							new_buf_size = blob->len;
							
							if (buf)
							{
								if (new_buf_size <= old_buf_size)
								{
									_everything3_copy_memory(buf,_EVERYTHING3_BLOB8_DATA(blob),new_buf_size);
									
									ret = TRUE;
								}
								else
								{
									_everything3_copy_memory(buf,_EVERYTHING3_BLOB8_DATA(blob),old_buf_size);
									
									SetLastError(EVERYTHING3_ERROR_INSUFFICIENT_BUFFER);
								}
							}
							else
							{
								ret = TRUE;
							}
						}
						else
						{
							new_buf_size = 0;
							
							ret = TRUE;
						}
					}
					break;
				
				case EVERYTHING3_PROPERTY_VALUE_TYPE_BLOB16: 
					{
						_everything3_blob16_t *blob;
						
						blob = *(_everything3_blob16_t **)property_p;

						if (blob)
						{
							new_buf_size = blob->len;
							
							if (buf)
							{
								if (new_buf_size <= old_buf_size)
								{
									_everything3_copy_memory(buf,_EVERYTHING3_BLOB16_DATA(blob),new_buf_size);
									
									ret = TRUE;
								}
								else
								{
									_everything3_copy_memory(buf,_EVERYTHING3_BLOB16_DATA(blob),old_buf_size);
									
									SetLastError(EVERYTHING3_ERROR_INSUFFICIENT_BUFFER);
								}
							}
							else
							{
								ret = TRUE;
							}
						}
						else
						{
							new_buf_size = 0;
							
							ret = TRUE;
						}
					}
					break;
				
				default:
					SetLastError(EVERYTHING3_ERROR_INVALID_PROPERTY_VALUE_TYPE);
					break;
			}
		}
		else
		{
			SetLastError(EVERYTHING3_ERROR_INVALID_PARAMETER);
		}

		*pbufsize = new_buf_size;
	}
	else
	{
		SetLastError(EVERYTHING3_ERROR_INVALID_PARAMETER);
	}
		
	return ret;
}

EVERYTHING3_USERAPI EVERYTHING3_SIZE_T EVERYTHING3_API Everything3_GetResultNameUTF8(const EVERYTHING3_RESULT_LIST *result_list,EVERYTHING3_SIZE_T result_index,EVERYTHING3_UTF8 *buf,EVERYTHING3_SIZE_T bufsize)
{
	return Everything3_GetResultPropertyTextUTF8(result_list,result_index,EVERYTHING3_PROPERTY_ID_NAME,buf,bufsize);
}

EVERYTHING3_USERAPI EVERYTHING3_SIZE_T EVERYTHING3_API Everything3_GetResultNameW(const EVERYTHING3_RESULT_LIST *result_list,EVERYTHING3_SIZE_T result_index,EVERYTHING3_WCHAR *wbuf,EVERYTHING3_SIZE_T wbuf_size_in_wchars)
{
	return Everything3_GetResultPropertyTextW(result_list,result_index,EVERYTHING3_PROPERTY_ID_NAME,wbuf,wbuf_size_in_wchars);
}

EVERYTHING3_USERAPI EVERYTHING3_SIZE_T EVERYTHING3_API Everything3_GetResultNameA(const EVERYTHING3_RESULT_LIST *result_list,EVERYTHING3_SIZE_T result_index,EVERYTHING3_CHAR *buf,EVERYTHING3_SIZE_T bufsize)
{
	return Everything3_GetResultPropertyTextA(result_list,result_index,EVERYTHING3_PROPERTY_ID_NAME,buf,bufsize);
}

EVERYTHING3_USERAPI EVERYTHING3_SIZE_T EVERYTHING3_API Everything3_GetResultPathUTF8(const EVERYTHING3_RESULT_LIST *result_list,EVERYTHING3_SIZE_T result_index,EVERYTHING3_UTF8 *buf,EVERYTHING3_SIZE_T bufsize)
{
	return Everything3_GetResultPropertyTextUTF8(result_list,result_index,EVERYTHING3_PROPERTY_ID_PATH,buf,bufsize);
}

EVERYTHING3_USERAPI EVERYTHING3_SIZE_T EVERYTHING3_API Everything3_GetResultPathW(const EVERYTHING3_RESULT_LIST *result_list,EVERYTHING3_SIZE_T result_index,EVERYTHING3_WCHAR *wbuf,EVERYTHING3_SIZE_T wbuf_size_in_wchars)
{
	return Everything3_GetResultPropertyTextW(result_list,result_index,EVERYTHING3_PROPERTY_ID_PATH,wbuf,wbuf_size_in_wchars);
}

EVERYTHING3_USERAPI EVERYTHING3_SIZE_T EVERYTHING3_API Everything3_GetResultPathA(const EVERYTHING3_RESULT_LIST *result_list,EVERYTHING3_SIZE_T result_index,EVERYTHING3_CHAR *buf,EVERYTHING3_SIZE_T bufsize)
{
	return Everything3_GetResultPropertyTextA(result_list,result_index,EVERYTHING3_PROPERTY_ID_PATH,buf,bufsize);
}

EVERYTHING3_USERAPI EVERYTHING3_SIZE_T EVERYTHING3_API Everything3_GetResultFullPathNameUTF8(const EVERYTHING3_RESULT_LIST *result_list,EVERYTHING3_SIZE_T result_index,EVERYTHING3_UTF8 *buf,EVERYTHING3_SIZE_T bufsize)
{
	return Everything3_GetResultPropertyTextUTF8(result_list,result_index,EVERYTHING3_PROPERTY_ID_FULL_PATH,buf,bufsize);
}

EVERYTHING3_USERAPI EVERYTHING3_SIZE_T EVERYTHING3_API Everything3_GetResultFullPathNameW(const EVERYTHING3_RESULT_LIST *result_list,EVERYTHING3_SIZE_T result_index,EVERYTHING3_WCHAR *wbuf,EVERYTHING3_SIZE_T wbuf_size_in_wchars)
{
	return Everything3_GetResultPropertyTextW(result_list,result_index,EVERYTHING3_PROPERTY_ID_FULL_PATH,wbuf,wbuf_size_in_wchars);
}

EVERYTHING3_USERAPI EVERYTHING3_SIZE_T EVERYTHING3_API Everything3_GetResultFullPathNameA(const EVERYTHING3_RESULT_LIST *result_list,EVERYTHING3_SIZE_T result_index,EVERYTHING3_CHAR *buf,EVERYTHING3_SIZE_T bufsize)
{
	return Everything3_GetResultPropertyTextA(result_list,result_index,EVERYTHING3_PROPERTY_ID_FULL_PATH,buf,bufsize);
}

EVERYTHING3_USERAPI EVERYTHING3_UINT64 EVERYTHING3_API Everything3_GetResultSize(const EVERYTHING3_RESULT_LIST *result_list,SIZE_T result_index)
{
	return Everything3_GetResultPropertyUINT64(result_list,result_index,EVERYTHING3_PROPERTY_ID_SIZE);
}

EVERYTHING3_USERAPI EVERYTHING3_SIZE_T EVERYTHING3_API Everything3_GetResultExtensionUTF8(const EVERYTHING3_RESULT_LIST *result_list,EVERYTHING3_SIZE_T result_index,EVERYTHING3_UTF8 *buf,EVERYTHING3_SIZE_T bufsize)
{
	return Everything3_GetResultPropertyTextUTF8(result_list,result_index,EVERYTHING3_PROPERTY_ID_EXTENSION,buf,bufsize);
}

EVERYTHING3_USERAPI EVERYTHING3_SIZE_T EVERYTHING3_API Everything3_GetResultExtensionW(const EVERYTHING3_RESULT_LIST *result_list,EVERYTHING3_SIZE_T result_index,EVERYTHING3_WCHAR *wbuf,EVERYTHING3_SIZE_T wbuf_size_in_wchars)
{
	return Everything3_GetResultPropertyTextW(result_list,result_index,EVERYTHING3_PROPERTY_ID_EXTENSION,wbuf,wbuf_size_in_wchars);
}

EVERYTHING3_USERAPI EVERYTHING3_SIZE_T EVERYTHING3_API Everything3_GetResultExtensionA(const EVERYTHING3_RESULT_LIST *result_list,EVERYTHING3_SIZE_T result_index,EVERYTHING3_CHAR *buf,EVERYTHING3_SIZE_T bufsize)
{
	return Everything3_GetResultPropertyTextA(result_list,result_index,EVERYTHING3_PROPERTY_ID_EXTENSION,buf,bufsize);
}

EVERYTHING3_USERAPI EVERYTHING3_SIZE_T EVERYTHING3_API Everything3_GetResultTypeUTF8(const EVERYTHING3_RESULT_LIST *result_list,EVERYTHING3_SIZE_T result_index,EVERYTHING3_UTF8 *buf,EVERYTHING3_SIZE_T bufsize)
{
	return Everything3_GetResultPropertyTextUTF8(result_list,result_index,EVERYTHING3_PROPERTY_ID_TYPE,buf,bufsize);
}

EVERYTHING3_USERAPI EVERYTHING3_SIZE_T EVERYTHING3_API Everything3_GetResultTypeW(const EVERYTHING3_RESULT_LIST *result_list,EVERYTHING3_SIZE_T result_index,EVERYTHING3_WCHAR *wbuf,EVERYTHING3_SIZE_T wbuf_size_in_wchars)
{
	return Everything3_GetResultPropertyTextW(result_list,result_index,EVERYTHING3_PROPERTY_ID_TYPE,wbuf,wbuf_size_in_wchars);
}

EVERYTHING3_USERAPI EVERYTHING3_SIZE_T EVERYTHING3_API Everything3_GetResultTypeA(const EVERYTHING3_RESULT_LIST *result_list,EVERYTHING3_SIZE_T result_index,EVERYTHING3_CHAR *buf,EVERYTHING3_SIZE_T bufsize)
{
	return Everything3_GetResultPropertyTextA(result_list,result_index,EVERYTHING3_PROPERTY_ID_TYPE,buf,bufsize);
}

EVERYTHING3_USERAPI EVERYTHING3_UINT64 EVERYTHING3_API Everything3_GetResultDateModified(const EVERYTHING3_RESULT_LIST *result_list,SIZE_T result_index)
{
	return Everything3_GetResultPropertyUINT64(result_list,result_index,EVERYTHING3_PROPERTY_ID_DATE_MODIFIED);
}

EVERYTHING3_USERAPI EVERYTHING3_UINT64 EVERYTHING3_API Everything3_GetResultDateCreated(const EVERYTHING3_RESULT_LIST *result_list,SIZE_T result_index)
{
	return Everything3_GetResultPropertyUINT64(result_list,result_index,EVERYTHING3_PROPERTY_ID_DATE_CREATED);
}

EVERYTHING3_USERAPI EVERYTHING3_UINT64 EVERYTHING3_API Everything3_GetResultDateAccessed(const EVERYTHING3_RESULT_LIST *result_list,SIZE_T result_index)
{
	return Everything3_GetResultPropertyUINT64(result_list,result_index,EVERYTHING3_PROPERTY_ID_DATE_ACCESSED);
}

EVERYTHING3_USERAPI DWORD EVERYTHING3_API Everything3_GetResultAttributes(const EVERYTHING3_RESULT_LIST *result_list,SIZE_T result_index)
{
	return Everything3_GetResultPropertyDWORD(result_list,result_index,EVERYTHING3_PROPERTY_ID_ATTRIBUTES);
}

EVERYTHING3_USERAPI EVERYTHING3_UINT64 EVERYTHING3_API Everything3_GetResultDateRecentlyChanged(const EVERYTHING3_RESULT_LIST *result_list,SIZE_T result_index)
{
	return Everything3_GetResultPropertyUINT64(result_list,result_index,EVERYTHING3_PROPERTY_ID_DATE_RECENTLY_CHANGED);
}

EVERYTHING3_USERAPI DWORD EVERYTHING3_API Everything3_GetResultRunCount(const EVERYTHING3_RESULT_LIST *result_list,SIZE_T result_index)
{
	return Everything3_GetResultPropertyDWORD(result_list,result_index,EVERYTHING3_PROPERTY_ID_RUN_COUNT);
}

EVERYTHING3_USERAPI EVERYTHING3_UINT64 EVERYTHING3_API Everything3_GetResultDateRun(const EVERYTHING3_RESULT_LIST *result_list,SIZE_T result_index)
{
	return Everything3_GetResultPropertyUINT64(result_list,result_index,EVERYTHING3_PROPERTY_ID_DATE_RUN);
}

EVERYTHING3_USERAPI EVERYTHING3_SIZE_T EVERYTHING3_API Everything3_GetResultFilelistFilenameUTF8(const EVERYTHING3_RESULT_LIST *result_list,EVERYTHING3_SIZE_T result_index,EVERYTHING3_UTF8 *buf,EVERYTHING3_SIZE_T bufsize)
{
	return Everything3_GetResultPropertyTextUTF8(result_list,result_index,EVERYTHING3_PROPERTY_ID_FILE_LIST_FILENAME,buf,bufsize);
}

EVERYTHING3_USERAPI EVERYTHING3_SIZE_T EVERYTHING3_API Everything3_GetResultFilelistFilenameW(const EVERYTHING3_RESULT_LIST *result_list,EVERYTHING3_SIZE_T result_index,EVERYTHING3_WCHAR *wbuf,EVERYTHING3_SIZE_T wbuf_size_in_wchars)
{
	return Everything3_GetResultPropertyTextW(result_list,result_index,EVERYTHING3_PROPERTY_ID_FILE_LIST_FILENAME,wbuf,wbuf_size_in_wchars);
}

EVERYTHING3_USERAPI EVERYTHING3_SIZE_T EVERYTHING3_API Everything3_GetResultFilelistFilenameA(const EVERYTHING3_RESULT_LIST *result_list,EVERYTHING3_SIZE_T result_index,EVERYTHING3_CHAR *buf,EVERYTHING3_SIZE_T bufsize)
{
	return Everything3_GetResultPropertyTextA(result_list,result_index,EVERYTHING3_PROPERTY_ID_FILE_LIST_FILENAME,buf,bufsize);
}

static DWORD _everything3_get_run_count(EVERYTHING3_CLIENT *client,const EVERYTHING3_UTF8 *filename,SIZE_T filename_length_in_bytes)
{
	DWORD ret;
	DWORD value;
	SIZE_T numread;

	ret = 0;
	
	if (_everything3_ioctrl(client,_EVERYTHING3_COMMAND_GET_RUN_COUNT,filename,filename_length_in_bytes,&value,sizeof(DWORD),&numread))
	{
		if (numread == sizeof(DWORD))
		{
			ret = value;

			if (ret == 0)
			{
				SetLastError(EVERYTHING3_OK);
			}
		}
		else
		{
			SetLastError(EVERYTHING3_ERROR_BAD_RESPONSE);
		}
	}
	
	return ret;
}

EVERYTHING3_USERAPI EVERYTHING3_DWORD EVERYTHING3_API Everything3_GetRunCountFromFilenameUTF8(EVERYTHING3_CLIENT *client,const EVERYTHING3_UTF8 *lpFilename)
{
	return _everything3_get_run_count(client,lpFilename,_everything3_utf8_string_get_length_in_bytes(lpFilename));
}

EVERYTHING3_USERAPI EVERYTHING3_DWORD EVERYTHING3_API Everything3_GetRunCountFromFilenameW(EVERYTHING3_CLIENT *client,LPCWSTR lpFilename)
{
	DWORD ret;
	_everything3_utf8_buf_t filename_cbuf;

	ret = 0;
	
	_everything3_utf8_buf_init(&filename_cbuf);

	if (_everything3_utf8_buf_copy_wchar_string(&filename_cbuf,lpFilename))
	{
		ret = _everything3_get_run_count(client,filename_cbuf.buf,filename_cbuf.length_in_bytes);
	}
	
	_everything3_utf8_buf_kill(&filename_cbuf);
	
	return ret;
}

EVERYTHING3_USERAPI EVERYTHING3_DWORD EVERYTHING3_API Everything3_GetRunCountFromFilenameA(EVERYTHING3_CLIENT *client,LPCSTR lpFilename)
{
	DWORD ret;
	_everything3_utf8_buf_t filename_cbuf;

	ret = 0;
	
	_everything3_utf8_buf_init(&filename_cbuf);

	if (_everything3_utf8_buf_copy_ansi_string(&filename_cbuf,lpFilename))
	{
		ret = _everything3_get_run_count(client,filename_cbuf.buf,filename_cbuf.length_in_bytes);
	}
	
	_everything3_utf8_buf_kill(&filename_cbuf);
	
	return ret;
}

static BOOL _everything3_set_run_count(EVERYTHING3_CLIENT *client,const EVERYTHING3_UTF8 *filename,SIZE_T filename_length_in_bytes,DWORD run_count)
{
	BOOL ret;
	_everything3_utf8_buf_t cbuf;
	SIZE_T packet_size;
	BYTE *d;

	ret = FALSE;
	_everything3_utf8_buf_init(&cbuf);

	packet_size = _everything3_safe_size_add(filename_length_in_bytes,sizeof(DWORD));
	_everything3_utf8_buf_grow_size(&cbuf,packet_size);
	d = cbuf.buf;
	
	d = _everything3_copy_memory(d,filename,filename_length_in_bytes);
	d = _everything3_copy_memory(d,&run_count,sizeof(DWORD));
	
	if (_everything3_ioctrl(client,_EVERYTHING3_COMMAND_SET_RUN_COUNT,cbuf.buf,packet_size,NULL,0,NULL))
	{
		ret = TRUE;
	}
	
	_everything3_utf8_buf_kill(&cbuf);
	
	return ret;
}

EVERYTHING3_USERAPI BOOL EVERYTHING3_API Everything3_SetRunCountFromFilenameUTF8(EVERYTHING3_CLIENT *client,const EVERYTHING3_UTF8 *lpFilename,EVERYTHING3_DWORD dwRunCount)
{
	return _everything3_set_run_count(client,lpFilename,_everything3_utf8_string_get_length_in_bytes(lpFilename),dwRunCount);
}

EVERYTHING3_USERAPI BOOL EVERYTHING3_API Everything3_SetRunCountFromFilenameW(EVERYTHING3_CLIENT *client,LPCWSTR lpFilename,EVERYTHING3_DWORD dwRunCount)
{
	BOOL ret;
	_everything3_utf8_buf_t filename_cbuf;

	ret = FALSE;
	
	_everything3_utf8_buf_init(&filename_cbuf);

	if (_everything3_utf8_buf_copy_wchar_string(&filename_cbuf,lpFilename))
	{
		ret = _everything3_set_run_count(client,filename_cbuf.buf,filename_cbuf.length_in_bytes,dwRunCount);
	}
	
	_everything3_utf8_buf_kill(&filename_cbuf);
	
	return ret;
}

EVERYTHING3_USERAPI BOOL EVERYTHING3_API Everything3_SetRunCountFromFilenameA(EVERYTHING3_CLIENT *client,LPCSTR lpFilename,EVERYTHING3_DWORD dwRunCount)
{
	BOOL ret;
	_everything3_utf8_buf_t filename_cbuf;

	ret = FALSE;
	
	_everything3_utf8_buf_init(&filename_cbuf);

	if (_everything3_utf8_buf_copy_ansi_string(&filename_cbuf,lpFilename))
	{
		ret = _everything3_set_run_count(client,filename_cbuf.buf,filename_cbuf.length_in_bytes,dwRunCount);
	}
	
	_everything3_utf8_buf_kill(&filename_cbuf);
	
	return ret;
}

static DWORD _everything3_inc_run_count(EVERYTHING3_CLIENT *client,const EVERYTHING3_UTF8 *filename,SIZE_T filename_length_in_bytes)
{
	DWORD ret;
	DWORD value;
	SIZE_T numread;

	ret = 0;
	
	if (_everything3_ioctrl(client,_EVERYTHING3_COMMAND_INC_RUN_COUNT,filename,filename_length_in_bytes,&value,sizeof(DWORD),&numread))
	{
		if (numread == sizeof(DWORD))
		{
			ret = value;

			if (ret == 0)
			{
				SetLastError(EVERYTHING3_OK);
			}
		}
		else
		{
			SetLastError(EVERYTHING3_ERROR_BAD_RESPONSE);
		}
	}
	
	return ret;
}

EVERYTHING3_USERAPI EVERYTHING3_DWORD EVERYTHING3_API Everything3_IncRunCountFromFilenameUTF8(EVERYTHING3_CLIENT *client,const EVERYTHING3_UTF8 *lpFilename)
{
	return _everything3_inc_run_count(client,lpFilename,_everything3_utf8_string_get_length_in_bytes(lpFilename));
}

EVERYTHING3_USERAPI EVERYTHING3_DWORD EVERYTHING3_API Everything3_IncRunCountFromFilenameW(EVERYTHING3_CLIENT *client,LPCWSTR lpFilename)
{
	DWORD ret;
	_everything3_utf8_buf_t filename_cbuf;

	ret = 0;
	
	_everything3_utf8_buf_init(&filename_cbuf);

	if (_everything3_utf8_buf_copy_wchar_string(&filename_cbuf,lpFilename))
	{
		ret = _everything3_inc_run_count(client,filename_cbuf.buf,filename_cbuf.length_in_bytes);
	}
	
	_everything3_utf8_buf_kill(&filename_cbuf);
	
	return ret;
}

EVERYTHING3_USERAPI EVERYTHING3_DWORD EVERYTHING3_API Everything3_IncRunCountFromFilenameA(EVERYTHING3_CLIENT *client,LPCSTR lpFilename)
{
	DWORD ret;
	_everything3_utf8_buf_t filename_cbuf;

	ret = 0;
	
	_everything3_utf8_buf_init(&filename_cbuf);

	if (_everything3_utf8_buf_copy_ansi_string(&filename_cbuf,lpFilename))
	{
		ret = _everything3_inc_run_count(client,filename_cbuf.buf,filename_cbuf.length_in_bytes);
	}
	
	_everything3_utf8_buf_kill(&filename_cbuf);
	
	return ret;
}

static EVERYTHING3_UINT64 _everything3_get_folder_size(EVERYTHING3_CLIENT *client,const EVERYTHING3_UTF8 *filename,SIZE_T filename_length_in_bytes)
{
	EVERYTHING3_UINT64 ret;
	EVERYTHING3_UINT64 value;
	SIZE_T numread;

	ret = EVERYTHING3_UINT64_MAX;
	
	if (_everything3_ioctrl(client,_EVERYTHING3_COMMAND_GET_FOLDER_SIZE,filename,filename_length_in_bytes,&value,sizeof(EVERYTHING3_UINT64),&numread))
	{
		if (numread == sizeof(EVERYTHING3_UINT64))
		{
			ret = value;

			if (ret == EVERYTHING3_UINT64_MAX)
			{
				SetLastError(EVERYTHING3_OK);
			}
		}
		else
		{
			SetLastError(EVERYTHING3_ERROR_BAD_RESPONSE);
		}
	}
	
	return ret;
}

// returns EVERYTHING3_UINT64_MAX if size is unknown.
// returns EVERYTHING3_UINT64_MAX if an error occurs. Call GetLastError() for more information.
EVERYTHING3_USERAPI EVERYTHING3_UINT64 EVERYTHING3_API Everything3_GetFolderSizeFromFilenameUTF8(EVERYTHING3_CLIENT *client,const EVERYTHING3_UTF8 *lpFilename)
{
	return _everything3_get_folder_size(client,lpFilename,_everything3_utf8_string_get_length_in_bytes(lpFilename));
}

// returns EVERYTHING3_UINT64_MAX if size is unknown.
// returns EVERYTHING3_UINT64_MAX if an error occurs. Call GetLastError() for more information.
EVERYTHING3_USERAPI EVERYTHING3_UINT64 EVERYTHING3_API Everything3_GetFolderSizeFromFilenameW(EVERYTHING3_CLIENT *client,LPCWSTR lpFilename)
{
	EVERYTHING3_UINT64 ret;
	_everything3_utf8_buf_t filename_cbuf;

	ret = EVERYTHING3_UINT64_MAX;
	
	_everything3_utf8_buf_init(&filename_cbuf);

	if (_everything3_utf8_buf_copy_wchar_string(&filename_cbuf,lpFilename))
	{
		ret = _everything3_get_folder_size(client,filename_cbuf.buf,filename_cbuf.length_in_bytes);
	}
	
	_everything3_utf8_buf_kill(&filename_cbuf);
	
	return ret;
}

// returns EVERYTHING3_UINT64_MAX if size is unknown.
// returns EVERYTHING3_UINT64_MAX if an error occurs. Call GetLastError() for more information.
EVERYTHING3_USERAPI EVERYTHING3_UINT64 EVERYTHING3_API Everything3_GetFolderSizeFromFilenameA(EVERYTHING3_CLIENT *client,LPCSTR lpFilename)
{
	EVERYTHING3_UINT64 ret;
	_everything3_utf8_buf_t filename_cbuf;

	ret = EVERYTHING3_UINT64_MAX;
	
	_everything3_utf8_buf_init(&filename_cbuf);

	if (_everything3_utf8_buf_copy_ansi_string(&filename_cbuf,lpFilename))
	{
		ret = _everything3_get_folder_size(client,filename_cbuf.buf,filename_cbuf.length_in_bytes);
	}
	
	_everything3_utf8_buf_kill(&filename_cbuf);
	
	return ret;
}

static BOOL _everything3_get_file_attributes_ex(EVERYTHING3_CLIENT *client,const EVERYTHING3_UTF8 *filename,SIZE_T filename_length_in_bytes,_everything3_utf8_buf_t *data_cbuf)
{
	BOOL ret;

	ret = FALSE;
	
	if (_everything3_ioctrl_get_string(client,_EVERYTHING3_COMMAND_GET_FILE_ATTRIBUTES_EX,filename,filename_length_in_bytes,data_cbuf))
	{
		if (data_cbuf->length_in_bytes >= sizeof(_everything3_win32_find_data_t))
		{
			ret = TRUE;
		}
		else
		{
			SetLastError(EVERYTHING3_ERROR_BAD_RESPONSE);
		}
	}

	return ret;
}

// Get the file WIN32_FIND_DATA from the specified filename.
// only indexed information is returned.
// non-indexed dates and sizes will be -1
// The FILE_ATTRIBUTE_DIRECTORY bit is always valid.
// dwReserved0 and dwReserved1 are always set to 0.
// cAlternateFileName is always set to an empty string.
EVERYTHING3_USERAPI BOOL EVERYTHING3_API Everything3_GetFileAttributesExW(EVERYTHING3_CLIENT *client,const EVERYTHING3_WCHAR *lpFilename,WIN32_FIND_DATAW *pfd)
{
	BOOL ret;
	_everything3_utf8_buf_t filename_cbuf;

	ret = FALSE;
	
	_everything3_utf8_buf_init(&filename_cbuf);

	if (_everything3_utf8_buf_copy_wchar_string(&filename_cbuf,lpFilename))
	{
		_everything3_utf8_buf_t data_cbuf;

		_everything3_utf8_buf_init(&data_cbuf);
		
		if (_everything3_get_file_attributes_ex(client,filename_cbuf.buf,filename_cbuf.length_in_bytes,&data_cbuf))
		{
			_everything3_win32_find_data_t *fd;
			
			fd = ((_everything3_win32_find_data_t *)data_cbuf.buf);
			
			pfd->dwFileAttributes = fd->attributes;
			pfd->ftCreationTime.dwLowDateTime = (DWORD)(fd->date_created & 0xffffffff);
			pfd->ftCreationTime.dwHighDateTime = (DWORD)(fd->date_created >> 32);
			pfd->ftLastAccessTime.dwLowDateTime = (DWORD)(fd->date_accessed & 0xffffffff);
			pfd->ftLastAccessTime.dwHighDateTime = (DWORD)(fd->date_accessed >> 32);
			pfd->ftLastWriteTime.dwLowDateTime = (DWORD)(fd->date_modified & 0xffffffff);
			pfd->ftLastWriteTime.dwHighDateTime = (DWORD)(fd->date_modified >> 32);
			pfd->nFileSizeHigh = (DWORD)(fd->size >> 32);
			pfd->nFileSizeLow = (DWORD)(fd->size & 0xffffffff);
			pfd->dwReserved0 = 0;
			pfd->dwReserved1 = 0;
			_everything3_safe_wchar_string_copy_utf8_string_n(pfd->cFileName,MAX_PATH,data_cbuf.buf + sizeof(_everything3_win32_find_data_t),data_cbuf.length_in_bytes - sizeof(_everything3_win32_find_data_t));
			pfd->cAlternateFileName[0] = 0;
			
			ret = TRUE;
		}

		_everything3_utf8_buf_kill(&data_cbuf);
	}
	
	_everything3_utf8_buf_kill(&filename_cbuf);
	
	return ret;
}

// Get the file WIN32_FIND_DATA from the specified filename.
// only indexed information is returned.
// non-indexed dates and sizes will be -1
// The FILE_ATTRIBUTE_DIRECTORY bit is always valid.
// dwReserved0 and dwReserved1 are always set to 0.
// cAlternateFileName is always set to an empty string.
EVERYTHING3_USERAPI BOOL EVERYTHING3_API Everything3_GetFileAttributesExA(EVERYTHING3_CLIENT *client,const EVERYTHING3_CHAR *lpFilename,WIN32_FIND_DATAA *pfd)
{
	BOOL ret;
	_everything3_utf8_buf_t filename_cbuf;

	ret = FALSE;
	
	_everything3_utf8_buf_init(&filename_cbuf);

	if (_everything3_utf8_buf_copy_ansi_string(&filename_cbuf,lpFilename))
	{
		_everything3_utf8_buf_t data_cbuf;

		_everything3_utf8_buf_init(&data_cbuf);
		
		if (_everything3_get_file_attributes_ex(client,filename_cbuf.buf,filename_cbuf.length_in_bytes,&data_cbuf))
		{
			_everything3_win32_find_data_t *fd;
			
			fd = ((_everything3_win32_find_data_t *)data_cbuf.buf);
			
			pfd->dwFileAttributes = fd->attributes;
			pfd->ftCreationTime.dwLowDateTime = (DWORD)(fd->date_created & 0xffffffff);
			pfd->ftCreationTime.dwHighDateTime = (DWORD)(fd->date_created >> 32);
			pfd->ftLastAccessTime.dwLowDateTime = (DWORD)(fd->date_accessed & 0xffffffff);
			pfd->ftLastAccessTime.dwHighDateTime = (DWORD)(fd->date_accessed >> 32);
			pfd->ftLastWriteTime.dwLowDateTime = (DWORD)(fd->date_modified & 0xffffffff);
			pfd->ftLastWriteTime.dwHighDateTime = (DWORD)(fd->date_modified >> 32);
			pfd->nFileSizeHigh = (DWORD)(fd->size >> 32);
			pfd->nFileSizeLow = (DWORD)(fd->size & 0xffffffff);
			pfd->dwReserved0 = 0;
			pfd->dwReserved1 = 0;
			_everything3_safe_ansi_string_copy_utf8_string_n(pfd->cFileName,MAX_PATH,data_cbuf.buf + sizeof(_everything3_win32_find_data_t),data_cbuf.length_in_bytes - sizeof(_everything3_win32_find_data_t));
			pfd->cAlternateFileName[0] = 0;
			
			ret = TRUE;
		}

		_everything3_utf8_buf_kill(&data_cbuf);
	}
	
	_everything3_utf8_buf_kill(&filename_cbuf);
	
	return ret;
}

static EVERYTHING3_DWORD _everything3_get_file_attributes(EVERYTHING3_CLIENT *client,const EVERYTHING3_UTF8 *filename,SIZE_T filename_length_in_bytes)
{
	BOOL ret;
	DWORD value;
	SIZE_T numread;

	ret = INVALID_FILE_ATTRIBUTES;
	
	if (_everything3_ioctrl(client,_EVERYTHING3_COMMAND_GET_FILE_ATTRIBUTES,filename,filename_length_in_bytes,&value,sizeof(DWORD),&numread))
	{
		if (numread == sizeof(DWORD))
		{
			ret = value;
			
			if (ret == INVALID_FILE_ATTRIBUTES)
			{
				SetLastError(EVERYTHING3_OK);
			}
		}
		else
		{
			SetLastError(EVERYTHING3_ERROR_BAD_RESPONSE);
		}
	}

	return ret;
}

// Emulate GetFileAttributes()
// returns the file attributes from the specified filename.
// returns INVALID_FILE_ATTRIBUTES on error. Call GetLastError() for more information.
EVERYTHING3_USERAPI EVERYTHING3_DWORD EVERYTHING3_API Everything3_GetFileAttributesUTF8(EVERYTHING3_CLIENT *client,const EVERYTHING3_UTF8 *lpFilename)
{
	return _everything3_get_file_attributes(client,lpFilename,_everything3_utf8_string_get_length_in_bytes(lpFilename));
}

// Emulate GetFileAttributes()
// returns the file attributes from the specified filename.
// returns INVALID_FILE_ATTRIBUTES on error. Call GetLastError() for more information.
EVERYTHING3_USERAPI EVERYTHING3_DWORD EVERYTHING3_API Everything3_GetFileAttributesW(EVERYTHING3_CLIENT *client,const EVERYTHING3_WCHAR *lpFilename)
{
	BOOL ret;
	_everything3_utf8_buf_t filename_cbuf;

	ret = INVALID_FILE_ATTRIBUTES;
	
	_everything3_utf8_buf_init(&filename_cbuf);

	if (_everything3_utf8_buf_copy_wchar_string(&filename_cbuf,lpFilename))
	{
		ret = _everything3_get_file_attributes(client,filename_cbuf.buf,filename_cbuf.length_in_bytes);
	}
	
	_everything3_utf8_buf_kill(&filename_cbuf);
	
	return ret;
}

// Emulate GetFileAttributes()
// returns the file attributes from the specified filename.
// returns INVALID_FILE_ATTRIBUTES on error. Call GetLastError() for more information.
EVERYTHING3_USERAPI EVERYTHING3_DWORD EVERYTHING3_API Everything3_GetFileAttributesA(EVERYTHING3_CLIENT *client,const EVERYTHING3_CHAR *lpFilename)
{
	BOOL ret;
	_everything3_utf8_buf_t filename_cbuf;

	ret = INVALID_FILE_ATTRIBUTES;
	
	_everything3_utf8_buf_init(&filename_cbuf);

	if (_everything3_utf8_buf_copy_ansi_string(&filename_cbuf,lpFilename))
	{
		ret = _everything3_get_file_attributes(client,filename_cbuf.buf,filename_cbuf.length_in_bytes);
	}
	
	_everything3_utf8_buf_kill(&filename_cbuf);
	
	return ret;
}

static void _everything3_find_handle_chunk_free(_everything3_find_handle_chunk_t *chunk)
{
	_everything3_mem_free(chunk);
}

// get the directory snapshot.
static EVERYTHING3_FIND_HANDLE *_everything3_find_first_file(EVERYTHING3_CLIENT *client,const EVERYTHING3_UTF8 *filename,SIZE_T filename_length_in_bytes)
{
	EVERYTHING3_FIND_HANDLE *ret;

	ret = NULL;

	if (client)
	{
		_everything3_find_handle_chunk_t *chunk_start;
		_everything3_find_handle_chunk_t *chunk_last;

		chunk_start = NULL;
		chunk_last = NULL;
		
		_everything3_Lock(client);
		
		if (_everything3_send(client,_EVERYTHING3_COMMAND_GET_FIND_FIRST_FILE,filename,filename_length_in_bytes))
		{
			for(;;)
			{
				_everything3_message_t recv_header;
				
				if (!_everything3_recv_header(client,&recv_header))
				{
					break;
				}
				
				if (recv_header.size)
				{
					uintptr_t chunk_size;
					_everything3_find_handle_chunk_t *chunk;
					
					chunk_size = sizeof(_everything3_find_handle_chunk_t);
					chunk_size = _everything3_safe_size_add(chunk_size,recv_header.size);
					
					chunk = _everything3_mem_alloc(chunk_size);
					if (chunk)
					{
						chunk->size = recv_header.size;
						
						if (_everything3_recv_data(client,_EVERYTHING3_FIND_HANDLE_CHUNK_DATA(chunk),recv_header.size))
						{
							if (chunk_start)
							{
								chunk_last->next = chunk;
							}
							else
							{
								chunk_start = chunk;
							}
							
							chunk->next = NULL;
							chunk_last = chunk;
						}
						else
						{
							break;
						}
					}
					else
					{
						SetLastError(EVERYTHING3_ERROR_OUT_OF_MEMORY);
						
						break;
					}
				}

				if (recv_header.code == _EVERYTHING3_RESPONSE_OK)
				{
					_everything3_find_handle_t *find_handle;
					
					find_handle = _everything3_mem_alloc(sizeof(_everything3_find_handle_t));
					if (find_handle)
					{
						find_handle->p = NULL;
						find_handle->avail = 0;
						find_handle->chunk_start = chunk_start;
						find_handle->chunk_cur = chunk_start;
						find_handle->error_code = 0;
						
						// find handle owns chunks now.
						chunk_start = NULL;
					
						// caller owns find handle now.
						ret = find_handle;
					}
					else
					{
						SetLastError(EVERYTHING3_ERROR_OUT_OF_MEMORY);
					}
					
					break;
				}
			}
		}		
		
		// free unused chunks.
		{
			_everything3_find_handle_chunk_t *chunk;
			
			chunk = chunk_start;
			
			while(chunk)
			{
				_everything3_find_handle_chunk_t *next_chunk;
				
				next_chunk = chunk->next;
				
				_everything3_find_handle_chunk_free(chunk);
				
				chunk = next_chunk;
			}
		}
		
		_everything3_Unlock(client);
	}
	else
	{
		SetLastError(EVERYTHING3_ERROR_INVALID_PARAMETER);
	}

	return ret;
}

// returns a snapshot of the filename search from a specific location.
// Call Everything3_FindClose to free the snapshot.
// * and ? wildcards are supported.
// only indexed values are returned.
// dwFileAttributes is zero or FILE_ATTRIBUTE_DIRECTORY if not indexed.
// The FILE_ATTRIBUTE_DIRECTORY bit in dwFileAttributes is always valid.
// ftCreationTime is -1 if not indexed.
// ftLastAccessTime is -1 if not indexed.
// ftLastWriteTime is -1 if not indexed.
// nFileSizeHigh is -1 if not indexed.
// nFileSizeLow is -1 if not indexed.
// dwReserved0 is always zero
// dwReserved1 is always zero
// cAlternateFileName is always empty.
// the returned EVERYTHING3_FIND_HANDLE should only be used in the same thread. 
// -doesn't have to be the same thread that called Everything3_FindFirstFileW.
EVERYTHING3_USERAPI EVERYTHING3_FIND_HANDLE *EVERYTHING3_API Everything3_FindFirstFileW(EVERYTHING3_CLIENT *client,const EVERYTHING3_WCHAR *lpFilename,WIN32_FIND_DATAW *pfd)
{
	EVERYTHING3_FIND_HANDLE *ret;
	_everything3_utf8_buf_t filename_cbuf;

	ret = NULL;
	
	_everything3_utf8_buf_init(&filename_cbuf);

	if (_everything3_utf8_buf_copy_wchar_string(&filename_cbuf,lpFilename))
	{
		ret = _everything3_find_first_file(client,filename_cbuf.buf,filename_cbuf.length_in_bytes);
		if (ret)
		{
			if (!Everything3_FindNextFileW(ret,pfd))
			{
				Everything3_FindClose(ret);
				ret = NULL;
			}
		}
	}
	
	_everything3_utf8_buf_kill(&filename_cbuf);
	
	return ret;
}

// returns a snapshot of the filename search from a specific location.
// Call Everything3_FindClose to free the snapshot.
// * and ? wildcards are supported.
// only indexed values are returned.
// dwFileAttributes is zero or FILE_ATTRIBUTE_DIRECTORY if not indexed.
// The FILE_ATTRIBUTE_DIRECTORY bit in dwFileAttributes is always valid.
// ftCreationTime is -1 if not indexed.
// ftLastAccessTime is -1 if not indexed.
// ftLastWriteTime is -1 if not indexed.
// nFileSizeHigh is -1 if not indexed.
// nFileSizeLow is -1 if not indexed.
// dwReserved0 is always zero
// dwReserved1 is always zero
// cAlternateFileName is always empty.
// the returned EVERYTHING3_FIND_HANDLE should only be used in the same thread. 
// -doesn't have to be the same thread that called Everything3_FindFirstFileW.
EVERYTHING3_USERAPI EVERYTHING3_FIND_HANDLE *EVERYTHING3_API Everything3_FindFirstFileA(EVERYTHING3_CLIENT *client,const EVERYTHING3_CHAR *lpFilename,WIN32_FIND_DATAA *pfd)
{
	EVERYTHING3_FIND_HANDLE *ret;
	_everything3_utf8_buf_t filename_cbuf;

	ret = NULL;
	
	_everything3_utf8_buf_init(&filename_cbuf);

	if (_everything3_utf8_buf_copy_ansi_string(&filename_cbuf,lpFilename))
	{
		ret = _everything3_find_first_file(client,filename_cbuf.buf,filename_cbuf.length_in_bytes);
		if (ret)
		{
			if (!Everything3_FindNextFileA(ret,pfd))
			{
				Everything3_FindClose(ret);
				ret = NULL;
			}
		}
	}
	
	_everything3_utf8_buf_kill(&filename_cbuf);
	
	return ret;
}

// read some data from the find handle.
static void _everything3_find_handle_chunk_read_data(EVERYTHING3_FIND_HANDLE *find_handle,void *buf,uintptr_t size)
{
	BYTE *d;
	uintptr_t run;
	
	d = buf;
	run = size;
	
	while(run)
	{
		uintptr_t chunk_size;
		
		chunk_size = run;
		if (chunk_size > find_handle->avail)
		{
			if (!find_handle->avail)
			{
				if (find_handle->chunk_cur)
				{
					find_handle->p = _EVERYTHING3_FIND_HANDLE_CHUNK_DATA(find_handle->chunk_cur);
					find_handle->avail = find_handle->chunk_cur->size;
					
					find_handle->chunk_cur = find_handle->chunk_cur->next;
				}
				else
				{
					// no more data..
					_everything3_zero_memory(d,run);
					find_handle->error_code = EVERYTHING3_ERROR_BAD_RESPONSE;
					return;
				}
			}
			
			if (chunk_size > find_handle->avail)
			{
				chunk_size = find_handle->avail;
			}
		}
		
		_everything3_copy_memory(d,find_handle->p,chunk_size);
		
		find_handle->p += chunk_size;
		find_handle->avail -= chunk_size;
		
		d += chunk_size;
		run -= chunk_size;
	}
}

static BYTE _everything3_find_handle_chunk_read_byte(EVERYTHING3_FIND_HANDLE *find_handle)
{
	BYTE byte_value;

	_everything3_find_handle_chunk_read_data(find_handle,&byte_value,sizeof(BYTE));
	
	return byte_value;
}

static WORD _everything3_find_handle_chunk_read_word(EVERYTHING3_FIND_HANDLE *find_handle)
{
	WORD word_value;

	_everything3_find_handle_chunk_read_data(find_handle,&word_value,sizeof(WORD));

	return word_value;
}

static DWORD _everything3_find_handle_chunk_read_dword(EVERYTHING3_FIND_HANDLE *find_handle)
{
	DWORD dword_value;

	_everything3_find_handle_chunk_read_data(find_handle,&dword_value,sizeof(DWORD));

	return dword_value;
}

static EVERYTHING3_UINT64 _everything3_find_handle_chunk_read_uint64(EVERYTHING3_FIND_HANDLE *find_handle)
{
	EVERYTHING3_UINT64 uint64_value;

	_everything3_find_handle_chunk_read_data(find_handle,&uint64_value,sizeof(EVERYTHING3_UINT64));

	return uint64_value;
}

// read a variable length quantity.
// Doesn't have to be too efficient as the data will follow immediately.
// Sets the error code if the length would overflow (32bit dll, 64bit Everything, len > 0xffffffff )
static SIZE_T _everything3_find_handle_chunk_read_len_vlq(EVERYTHING3_FIND_HANDLE *find_handle)
{
	BYTE byte_value;
	WORD word_value;
	DWORD dword_value;
	SIZE_T start;
	
	start = 0;

	// BYTE
	byte_value =_everything3_find_handle_chunk_read_byte(find_handle);
	
	if (byte_value < 0xff)
	{
		return byte_value;
	}

	// WORD
	start = _everything3_safe_size_add(start,0xff);
	
	word_value =_everything3_find_handle_chunk_read_word(find_handle);
	
	if (word_value < 0xffff)
	{
		return _everything3_safe_size_add(start,word_value);
	}	
	
	// DWORD
	start = _everything3_safe_size_add(start,0xffff);

	dword_value =_everything3_find_handle_chunk_read_dword(find_handle);
	
	if (dword_value < 0xffffffff)
	{
		return _everything3_safe_size_add(start,dword_value);
	}

#if SIZE_MAX == 0xFFFFFFFFFFFFFFFFUI64

	{
		EVERYTHING3_UINT64 uint64_value;
		
		// UINT64
		start = _everything3_safe_size_add(start,0xffffffff);

		uint64_value =_everything3_find_handle_chunk_read_uint64(find_handle);

		if (uint64_value < 0xFFFFFFFFFFFFFFFFUI64)
		{
			return _everything3_safe_size_add(start,uint64_value);
		}

		find_handle->error_code = EVERYTHING3_ERROR_OUT_OF_MEMORY;
		return EVERYTHING3_UINT64_MAX;
	}
	
#elif SIZE_MAX == 0xffffffffui32
		
	find_handle->error_code = EVERYTHING3_ERROR_OUT_OF_MEMORY;
	return EVERYTHING3_DWORD_MAX;

#else

	#error unknown UINTPTR_MAX

#endif
}

EVERYTHING3_USERAPI BOOL EVERYTHING3_API Everything3_FindNextFileW(EVERYTHING3_FIND_HANDLE *find_handle,WIN32_FIND_DATAW *pfd)
{
	BOOL ret;
	
	ret = FALSE;
	
	if ((!find_handle->avail) && (!find_handle->chunk_cur))
	{
		// EOF
		SetLastError(EVERYTHING3_OK);
	}
	else
	{
		_everything3_win32_find_data_t fd;
		SIZE_T filename_len;
		_everything3_utf8_buf_t filename_cbuf;

		_everything3_utf8_buf_init(&filename_cbuf);
		
		_everything3_find_handle_chunk_read_data(find_handle,&fd,sizeof(_everything3_win32_find_data_t));
		
		pfd->dwFileAttributes = fd.attributes;
		pfd->ftCreationTime.dwLowDateTime = (DWORD)(fd.date_created & 0xffffffff);
		pfd->ftCreationTime.dwHighDateTime = (DWORD)(fd.date_created >> 32);
		pfd->ftLastAccessTime.dwLowDateTime = (DWORD)(fd.date_accessed & 0xffffffff);
		pfd->ftLastAccessTime.dwHighDateTime = (DWORD)(fd.date_accessed >> 32);
		pfd->ftLastWriteTime.dwLowDateTime = (DWORD)(fd.date_modified & 0xffffffff);
		pfd->ftLastWriteTime.dwHighDateTime = (DWORD)(fd.date_modified >> 32);
		pfd->nFileSizeHigh = (DWORD)(fd.size >> 32);
		pfd->nFileSizeLow = (DWORD)(fd.size & 0xffffffff);
		pfd->dwReserved0 = 0;
		pfd->dwReserved1 = 0;
		
		filename_len = _everything3_find_handle_chunk_read_len_vlq(find_handle);

		if (_everything3_utf8_buf_grow_length(&filename_cbuf,filename_len))
		{
			_everything3_find_handle_chunk_read_data(find_handle,filename_cbuf.buf,filename_len);
			
			_everything3_safe_wchar_string_copy_utf8_string_n(pfd->cFileName,MAX_PATH,filename_cbuf.buf,filename_cbuf.length_in_bytes);
			pfd->cAlternateFileName[0] = 0;
			
			if (!find_handle->error_code)
			{
				ret = TRUE;
			}
			else
			{
				SetLastError(find_handle->error_code);
			}
		}
		
		_everything3_utf8_buf_kill(&filename_cbuf);
	}
	
	return ret;
}

// Like FindNextFile, find the next file in the directory snapshot.
EVERYTHING3_USERAPI BOOL EVERYTHING3_API Everything3_FindNextFileA(EVERYTHING3_FIND_HANDLE *find_handle,WIN32_FIND_DATAA *pfd)
{
	BOOL ret;
	
	ret = FALSE;
	
	if ((!find_handle->avail) && (!find_handle->chunk_cur))
	{
		// EOF
		SetLastError(EVERYTHING3_OK);
	}
	else
	{
		_everything3_win32_find_data_t fd;
		SIZE_T filename_len;
		_everything3_utf8_buf_t filename_cbuf;

		_everything3_utf8_buf_init(&filename_cbuf);
		
		_everything3_find_handle_chunk_read_data(find_handle,&fd,sizeof(_everything3_win32_find_data_t));
		
		pfd->dwFileAttributes = fd.attributes;
		pfd->ftCreationTime.dwLowDateTime = (DWORD)(fd.date_created & 0xffffffff);
		pfd->ftCreationTime.dwHighDateTime = (DWORD)(fd.date_created >> 32);
		pfd->ftLastAccessTime.dwLowDateTime = (DWORD)(fd.date_accessed & 0xffffffff);
		pfd->ftLastAccessTime.dwHighDateTime = (DWORD)(fd.date_accessed >> 32);
		pfd->ftLastWriteTime.dwLowDateTime = (DWORD)(fd.date_modified & 0xffffffff);
		pfd->ftLastWriteTime.dwHighDateTime = (DWORD)(fd.date_modified >> 32);
		pfd->nFileSizeHigh = (DWORD)(fd.size >> 32);
		pfd->nFileSizeLow = (DWORD)(fd.size & 0xffffffff);
		pfd->dwReserved0 = 0;
		pfd->dwReserved1 = 0;
		
		filename_len = _everything3_find_handle_chunk_read_len_vlq(find_handle);

		if (_everything3_utf8_buf_grow_length(&filename_cbuf,filename_len))
		{
			_everything3_find_handle_chunk_read_data(find_handle,filename_cbuf.buf,filename_len);
			
			_everything3_safe_ansi_string_copy_utf8_string_n(pfd->cFileName,MAX_PATH,filename_cbuf.buf,filename_cbuf.length_in_bytes);
			pfd->cAlternateFileName[0] = 0;
			
			if (!find_handle->error_code)
			{
				ret = TRUE;
			}
			else
			{
				SetLastError(find_handle->error_code);
			}
		}
		
		_everything3_utf8_buf_kill(&filename_cbuf);
	}
	
	return ret;
}

// Close the find handle, releasing any memory back to the system.
EVERYTHING3_USERAPI BOOL EVERYTHING3_API Everything3_FindClose(EVERYTHING3_FIND_HANDLE *find_handle)
{
	BOOL ret;
	
	ret = FALSE;
	
	if (find_handle)
	{
		_everything3_mem_free(find_handle);
		
		ret = TRUE;
	}
	else
	{
		SetLastError(EVERYTHING3_ERROR_INVALID_PARAMETER);
	}
	
	return ret;
}

// safely copy a utf8 string to a wchar buffer.
static SIZE_T _everything3_safe_wchar_string_copy_utf8_string_n(EVERYTHING3_WCHAR *wbuf,SIZE_T wbuf_size_in_wchars,const EVERYTHING3_UTF8 *s,SIZE_T len_in_bytes)
{
	if (wbuf_size_in_wchars)
	{
		EVERYTHING3_WCHAR *d ;
		SIZE_T avail;
		const EVERYTHING3_UTF8 *p;
		SIZE_T run;
		SIZE_T len;
		
		d = wbuf;
		avail = wbuf_size_in_wchars - 1;
		p = s;
		run = len_in_bytes;
		
		while(run)
		{
			if (*p & 0x80)
			{
				if (((*p & 0xE0) == 0xC0) && (run >= 2))
				{
					// 2 byte UTF-8
					if (avail)
					{
						*d++ = ((*p & 0x1f) << 6) | (p[1] & 0x3f);
						avail--;
					}
					else
					{
						break;
					}

					p += 2;
					run -= 2;
				}
				else
				if (((*p & 0xF0) == 0xE0) && (run >= 3))
				{
					// 3 byte UTF-8
					if (avail)
					{
						*d++ = ((*p & 0x0f) << (12)) | ((p[1] & 0x3f) << 6) | (p[2] & 0x3f);
						avail--;
					}
					else
					{
						break;
					}

					p += 3;
					run -= 3;
				}
				else
				if (((*p & 0xF8) == 0xF0) && (run >= 4))
				{
					int c;
					
					// 4 byte UTF-8
					c = ((*p & 0x07) << 18) | ((p[1] & 0x3f) << 12) | ((p[2] & 0x3f) << 6) | (p[3] & 0x3f);
					
					if (c > 0xFFFF)
					{
						// surrogate.
						c -= 0x10000;
						
						if (avail >= 2)
						{
							*d++ = 0xD800 + (c >> 10);
							*d++ = 0xDC00 + (c & 0x03FF);
							
							avail -= 2;
						}
						else
						{
							break;
						}
					}
					else
					{
						// 4 byte UTF-8
						if (avail)
						{
							*d++ = c;
							avail--;
						}
						else
						{
							break;
						}
					}

					p += 4;
					run -= 4;
				}
				else
				{
					// invalid char..
					p++;
					run--;
				}
			}
			else
			{
				if (avail)
				{
					*d++ = *p;
					avail--;
				}
				else
				{
					break;
				}

				p++;
				run--;
			}
		}
		
		*d = 0;

		len = d - wbuf;
		
		if (!len)
		{
			// empty string.
			SetLastError(EVERYTHING3_OK);
		}

		return len;
	}
	else
	{
		SIZE_T required_size;
		const EVERYTHING3_UTF8 *p;
		SIZE_T run;
		
		required_size = 1;
		p = s;
		run = len_in_bytes;
		
		while(run)
		{
			if (*p & 0x80)
			{
				if (((*p & 0xE0) == 0xC0) && (run >= 2))
				{
					// 2 byte UTF-8
					required_size = _everything3_safe_size_add(required_size,1);

					p += 2;
					run -= 2;
				}
				else
				if (((*p & 0xF0) == 0xE0) && (run >= 3))
				{
					// 3 byte UTF-8
					required_size = _everything3_safe_size_add(required_size,1);

					p += 3;
					run -= 3;
				}
				else
				if (((*p & 0xF8) == 0xF0) && (run >= 4))
				{
					int c;
					
					// 4 byte UTF-8
					c = ((*p & 0x07) << 18) | ((p[1] & 0x3f) << 12) | ((p[2] & 0x3f) << 6) | (p[3] & 0x3f);
					
					if (c > 0xFFFF)
					{
						// surrogate.
						required_size = _everything3_safe_size_add(required_size,2);
					}
					else
					{
						// 4 byte UTF-8
						required_size = _everything3_safe_size_add(required_size,1);
					}

					p += 4;
					run -= 4;
				}
				else
				{
					// invalid char..
					p++;
					run--;
				}
			}
			else
			{
				required_size = _everything3_safe_size_add(required_size,1);
				
				p++;
				run--;
			}
		}
		
		// includes space for NULL terminator.
		return required_size;
	}
}

// safely copies a UTF-8 string to a UTF-8 buffer.
// prevents overflow by truncating the string
// always adds a NULL terminator.
// if bufsize is 0, the required size in bytes is returned. 
// same behavior as MultiByteToWideChar and other Win32 APIs.
static SIZE_T _everything3_safe_utf8_string_copy_utf8_string_n(EVERYTHING3_UTF8 *buf,SIZE_T bufsize,const EVERYTHING3_UTF8 *s,SIZE_T len_in_bytes)
{
	if (bufsize)
	{
		SIZE_T copied_len;
		
		copied_len = len_in_bytes;
		
		if (copied_len > bufsize - 1)
		{
			copied_len = bufsize - 1;
		}
		
		CopyMemory(buf,s,copied_len);
		
		if (copied_len)
		{
			if (buf[copied_len - 1] & 0x80)
			{
				SIZE_T last_ch_start;
				SIZE_T byte_run;
				
				last_ch_start = copied_len - 1;
				
				while(last_ch_start > 0)
				{
					if ((buf[last_ch_start] & 0xC0) == 0xC0)
					{
						break;
					}
					
					last_ch_start--;
				}

				// byte run
				byte_run = copied_len - last_ch_start;
				
				if ((buf[last_ch_start] & 0xE0) == 0xC0)
				{
					// 2 byte UTF-8
					if (byte_run != 2)
					{
						copied_len = last_ch_start;
					}
				}
				else
				if ((buf[last_ch_start] & 0xF0) == 0xE0)
				{
					// 3 byte UTF-8
					if (byte_run != 3)
					{
						copied_len = last_ch_start;
					}
				}
				else
				if ((buf[last_ch_start] & 0xF8) == 0xF0)
				{
					if (byte_run != 4)
					{
						copied_len = last_ch_start;
					}
				}
				else
				{
					copied_len = last_ch_start;
				}
			}
		}
		
		buf[copied_len] = 0;
		
		if (!copied_len)
		{
			// empty string.
			SetLastError(EVERYTHING3_OK);
		}
		
		return copied_len;
	}
	else
	{
		return _everything3_safe_size_add(len_in_bytes,1);
	}
}

// this one can fail.
static SIZE_T _everything3_safe_ansi_string_copy_utf8_string_n(EVERYTHING3_CHAR *buf,SIZE_T bufsize,const EVERYTHING3_UTF8 *s,SIZE_T len_in_bytes)
{
	SIZE_T ret;
	_everything3_wchar_buf_t wcbuf;
	
	ret = 0;
	_everything3_wchar_buf_init(&wcbuf);

	if (bufsize)
	{
		*buf = 0;
	}

	if (_everything3_wchar_buf_copy_utf8_string_n(&wcbuf,s,len_in_bytes))
	{
		if (wcbuf.length_in_wchars <= INT_MAX)
		{
			_everything3_utf8_buf_t ansi_cbuf;
			int ansi_len;

			_everything3_utf8_buf_init(&ansi_cbuf);

			ansi_len = WideCharToMultiByte(CP_ACP,0,wcbuf.buf,(int)wcbuf.length_in_wchars,NULL,0,NULL,NULL);
			if (ansi_len >= 0)
			{
				if (_everything3_utf8_buf_grow_length(&ansi_cbuf,(SIZE_T)ansi_len))
				{
					SIZE_T copied_len;

					WideCharToMultiByte(CP_ACP,0,wcbuf.buf,(int)wcbuf.length_in_wchars,ansi_cbuf.buf,ansi_len,NULL,NULL);

					ansi_cbuf.buf[(SIZE_T)ansi_len] = 0;
					
					copied_len = (SIZE_T)ansi_len;
					
					if (copied_len > bufsize - 1)
					{
						copied_len = bufsize - 1;
					}
					
					CopyMemory(buf,s,copied_len);
					
					buf[copied_len] = 0;
					
					if (!copied_len)
					{
						// empty string.
						SetLastError(EVERYTHING3_OK);
					}
					
					ret = copied_len;
				}
			}
			else
			{
				SetLastError(EVERYTHING3_ERROR_OUT_OF_MEMORY);
			}
			
			_everything3_utf8_buf_kill(&ansi_cbuf);
		}
		else
		{
			SetLastError(EVERYTHING3_ERROR_OUT_OF_MEMORY);
		}
	}

	_everything3_wchar_buf_kill(&wcbuf);
	
	return ret;
}

static void _everything3_pool_init(_everything3_pool_t *pool)
{
	pool->chunk_start = NULL;
	pool->chunk_last = NULL;
	pool->p = NULL;
	pool->avail = 0;
}

static void _everything3_pool_kill(_everything3_pool_t *pool)
{
	_everything3_pool_chunk_t *pool_chunk;
	
	pool_chunk = pool->chunk_start;
	
	while(pool_chunk)
	{
		_everything3_pool_chunk_t *next_pool_chunk;
		
		next_pool_chunk = pool_chunk->next;

		_everything3_mem_free(pool_chunk);

		pool_chunk = next_pool_chunk;
	}
}

static void *_everything3_pool_alloc(_everything3_pool_t *pool,SIZE_T size)
{
	void *p;
	
	if (size > pool->avail)
	{
		SIZE_T chunk_size;
		_everything3_pool_chunk_t *chunk;
		
		// new chunk....
		// with atleast 'size' available.
		chunk_size = _everything3_safe_size_add(sizeof(_everything3_pool_chunk_t),size);
		
		if (chunk_size < _EVERYTHING3_POOL_MIN_CHUNK_SIZE)
		{
			chunk_size = _EVERYTHING3_POOL_MIN_CHUNK_SIZE;
		}

		chunk = _everything3_mem_alloc(chunk_size);
		
		chunk->size = chunk_size - sizeof(_everything3_pool_chunk_t);
		
		if (pool->chunk_start)
		{
			pool->chunk_last->next = chunk;
		}
		else
		{
			pool->chunk_start = chunk;
		}
		
		chunk->next = NULL;
		pool->chunk_last = chunk;
		
		pool->p = _EVERYTHING3_POOL_CHUNK_DATA(chunk);
		pool->avail = chunk->size;
	}

	p = pool->p;

	pool->p += size;
	pool->avail -= size;
	
	return p;	
}

// src or dst can be unaligned.
// returns the dst + size.
static void *_everything3_copy_memory(void *dst,const void *src,SIZE_T size)
{
	BYTE *d;
	
	d = dst;
	
	CopyMemory(d,src,size);
	
	return d + size;
}

// src or dst can be unaligned.
// returns the dst + size.
static void *_everything3_zero_memory(void *dst,SIZE_T size)
{
	BYTE *d;
	
	d = dst;
	
	ZeroMemory(d,size);
	
	return d + size;
}

// calculate the size need to store a pstring.
static SIZE_T _everything3_utf8_pstring_calculate_size(SIZE_T len)
{
	SIZE_T size;
	
	size = _everything3_safe_size_add(sizeof(_everything3_utf8_pstring_t),len);
	
	if (len >= 255)
	{
		size = _everything3_safe_size_add(size,sizeof(SIZE_T));
	}
	
	return size;
}

// initialize a pstring with the specified length.
// caller should set the text after the call.
// returns a pointer to the text.
static EVERYTHING3_UTF8 *_everything3_utf8_pstring_init_len(_everything3_utf8_pstring_t *pstring,SIZE_T len)
{
	if (len >= 255)
	{
		BYTE *p;
		
		p = (BYTE *)(pstring + 1);
		
		pstring->len = 255;

		_everything3_copy_memory(p,&len,sizeof(SIZE_T));

		p += sizeof(SIZE_T);

		return (EVERYTHING3_UTF8 *)p;
	}
	else
	{
		pstring->len = (BYTE)len;

		return (EVERYTHING3_UTF8 *)(pstring + 1);
	}
}

// get the pstring text.
// if pstring is NULL, returns an empty string.
static const EVERYTHING3_UTF8 *_everything3_utf8_pstring_get_text(const _everything3_utf8_pstring_t *pstring)
{
	if (pstring)
	{
		if (pstring->len == 255)
		{
			BYTE *p;
			
			p = (BYTE *)(pstring + 1);
			
			p += sizeof(SIZE_T);
			
			return (EVERYTHING3_UTF8 *)p;
		}
		else
		{
			return (EVERYTHING3_UTF8 *)(pstring + 1);
		}
	}
	else
	{
		return "";
	}
}

// returns the length of the pstring text in bytes.
// returns 0 if pstring is NULL.
static SIZE_T _everything3_utf8_pstring_get_len(const _everything3_utf8_pstring_t *pstring)
{
	if (pstring)
	{
		if (pstring->len == 255)
		{
			BYTE *p;
			SIZE_T len;
			
			p = (BYTE *)(pstring + 1);
			
			_everything3_copy_memory(&len,p,sizeof(SIZE_T));
			
			return len;
		}
		else
		{
			return pstring->len;
		}
	}
	else
	{
		return 0;
	}
}
		
// merge left and right sorted arrays into one.
static void _everything3_sort_merge(void **dst,void **left,SIZE_T left_count,void **right,SIZE_T right_count,int (*comp)(const void *,const void *))
{
	void **d;
	void **l;
	void **r;
	SIZE_T lrun;
	SIZE_T rrun;
	
	d = dst;
	l = left;
	lrun = left_count;
	r = right;
	rrun = right_count;
	
	for(;;)
	{
		// find lowest
		if (comp(*l,*r) <= 0)
		{
			*d++ = *l;
			lrun--;

			if (!lrun)		
			{
				// copy the rest of right array
				CopyMemory(d,r,rrun * sizeof(void *));
				break;
			}

			l++;
		}
		else
		{
			*d++ = *r;
			rrun--;

			if (!rrun)		
			{
				// copy the rest of left array
				CopyMemory(d,l,lrun * sizeof(void *));
				break;
			}

			r++;
		}
	}
}

// split array into two, sort and merge.
static void _everything3_sort_split(void **dst,void **src,SIZE_T count,int (*comp)(const void *,const void *))
{
	SIZE_T mid;
	
	if (count == 1)
	{
		// already done
		dst[0] = src[0];
		
		return;
	}
	
	mid = count / 2;
	
	// dst contains a copy of the array 
	// and it doesn't matter what order it is in.
	_everything3_sort_split(src,dst,mid,comp);
	_everything3_sort_split(src+mid,dst+mid,count - mid,comp);

	_everything3_sort_merge(dst,src,mid,src+mid,count - mid,comp);	
}

static BOOL _everything3_sort(void **base,SIZE_T count,int (*comp)(const void *,const void *))
{
	if (count < 2)
	{
		// already sorted.
		return TRUE;
	}
	else
	{
		SIZE_T temp_size;
		void **temp;
		
		temp_size = _everything3_safe_size_mul_size_of_pointer(count);
		temp = _everything3_mem_alloc(temp_size);
		if (temp)
		{
			CopyMemory(temp,base,temp_size);

			_everything3_sort_split(base,temp,count,comp);
			
			_everything3_mem_free(temp);
			
			return TRUE;
		}
		
		return FALSE;
	}
}

static int _everything3_result_list_property_request_compare(const _everything3_result_list_property_request_t *a,const _everything3_result_list_property_request_t *b)
{
	if (a->property_id < b->property_id)
	{
		return -1;
	}

	if (a->property_id > b->property_id)
	{
		return 1;
	}
	
	if (a->flags < b->flags)
	{
		return -1;
	}

	if (a->flags > b->flags)
	{
		return 1;
	}
	
	return 0;
}

// ES enum
// -we want the first results immediately..
// Enumerate results from a search.
// item_callback is called for each item, before the property callback.
// property_callback is called for each property in the item.
// callbacks should return TRUE to continue, FALSE to cancel enumeration.
// Pros: 
// no allocation
// no looking up properies by id
// first result is sent via the callback immediately.
BOOL Everything3_SearchEnumResults(EVERYTHING3_CLIENT *client,EVERYTHING3_SEARCH_STATE *search_state,void *user_data,BOOL (*header_callback)(void *user_data,EVERYTHING3_UINT64 total_result_size,SIZE_T folder_result_count,SIZE_T file_result_count,SIZE_T viewport_offset,SIZE_T viewport_count),BOOL (*item_callback)(void *user_data),BOOL (*property_callback)(void *user_data,DWORD property_id,DWORD property_value_type,void *property_data))
{
	BOOL ret;
	
	ret = FALSE;
	
	if ((client) && (search_state) && (item_callback) && (property_callback))
	{
		BYTE *packet_data;
		SIZE_T packet_size;
		
		_everything3_Lock(client);
		
		EnterCriticalSection(&search_state->cs);
		
		// search_flags
		packet_size = sizeof(DWORD);
		
		// search_len
		packet_size = _everything3_safe_size_add(packet_size,(SIZE_T)_everything3_copy_len_vlq(NULL,search_state->search_len));
		
		// search_text
		packet_size = _everything3_safe_size_add(packet_size,search_state->search_len);

		// view port offset
		packet_size = _everything3_safe_size_add(packet_size,sizeof(SIZE_T));
		packet_size = _everything3_safe_size_add(packet_size,sizeof(SIZE_T));

		// sort
		packet_size = _everything3_safe_size_add(packet_size,(SIZE_T)_everything3_copy_len_vlq(NULL,search_state->sort_count));
		packet_size = _everything3_safe_size_add(packet_size,search_state->sort_count * sizeof(_everything3_search_sort_t));

		// property request 
		packet_size = _everything3_safe_size_add(packet_size,(SIZE_T)_everything3_copy_len_vlq(NULL,search_state->property_request_count));
		packet_size = _everything3_safe_size_add(packet_size,search_state->property_request_count * sizeof(_everything3_search_property_request_t));
		
		packet_data = _everything3_mem_alloc(packet_size);
		if (packet_data)
		{
			BYTE *packet_d;
			
			packet_d = packet_data;
			
			// search flags
			packet_d = _everything3_copy_dword(packet_d,search_state->search_flags);
			
			// search text
			packet_d = _everything3_copy_len_vlq(packet_d,search_state->search_len);
			packet_d = _everything3_copy_memory(packet_d,search_state->search_text,search_state->search_len);

			// viewport
			packet_d = _everything3_copy_size_t(packet_d,search_state->viewport_offset);
			packet_d = _everything3_copy_size_t(packet_d,search_state->viewport_count);

			// sort
			
			packet_d = _everything3_copy_len_vlq(packet_d,search_state->sort_count);

			{
				_everything3_search_sort_t *sort_p;
				SIZE_T sort_run;
				
				sort_p = search_state->sort_array;
				sort_run = search_state->sort_count;
				
				while(sort_run)
				{
					packet_d = _everything3_copy_dword(packet_d,sort_p->property_id);
					packet_d = _everything3_copy_dword(packet_d,sort_p->flags);
					
					sort_p++;
					sort_run--;
				}
			}

			// property requests

			packet_d = _everything3_copy_len_vlq(packet_d,search_state->property_request_count);

			{
				_everything3_search_property_request_t *property_request_p;
				SIZE_T property_request_run;
				
				property_request_p = search_state->property_request_array;
				property_request_run = search_state->property_request_count;
				
				while(property_request_run)
				{
					packet_d = _everything3_copy_dword(packet_d,property_request_p->property_id);
					packet_d = _everything3_copy_dword(packet_d,property_request_p->flags);
					
					property_request_p++;
					property_request_run--;
				}
			}

			assert((packet_d - packet_data) == packet_size);
			
			if (_everything3_send(client,_EVERYTHING3_COMMAND_SEARCH,packet_data,packet_size))
			{
				_everything3_stream_t stream;
				DWORD valid_flags;
				SIZE_T size_t_size;
				EVERYTHING3_UINT64 total_result_size;
				SIZE_T folder_result_count;
				SIZE_T file_result_count;
				SIZE_T viewport_offset;
				SIZE_T viewport_count;
				SIZE_T sort_count;
				SIZE_T property_request_count;
				
				stream.client = client;
				stream.buf = NULL;
				stream.p = NULL;
				stream.avail = 0;
				stream.error_code = 0;
				stream.got_last = 0;
				stream.is_64bit = 0;
				size_t_size = sizeof(DWORD);
				
				valid_flags = _everything3_stream_read_dword(&stream);

				if (valid_flags & _EVERYTHING3_SEARCH_FLAG_64BIT)
				{
					stream.is_64bit = 1;
					size_t_size = sizeof(EVERYTHING3_UINT64);
				}

				// result counts
				folder_result_count = _everything3_stream_read_size_t(&stream);
				file_result_count = _everything3_stream_read_size_t(&stream);
				
				// total size.
				if (valid_flags & _EVERYTHING3_SEARCH_FLAG_TOTAL_SIZE)
				{
					total_result_size = _everything3_stream_read_uint64(&stream);
				}
				
				// viewport
				viewport_offset = _everything3_stream_read_size_t(&stream);
				viewport_count = _everything3_stream_read_size_t(&stream);
				
				// sort
				sort_count = _everything3_stream_read_len_vlq(&stream);
				
				assert(sizeof(_everything3_result_list_sort_t) == 8);
						
				if (sort_count)
				{
					SIZE_T sort_size;
					
					sort_size = sort_count;

					assert(sizeof(_everything3_result_list_sort_t) == 8);
								
					sort_size = _everything3_safe_size_add(sort_size,sort_size); // x2
					sort_size = _everything3_safe_size_add(sort_size,sort_size); // x4
					sort_size = _everything3_safe_size_add(sort_size,sort_size); // x8
					
					sort_array = _everything3_mem_alloc(sort_size);

					if (!sort_array)
					{
						SetLastError(EVERYTHING3_ERROR_OUT_OF_MEMORY);
						
						goto got_error;
					}
					
					{
						SIZE_T sort_run;
						_everything3_result_list_sort_t *sort_d;
						
						sort_run = sort_count;
						sort_d = sort_array;
						
						while(sort_run)
						{
							sort_d->property_id = _everything3_stream_read_dword(&stream);
							sort_d->flags = _everything3_stream_read_dword(&stream);
							
							sort_d++;
							sort_run--;
						}
					}
				}

				// property requests
				property_request_count = _everything3_stream_read_len_vlq(&stream);
				
				if (property_request_count)
				{
					SIZE_T property_request_size;
					
					property_request_size = property_request_count;
					
					assert(sizeof(_everything3_result_list_property_request_t) <= 24);

					property_request_size = _everything3_safe_size_add(property_request_size,property_request_size); // x2
					property_request_size = _everything3_safe_size_add(property_request_size,property_request_size); // x4
					property_request_size = _everything3_safe_size_add(property_request_size,property_request_size); // x8
					property_request_size = _everything3_safe_size_add(property_request_size,property_request_size); // x16
					property_request_size = _everything3_safe_size_add(property_request_size,property_request_size); // x32 (over allocate)
					
					property_request_array = _everything3_mem_alloc(property_request_size);
					
					if (!property_request_array)
					{
						SetLastError(EVERYTHING3_ERROR_OUT_OF_MEMORY);
						
						goto got_error;
					}
					
					sorted_property_request_array = _everything3_mem_alloc(_everything3_safe_size_mul_size_of_pointer(result_list->property_request_count));
					
					if (!result_list->sorted_property_request_array)
					{
						SetLastError(EVERYTHING3_ERROR_OUT_OF_MEMORY);
						
						goto got_error;
					}
					
					{
						SIZE_T property_request_run;
						_everything3_result_list_property_request_t *property_request_d;
						_everything3_result_list_property_request_t **sorted_property_request_d;
						
						property_request_run = result_list->property_request_count;
						property_request_d = result_list->property_request_array;
						sorted_property_request_d = result_list->sorted_property_request_array;
						
						while(property_request_run)
						{
							property_request_d->offset = item_total_property_size;
							property_request_d->property_id = _everything3_stream_read_dword(&stream);
							property_request_d->flags = _everything3_stream_read_dword(&stream);
							property_request_d->value_type = _everything3_stream_read_byte(&stream);
							
							if (property_request_d->flags & (_EVERYTHING3_SEARCH_PROPERTY_REQUEST_FLAG_FORMAT|_EVERYTHING3_SEARCH_PROPERTY_REQUEST_FLAG_HIGHLIGHT))
							{
								// pstring.
								item_total_property_size = _everything3_safe_size_add(item_total_property_size,sizeof(void *));
							}
							else
							{
								// add to total item size.
								switch(property_request_d->value_type)
								{
									case EVERYTHING3_PROPERTY_VALUE_TYPE_PSTRING: 
									case EVERYTHING3_PROPERTY_VALUE_TYPE_PSTRING_MULTISTRING: 
									case EVERYTHING3_PROPERTY_VALUE_TYPE_PSTRING_STRING_REFERENCE:
									case EVERYTHING3_PROPERTY_VALUE_TYPE_PSTRING_FOLDER_REFERENCE:
									case EVERYTHING3_PROPERTY_VALUE_TYPE_PSTRING_FILE_OR_FOLDER_REFERENCE:
										item_total_property_size = _everything3_safe_size_add(item_total_property_size,sizeof(void *));
										break;
										
									case EVERYTHING3_PROPERTY_VALUE_TYPE_BYTE:
									case EVERYTHING3_PROPERTY_VALUE_TYPE_BYTE_GET_TEXT:
										item_total_property_size = _everything3_safe_size_add(item_total_property_size,sizeof(BYTE));
										break;

									case EVERYTHING3_PROPERTY_VALUE_TYPE_WORD:
									case EVERYTHING3_PROPERTY_VALUE_TYPE_WORD_GET_TEXT:
										item_total_property_size = _everything3_safe_size_add(item_total_property_size,sizeof(WORD));
										break;

									case EVERYTHING3_PROPERTY_VALUE_TYPE_DWORD: 
									case EVERYTHING3_PROPERTY_VALUE_TYPE_DWORD_FIXED_Q1K: 
									case EVERYTHING3_PROPERTY_VALUE_TYPE_DWORD_GET_TEXT: 
										item_total_property_size = _everything3_safe_size_add(item_total_property_size,sizeof(DWORD));
										break;
										
									case EVERYTHING3_PROPERTY_VALUE_TYPE_UINT64:
										item_total_property_size = _everything3_safe_size_add(item_total_property_size,sizeof(unsigned __int64));
										break;

									case EVERYTHING3_PROPERTY_VALUE_TYPE_UINT128:
										item_total_property_size = _everything3_safe_size_add(item_total_property_size,sizeof(EVERYTHING3_UINT128));
										break;

									case EVERYTHING3_PROPERTY_VALUE_TYPE_DIMENSIONS:
										item_total_property_size = _everything3_safe_size_add(item_total_property_size,sizeof(EVERYTHING3_DIMENSIONS));
										break;

									case EVERYTHING3_PROPERTY_VALUE_TYPE_SIZE_T:
										item_total_property_size = _everything3_safe_size_add(item_total_property_size,size_t_size);
										break;

									case EVERYTHING3_PROPERTY_VALUE_TYPE_INT32_FIXED_Q1K:
									case EVERYTHING3_PROPERTY_VALUE_TYPE_INT32_FIXED_Q1M:
										item_total_property_size = _everything3_safe_size_add(item_total_property_size,sizeof(EVERYTHING3_INT32));
										break;

									case EVERYTHING3_PROPERTY_VALUE_TYPE_BLOB8:
									case EVERYTHING3_PROPERTY_VALUE_TYPE_BLOB16:
										item_total_property_size = _everything3_safe_size_add(item_total_property_size,sizeof(void *));
										break;
								}
							}
							
							*sorted_property_request_d++ = property_request_d;
							
							property_request_d++;
							property_request_run--;
						}
					}
					
					// sort properties..
					if (!_everything3_sort(result_list->sorted_property_request_array,result_list->property_request_count,_everything3_result_list_property_request_compare))
					{
						SetLastError(EVERYTHING3_ERROR_OUT_OF_MEMORY);
						
						goto got_error;
					}
					
					{
						SIZE_T property_request_run;
						_everything3_result_list_property_request_t **sorted_property_request_p;
						
						property_request_run = result_list->property_request_count;
						sorted_property_request_p = result_list->sorted_property_request_array;
						
						while(property_request_run)
						{
							_everything3_debug_printf("PROPERTY %u %08x %u %p\n",(*sorted_property_request_p)->property_id,(*sorted_property_request_p)->flags,(*sorted_property_request_p)->value_type,(*sorted_property_request_p)->offset);
							
							sorted_property_request_p++;
							property_request_run--;
						}
					}

					_everything3_debug_printf("total item property size: %p\n",item_total_property_size);
				}
				
				// read items
				
				if (result_list->viewport_count)
				{
					SIZE_T item_array_size;
					SIZE_T viewport_run;
					_everything3_result_list_item_t *item_d;
					
					item_array_size = _everything3_safe_size_mul_size_of_pointer(result_list->viewport_count);
					
					result_list->item_array = _everything3_mem_alloc(item_array_size);
					
					if (!result_list->item_array)
					{
						SetLastError(EVERYTHING3_ERROR_OUT_OF_MEMORY);
						
						goto got_error;
					}
					
					item_d = result_list->item_array;
					
					viewport_run = result_list->viewport_count;
					
					while(viewport_run)
					{
						BYTE *property_d;
						
						item_d->property_data = _everything3_pool_alloc(&result_list->pool,item_total_property_size);
						
						property_d = item_d->property_data;
						
						*property_d++ = _everything3_stream_read_byte(&stream);
						
						// read properties..
						{
							SIZE_T property_request_run;
							const _everything3_result_list_property_request_t *property_request_p;
							
							property_request_run = result_list->property_request_count;
							property_request_p = result_list->property_request_array;
							
							while(property_request_run)
							{
								if (property_request_p->flags & (_EVERYTHING3_SEARCH_PROPERTY_REQUEST_FLAG_FORMAT|_EVERYTHING3_SEARCH_PROPERTY_REQUEST_FLAG_HIGHLIGHT))
								{
									SIZE_T len;
									SIZE_T pstring_size;
									_everything3_utf8_pstring_t *pstring;
									BYTE *pstring_text;
									
									len = _everything3_stream_read_len_vlq(&stream);
									
									if (len)
									{
										// _everything3_debug_printf("LEN %p\n",len);
										pstring_size = _everything3_utf8_pstring_calculate_size(len);
										
										pstring = _everything3_pool_alloc(&result_list->pool,pstring_size);
										
										pstring_text = _everything3_utf8_pstring_init_len(pstring,len);
										
										_everything3_stream_read_data(&stream,pstring_text,len);
										// _everything3_debug_printf("%s\n",pstring_text);
									}
									else
									{
										pstring = NULL;
									}

									property_d = _everything3_copy_memory(property_d,&pstring,sizeof(_everything3_utf8_pstring_t *));
								}
								else
								{
									// add to total item size.
									switch(property_request_p->value_type)
									{
										case EVERYTHING3_PROPERTY_VALUE_TYPE_PSTRING: 
										case EVERYTHING3_PROPERTY_VALUE_TYPE_PSTRING_MULTISTRING: 
										case EVERYTHING3_PROPERTY_VALUE_TYPE_PSTRING_STRING_REFERENCE:
										case EVERYTHING3_PROPERTY_VALUE_TYPE_PSTRING_FOLDER_REFERENCE:
										case EVERYTHING3_PROPERTY_VALUE_TYPE_PSTRING_FILE_OR_FOLDER_REFERENCE:

											{
												SIZE_T len;
												SIZE_T pstring_size;
												_everything3_utf8_pstring_t *pstring;
												BYTE *pstring_text;
												
												len = _everything3_stream_read_len_vlq(&stream);
												
												if (len)
												{
													// _everything3_debug_printf("LEN %p\n",len);
													pstring_size = _everything3_utf8_pstring_calculate_size(len);
													
													pstring = _everything3_pool_alloc(&result_list->pool,pstring_size);
													
													pstring_text = _everything3_utf8_pstring_init_len(pstring,len);
													
													_everything3_stream_read_data(&stream,pstring_text,len);
													// _everything3_debug_printf("%s\n",pstring_text);
												}
												else
												{
													pstring = NULL;
												}

												property_d = _everything3_copy_memory(property_d,&pstring,sizeof(_everything3_utf8_pstring_t *));
											}

											break;

										case EVERYTHING3_PROPERTY_VALUE_TYPE_BYTE:
										case EVERYTHING3_PROPERTY_VALUE_TYPE_BYTE_GET_TEXT:

											_everything3_stream_read_data(&stream,property_d,sizeof(BYTE));
											property_d += sizeof(BYTE);

											break;

										case EVERYTHING3_PROPERTY_VALUE_TYPE_WORD:
										case EVERYTHING3_PROPERTY_VALUE_TYPE_WORD_GET_TEXT:

											_everything3_stream_read_data(&stream,property_d,sizeof(WORD));
											property_d += sizeof(WORD);

											break;

										case EVERYTHING3_PROPERTY_VALUE_TYPE_DWORD: 
										case EVERYTHING3_PROPERTY_VALUE_TYPE_DWORD_FIXED_Q1K: 
										case EVERYTHING3_PROPERTY_VALUE_TYPE_DWORD_GET_TEXT: 

											_everything3_stream_read_data(&stream,property_d,sizeof(DWORD));
											property_d += sizeof(DWORD);

											break;
											
										case EVERYTHING3_PROPERTY_VALUE_TYPE_UINT64: 

											_everything3_stream_read_data(&stream,property_d,sizeof(EVERYTHING3_UINT64));
											property_d += sizeof(EVERYTHING3_UINT64);

											break;
											
										case EVERYTHING3_PROPERTY_VALUE_TYPE_UINT128: 

											_everything3_stream_read_data(&stream,property_d,sizeof(EVERYTHING3_UINT128));
											property_d += sizeof(EVERYTHING3_UINT128);

											break;
											
										case EVERYTHING3_PROPERTY_VALUE_TYPE_DIMENSIONS: 

											_everything3_stream_read_data(&stream,property_d,sizeof(EVERYTHING3_DIMENSIONS));
											property_d += sizeof(EVERYTHING3_DIMENSIONS);

											break;
											
										case EVERYTHING3_PROPERTY_VALUE_TYPE_SIZE_T: 
											_everything3_stream_read_data(&stream,property_d,size_t_size);
											property_d += size_t_size;
											break;
											
										case EVERYTHING3_PROPERTY_VALUE_TYPE_INT32_FIXED_Q1K: 
										case EVERYTHING3_PROPERTY_VALUE_TYPE_INT32_FIXED_Q1M: 
											_everything3_stream_read_data(&stream,property_d,sizeof(EVERYTHING3_INT32));
											property_d += sizeof(EVERYTHING3_INT32);
											break;

										case EVERYTHING3_PROPERTY_VALUE_TYPE_BLOB8:

											{
												BYTE len;
												_everything3_blob8_t *blob;
												
												len = _everything3_stream_read_byte(&stream);
												
												if (len)
												{	
													SIZE_T blob_size;
													
													// _everything3_debug_printf("LEN %p\n",len);
													blob_size = _everything3_safe_size_add(len,1);
													
													blob = _everything3_pool_alloc(&result_list->pool,blob_size);
													
													blob->len = len;
													
													_everything3_stream_read_data(&stream,_EVERYTHING3_BLOB8_DATA(blob),len);
												}
												else
												{
													blob = NULL;
												}

												property_d = _everything3_copy_memory(property_d,&blob,sizeof(_everything3_blob8_t *));
											}

											break;

										case EVERYTHING3_PROPERTY_VALUE_TYPE_BLOB16:

											{
												WORD len;
												_everything3_blob16_t *blob;
												
												len = _everything3_stream_read_word(&stream);
												
												if (len)
												{	
													SIZE_T blob_size;
													
													// _everything3_debug_printf("LEN %p\n",len);
													blob_size = _everything3_safe_size_add(len,1);
													
													blob = _everything3_pool_alloc(&result_list->pool,blob_size);
													
													blob->len = len;
													
													_everything3_stream_read_data(&stream,_EVERYTHING3_BLOB16_DATA(blob),len);
												}
												else
												{
													blob = NULL;
												}

												property_d = _everything3_copy_memory(property_d,&blob,sizeof(_everything3_blob16_t *));
											}

											break;

									}
								}
								
								property_request_p++;
								property_request_run--;
							}
						}
						
						item_d++;
						viewport_run--;
					}
				}
				
				if (stream.error_code)
				{
					SetLastError(stream.error_code);
					
					goto got_error;
				}

				ret = result_list;
				result_list = NULL;
				
got_error:
				
				if (stream.buf)
				{
					_everything3_mem_free(stream.buf);
				}
			}
			
			_everything3_mem_free(packet_data);
		}
		
		LeaveCriticalSection(&search_state->cs);
	
		_everything3_Unlock(client);
	}
	else
	{
		SetLastError(EVERYTHING3_ERROR_INVALID_PARAMETER);
	}
	
	return ret;
}
