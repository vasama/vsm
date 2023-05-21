identifier: [_a-zA-Z] [_a-zA-Z0-9]* ;

@fragment integer_bin: '0b' [0-1]+ ;
@fragment integer_oct: '0' [0-7]* ;
@fragment integer_dec: [1-9] [0-9]* ;
@fragment integer_hex: '0x' [0-9a-fA-F]+ ;

integer:
	integer_bin
|	integer_oct
|	integer_dec
|	integer_hex
;

hash:               '#' ;
hash_hash:          '##' ;
lparen:             '(' ;
rparen:             ')' ;
comma:              ',' ;
ellipsis:           '...' ;
whitespace:         [ \t] ;
end_of_line:        '\r'? '\n' ;
add:                '+' ;
sub:                '-' ;
mul:                '*' ;
div:                '/' ;
mod:                '%' ;
and:                '&' ;
or:                 '|' ;
xor:                '^' ;
logical_and:        '&&' ;
logical_or:         '||' ;
eq:                 '==' ;
ne:                 '!=' ;
lt:                 '<' ;
gt:                 '>' ;
le:                 '<=' ;
ge:                 '>=' ;
