/// reader ///
typedef struct reader_state
{
    FILE* stream;
    cell  word_length;
    cell  line_length;
    char* current_word;
    char* current_line;
    char* remaining_words;
} reader_state_t;

static FILE* current_stream = stdin;
static cell  word_length = WORDBUF_LENGTH;
static cell  line_length = LINEBUF_LENGTH;
static char* current_word = NULL;
static char* current_line = NULL;
static char* remaining_words = NULL;

/* The primary data output function. This is the place to change if you want
* to e.g. output data on a microcontroller via a serial interface. */
void emit(const char c, FILE* stream)
{
    fputs(c, stream);
}

/* C string output */
void tell(const char *str)
{
    while (*str)
        emit(*str++);
}

static void skip_whitespace(reader_state_t* reader_state) 
{
  while(isspace(*reader_state->remaining_words)) reader_state->remaining_words++;
}

static cell is_eol(reader_state_t* reader_state)
{
  skip_whitespace(reader_state);
  return (cell)(*reader_state->remaining_words == '\0');
}

static cell is_eof(reader_state_t* reader_state)
{
    return (cell)(is_eol(reader_state) && feof(reader_state->stream));
}

static char* get_next_line(reader_state_t* reader_state)
{
    if (reader_state->stream == stdin)
    {
        printf("forth> ");
        fflush(stdout);
    }
    if (!fgets(reader_state->current_line, reader_state->line_length, reader_state->stream)) return NULL;

    reader_state->remaining_words = reader_state->current_line;
    reader_state->remaining_words[strcspn(reader_state->remaining_words, "\n")] = '\0'; // todo: is this necessary? -- yes... change new line to termination character

    return reader_state->remaining_words;
}

static char* get_next_word(reader_state_t* reader_state)
{
    char* new_word_buffer = reader_state->current_word; // setting up a temp buffer to copy new word into current_word

    // buffer exhausted? get_next_line
    while (1)
    {
        // Skip whitespace using isspace() for all whitespace characters
        while (*reader_state->remaining_words && isspace(*reader_state->remaining_words))
        {
            reader_state->remaining_words++;
        }

        // If we're at the end of the current buffer
        if (*reader_state->remaining_words == '\0')
        {
            printf("Buffer exhausted, getting next line...\n");
            if (!get_next_line(reader_state))
            {
                printf("No more lines, returning NULL\n");
                reader_state->current_word[0] = '\0';
                return NULL;
            }
            // Continue to skip whitespace on the new line
            continue;  // <-- This goes back to the START of the while loop
        }

        // Found a non-whitespace character, start reading word
        break;
    }

    // // Copy word until whitespace or end of string
    while (*reader_state->remaining_words && !isspace(*reader_state->remaining_words))
    {
        *new_word_buffer++ = *reader_state->remaining_words++;
    }

    *new_word_buffer = '\0'; // todo: what's this doing?

    return reader_state->current_word;;
}

static int key(reader_state_t* reader_state)
{
    if (*reader_state->remaining_words == '\0')
    {
        if (!get_next_line(reader_state)) return -1;
    }

    return *reader_state->remaining_words++;
}