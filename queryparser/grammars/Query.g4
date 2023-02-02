grammar Query;

prog : query ';'
    ;

aggExpr : AggOp '(' ID ')'
    | ID;

value : INT
    | STRING;

valueList : '(' (value ',')* value ')';

where : aggExpr CmpOp valueList;

having : where
    | having 'AND' having
    | having 'OR' having
    | 'NOT' having
    | '(' having ')'
    ;

term : ID
//    | ID ':' ID // 前缀限定单词的查询范围 -> 可用 having a = 'b' 替代
    ;

terms : terms 'AND' terms
    | terms 'OR' terms
    | 'NOT' terms
    | '(' terms ')'
    | term
    ;

query : terms? ('having' having)? ('order by' aggExpr)?;

AggOp : 'SUM'
    | 'COUNT'
    | 'AVG'
    | 'MAX'
    | 'MIN'
    ; // More ...

CmpOp : '>'
    | '<'
    | '='
    | '<='
    | '>='
    | '!='
    | 'IN'
    | 'NOT IN'
    ;


ID: [a-zA-Z_]+ ;

INT : [0-9]+ ;

STRING:  '"' .*? '"';

WS : [ \r\t\n]+ -> skip ;