BUILTIN(FETCH,
{
    printf("[ @ ]\n");

    cell* addr  = (cell*)POP();
    cell  value = *addr;
    PUSH(value);
})