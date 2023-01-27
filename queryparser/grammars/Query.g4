grammar Query;

prog : query ';'
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

terms : terms 'AND' terms
    | terms 'OR' terms
    | 'NOT' terms
    | '(' terms ')'
    | term
    ;

query : terms? ('having' having)? ('order by' (ID|agg_expr))?;

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

STRING:  '"' .*? '"';

WS : [ \r\t\n]+ -> skip ;