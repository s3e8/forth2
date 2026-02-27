    BUILTIN(DIE, { return; })

    BUILTIN(BYE, 
    {
        printf("bye bye now\n"); 
        goto OP(DIE); // todo: goto OP(DIE); ??
    })