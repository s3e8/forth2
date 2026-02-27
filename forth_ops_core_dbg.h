BUILTIN(PRINT_STACK, 
{
    printf("[ ps ]\n");
    print_stack(ds, s0);
})

BUILTIN(TODO,
{
    printf("[ ps ]\n");
    todo(current_word); 
    return;
})