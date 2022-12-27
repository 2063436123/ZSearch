grammar Query;


prog : query ';'
    | cquery ';'
    ;

parameter : ID
    | '*'
    ;

agg_expr : Agg_op '(' parameter ')' ;

where : agg_expr Cmp_op INT ;

having : where
    | having 'AND' having
    | having 'OR' having
    | 'NOT' having
    | '(' having ')'
    ;

term : ID
    | ID ':' ID // 前缀限定单词的查询范围
    ;

query : query 'AND' query
    | query 'OR' query
    | 'NOT' query
    | '(' query ')'
    | term
    ;

cquery : query 'having' having;

Agg_op : 'SUM'
    | 'COUNT'
    | 'AVG'
    | 'MAX'
    | 'MIN'
    ; // More ...

Cmp_op : '>'
    | '<'
    | '='
    | '<='
    | '>='
    | '!='
//    | 'IN'
//    | 'NOT IN'
    ;


ID: [a-zA-Z_]+ ;

INT : [0-9]+ ;

STRING: [a-zA-Z]+ ;

WS : [ \r\t\n]+ -> skip ;