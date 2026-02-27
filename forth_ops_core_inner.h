BUILTIN(LATEST,
{
    printf("[ latest ]\n");
    PUSH(&latest);
})

BUILTIN(ZBRANCH,
{
    printf("[ 0branch ]\n");

    tmp = INTARG();
    printf("INTARG: %d\n", (int)tmp);
    if (!POP()) ip += (tmp / sizeof(void*)) - 1;
})