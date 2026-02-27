BUILTIN(COLON,
{
    printf("[ colon ]\n");

    get_next_word();

    create(current_word, 0);
    latest->flags |= FLAG_HIDDEN;

    printf("created word: %s\n", current_word);
    state = STATE_COMPILE;
})

BUILTIN(SEMICOLON,
{
    printf("[ semicolon ]\n");

    // finish current definition
    comma((cell)getcode("exit")); // todo: why cant I use OP(EXIT)?
    latest->flags &= ~FLAG_HIDDEN; // todo: I forgot what's happening here

    printf("finished compiling word: '%s'\n", latest->name);
    state = STATE_IMMEDIATE;
})

BUILTIN(TICK,
{
    header = NULL;
    header = find( get_next_word() );
    if (header)
    {
        cell token = tick(header); // todo: can change to tmp register later...
        
        if (state == STATE_IMMEDIATE) PUSH(token);
        else {
            comma((cell)CODE(LIT));
            comma(token);
        }
    }
})

BUILTIN(COMMA,
{
    printf("[ , ]\n");

    value = POP();
    comma(value);
})

BUILTIN(IMMEDIATE,
{
    printf("[ immediate ]\n");

    latest->flags ^= FLAG_IMMEDIATE; // todo: check -- slarba's def, no args

    // todo: does this need to pop the word header off first?
    // word_header_t* word_header = (word_header_t*)POP();
    // word_header->flags ^= FLAG_IMMEDIATE;
})