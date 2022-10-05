// 符号[...]表示方括号内包含的为可选项
// 符号{...}表示花括号内包含的为可重复0次或多次的项
// 终结符或者是由单引号括起的串，或者是 Ident、InstConst 这样的记号

CompUnit ::= {Decl} {FuncDef} MainFuncDef
//FIRST Decl = CONSTTK | INTTK
//FIRST FuncDef = VOIDTK | INTTK
//FIRST MainFuncDef = INTTK
// Decl与FuncDef可以根据超前判断得出

Decl ::=  ConstDecl // FIRST = CONSTTK
        | VarDecl   // FIRST = INTTK

ConstDecl ::= 'const' BType ConstDef { ',' ConstDef } ';'
// 根据逗号判断是否解析ConstDef

BType ::= 'int'

ConstDef ::= Ident { '[' ConstExp ']' } '=' ConstInitVal
// 根据[判断是否解析ConstExp

ConstInitVal ::=  ConstExp // FIRST = LPARENT | IDENFR | INTCON | PLUS | MINU | NOT
                | '{' [ ConstInitVal { ',' ConstInitVal } ] '}'
// 首先根据{判断走哪一条路，根据}判断是否解析一个或多个ConstInitVal

VarDecl ::= BType VarDef { ',' VarDef } ';'
// 根据逗号判断是否解析VarDef

VarDef ::=  Ident { '[' ConstExp ']' }
          | Ident { '[' ConstExp ']' } '=' InitVal
// 解析到最后看有没有等号就可以了。

InitVal ::= Exp // FIRST = LPARENT | IDENFR | INTCON | PLUS | MINU | NOT
          | '{' [ InitVal { ',' InitVal } ] '}' 
// 可以根据{决定走哪一条路 

FuncDef ::= FuncType Ident '(' [FuncFParams] ')' Block
// 根据)判断是否有FuncFParams

MainFuncDef ::= 'int' 'main' '(' ')' Block

FuncType ::=  'void' 
            | 'int'

FuncFParams ::= FuncFParam { ',' FuncFParam }

FuncFParam ::= BType Ident [ '[' ']' { '[' ConstExp ']' } ] 
// 可以根据[判断是否有可选项

Block ::= '{' { BlockItem } '}'
// 如果{后面不是}，就直接解析BlockItem

BlockItem ::= Decl  // FIRST = CONSTTK | INTTK
            | Stmt  
// 根据FIRST选择Decl还是Stmt

Stmt ::=  LVal '=' Exp ';' // FIRST = IDENFR
        | [Exp] ';' // FIRST = SEMICN | // FIRST = LPARENT | IDENFR | INTCON | PLUS | MINU | NOT  
        | Block // FIRST = LBRACE
        | 'if' '(' Cond ')' Stmt [ 'else' Stmt ]
        | 'while' '(' Cond ')' Stmt
        | 'break' ';' 
        | 'continue' ';'
        | 'return' [Exp] ';'
        | LVal '=' 'getint' '(' ')' ';'
        | 'printf' '(' FormatString{ ',' Exp} ')' ';'

Exp ::= AddExp

Cond ::= LOrExp

LVal ::= Ident { '[' Exp ']' }

PrimaryExp ::=  '(' Exp ')' 
              | LVal    // FIRST = IDENFR
              | Number  // FIRST = INTCON

Number ::= IntConst

UnaryExp ::=  PrimaryExp // FIRST = LPARENT | IDENFR | INTCON
            | Ident '(' [FuncRParams] ')' // FIRST = IDENFR
            | UnaryOp UnaryExp  // FIRST = PLUS | MINU | NOT

UnaryOp ::= '+' | '-' | '!'

FuncRParams ::= Exp { ',' Exp }

MulExp ::=  UnaryExp 
          | MulExp ( '*' | '/' | '%' ) UnaryExp // 左递归

MulExp ::= UnaryExp{ ( '*' | '/' | '%' ) UnaryExp } 

AddExp ::=  MulExp
          | AddExp ( '+' | '-' ) MulExp       // 左递归

AddExp ::= MulExp{( '+' | '-' ) MulExp }

RelExp ::=  AddExp 
          | RelExp ('<' | '>' | '<=' | '>=' ) AddExp  // 左递归


EqExp ::= RelExp 
        | EqExp ( '==' | '!=' ) RelExp  // 左递归

LAndExp ::= EqExp 
          | LAndExp '&&' EqExp  // 左递归

LOrExp ::=  LAndExp
          | LOrExp '||' LAndExp // 左递归

ConstExp ::= AddExp




