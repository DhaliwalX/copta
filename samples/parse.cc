#include "jast/parser-builder.h"
#include "jast/semantics/type-analysis.h"
#include "jast/ir/code-generator.h"
#include "dump-ast.h"

#include <iostream>
#include <sstream>
int main()
{
    using namespace jast;

    ParserBuilder builder(std::cin, "STDIN");
    Parser *parser = builder.Build();
    Handle<Expression> ast;

    try {
        ast = ParseProgram(parser);
    } catch (std::exception &) {
        std::cout << "\x1b[33mError\x1b[0m" << std::endl;
        return -1;
    }
    std::cout << "Parsed correctly" << std::endl;

    printer::DumpAST p(std::cout, 1);
    ast->Accept(&p);
    TypeAnalysis analysis;

    ast->Accept(&analysis);
    analysis.dump();

    CodeGenerator cg;
    auto mod = cg.RunOn(ast);
    mod->dump(std::cout);
    builder.context()->Counters().dump();
    return 0;
}
