BUILTIN(PRINT_STACK, 
{
    printf("[ ps ]\n");

    printf("stack: [ ");
    for (cell* p = ds; p < s0; p++)
        printf("%ld ", (long)*p);
    printf("]\n");
})

BUILTIN(TODO,
{
    printf("[ ps ]\n");
    todo(current_word); 
    return;
})