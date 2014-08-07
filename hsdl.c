#include "hsdl.h"

#define MAX(a,b) a < b ? b : a
#define HSDL_INSTRUCTION_PREFIX ':'

static void hsdl_output_function( 
	struct HSDLState *hsdlState, 
	HtmlElement *hsdl, 
	HtmlElement *html, 
	HtmlAttrib *attrib, 
	enum HSDL_INSTRUCTION instruction,
	int count
);

const char *hsdl_instructions[HSDL_INSTRUCTION_ALL] = {
	[HSDL_INSTRUCTION_NONE] = "",
	[HSDL_INSTRUCTION_VARNAME_CONTENT] = "",
	
	[HSDL_INSTRUCTION_ATTRIBUTE] = "attribute",
	
	[HSDL_INSTRUCTION_CONTENT] = "content",
	[HSDL_INSTRUCTION_CONTENT_AFTER] = "content_after",

	[HSDL_INSTRUCTION_EXTRA] = "extra",
	
	[HSDL_INSTRUCTION_FIRST] = "first",
	[HSDL_INSTRUCTION_FROM] = "from",

	[HSDL_INSTRUCTION_ID] = "id",
	[HSDL_INSTRUCTION_INDEX] = "index",
	[HSDL_INSTRUCTION_INTERVAL] = "interval",

	[HSDL_INSTRUCTION_LAST] = "last",
	
	[HSDL_INSTRUCTION_SEQUENCE] = "sequence",
	[HSDL_INSTRUCTION_SKIP] = "skip",
	
	[HSDL_INSTRUCTION_TEXTALL] = "textall",
	[HSDL_INSTRUCTION_TO] = "to",

	[HSDL_INSTRUCTION_VARNAME] = "varname",
};

const char *hsdl_states[HSDL_STATES] = {
	[HSDL_STATE_NONE] = "",
	
	[HSDL_STATE_OUTPUT] = "output",
	
	[HSDL_STATE_URL_ENCODE_CONTENT] = "urlencode",
};

const char *hsdl_states_output[HSDL_OUTPUT_ALL] = {
	[HSDL_OUTPUT_NONE] = "",
	
	[HSDL_OUTPUT_INI] = "ini",
};

static int hsdl_lookup_instructuion(const char *string)
{
	int i, imin = 0, imax = HSDL_INSTRUCTION_ALL, res;

	while(imax >= imin) {
		i = (imax - imin)/2 + imin;
		if (i==HSDL_INSTRUCTION_ALL)
			break;
		res = stringcompare(string, hsdl_instructions[i], MAX(strlen(string), strlen(hsdl_instructions[i])));
		if(res < 0)
			imax = i - 1;
		else if(res > 0)
			imin = i + 1;
		else
			return i;
	}
	return HSDL_INSTRUCTION_NONE;
}

static int hsdl_states_output_lookup(const char *string)
{
	int i, imin = 0, imax = HSDL_OUTPUT_ALL, res;

	while(imax >= imin) {
		i = (imax - imin)/2 + imin;
		if (i==HSDL_OUTPUT_ALL)
			break;
		res = stringcompare(string, hsdl_states_output[i], MAX(strlen(string), strlen(hsdl_states_output[i])));
		if(res < 0)
			imax = i - 1;
		else if(res > 0)
			imin = i + 1;
		else
			return i;
	}
	return HSDL_INSTRUCTION_NONE;
}

static int int_to_array(const int i, char **string) {
	*string = (char*)malloc(16);
	return snprintf(*string, 16, "%d", i);
}

int hsdl_url_encode( char **dest, const char *src, int length )
{
	char *d;
	int i;

	*dest = (char*)malloc( 3 * length + 1 );

	d = *dest;
	for( i = 0 ; i < length ; i++ )
	{
		if ( isalnum( *(src+i) ) || *(src+i) == '-' || *(src+i) == '_' || *(src+i) == '.' || *(src+i) == '~' )
		{
			*(d++) = *(src+i);
		}
		else
		{
			sprintf( d, "%%%.2X", (unsigned char)*(src+i) );
			d += 3;
		}
	}
	*d = 0;
	return d-*dest;
}

static void hsdl_attrib_print( HtmlAttrib *attrib, FILE *f ) {
	fprintf( f, " [" );
	while( attrib ) {
		fprintf( f, " %s=\"%s\"", attrib->key_name, attrib->value );
		attrib = attrib->next;
	}
	fprintf( f, " ]" );
}

/*
 * Finds attribute by key
 */
HtmlAttrib *hsdl_attrib_find_by_key(HtmlAttrib *attrib, const char *key) {
	while (attrib) {
		if (attrib->key_name) {
			if (!strcmp(attrib->key_name, key)) {
				return attrib;
			}
		}
		attrib = attrib->next;
	}
	return 0;
}

static int hsdl_contains_attribute(HtmlAttrib* html_attrib, HtmlAttrib* hsdl_attrib)
{
	unsigned char attribs_set = 0;

	if ((!html_attrib && !hsdl_attrib) || !hsdl_attrib) {
		return 1;
	}
	if (hsdl_attrib->key == HTML_ATTRIB_UNKNOWN &&
	    hsdl_attrib->key_name && *hsdl_attrib->key_name == HSDL_INSTRUCTION_PREFIX
	   ) {
		// Skip hsdl instructions
		return 1;
	}
	if (!html_attrib) {
		// No attribs on html element but on hsdl
		if ( hsdl_attrib->value && *hsdl_attrib->value == '!' ) {
			return 1;
		}
		return 0;
	}

	while(html_attrib) {

		attribs_set = 1;

		if (html_attrib->key == hsdl_attrib->key) {
			if (!html_attrib->value && !hsdl_attrib->value) {
				return 1;
			}
			if (!html_attrib->value && hsdl_attrib->value) {
				return 0;
			}
			if (html_attrib->value && !hsdl_attrib->value) {
				// TODO: Think about to match element if html attribute is 
				// defined and hsdl attribute is not
				return 0;
			}
			
			if ( *hsdl_attrib->value == '!' ) {
				if ( strcmp( hsdl_attrib->value+1, html_attrib->value) ) {
					return 1;
				} else {
					return 0;
				}
			} else {
				if ( !strcmp( hsdl_attrib->value, html_attrib->value) ) {
					return 1;
				} else {
					return 0;
				}
			}
		}
		html_attrib = html_attrib->next;
	}
	return attribs_set == 0;
}

static int hsdl_contains_attributes(HtmlAttrib* html_attrib, HtmlAttrib* hsdl_attrib)
{
	if (!hsdl_attrib) {
		return 1;
	}
	while(hsdl_attrib) {
		if (!hsdl_contains_attribute(html_attrib, hsdl_attrib)) {
			return 0;
		}
		hsdl_attrib = hsdl_attrib->next;
	}
	return 1;
}

static HtmlElement *html_contains_element(HtmlElement *html, HtmlElement *hsdl)
{
	while(html) {
		if ( ( hsdl->tag != HTML_TAG_UNKNOWN && html->tag == hsdl->tag ) || ( hsdl->tag_name && html->tag_name && !strcmp( html->tag_name, hsdl->tag_name ) ) ) {
			if (hsdl_contains_attributes(html->attrib, hsdl->attrib)) {
				return html;
			}
		}
		html = html->sibling;
	}
	return html;
}

/*
 * Prints all contents of all child elements
 */
static int hsdl_instruction_text_all(HtmlElement* html, unsigned char url_encode)
{
	char *text;
	
	if (!html) {
		return 0;
	}
	while(html) {
		if ( ( html->tag == HTML_TAG_NONE && html->tag_name == NULL && html->text ) ){
			if (url_encode) {
				hsdl_url_encode(&text, html->text, strlen(html->text));
			
				printf("%s ", text);
				fflush(stdout);
			
				free(text);
			} else {
				printf("%s ", html->text);
				fflush(stdout);
			}
		}
		hsdl_instruction_text_all(html->child, url_encode);
		html = html->sibling;
	}
	return 0;
}

/*
 * Applies all instructions on html element
 */
static int hsdl_instructions_apply( struct HSDLState *hsdlState, HtmlElement* html, HtmlElement* hsdl, struct HSDLCounter *counter )
{
	HtmlAttrib *attrib;
	enum HSDL_INSTRUCTION instruction;

	if (!hsdl->attrib) {
		return 0;
	}

	attrib = hsdl->attrib;
	while(attrib) {
		
		if (attrib->key_name && *(attrib->key_name) == HSDL_INSTRUCTION_PREFIX) {
			// lookup instruction
			instruction = hsdl_lookup_instructuion(attrib->key_name+1);
			if (instruction != HSDL_INSTRUCTION_NONE) {
				// Apply instruction
				hsdl_output_function( hsdlState, hsdl, html, attrib, instruction, counter->count );
			} else {
				fprintf(stderr, "<Unknown instruction> %s\n", hsdl_instructions[ instruction ]);
				return 0;
			}
		}
		attrib = attrib->next;
	}
	return 1;
}

static struct HSDLCounter* hsdl_counter_lookup( struct HSDLCounter *counter, const char* name ) {
	if ( !name ) {
		return NULL;
	}
	
	while ( counter ) {
		if ( !strcmp( counter->tag, name ) ) {
			break;
		}
		counter = counter->next;
	}
	return counter;
}

/*
 * Checks if instruction restrictions are fullfilled
 */
static int hsdl_instruction_check( HtmlElement *html, HtmlElement *hsdl, struct HSDLCounter *counter )
{
	HtmlAttrib *attrib;
	HtmlAttrib *attrib_tmp_1;
	HtmlAttrib *attrib_tmp_2;
	char *tmp_string;
	char *tmp_string_2;
	char *tmp_string_token;
	enum HSDL_INSTRUCTION instruction;
	unsigned char should_return = 0;
	
	if (!hsdl->attrib) {
		return 1;
	}
	
	if (!counter) {
		// no counter, so no valid instructions to be done
		return 0;
	}
	
	attrib = hsdl->attrib;
	while (attrib) {
		if (*(attrib->key_name) == HSDL_INSTRUCTION_PREFIX) {
			instruction = hsdl_lookup_instructuion(attrib->key_name+1);
			if (instruction != HSDL_INSTRUCTION_NONE) {
				// handle instruction here
				switch(instruction) {
					case HSDL_INSTRUCTION_ATTRIBUTE:
						// nothing to be done here
						should_return = 1;
						break;
					case HSDL_INSTRUCTION_EXTRA:
						// nothing to be done here
						should_return = 1;
						break;
					case HSDL_INSTRUCTION_FIRST:
						return counter->count == 1;
					case HSDL_INSTRUCTION_FROM:
						if (counter->count < atoi(attrib->value)) {
							// index too low
							return 0;
						}
						attrib_tmp_1 = hsdl_attrib_find_by_key(hsdl->attrib, ":interval");
						if (attrib_tmp_1) {
							// do not handle this here
							break;
						}
						
						attrib_tmp_2 = hsdl_attrib_find_by_key(hsdl->attrib, ":to");
						if (attrib_tmp_2) {
							// :to found
							if (counter->count >= atoi(attrib_tmp_2->value)) {
								// index too high
								return 0;
							}
						}
						return 1;
					case HSDL_INSTRUCTION_TO:
						if (counter->count >= atoi(attrib->value)) {
							// index too high
							return 0;
						}
						attrib_tmp_1 = hsdl_attrib_find_by_key(hsdl->attrib, ":interval");
						if (attrib_tmp_1) {
							// do not handle this here
							return 0;
						}
						
						attrib_tmp_2 = hsdl_attrib_find_by_key(hsdl->attrib, ":from");
						if (attrib_tmp_2) {
							// :to found
							if (counter->count < atoi(attrib_tmp_2->value)) {
								// index too low
								return 0;
							}
						}
						return 1;
					case HSDL_INSTRUCTION_ID:
						// nothing to be done here
						should_return = 1;
						break;
					case HSDL_INSTRUCTION_INDEX:
						return counter->count == atoi(attrib->value);
					case HSDL_INSTRUCTION_INTERVAL:
						attrib_tmp_1 = hsdl_attrib_find_by_key(hsdl->attrib, ":from");
						attrib_tmp_2 = hsdl_attrib_find_by_key(hsdl->attrib, ":to");
						if (!attrib_tmp_1 && !attrib_tmp_2) {
							// simple modulo check
							return ( counter->count % atoi(attrib->value) ) == 0;
						} else if(attrib_tmp_1 && !attrib_tmp_2) {
							// only :from found
							if (counter->count < atoi(attrib_tmp_1->value)) {
								// index too low
								return 0;
							}
							return ( ( counter->count + atoi(attrib_tmp_1->value) ) % atoi(attrib->value) ) == 0;
						}  else if(!attrib_tmp_1 && attrib_tmp_2) {
							// only :to found
							if (counter->count >= atoi(attrib_tmp_2->value)) {
								// index too high
								return 0;
							}
							return ( ( counter->count + atoi(attrib_tmp_2->value) ) % atoi(attrib->value) ) == 0;
						} else if (attrib_tmp_1 && attrib_tmp_2){
							// check interval range
							if (counter->count < atoi(attrib_tmp_1->value)) {
								// index too low
								return 0;
							}
							if (counter->count >= atoi(attrib_tmp_2->value)) {
								// index too high
								return 0;
							}
							return ( ( counter->count + atoi(attrib_tmp_1->value) ) % atoi(attrib->value) ) == 0;
						}
						
						break;
					case HSDL_INSTRUCTION_LAST:
						// Search for last matching element in DOM.
						fprintf(stderr, "<Unimplemented instruction> %s\n", hsdl_instructions[instruction]);
						return 0;
					case HSDL_INSTRUCTION_SEQUENCE:
						// split sequence and check
						tmp_string_2 = stringduplicate_length(attrib->value, strlen(attrib->value));
						tmp_string_token = strtok(tmp_string_2, ",");
						
						int_to_array(counter->count, &tmp_string);
						while (tmp_string_token)
						{
							if (!strcmp(tmp_string_token, tmp_string)) {
								free(tmp_string);
								free(tmp_string_2);
								return 1;
							}
							tmp_string_token = strtok(NULL, ",");
						}
						free(tmp_string);
						free(tmp_string_2);
						return 0;
					case HSDL_INSTRUCTION_SKIP:
						should_return = 1;
						break;
					case HSDL_INSTRUCTION_TEXTALL:
						// nothing to be done here
						should_return = 1;
						break;
					case HSDL_INSTRUCTION_CONTENT:
						// nothing to be done here
						should_return = 1;
						break;
					case HSDL_INSTRUCTION_CONTENT_AFTER:
						// nothing to be done here
						should_return = 1;
						break;
					case HSDL_INSTRUCTION_VARNAME:
						// nothing to be done here
						should_return = 1;
						break;
					default:
						fprintf(stderr, "<Unimplemented instruction> %s\n", hsdl_instructions[instruction]);
						return 0;
				}
			}
		} else {
			// TODO: Return true by default if attribute found which is not an instruction?
			should_return = 1;
		}
		attrib = attrib->next;
	}
	return should_return;
}

/*
 * Creates a new counter structure
 */
static struct HSDLCounter *hsdl_new_counter(char *tag_name) {
	struct HSDLCounter *counter;
	counter = (struct HSDLCounter*)malloc( sizeof( struct HSDLCounter ) );
	memset( counter, 0, sizeof( struct HSDLCounter ) );
	counter->tag = tag_name;
	return counter;
}

/*
 * Frees a counter structure
 */
static void hsdl_counter_free( struct HSDLCounter *counter ) {
	if ( !counter ) {
		return;
	}
	hsdl_counter_free( counter->next );
	free( counter );
}

/*
 * Append counter to list
 */
static void hsdl_counter_append( struct HSDLCounter **counter, struct HSDLCounter *counter2 ) {
	if ( (*counter)->next ) {
		while( (*counter)->next ) {
			*counter = (*counter)->next;
		}
	} else {
		hsdl_counter_free( *counter );
	}
	counter2->count = 0;
	*counter = counter2;
}

/*
 * Increment the counter's counter if available
 */
static int hsdl_counter_increment( struct HSDLCounter *counter, char *tag_name ) {
	while( counter ) {
		if ( counter->tag ) {
			if ( !strcmp( counter->tag, tag_name ) ) {
				counter->count++;
				return 1;
			}
		}
		counter = counter->next;
	}
	return 0;
}

unsigned char hsdl_element_should_skipped( HtmlAttrib* hsdl_attrib ) {
	while( hsdl_attrib ) {
		if ( *( hsdl_attrib->key_name ) == HSDL_INSTRUCTION_PREFIX ) {
			if ( hsdl_lookup_instructuion( hsdl_attrib->key_name+1 ) == HSDL_INSTRUCTION_SKIP ) {
				return 1;
			}
		}
		hsdl_attrib = hsdl_attrib->next;
	}
	return 0;
}

/*
 * Parse the DOM
 */

void *hsdl_parse( struct HSDLState *hsdlState, HtmlElement* html, HtmlElement* hsdl )
{
	HtmlElement* html_it;
	struct HSDLCounter *counter;
	
	if( !html || !hsdl )
		return NULL;
	
	counter = hsdl_new_counter( NULL );
	// Iterate all hsdl elements to keep order
	while ( hsdl ) {
		html_it = html_contains_element( html, hsdl );
		if ( html_it ) {
			// Iterate all valid html siblings
			while ( html_it ) {
				// increment hsdl counter if available
				if ( !hsdl_counter_increment( counter, hsdl->tag_name ) ) {
					// not available, append to counter list
					hsdl_counter_append( &counter, hsdl_new_counter( hsdl->tag_name ) );
				}
				// check instruction restrictions like index number
				if ( hsdl_instruction_check( html_it, hsdl, counter ) ) {
					// do instruction on element
					hsdl_instructions_apply( hsdlState, html_it, hsdl, hsdl_counter_lookup( counter, html_it->tag_name ) );

					hsdl_parse( hsdlState, html_it->child, hsdl->child );
				}
				html_it = html_contains_element( html_it->sibling, hsdl );
			}
			hsdl_counter_free( counter );
			counter = hsdl_new_counter( NULL );
		} else {
			if ( hsdl->tag == HTML_TAG_NONE ) {
//				fprintf( stderr, "<HSDL ignore content> %s", hsdl->text );
			} else {
//				fprintf( stderr, "<HSDL element not found> %s", hsdl->tag_name );
			}
//			hsdl_attrib_print( hsdl->attrib, stderr );
//			fprintf( stderr, "\n" );
		}
		
		hsdl = hsdl->sibling;
	}
	hsdl_counter_free( counter );
	return NULL;
}

static void hsdl_state_parse( struct HSDLState **state, HtmlElement **hsdl ) {
	HtmlAttrib *hsdl_attrib;
	HtmlElement *hsdl_element;
	
	// Init hsdl state
	hsdl_element = (*hsdl)->child;
	if ( hsdl_element && !strcmp( hsdl_element->tag_name, "?hsdl" ) ) {
		hsdl_attrib = hsdl_element->attrib;
		while(hsdl_attrib) {
			if(*hsdl_attrib->key_name == HSDL_INSTRUCTION_PREFIX && !strcmp(hsdl_attrib->key_name+1, hsdl_states[HSDL_STATE_OUTPUT]) && hsdl_attrib->value) {
				(*state)->output = hsdl_states_output_lookup(hsdl_attrib->value);
			}
			if(*hsdl_attrib->key_name == HSDL_INSTRUCTION_PREFIX && !strcmp(hsdl_attrib->key_name+1, hsdl_states[HSDL_STATE_URL_ENCODE_CONTENT])) {
				(*state)->url_encode = 1;
			}
			hsdl_attrib = hsdl_attrib->next;
		}
		
		// Remove state element from DOM
		(*hsdl)->child = (*hsdl)->child->sibling;
		
		hsdl_element->sibling = 0;
		html_free_element( hsdl_element );
	}
}

static void hsdl_state_free( struct HSDLState *hsdl_state ) {
	memset( hsdl_state, 0, sizeof( struct HSDLState ) );
	free( hsdl_state );
}

static void hsdl_post_output_function( 
	struct HSDLState *hsdlState, 
	enum HSDL_INSTRUCTION instruction, 
	struct HtmlElement *html,
	const char* id, 
	const char* key, 
	const char* value, 
	int count,
	void* extra
) {
	char* text_tmp;
	
	switch(instruction) {
		case HSDL_INSTRUCTION_ATTRIBUTE:
			printf("%s\n", value);
			break;
		case HSDL_INSTRUCTION_VARNAME:
			if ( value ) {
				if (hsdlState->output == HSDL_OUTPUT_INI) {
					printf("[");
				}
				
				printf("%s", value);
				if ( count >= 0 ) {
					printf("_%i", count);
				}
				
				if (hsdlState->output == HSDL_OUTPUT_INI) {
					printf("]");
				}
			}
			printf("\n");
			break;
		case HSDL_INSTRUCTION_VARNAME_CONTENT:
			if ( value ) {
				printf("%s", value);
				if ( count >= 0 ) {
					printf("_%i", count);
				}
				printf("=");
			}
			break;
		case HSDL_INSTRUCTION_TEXTALL:
			if ( value ) {
				if ( count >= 0 ) {
					printf("%s_%i=", value, count);
				} else {
					printf("%s=", value);
				}
			}
			hsdl_instruction_text_all(html->child, hsdlState->url_encode);
			printf("\n");
			break;
		case HSDL_INSTRUCTION_CONTENT:
		case HSDL_INSTRUCTION_CONTENT_AFTER:
			if ( hsdlState->url_encode ) {
				hsdl_url_encode( &text_tmp, value, strlen( value ) );
				printf("%s\n", text_tmp);
				fflush( stdout );
				free( text_tmp );
			} else {
				printf("%s\n", value);
			}
			break;
		default:
			break;
	}
}

/*
 * Start DOM parsing
 */
void *hsdl_generate( struct HSDL* hsdl )
{
	hsdl_state_parse( &hsdl->state, &hsdl->hsdl_doc->root_element );
	if ( !hsdl->state->hsdl_post_output_function ) {
		hsdl->state->hsdl_post_output_function = &hsdl_post_output_function;
	}
	hsdl_parse( hsdl->state, hsdl->html_doc->root_element, hsdl->hsdl_doc->root_element );
	return 0;
}

void hsdl_init( struct HSDL* hsdl, unsigned long options ) {
	memset( hsdl, 0, sizeof( struct HSDL ) );
	
	hsdl->state = (struct HSDLState *)malloc( sizeof( struct HSDLState ) );
	memset( hsdl->state, 0, sizeof( struct HSDLState ) );
	
	hsdl->state->options = options;
}

void hsdl_free( struct HSDL* hsdl ) {
	html_free_document( hsdl->html_doc );
	
	if ( !(hsdl->state->options & HSDL_STATE_OPTION_THREAD_SAFE) ) {
		html_free_document( hsdl->hsdl_doc );
	}

	hsdl_state_free( hsdl->state );
}

static void hsdl_output_function( 
	struct HSDLState *hsdlState, 
	HtmlElement *hsdl, 
	HtmlElement *html, 
	HtmlAttrib *attrib, 
	enum HSDL_INSTRUCTION instruction,
	int count
) {
	HtmlAttrib *attrib_tmp;
	char *hsdl_id = NULL;
	
	attrib_tmp = hsdl_attrib_find_by_key(hsdl->attrib, ":id");
	if ( attrib_tmp ) {
		hsdl_id = stringduplicate_length( attrib_tmp->value, strlen( attrib_tmp->value ) );
	}
	
	switch(instruction) {
		case HSDL_INSTRUCTION_ATTRIBUTE:
			attrib_tmp = hsdl_attrib_find_by_key( hsdl->attrib, ":varname" );
			if (attrib_tmp) {
				hsdlState->hsdl_post_output_function( hsdlState, HSDL_INSTRUCTION_VARNAME_CONTENT, html, hsdl_id, attrib_tmp->key_name, attrib_tmp->value, count, hsdlState->hsdl_post_output_function_extra );
			}
			
			// Find attribute specified from hsdl
			attrib_tmp = hsdl_attrib_find_by_key( html->attrib, attrib->value );
			if (attrib_tmp) {
				hsdlState->hsdl_post_output_function( hsdlState, HSDL_INSTRUCTION_ATTRIBUTE, html, hsdl_id, attrib_tmp->key_name, attrib_tmp->value, count, hsdlState->hsdl_post_output_function_extra );
			} else {
				hsdlState->hsdl_post_output_function( hsdlState, HSDL_INSTRUCTION_ATTRIBUTE, html, hsdl_id, "", "", count, hsdlState->hsdl_post_output_function_extra );
			}
			break;
		case HSDL_INSTRUCTION_EXTRA:
			attrib_tmp = hsdl_attrib_find_by_key( hsdl->attrib, ":extra" );
			if (attrib_tmp) {
				hsdlState->hsdl_post_output_function( hsdlState, HSDL_INSTRUCTION_EXTRA, html, hsdl_id, attrib_tmp->key_name, attrib_tmp->value, count, hsdlState->hsdl_post_output_function_extra );
			}
			break;
		case HSDL_INSTRUCTION_ID:
			attrib_tmp = hsdl_attrib_find_by_key( hsdl->attrib, ":id" );
			if (!attrib_tmp) {
				break;
			}
			hsdlState->hsdl_post_output_function( hsdlState, HSDL_INSTRUCTION_ID, html, hsdl_id, attrib_tmp->key_name, attrib_tmp->value, count, hsdlState->hsdl_post_output_function_extra );
			break;
		case HSDL_INSTRUCTION_VARNAME:
		
			// If there is an attribute selector => do not print varname here
			attrib_tmp = hsdl_attrib_find_by_key( hsdl->attrib, ":attribute" );
			if (attrib_tmp) {
				break;
			}
		
			attrib_tmp = hsdl_attrib_find_by_key(hsdl->attrib, ":content");
			if (!attrib_tmp) {
				attrib_tmp = hsdl_attrib_find_by_key(hsdl->attrib, ":textall");
			}
			
			if (!attrib_tmp) {
				// TODO: Check this
				if (hsdlState->output == HSDL_OUTPUT_INI) {
					hsdlState->hsdl_post_output_function( hsdlState, HSDL_INSTRUCTION_VARNAME, html, hsdl_id, attrib->key_name, attrib->value, count, hsdlState->hsdl_post_output_function_extra );
				} else {
					hsdlState->hsdl_post_output_function( hsdlState, HSDL_INSTRUCTION_VARNAME, html, hsdl_id, attrib->key_name, attrib->value, count, hsdlState->hsdl_post_output_function_extra );
				}
			}
			break;
		case HSDL_INSTRUCTION_TEXTALL:
			attrib_tmp = hsdl_attrib_find_by_key(hsdl->attrib, ":varname");
			if (attrib_tmp) {
				hsdlState->hsdl_post_output_function( hsdlState, HSDL_INSTRUCTION_TEXTALL, html, hsdl_id, attrib_tmp->key_name, attrib_tmp->value, count, hsdlState->hsdl_post_output_function_extra );
			} else {
				hsdlState->hsdl_post_output_function( hsdlState, HSDL_INSTRUCTION_TEXTALL, html, hsdl_id, NULL, NULL, count, hsdlState->hsdl_post_output_function_extra );
			}
			break;
		case HSDL_INSTRUCTION_CONTENT_AFTER:
			html = html->sibling;
			if (!html) {
				break;
			}
			if (html->tag == HTML_TAG_NONE) {
				hsdlState->hsdl_post_output_function( hsdlState, HSDL_INSTRUCTION_CONTENT_AFTER, html, hsdl_id, html->tag_name, html->text, count, hsdlState->hsdl_post_output_function_extra );
			}
			break;
		case HSDL_INSTRUCTION_CONTENT:
			attrib_tmp = hsdl_attrib_find_by_key(hsdl->attrib, ":varname");
			if (attrib_tmp) {
				// TODO: Even check this
				if (attrib_tmp->value) {
					hsdlState->hsdl_post_output_function( hsdlState, HSDL_INSTRUCTION_VARNAME_CONTENT, html, hsdl_id, attrib_tmp->key_name, attrib_tmp->value, count, hsdlState->hsdl_post_output_function_extra );
				} else {
					hsdlState->hsdl_post_output_function( hsdlState, HSDL_INSTRUCTION_VARNAME, html, hsdl_id, attrib_tmp->key_name, html->tag_name, count, hsdlState->hsdl_post_output_function_extra );
				}
			}
			if (html->child && html->child->text) {
				hsdlState->hsdl_post_output_function( hsdlState, HSDL_INSTRUCTION_CONTENT, html, hsdl_id, html->tag_name, html->child->text, count, hsdlState->hsdl_post_output_function_extra );
			} else {
				hsdlState->hsdl_post_output_function( hsdlState, HSDL_INSTRUCTION_CONTENT, html, hsdl_id, html->tag_name, "", count, hsdlState->hsdl_post_output_function_extra );
			}
			break;
		default:
			break;
	}
	free( hsdl_id );
}