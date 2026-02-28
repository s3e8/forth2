BUILTIN(LATEST,
{
    printf("[ latest ]\n");
    
    PUSH(&latest);
})

BUILTIN(0BRANCH,
{
    printf("[ 0branch ]\n");

    tmp = INTARG();

    if (!POP()) ip += (tmp / sizeof(void*)) - 1;
})