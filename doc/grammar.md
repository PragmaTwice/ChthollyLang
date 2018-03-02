# ![Chtholly Lang Logo](logo/logo180.png) Grammar of Chtholly Lang

## Conventions

*The grammar will be described in a form of an abstract syntax, and the following conventions are adopted in defining grammar rules for abstract syntax.*

### **rules identical with EBNF** *([ISO/IEC 14977: 1996](http://www.cl.cam.ac.uk/~mgk25/iso-14977.pdf))*

* **'c'** :  a terminal derived as 'c'

Let A and B be two (non-)terminal symbols:

* **A | B** : a non-terminal possibly derived as A or B
* **A - B** : a non-terminal derived as A except B
* **A = B** : A will be derived as B

### **rules only for Chtholly Lang**

* **"characters"** :  a terminal derived as 'characters'

* **'b' ... 'e'** : a terminal possibly derived as 'b','c','d'... or 'e', with a dictionary order

Let A and B be two (non-)terminal symbols:

* **A*** : a possibly empty sequence of iterations of A
* **A+** : a non-empty sequence of iterations of A
* **A?** : an optional occurrence of A
* **A  B** : a non-terminal derived as A followed with B
* **A ^ B** : A except followed with B

* **AnyChar** : a non-terminal derived as any character

* **Gen(formal-parameter-list) = rule expression** : define a non-terminal generator, formal parameters in formal-parameter-list will appear in rule expression.
* **Gen(actual-parameter-list)** : generate a non-terminal by giving parameters to a pre-defined generator, and actual parameters in actual-parameter-list will replace the formal parameters in defination.

* ***formal-parameter-list** : a non-empty sequence of identifiers separated by ','*
* ***actual-parameter-list** : a non-empty sequence of rule expressions separated by ','*


## Grammars

### ***generators***

* **AnyCharUntil(A)** = A | ( (AnyChar ^ A), AnyChar, A )

* **Term(A)** = ( Space* Comment ) Space* A

* **MultiExpressionPackage(A)** = A ( (';'|',') A )* (';'|',')?

* **BinaryOperator(A,B)** = A (B A)*

### ***tokens***

* **Space** = '\t' | '\n' | '\v' | '\f' | '\r' | ' '

* **Digit** = '0' ... '9'

* **UpperCaseLetter** = 'A' ... 'Z'

* **LowerCaseLetter** = 'a' ... 'z'

* **Letter** = UpperCaseLetter | LowerCaseLetter

* **DigitOrLetter** = Digit | Letter

* **IntLiteral** = Digit+

* **FloatLiteral** = Digit+ '.' Digit* (('E'|'e') ('+'|'-')? Digit+)?

* **UnescapedCharacter** = AnyChar - ('"' | '\\')

* **EscapedCharacter** = '\\' ('"' | '\\' | 'b' | 'f' | 'n' | 'r' | 't' | 'v')

* **StringLiteral** = '"' (EscapedCharacter|UnescapedCharacter)* '"'

* **Identifier** = (Letter | '\_') (DigitOrLetter | '\_')*

* **NullLiteral** = "null"

* **UndefinedLiteral** = "undef"

* **TrueLiteral** = "true"

* **FalseLiteral** = "false"

* **Literal** = FloatLiteral | IntLiteral | StringLiteral | NullLiteral | UndefinedLiteral | TrueLiteral | FalseLiteral

### ***comments***

* **MultiLineComment** = "/\*" AnyCharUntil("*/")

* **SingleLineComment** = "//" AnyCharUntil('\n')

* **Comment** = SingleLineComment | MultiLineComment

### ***expressions***

* **SigleExpression** = PairExpression

* **Expression** = MultiExpressionPackage(SigleExpression)

* **ExpressionList** = '(' Expression ')'

* **ArrayList** = '[' (SigleExpression (',' SigleExpression)*)? ']'

* **DictList** = '{' (SigleExpression (',' SigleExpression)*)? '}'

* **UndefExpression** = '(' ')'

* **List** = UndefExpression | ExpressionList | ArrayList | DictList

* **PrimaryExpression** = Literal | Identifier | List

* **ConstraintPartAtConstraintExperssion** = ':' PrimaryExpression

* **ConstraintPartAtConstraintExperssionAtPatternExperssion** = ':' SigleExpression

* **ConstraintExperssion** = Identifier ConstraintPartAtConstraintExperssion?

* **ConstraintExperssionAtPatternExperssion** = Identifier "..."? ConstraintPartAtConstraintExperssionAtPatternExperssion?

* **PatternExperssion** = '(' MultiExpressionPackage(ConstraintExperssionAtPatternExperssion) ')' ConstraintPartAtConstraintExperssion?

* **VarDefineExpression** = "var" (ConstraintExperssion | PatternExperssion) List?

* **ConstDefineExpression** = "const" (ConstraintExperssion | PatternExperssion) List?

* **DefineExpression** = PrimaryExpression | VarDefineExpression | ConstDefineExpression

* **LambdaExperssion** = DefineExpression | "fn" PatternExperssion SigleExpression

* **ConditionExpression** = LambdaExperssion | "if" ExpressionList SigleExpression ("else" SigleExpression)?

* **ReturnExpression** = ConditionExpression | "return" SigleExpression?

* **BreakExpression** = "break" SigleExpression?

* **ContinueExpression** = "continue" SigleExpression?

* **LoopControlExpression** = ReturnExpression | (BreakExpression | ContinueExpression)

* **WhileLoopExpression** = LoopControlExpression | "while" ExpressionList SigleExpression

* **DoWhileLoopExpression** = WhileLoopExpression | "do" SigleExpression "while" ExpressionList

* **FunctionExpression** = DoWhileLoopExpression List*

* **PointExpression** = FunctionExpression | PointExpression '->' FunctionExpression

* **FoldExperssion** = PointExpression "..."*

* **UnaryExpression** = FoldExperssion | (('+' | '-') | "not") UnaryExpression

* **MultiplicativeExpression** = BinaryOperator( UnaryExpression, '*'|'/'|'%')

* **AdditiveExpression** = BinaryOperator( MultiplicativeExpression, '+'|'-')

* **RelationalExpression** = BinaryOperator( AdditiveExpression, '<'|'>'|"<="|">=")

* **EqualityExpression** = BinaryOperator( RelationalExpression, "=="|"<>")

* **LogicalAndExpression** = BinaryOperator( EqualityExpression, "and" )

* **LogicalOrExpression** = BinaryOperator( LogicalAndExpression, "or" )

* **AssignmentOperator** = '=' | '*=' | '/=' | '%=' | '+=' | '-='

* **AssignmentExpression** = LogicalOrExpression | LogicalOrExpression AssignmentOperator SigleExpression

* **PairExperssion** = AssignmentExpression | PairExperssion ':' SigleExpression
