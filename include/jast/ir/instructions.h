#ifndef R
#define R(i, s)
#endif

R(Alloc, "alloc")
R(Load, "load")
R(Store, "store")
R(Add, "add")
R(Sub, "sub")
R(Mul, "mul")
R(Div, "div")
R(Mod, "mod")
R(Shr, "shr")
R(Shl, "shl")
R(Szr, "szr")
R(Lt, "lt")
R(Gt, "gt")
R(Eq, "eq")
R(Neq, "neq")
R(And, "and")
R(Or, "or")
R(Xor, "xor")
R(Brk, "brk")
R(Jmp, "jmp")
R(AddrOf, "addr_of")
R(Gea, "gea")
R(Idx, "idx")
R(Ret, "ret")
R(Invoke, "invoke")

#ifndef B
#define B(i, s)
#endif

B(Add, "add")
B(Sub, "sub")
B(Mul, "mul")
B(Div, "div")
B(Mod, "mod")
B(Shr, "shr")
B(Shl, "shl")
B(Szr, "szr")
B(Lt, "lt")
B(Gt, "gt")
B(Eq, "eq")
B(And, "and")
B(Or, "or")
B(Xor, "xor")

#undef R
#undef B
