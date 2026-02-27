// latest

BUILTIN(IMMEDIATE,
{
    printf("[ immediate ]\n");

    latest->flags ^= FLAG_IMMEDIATE; // todo: check -- slarba's def, no args

    // todo: does this need to pop the word header off first?
    // word_header_t* word_header = (word_header_t*)POP();
    // word_header->flags ^= FLAG_IMMEDIATE;
})

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