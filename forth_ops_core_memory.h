BUILTIN(FETCH,
{
    printf("[ @ ]\n");

    cell* addr  = (cell*)POP();
    cell  val = *addr;
    PUSH(val);
})

BUILTIN(STORE,
{
    printf("[ ! ]\n");

    cell* addr = (cell*)POP();
    cell  val   = POP();

    *addr = val;
})