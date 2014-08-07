#ifndef __HTML_H_
#define __HTML_H_

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stack.h"
#include "attrib.h"
#include "tag.h"
#include "util.h"

#ifndef HTML_PARSE_STATE_TYPE
#define HTML_PARSE_STATE_TYPE struct HtmlParseState
#endif

#define HTML_BUFFSIZE 1024

typedef struct HtmlElement HtmlElement;
struct HtmlElement {
	HtmlTag tag;
	char *tag_name;
	
	char *text;
	HtmlAttrib *attrib;
	
	HtmlElement *parent;
	HtmlElement *child;
	HtmlElement *sibling;
	
	int count;
};

typedef struct HtmlDocument HtmlDocument;
struct HtmlDocument {
	HtmlElement *root_element;
};

enum State {
	STATE_CHILD,
	STATE_OPEN,
	STATE_DECLARATION,
	STATE_BEGIN,
	STATE_END,
	STATE_ATTRIB,
	STATE_ATTRIB_KEY,
	STATE_ATTRIB_VALUE,
	STATE_ATTRIB_QUOTEVALUE,
	STATE_CLOSE,
	STATE_SELFCLOSE,
	STATE_END_CLOSE,
	STATE_ENTITY,
	
	/*This is silly*/
	STATE_COMMENT_BEGIN,
	STATE_COMMENT,
	STATE_COMMENT_END1,
	STATE_COMMENT_END2,
	
	STATES,
};

typedef struct HtmlParseState HtmlParseState;
struct HtmlParseState {
	HtmlDocument *document;
	Stack *stack;
	HtmlElement *elem;
	HtmlTag tag;
	char *tag_name;
	
	HtmlAttrib *attrib;
	HtmlAttribKey attrib_key;
	char *attrib_key_name;
	
	enum State state;
	
	/*used for stripping out spaces*/
	size_t stringlen;
	char space;
};

extern const char const *html_tag[HTML_TAGS];

HtmlTag html_lookup_tag(const char *string);

HtmlDocument *html_parse_file(const char* filepath);
HtmlDocument *html_parse_memory(const char* content, const int length);

HtmlParseState *html_parse_begin();
const char *html_parse_stream(HtmlParseState *state, const char *stream, const char *token, size_t len);
HtmlDocument *html_parse_end(HtmlParseState *state);

void *html_free_element(HtmlElement *element);

// printing
void *html_print_dom(HtmlDocument *document);
void *html_print_dom_element(HtmlElement *element, int level);

void *html_free_document(HtmlDocument *document);

#endif
