BUILTIN(LATEST,
{
    printf("[ latest ]\n");
    PUSH(&latest);
})

BUILTIN(0BRANCH,
{
    printf("[ 0branch ]\n");

    tmp = INTARG();
    
    printf("INTARG: %d\n", (int)tmp);
    printf("0branch stack: ");
    print_stack(ds, s0);

    if (!POP()) ip += (tmp / sizeof(void*)) - 1;
})