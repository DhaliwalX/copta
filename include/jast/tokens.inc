#ifndef O
#define O(t, k, p)
#endif

O(LPAREN, "(", 0)
O(RPAREN, ")", 0)
O(LBRACK, "[", 0)
O(RBRACK, "]", 0)
O(LBRACE, "{", 0)
O(RBRACE, "}", 0)
O(COLON, ":", 0)
O(SEMICOLON, ";", 0)
O(PERIOD, ".", 0)
O(ELLIPSIS, "...", 0)
O(CONDITIONAL, "?", 2)
O(INC, "++", 0)
O(DEC, "--", 0)
O(ARROW, "=>", 0)
O(ASSIGN, "=", 2)
O(ASSIGN_BIT_OR, "|=", 2)
O(ASSIGN_BIT_XOR, "^=", 2)
O(ASSIGN_BIT_AND, "&=", 2)
O(ASSIGN_SHL, "<<=", 2)
O(ASSIGN_SAR, ">>=", 2)
O(ASSIGN_SHR, ">>>=", 2)
O(ASSIGN_ADD, "+=", 2)
O(ASSIGN_SUB, "-=", 2)
O(ASSIGN_MUL, "*=", 2)
O(ASSIGN_DIV, "/=", 2)
O(ASSIGN_MOD, "%=", 2)
O(COMMA, ",", 1)
O(OR, "||", 4)
O(AND, "&&", 5)
O(BIT_OR, "|", 6)
O(BIT_XOR, "^", 7)
O(BIT_AND, "&", 8)
O(SHL, "<<", 11)
O(SAR, ">>", 11)
O(SHR, ">>>", 11)
O(ADD, "+", 12)
O(SUB, "-", 12)
O(MUL, "*", 13)
O(DIV, "/", 13)
O(MOD, "%", 13)
O(EQ, "==", 9)
O(NE, "!=", 9)
O(EQ_STRICT, "===", 9)
O(NE_STRICT, "!==", 9)
O(LT, "<", 10)
O(GT, ">", 10)
O(NOT, "!", 0)
O(BIT_NOT, "~", 0)
O(LTE, "<=", 10)
O(GTE, ">=", 10)

#ifdef O
#undef O
#endif

#ifndef K
#define K(t, k, p)
#endif

K(INSTANCEOF, "instanceof", 10)
K(IN, "in", 10)
K(DELETE, "delete", 0)
K(TYPEOF, "typeof", 0)
K(DO, "do", 0)
K(ELSE, "else", 0)
K(FOR, "for", 0)
K(FUNCTION, "function", 0)
K(IF, "if", 0)
K(RETURN, "return", 0)
K(THIS, "this", 0)
K(WHILE, "while", 0)
K(EXTERN, "extern", 0)
K(NULL_LITERAL, "null", 0)
K(TRUE_LITERAL, "true", 0)
K(FALSE_LITERAL, "false", 0)
K(TYPE, "type", 0)
K(CONST, "const", 0)
K(LET, "let", 0)


#ifdef K
#undef K
#endif

#ifndef T
#define T(t, k, p)
#endif

T(NUMBER, "NUMBER", 0)
T(STRING, "STRING", 0)
T(TEMPLATE, "TEMPLATE", 0)
T(IDENTIFIER, "ID", 0)
T(REGEX, "REGEX", 0)
T(END_OF_FILE, "EOF", 0)
T(ERROR, "ERROR", 0)
T(INVALID, "INVALID", 0)

#ifdef T
#undef T
#endif
