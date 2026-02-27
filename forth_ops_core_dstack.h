BUILTIN(DUP,
{
    printf("[ dup ]\n");

    cell value = TOP();
    PUSH(value);
})

BUILTIN(SWAP,
{
    printf("[ swap ]\n");
    
    // BYTECODE(SWAP, "swap", 2, 0, 0, {
    //     tmp = AT(1);
    //     AT(1) = AT(0);
    //     AT(0) = tmp;
    // })

    //  // Stack: ( a b -- b a )
    // // Access stack items directly

    // cell b = AT(-1);     // Top item (b)
    // cell a = AT(-2);     // Second item (a)

    // // Swap them in place
    // AT(-1) = a;
    // AT(-2) = b;

    // print_stack(ds, s0);

    cell b = POP();      // Remove top value (b)
    cell a = POP();      // Remove next value (a)

    PUSH(b);             // Push b first
    PUSH(a);             // Then push a
})