whitespace_trivia: [ \t\r\n]+ ;
line_comment_trivia: '#' [^\n]* '\n'? ;

identifier: [_a-zA-Z] [_a-zA-Z0-9]* ;

integer:
	[0-9]+
|	'0x' [0-9a-fA-F]+
;

at:             '@' ;
colon:          ':' ;
semicolon:      ';' ;
comma:          ',' ;
lparen:         '(' ;
rparen:         ')' ;
lbrace:         '{' ;
rbrace:         '}' ;
question:       '?' ;
star:           '*' ;
plus:           '+' ;
pipe:           '|' ;


@fragment escape_sequence_value:
	[\\rnt]
|	'x' [0-9a-fA-F]{2}
;

@fragment escape_sequence:
	'\\' escape_sequence_value ;


@fragment literal_value:
	[^']
|	'\\\''
|	escape_sequence
;

literal_sequence: '\'' literal_value+ '\'' ;


@fragment literal_alternative_value:
	[^\^\-\]]
|	'\\^'
|	'\\-'
|	'\\]'
|	escape_sequence
;

@fragment literal_alternative_range:
	literal_alternative_value
|	literal_alternative_value '-' literal_alternative_value
;

literal_alternative: '[' literal_alternative_range+ ']' ;
