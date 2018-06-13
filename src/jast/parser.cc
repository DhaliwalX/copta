#include "jast/parser.h"
#include "jast/ast-builder.h"
#include "jast/token.h"
#include "jast/types/type-system.h"

#include <sstream>

namespace jast {

#define EXPECT(tok)     \
    do {    \
        if (peek() != tok)  \
            throw SyntaxError(lex()->currentToken(),    \
                        String("expected a ") + Token::str(tok)); \
        advance();  \
    } while (0)

class NewScope {
public:
    NewScope(Parser *parser)
        : parser_{parser}
    {
        Setup();
    }

    ~NewScope() {
        parser_->scope_manager()->PopScope();
    }

    void Setup() {
        // parser_->scope_manager()->NewScope()
    }

private:
    Parser *parser_;
};

class ForInLoopParsingEnvironment {
public:
    ForInLoopParsingEnvironment(Parser *parser)
        : parser_{ parser }, prev_flags_{ parser->flags_ }
    {
        parser_->flags_.SetForInLoopParsing(true);
    }

    ~ForInLoopParsingEnvironment()
    {
        parser_->flags_ = prev_flags_;
    }

private:
    Parser *parser_;
    ParserFlags prev_flags_;
};

bool IsAssign(TokenType tok)
{
    return tok >= ASSIGN && tok <= ASSIGN_MOD;
}

Parser::Parser(ParserContext *context, ASTBuilder *builder, Tokenizer *lex, ScopeManager *manager)
 : ctx_{ context }, builder_{ builder }, lex_{ lex }, manager_{ manager }
{
}

Parser::~Parser()
{
}

String Parser::GetStringLiteral()
{
    return lex()->currentToken().view();
}

double Parser::ParseNumber(const Token &token)
{
    try {
        double num = std::stod(token.view());
        return num;
    } catch(std::exception &e) {
        throw SyntaxError(token, "invalid number (" + token.view() + ")");
    }
}

String Parser::GetIdentifierName()
{
    return lex()->currentToken().view();
}

TokenType Parser::peek()
{
    return lex()->peek();
}

void Parser::advance(bool not_regex)
{
    lex()->advance(not_regex);
}

Handle<Expression> Parser::ParseArrayLiteral()
{
    ProxyArray exprs;
    // eat '['
    advance();
    auto tok = peek();

    if (tok == RBRACK) {
        // done
        return builder()->NewArrayLiteral(exprs);
    }
    while (true) {
        auto one = ParseAssignExpression();
        exprs.push_back(Handle<Expression>(one));

        tok = peek();

        if (tok == RBRACK)
            break;
        EXPECT(COMMA);
    }

    return builder()->NewArrayLiteral(exprs);
}

Handle<Expression> Parser::ParseObjectMethod(const std::string &name)
{
    // auto args = ParseParameterList();
    // auto body = ParseBlockStatement();
    // auto proto = builder()->NewFunctionPrototype(name, args);
    // return builder()->NewFunctionStatement(proto->AsFunctionPrototype(), body);
    return nullptr;
}

Handle<Expression> Parser::ParseObjectLiteral()
{
    ProxyObject proxy;

    // eat the left brace '{'
    advance();
    auto tok = peek();

    // if next tok is '}' then nothing to be done
    if (tok == RBRACE) {
        return builder()->NewObjectLiteral(proxy);
    }

    std::string name;
    Handle<Expression> prop;
    while (true) {
        tok = peek();
        if (tok == STRING) {
            name = GetStringLiteral();
        } else if (tok == IDENTIFIER || IsKeyword(tok) || tok == NUMBER) {
            name = lex()->currentToken().view();
        } else {
            throw SyntaxError(lex()->currentToken(),
                        "expected an Identifier or a string");
        }

        advance();

        if (peek() == COLON) {
            advance();
            prop = ParseAssignExpression();
        }
        else if (peek() == LPAREN) {
            prop = ParseObjectMethod(name);
        } else
        // TODO: create a getter list in the ProxyObject class that will keep track of getters
        // and setters
        if (peek() == IDENTIFIER && (name == "get" || name == "set")) {
            name = lex()->currentToken().view();
            advance();
            prop = ParseObjectMethod(name);
        }

        proxy[name] = Handle<Expression>(prop);
        // next token should be a ',' or '}'
        tok = peek();
        if (tok == RBRACE)
            break;
        if (tok != COMMA)
            throw SyntaxError(lex()->currentToken(), "expected a ',' or '}'");
        advance();
    }

    return builder()->NewObjectLiteral(proxy);
}

Handle<Expression> Parser::ParsePrimary()
{
    auto tok = peek();
    Handle<Expression> result;

    if (tok == NULL_LITERAL) {
        result = builder()->NewNullLiteral();
    } else if (tok == NUMBER) {
        result = builder()->NewIntegralLiteral(
                                        ParseNumber(lex()->currentToken()));
    // } else if (tok == TEMPLATE) {
        // result = builder()->NewTemplateLiteral(lex()->currentToken().view());
    // } else if (tok == REGEX) {
    //     std::string regex = lex()->currentToken().view();
    //     auto pos = regex.rfind("$");
    //     auto flags = regex.substr(pos);
    //     std::vector<RegExpFlags> fs;
    //     for (auto &flag : flags) {
    //         if (flag == 'g') {
    //             fs.push_back(RegExpFlags::kGlobal);
    //         } else if (flag == 'i') {
    //             fs.push_back(RegExpFlags::kIgnoreCase);
    //         } else if (flag == 'm') {
    //             fs.push_back(RegExpFlags::kMultiline);
    //         } else if (flag == 'y') {
    //             fs.push_back(RegExpFlags::kSticky);
    //         } else if (flag == 'u') {
    //             fs.push_back(RegExpFlags::kUnicode);
    //         }
    //     }
    //     result = builder()->NewRegExpLiteral(regex.substr(0, pos), fs);
    } else if (tok == STRING) {
        result = builder()->NewStringLiteral(lex()->currentToken().view());
    } else if (tok == IDENTIFIER) {
        result = builder()->NewIdentifier(lex()->currentToken().view());
    } else if (tok == TRUE_LITERAL) {
        result = builder()->NewBooleanLiteral(true);
    } else if (tok == FALSE_LITERAL) {
        result = builder()->NewBooleanLiteral(false);
    } else if (tok == THIS) {
        result = builder()->NewThisHolder();
    } else if (tok == LPAREN) {
        advance();    // eat '('
        result = ParseBinaryExpression();
        tok = peek();

        if (tok != RPAREN)
            throw SyntaxError(lex()->currentToken(), "expected a ')'");
    } else if (tok == LBRACK) {
        result = ParseArrayLiteral();
    } else if (tok == LBRACE) {
        result = ParseObjectLiteral();
    } else if (tok == FUNCTION) {
        result = ParseFunctionStatement();
        return result;
    } else {
        throw SyntaxError(lex()->currentToken(), "expected a primary expression");
    }

    advance(true);
    return result;
}

Handle<Expression> Parser::ParseDotExpression()
{
    // eat the '.'
    advance();

    auto tok = peek();

    // this token should be a valid identifier
    if (tok != IDENTIFIER && !IsKeyword(tok))
        throw SyntaxError(lex()->currentToken(), "expected a valid identifier");
    auto name = GetIdentifierName();

    auto ident = builder()->NewIdentifier(name);
    advance(true);
    return ident;
}

Handle<Expression> Parser::ParseIndexExpression()
{
    // eat the '['
    advance();
    auto expr = ParseAssignExpression();
    if (lex()->peek() != RBRACK)
        throw SyntaxError(lex()->currentToken(), "expected a ']'");

    advance(true); // consumex ']'
    return expr;
}

// MemberExpression :
//    PrimaryExpression
//    FunctionExpression
//    MemberExpression [ Expression ]
//    MemberExpression . IdentifierName
//    new MemberExpression Arguments

// NewExpression ::
//   ('new')+ MemberExpression

// Examples of new expression:
// new foo.bar().baz means (new (foo.bar)()).baz
// new foo()() means (new foo())()
// new new foo()() means (new (new foo())())
// new new foo means new (new foo)
// new new foo() means new (new foo())
// new new foo().bar().baz means (new (new foo()).bar()).baz
Handle<Expression> Parser::ParseMemberExpression()
{
    Handle<Expression> primary = nullptr;
    primary = ParsePrimary();

    // if next token is neither '[' or '.'
    if (peek() != LBRACK && peek() != PERIOD && peek() != LPAREN)
        return primary;

    Handle<Expression> temp;
    Handle<Expression> member = primary;
    MemberAccessKind kind;
    while (true) {
        if (peek() == PERIOD) {
            temp = ParseDotExpression();
            kind = MemberAccessKind::kDot;
        } else if (peek() == LBRACK) {
            temp = ParseIndexExpression();
            kind = MemberAccessKind::kIndex;
        } else if (peek() == LPAREN) {
            auto args = ParseArgumentList();
            temp = builder()->NewArgumentList(args);
            kind = MemberAccessKind::kCall;
        } else {
            break;
        }
        member = builder()->NewMemberExpression(kind, member, temp);
    }

    return member;
}

Handle<ExpressionList> Parser::ParseArgumentList()
{
    Handle<ExpressionList> exprs = builder()->NewExpressionList();

    auto tok = peek();
    EXPECT(LPAREN);

    tok = peek();
    if (tok == RPAREN) {
        advance(true);
        return { };
    }

    while (true) {
        auto one = ParseAssignExpression();
        exprs->Insert(one);

        tok = peek();
        if (tok == RPAREN)
            break;
        if (tok != COMMA)
            throw SyntaxError(lex()->currentToken(), "expected a ',' or ')'");
        advance();
    }

    advance(true); // eat the last ')'
    return exprs;
}

// CallExpression :
//      MemberExpression Arguments
//      CallExpression Arguments
//      CallExpression [ Expression ]
//      CallExpression . IdentifierName
Handle<Expression> Parser::ParseCallExpression()
{
    auto func = ParseMemberExpression();
    auto tok = peek();

    if (tok != PERIOD && tok != LBRACK && tok != LPAREN)
        return func;
    Handle<Expression> member = func, temp = nullptr;
    MemberAccessKind kind;

    while (true) {
        tok = peek();
        if (tok == PERIOD) {
            temp = ParseDotExpression();
            kind = MemberAccessKind::kDot;
        } else if (tok == LBRACK) {
            temp = ParseIndexExpression();
            kind = MemberAccessKind::kIndex;
        } else if (tok == LPAREN) {
            auto args = ParseArgumentList();
            temp = builder()->NewArgumentList(args);
            kind = MemberAccessKind::kCall;
        } else {
            break;
        }
        member = builder()->NewCallExpression(kind, member, temp);
    }

    return member;
}

// NewExpression :
//      MemberExpression
//      new NewExpression
// Handle<Expression> Parser::ParseNewExpression()
// {
//     return ParseMemberExpression();
// }

// LeftHandSideExpression :
//      NewExpression
//      CallExpression
Handle<Expression> Parser::ParseLeftHandSideExpression()
{
    return ParseCallExpression();
}

PrefixOperation MapTokenWithPrefixOperator(Token &tok)
{
    switch(tok.type()) {
        case INC:       return PrefixOperation::kIncrement;
        case DEC:       return PrefixOperation::kDecrement;
        case BIT_AND:    return PrefixOperation::kAddr;
        case BIT_NOT:   return PrefixOperation::kBitNot;
        case NOT:       return PrefixOperation::kNot;
        case MUL:      return PrefixOperation::kDeref;
        default:    throw SyntaxError(tok, "unrecognised token");
    }
}

Handle<Expression> Parser::ParseUnaryExpression()
{
    // UnaryExpression :
    //     PostfixExpression
    //     delete UnaryExpression
    //     void UnaryExpression
    //     typeof UnaryExpression
    //     ++ UnaryExpression
    //     -- UnaryExpression
    //     + UnaryExpression
    //     - UnaryExpression
    //     ~ UnaryExpression
    //     ! UnaryExpression
    auto tok = peek();

    if (tok == ADD) {
        advance();
        // convert + (Expr) to Expr * 1
        return builder()->NewBinaryExpression(BinaryOperation::kMultiplication,
            ParseUnaryExpression(), builder()->NewIntegralLiteral(1.0));
    } else if (tok == SUB) {
        advance();

        // similarly for `-Expr` to `Expr * -1`
        return builder()->NewBinaryExpression(BinaryOperation::kMultiplication,
            ParseUnaryExpression(), builder()->NewIntegralLiteral(-1.0));
    } else if (tok == INC || tok == DEC || tok == NOT || tok == BIT_NOT || tok == MUL || tok == BIT_AND) {
        auto token = lex()->currentToken();
        lex()->advance();

        return builder()->NewPrefixExpression(MapTokenWithPrefixOperator(token),
                                              ParseUnaryExpression());
    }

    // PostfixExpression :
    //      LeftHandSideExpression
    //      LeftHandSideExpression [no LineTerminator here] ++
    //      LeftHandSideExpression [no LineTerminator here] --
    auto left = ParseLeftHandSideExpression();

    tok = peek();
    if (tok == INC) {
        advance();
        return builder()->NewPostfixExpression(PostfixOperation::kIncrement,
            left);
    } else if (tok == TokenType::DEC) {
        advance();
        return builder()->NewPostfixExpression(PostfixOperation::kDecrement,
            left);
    } else {
        return left;
    }
}

BinaryOperation MapBinaryOperator(Token &tok)
{
    switch(tok.type()) {
        case ADD:           return BinaryOperation::kAddition;
        case SUB:           return BinaryOperation::kSubtraction;
        case MUL:           return BinaryOperation::kMultiplication;
        case DIV:           return BinaryOperation::kDivision;
        case MOD:           return BinaryOperation::kMod;
        case SHL:           return BinaryOperation::kShiftLeft;
        case SAR:           return BinaryOperation::kShiftRight;
        case SHR:           return BinaryOperation::kShiftZeroRight;
        case LT:           return BinaryOperation::kLessThan;
        case GT:           return BinaryOperation::kGreaterThan;
        case LTE:           return BinaryOperation::kLessThanEqual;
        case GTE:           return BinaryOperation::kGreaterThanEqual;
        case EQ:           return BinaryOperation::kEqual;
        case NE:           return BinaryOperation::kNotEqual;
        case EQ_STRICT:           return BinaryOperation::kStrictEqual;
        case NE_STRICT:           return BinaryOperation::kStrictNotEqual;
        case AND:           return BinaryOperation::kAnd;
        case OR:           return BinaryOperation::kOr;
        case BIT_AND:           return BinaryOperation::kBitAnd;
        case BIT_OR:           return BinaryOperation::kBitOr;
        case BIT_XOR:           return BinaryOperation::kBitXor;
        case IN:            return BinaryOperation::kIn;
        default:       throw SyntaxError(tok,
                                    "unexpected token as binary operator");
    }
}

/// reference for this function ::= llvm/examples/Kaleidoscope/chapter3/toy.cpp
/// if you are unable to understand the function just imagine you are
/// parsing 2 + 3 * 5 - 6 / 7, (I too used that as a reference)
Handle<Expression> Parser::ParseBinaryExpressionRhs(int prec, Handle<Expression> lhs)
{
    while (true) {
        int tokprec = Token::precedence(peek());

        if (tokprec < prec) {
            return lhs;
        }

        // now we definitely have a binary operator
        auto op =  MapBinaryOperator(lex()->currentToken());
        advance();

        auto rhs = ParseUnaryExpression();

        auto nextprec = Token::precedence(peek());
        if (tokprec < nextprec) {
            rhs = ParseBinaryExpressionRhs(tokprec + 1, rhs);
        }


        // merge lhs and rhs
        lhs = builder()->NewBinaryExpression(op, lhs, rhs);
    }
}

Handle<Expression> Parser::ParseBinaryExpression()
{
    auto lhs = ParseUnaryExpression();

    // parse the rhs, if any
    return ParseBinaryExpressionRhs(3, lhs);
}

Handle<Expression> Parser::ParseAssignExpression()
{
    auto lhs = ParseBinaryExpression();
    auto tok = peek();

    if (!IsAssign(tok))
        return lhs;

    // TODO: information about what kind of assignment is done here should
    //  be stored here. (in the AST?)
    advance();
    auto rhs = ParseAssignExpression();
    return builder()->NewAssignExpression((lhs), (rhs));
}

Handle<Expression> Parser::ParseExpression()
{
    return ParseAssignExpression();
}

Handle<Expression> Parser::ParseExpressionOptional()
{
    auto tok = peek();

    if (tok == SEMICOLON) {
        return nullptr;
    } else {
        return ParseExpression();
    }
}

Handle<Expression> Parser::ParseElseBranch()
{
    // eat 'else'
    advance();
    return ParseStatement();
}

Handle<Expression> Parser::ParseIfStatement()
{
    Handle<Expression> result;

    // eat 'if'
    advance();

    auto tok = peek();
    EXPECT(LPAREN);

    // parse the condition of if statement
    auto condition = ParseBinaryExpression();
    EXPECT(RPAREN);

    // parse the body of 'if'
    auto body = ParseStatement();

    tok = peek();
    if (tok == ELSE) {
        result = builder()->NewIfElseStatement(condition, body,
            ParseElseBranch());
    } else {
        result = builder()->NewIfStatement(condition, body);
    }

    return result;
}

Handle<Expression> Parser::ParseForInStatement(Handle<Expression> inexpr)
{
    EXPECT(RPAREN);

        // parse 'for (x = 10; x < 100; x = x + 1) >>rest<<...' part
    auto body = ParseStatement();
    return builder()->NewForStatement(ForKind::kForIn,
        inexpr, nullptr, nullptr, body);
}

Handle<Expression> Parser::ParseForStatement()
{
    // eat 'for'
    advance();
    auto tok = peek();
    EXPECT(LPAREN);

    // parse 'for ( >>this<< ;...' part
    ForInLoopParsingEnvironment env(this);
    auto init = ParseVariableOrExpressionOptional();

    if (peek() == IN) {
        advance();
        auto expr = ParseAssignExpression();
        /* this is bit wrong as ast for this kind of syntax will be like
         *
         *          in
         *         /  \
         *        /     \
         * DeclList     expr
         *
         * We need to simplify the ast to simple `a in x` expression
         */
        init = builder()->NewBinaryExpression(BinaryOperation::kIn, init, expr);
        return ParseForInStatement(init);
    }
    if (init && init->IsBinaryExpression()) {
        auto op = init->AsBinaryExpression()->op();
        if (op == BinaryOperation::kIn) {
            return ParseForInStatement(init);
        }
    }

    if (init && !init->IsDeclarationList()) {
        EXPECT(SEMICOLON);
    }

    if (!init) {
        EXPECT(SEMICOLON);
    }

    Handle<Expression> condition;
    if (peek() == SEMICOLON) {
        condition = builder()->NewBooleanLiteral(true);
    } else {
        // parse 'for (x = 10; >>this<< ...' part
        condition = ParseBinaryExpression();
    }

    tok = peek();
    if (tok != SEMICOLON)
        throw SyntaxError(lex()->currentToken(), "expected a ';'");
    advance();

    Handle<Expression> update;
    if (peek() != RPAREN) {
        // parse 'for (x = 10; x < 100; >>this<<...' part
        update = ParseBinaryExpression();
    } else {
    }

    tok = peek();
    if (tok != RPAREN)
        throw SyntaxError(lex()->currentToken(), "expected a ')'");
    advance();

    // parse 'for (x = 10; x < 100; x = x + 1) >>rest<<...' part
    auto body = ParseStatement();
    return builder()->NewForStatement(ForKind::kForOf,
        init, condition, update, body);
}

Handle<Expression> Parser::ParseWhileStatement()
{
    advance(); // eat 'while'

    auto tok = peek();

    if (tok != LPAREN) {
        throw SyntaxError(lex()->currentToken(), "expected a '('");
    }
    advance();

    auto condition = ParseBinaryExpression();
    tok = peek();
    if (tok != RPAREN)
        throw SyntaxError(lex()->currentToken(), "expected a ')'");

    advance();
    auto body = ParseStatement();

    return builder()->NewWhileStatement(condition, body);
}

Handle<Expression> Parser::ParseDoWhileStatement()
{
    advance(); // eat 'do'
    auto body = ParseStatement();

    auto tok = peek();
    if (tok != WHILE)
        throw SyntaxError(lex()->currentToken(), "expected 'while'");
    advance();

    tok = peek();
    if (tok != LPAREN) {
        throw SyntaxError(lex()->currentToken(), "expected a '('");
    }
    advance();

    auto condition = ParseBinaryExpression();
    tok = peek();
    if (tok != RPAREN) {
        throw SyntaxError(lex()->currentToken(), "expected a ')'");
    }
    advance();

    tok = peek();
    if (tok != SEMICOLON)
        throw SyntaxError(lex()->currentToken(), "expected a ';'");
    advance();

    return builder()->NewDoWhileStatement(condition, body);
}

std::vector<std::string> Parser::ParseParameterList(std::vector<Type*> *param_types)
{
    auto tok = peek();
    auto result = std::vector<std::string>();

    if (tok != LPAREN)
        throw SyntaxError(lex()->currentToken(), "expected a '('");
    advance();

    // check for ')'
    tok = peek();
    if (tok == RPAREN) {
        advance();
        return { }; // return an empty vector
    }

    while (true) {
        tok = peek();

        if (tok != IDENTIFIER)
            throw SyntaxError(lex()->currentToken(), "expected an identifier");

        result.push_back(GetIdentifierName());
        advance();

        tok = peek();

        if (tok != COLON) {
            throw SyntaxError(lex()->currentToken(), "expected a ':'");
        }
        advance();
        Type *arg_type = ParseType();
        param_types->push_back(arg_type);

        tok = peek();
        if (tok == RPAREN)
            break;

        if (tok != COMMA)
            throw SyntaxError(lex()->currentToken(), "expected a ',' or ')'");
        advance();
    }

    // eat the ')'
    advance();
    return result;
}

Handle<FunctionPrototype> Parser::ParseFunctionPrototype()
{
    // eat 'function'
    advance();
    Type *ret_type = nullptr;
    std::vector<Type*> arg_types;
    auto tok = peek();

    std::string name;
    if (tok == IDENTIFIER) {
        name = GetIdentifierName();
        // eat the IDENT
        advance();
    }

    // parse the argument list
    auto args = ParseParameterList(&arg_types);
    tok = peek();

    if (tok == COLON) {
        advance();
        ret_type = ParseType();
    } else {
        ret_type = TypeSystem::getUndefinedType();
    }
    Type *t = TypeSystem::getFunctionType(ret_type, arg_types);
    return builder()->NewFunctionPrototype(name, args, t)
                ->AsFunctionPrototype();
}

Handle<Expression> Parser::ParseFunctionStatement()
{
    auto proto = ParseFunctionPrototype();
    auto body = ParseStatement();

    return builder()->NewFunctionStatement(proto, body);
}

Handle<Expression> Parser::ParseBlockStatement()
{
    Handle<ExpressionList> stmts = builder()->NewExpressionList();
    advance(); // eat '{'

    while (true) {
        auto tok = peek();
        if (tok == RBRACE)
            break;

        auto stmt = ParseStatement();
        stmts->Insert(stmt);
    }

    advance(); // eat '}'
    return builder()->NewBlockStatement(stmts);
}

Handle<Expression> Parser::ParseReturnStatement()
{
    advance(); // eat 'return'
    auto tok = peek();

    if (tok == SEMICOLON) {
        return builder()->NewReturnStatement(nullptr);
    }

    auto expr = ParseBinaryExpression();
    tok = peek();

    if (tok != SEMICOLON)
        throw SyntaxError(lex()->currentToken(), "expected a ';'");
    advance();
    return builder()->NewReturnStatement(expr);
}

Handle<Declaration> Parser::ParseDeclaration()
{
    auto tok = peek();
    if (tok != IDENTIFIER) {
        throw SyntaxError(lex()->currentToken(), "expected an identifier");
    }
    std::string name = GetIdentifierName();
    advance();

    tok = peek();

    Type *type = nullptr;
    if (tok == COLON) {
        advance();
        type = ParseType();
        tok = peek();
    } else {
        type = TypeSystem::getUnresolvedType();
    }
    if (tok == SEMICOLON || tok == COMMA || (flags_.IsForInLoopParsing() && tok == IN)) {
        return builder()->factory()->NewDeclaration(
            builder()->locator()->loc(), scope_manager()->current(), name, nullptr, type);
    } else if (tok != ASSIGN) {
        throw SyntaxError(lex()->currentToken(), "expected a '='");
    }
    advance();
    return builder()->factory()->NewDeclaration(
            builder()->locator()->loc(), scope_manager()->current(), name, ParseAssignExpression(), type);
}

Handle<Expression> Parser::ParseVariableOrExpressionOptional() {
    auto tok = peek();
    if (tok == LET || tok == CONST) {
        return ParseVariableStatement();
    }

    return ParseExpressionOptional();
}

Type* Parser::ParseFunctionType() {
    advance(); // function
    EXPECT(LPAREN); // (
    auto tok = peek();
    auto parameters = std::vector<Type*>();
    while (tok != RPAREN) {
        auto type = ParseType();
        parameters.push_back(type);
        tok = peek();

        if (tok != COMMA && tok != RPAREN) {
            throw SyntaxError(lex()->currentToken(), "unexpected token");
        }
        if (tok == COMMA) {
            advance();
        }
    }

    advance(); // )

    tok = peek();
    if (tok != COLON) {
        return TypeSystem::getFunctionType(TypeSystem::getUndefinedType(), parameters);
    }

    advance(); // :
    Type *ret = ParseType();
    return TypeSystem::getFunctionType(ret, parameters);
}

Type* Parser::ParseType() {
    auto tok = peek();
    if (tok == IDENTIFIER) {
        auto name = GetIdentifierName();
        advance();
        return TypeSystem::getNamedType(name);
    } else if (tok == LBRACK) {
        advance();
        tok = peek();
        if (tok == RBRACK) {
            advance();
            Type *base = ParseType();
            return TypeSystem::getArrayType(base);
        } else if (tok == NUMBER) {
            auto size = ParseNumber(lex()->currentToken());
            advance();
            EXPECT(RBRACK);
            return TypeSystem::getArrayType(ParseType(), true, size_t(size));
        } else {
            // need to handle simple compile time constant expressions
            throw SyntaxError(lex()->currentToken(), "expected a [ or a constant integer");
        }
    } else if (tok == MUL) {
        advance();
        Type *base = ParseType();
        return TypeSystem::getPointerType(base);
    } else if (tok == FUNCTION) {
        return ParseFunctionType();
    } else {
        throw SyntaxError(lex()->currentToken(), "expected a type expression");
    }

    return nullptr;
}

Handle<Expression> Parser::ParseTypeDefinition() {
    advance(); // eat 'type'
    auto tok = peek();
    if (tok != IDENTIFIER) {
        throw SyntaxError(lex()->currentToken(), "expected a name");
    }

    std::string name = GetIdentifierName();
    advance();
    tok = peek();
    if (tok == IDENTIFIER) {
        // we are parsing type alias
        std::string original = GetIdentifierName();
        advance();
        TypeSystem::AliasType(name, original);
        return nullptr;
    } else if (tok == LBRACE) {
        // we are parsing struct type
        advance();
        std::map<std::string, Type*> types;
        while (true) {
            tok = peek();
            std::string name;
            Type *type;
            if (tok == RBRACE) {
                advance();
                break;
            }
            if (tok == IDENTIFIER) {
                name = GetIdentifierName();
                advance();
            } else {
                throw SyntaxError(lex()->currentToken(), "expected a name");
            }

            tok = peek();
            if (tok != COLON) {
                throw SyntaxError(lex()->currentToken(), "expected a :");
            }

            advance();
            type = ParseType();
            types[name] = type;
        }

        TypeSystem::getObjectType(types, name);
    } else if (tok == FUNCTION) {
        Type *fun = ParseFunctionType();
        TypeRegistry::Register(name, fun);
    } else {
        throw SyntaxError(lex()->currentToken(), "expected a type expression");
    }

    return nullptr;
}

Handle<Expression> Parser::ParseVariableStatement()
{
    advance();    // eat 'var'

    std::vector<Handle<Declaration>> decl_list;
    while (true) {
        decl_list.push_back(Handle<Declaration>(ParseDeclaration()));

        auto tok = peek();
        if (tok == SEMICOLON) {
            advance();
            break;
        } else if (tok == IN) {
            break;
        } else if (tok != COMMA)
            throw SyntaxError(lex()->currentToken(), "expected a ',' or ';'");
        advance(); // eat ','
    }

    return builder()->factory()->NewDeclarationList(
            builder()->locator()->loc(), scope_manager()->current(), decl_list);
}

// Handle<ExpressionList> Parser::ParseCaseBlock()
// {
//     advance();

//     // TODO ::= make it more abstract. use ParseExpression
//     Handle<Expression> clause = ParseAssignExpression();
//     EXPECT(COLON);

//     Handle<ExpressionList> cases = builder()->NewExpressionList();
//     Handle<ExpressionList> list = builder()->NewExpressionList();

//     cases->Insert(clause);
//     do {
//         auto tok = peek();
//         if (tok == CASE) {
//             advance();
//             clause = ParseAssignExpression();
//             cases->Insert(clause);
//             EXPECT(COLON);
//             continue;
//         }
//         Handle<Expression> stmt = ParseStatement();
//         list->Insert(stmt);
//     } while (peek() != CASE && peek() != DEFAULT && peek() != RBRACE);

//     Handle<ExpressionList> clauses = builder()->NewExpressionList();

//     auto block = builder()->NewBlockStatement(list);
//     for (auto &c : cases->raw_list()) {
//         clauses->Insert(builder()->NewCaseClauseStatement(c, block));
//     }
//     return clauses;
// }

// Handle<Expression> Parser::ParseDefaultClause()
// {
//     advance();

//     EXPECT(COLON);

//     Handle<ExpressionList> list = builder()->NewExpressionList();
//     do {
//         Handle<Expression> stmt = ParseStatement();
//         list->Insert(stmt);
//     } while (peek() != CASE && peek() != DEFAULT && peek() != RBRACE);

//     return builder()->NewBlockStatement(list);
// }

// Handle<Expression> Parser::ParseSwitchStatement()
// {
//     advance();
//     EXPECT(LPAREN);

//     Handle<Expression> expr = ParseAssignExpression();
//     EXPECT(RPAREN);
//     EXPECT(LBRACE);

//     bool has_default = false;
//     Handle<ClausesList> list = builder()->NewClausesList();
//     while (true) {
//         Handle<Expression> temp = nullptr;
//         if (peek() == CASE) {
//             auto tempList = ParseCaseBlock();

//             for (auto &t : tempList->raw_list()) {
//                 list->PushCase(t->AsCaseClauseStatement());
//             }
//         } else if (peek() == DEFAULT) {
//             if (has_default) {
//                 throw SyntaxError(lex()->currentToken(),
//                         "switch statement has already has one default case");
//             }
//             temp = ParseDefaultClause();
//             list->SetDefaultCase(temp);
//             has_default = true;
//         } else if (peek() != RBRACE) {
//             throw SyntaxError(lex()->currentToken(), "expected a '}'");
//         } else {
//             advance();
//             break;
//         }
//     }

//     return builder()->NewSwitchStatement(expr, list);
// }

// Handle<Expression> Parser::ParseBreakStatement()
// {
//     advance();
//     Handle<Expression> label = nullptr;

//     if (peek() == IDENTIFIER) {
//         label = builder()->NewIdentifier(GetIdentifierName());
//     }

//     return builder()->NewBreakStatement(label);
// }

// Handle<Expression> Parser::ParseContinueStatement()
// {

//     advance();
//     Handle<Expression> label = nullptr;

//     if (peek() == IDENTIFIER) {
//         label = builder()->NewIdentifier(GetIdentifierName());
//     }

//     return builder()->NewContinueStatement(label);
// }

// Handle<Expression> Parser::ParseTryCatchStatement()
// {
//     Handle<Expression> try_block = nullptr;
//     Handle<Expression> catch_expr = nullptr;
//     Handle<Expression> catch_block = nullptr;
//     Handle<Expression> finally = nullptr;
//     EXPECT(TRY);

//     try_block = ParseBlockStatement();

//     if (peek() == CATCH) {
//         advance();
//         EXPECT(LPAREN);
//         catch_expr = ParseExpression();
//         EXPECT(RPAREN);
//         catch_block = ParseBlockStatement();
//     }
//     if (peek() == FINALLY) {
//         advance();
//         finally = ParseBlockStatement();
//     }
//     return builder()->NewTryCatchStatement(try_block, catch_expr, catch_block,
//         finally);
// }

// // ThrowStatement :
// //      throw [no line terminator] Expression ;
// Handle<Expression> Parser::ParseThrowStatement()
// {
//     EXPECT(THROW);

//     Handle<Expression> expr = ParseExpression();

//     return builder()->NewThrowStatement(expr);
// }

Handle<Expression> Parser::ParseStatement()
{
    auto tok = peek();

    switch (tok) {
    default:
    {
        auto result = ParseExpressionOptional();
        tok = peek();

        if (tok != SEMICOLON)
            throw SyntaxError(lex()->currentToken(), "expected a ';'");
        advance();
        return result;
    }

    case IF:
        return ParseIfStatement();
    case FOR:
        return ParseForStatement();
    case FUNCTION:
        return ParseFunctionStatement();
    case LBRACE:
        return ParseBlockStatement();
    case RETURN:
        return ParseReturnStatement();
    case WHILE:
        return ParseWhileStatement();
    case DO:
        return ParseDoWhileStatement();
    case CONST:
    case LET:
        return ParseVariableStatement();

    case TYPE:
        return ParseTypeDefinition();
    }
}

std::string CreateLLVMLikePointer(size_t pos, size_t len)
{
    std::string result;
    if (pos >= len) {
        result += std::string(len, '~');
        result += '^';
    } else {
        result += std::string(pos - 1, '~');
        result += '^';
        result += std::string(len - pos, '~');
    }
    return result;
}

// std::string GetErrorMessagePointer(std::string &str, size_t seek, Position pos)
// {
//     const size_t threshold_length = 70;
//     std::string shown("  ");

//     if (str.length() > threshold_length) {
//         size_t e = (str.length() - seek < threshold_length / 2)
//                 ? str.length() - seek : threshold_length / 2;
//         shown += std::string(str.begin() + seek - threshold_length / 2,
//                         str.begin() + seek + e);
//         shown += "\n  " + CreateLLVMLikePointer(seek - threshold_length / 2,
//                                             threshold_length / 2 + e) + '\n';
//     } else {
//         shown += str + "\n  " + CreateLLVMLikePointer(pos.col_,
//             str.length()) + '\n';
//     }

//     return shown;
// }

Handle<Expression> Parser::ParseProgram()
{
    Handle<ExpressionList> exprs = builder()->NewExpressionList();
    try {
        while (peek() != END_OF_FILE) {
            auto stmt = ParseStatement();
            if (stmt)
                exprs->Insert(stmt);
        }
    } catch (std::exception &e) {
        auto error = dynamic_cast<SyntaxError*>(&e);

        if (error) {
            std::cerr << error->what() << " (" << error->token().position().row()
                    << ":" << error->token().position().col()
                    << ") (" << error->token().view() << ")" << std::endl;
        }
        throw;
    }

    return builder()->NewBlockStatement(exprs);
}

Handle<Expression> Parser::ParsePostfixExpression() {

    return nullptr;
}

Handle<Expression> ParseProgram(Parser *parser)
{
    // parse program and return AST
    return parser->ParseProgram();
}

}
