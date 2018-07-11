#include "jast/parser-builder.h"
#include "jast/semantics/type-analysis.h"
#include "jast/ir/code-generator.h"
#include "jast/ir/interp-native-functions.h"
#include "jast/ir/redundant-code-elimination.h"
#include "dump-ast.h"
#include "jast/ir/interp.h"

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

    RedundantCodeEliminator eliminator;
    eliminator.RunOn(mod);
    mod->dump(std::cout);

    // register native functions
    internal::RegisterNativeFunction f("print", &jast::internal::print);

    internal::Interpreter i;
    i.Execute(mod);
    builder.context()->Counters().dump();
    return 0;
}
