#ifndef __HSDL__
#define __HSDL__

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "util.h"
#include "html.h"

enum HSDL_INSTRUCTION {
	HSDL_INSTRUCTION_NONE,
	HSDL_INSTRUCTION_VARNAME_CONTENT,
	
	HSDL_INSTRUCTION_ATTRIBUTE,
	
	HSDL_INSTRUCTION_CONTENT,
	HSDL_INSTRUCTION_CONTENT_AFTER,
	
	HSDL_INSTRUCTION_EXTRA,
	
	HSDL_INSTRUCTION_FIRST,
	HSDL_INSTRUCTION_FROM,
	
	HSDL_INSTRUCTION_ID,
	HSDL_INSTRUCTION_INDEX,
	HSDL_INSTRUCTION_INTERVAL,
	
	HSDL_INSTRUCTION_LAST,
	
	HSDL_INSTRUCTION_SEQUENCE,
	HSDL_INSTRUCTION_SKIP,
	
	HSDL_INSTRUCTION_TEXTALL,
	HSDL_INSTRUCTION_TO,
	
	HSDL_INSTRUCTION_VARNAME,
	
	HSDL_INSTRUCTION_ALL,
};

enum HSDL_STATE {
	HSDL_STATE_NONE,
	
	HSDL_STATE_OUTPUT,
	HSDL_STATE_URL_ENCODE_CONTENT,
	
	HSDL_STATES,
};

enum HSDL_STATE_OPTION {
	HSDL_STATE_OPTION_THREAD_SAFE = 1
};

enum HSDL_STATE_OUTPUT {
	HSDL_OUTPUT_NONE,
	
	HSDL_OUTPUT_INI,
	
	HSDL_OUTPUT_ALL,
};

struct HSDLCounter {
	char *tag;
	int count;
	
	struct HSDLCounter *next;
};

struct HSDLState {
	enum HSDL_STATE_OUTPUT output;
	unsigned char url_encode;
	
	unsigned long options;
	
	void *hsdl_post_output_function_extra;
	
	void (*hsdl_post_output_function)( 
		struct HSDLState *hsdlState, 
		enum HSDL_INSTRUCTION instruction, 
		struct HtmlElement *html,
		const char* id, 
		const char* key, 
		const char* value, 
		int count,
		void* extra
	);
};

struct HSDL {
	struct HSDLState* state;
	
	HtmlDocument* html_doc;
	HtmlDocument* hsdl_doc;
};

extern const char *hsdl_instructions[HSDL_INSTRUCTION_ALL];
extern const char *hsdl_states[HSDL_STATES];
extern const char *hsdl_states_output[HSDL_OUTPUT_ALL];

void hsdl_init( struct HSDL* hsdl, unsigned long options );

void *hsdl_generate( struct HSDL* hsdl );

void hsdl_free( struct HSDL* hsdl );

extern HtmlAttrib *hsdl_attrib_find_by_key(HtmlAttrib *attrib, const char *key);
extern int hsdl_url_encode( char **dest, const char *src, int length );

#endif
