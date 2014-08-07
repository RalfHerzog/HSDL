#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "html.h"
#include "hsdl.h"

#define BUFFSIZE 1024

int main(int argc, char **argv) {
	FILE *f;
	HtmlParseState *parse_state;
	char buffer[BUFFSIZE + 1];
	const char *token = buffer;
	struct HSDL hsdl;
	
	size_t len = 0;
	size_t total_len = 0;
	size_t buffsize = BUFFSIZE;

	buffer[BUFFSIZE] = 0;
	
	hsdl_init( &hsdl, 0 );
	
	/* Parse html */
	parse_state = html_parse_begin();
	
	if (argc != 3) {
		f = stdin;
	} else {
		f = fopen( argv[2], "r" );
	}
	while(!feof(f)) {
		len = fread(buffer + (BUFFSIZE - buffsize), 1, buffsize, f);
		if (!len) {
			buffsize = BUFFSIZE;
			continue;
		}
		total_len += len;
		token = html_parse_stream(parse_state, buffer + (BUFFSIZE - buffsize), buffer, len);
		buffsize = (token - buffer);
		memmove(buffer, token, BUFFSIZE - buffsize);
	}
	hsdl.html_doc = html_parse_end(parse_state);
	fclose(f);
	
	/* Parse hsdl */
	parse_state = html_parse_begin();

	if( !(f = fopen( argv[1], "r" ) ) ) {
		fprintf( stderr, "error: cannot open file %s\n", argv[1] );
		return 1;
	}
	while(!feof(f)) {
		len = fread(buffer + (BUFFSIZE - buffsize), 1, buffsize, f);
		if (!len) {
			buffsize = BUFFSIZE;
			continue;
		}
		total_len += len;
		token = html_parse_stream(parse_state, buffer + (BUFFSIZE - buffsize), buffer, len);
		if (!token) {
			continue;
		}
		buffsize = (token - buffer);
		memmove(buffer, token, BUFFSIZE - buffsize);
	}
	hsdl.hsdl_doc = html_parse_end(parse_state);
	fclose(f);
	
//	parse_dom(doc);
//	html_print_dom(html);
	
	hsdl_generate( &hsdl );

	hsdl_free( &hsdl );
	return 0;
}
