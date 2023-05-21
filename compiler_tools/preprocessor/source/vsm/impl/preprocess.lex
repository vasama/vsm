@fragment line: ( [^\n] | '\\\n' )* '\n'? ;

identifier: [_a-zA-Z] [_a-zA-Z0-9]* ;

line_comment: '//' line ;
block_comment: '/*' .+? '*/' ;
string_literal: '"' .+? '"' ;

directive: '#' line ;
end_of_line: '\n' ;
