BUILTIN(ADD,
{
    printf("[ + ]\n");

    tmp = POP();
    AT(0) += tmp;

    // cell a = POP();
    // cell b = POP();
    // PUSH(a+b);
})

BUILTIN(SUB,
{
    printf("[ - ]\n");

    tmp = POP();
    AT(0) -= tmp;   
})

BUILTIN(EQ,
{
    printf("[ = ]\n");

    tmp = POP(); 
    AT(0) = AT(0) == tmp;
})

// BUILTIN(SUB,  { tmp = POP(); AT(0) -= tmp; })
// BUILTIN(ADD,  { tmp = POP(); AT(0) += tmp; })
// BUILTIN(EQ,   { tmp = POP(); AT(0) = AT(0) == tmp; })