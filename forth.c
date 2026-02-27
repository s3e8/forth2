/* yet another attempt */

#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <ctype.h> // todo: impl own isspace

/* the cell type */
#include <stdint.h>
typedef uintptr_t cell;

/* config defaults --------------------------------- */
#define FORTH_VERSION               0
/* -- io config */
#define WORDBUF_LENGTH              128
#define LINEBUF_LENGTH              512
#define NESTED_INCLUDE_MAX_DEPTH    32
/* -- dictionary config */
#define DICTIONARY_SIZE             10*1024*1024 // 10mb
/* -- todo: name other config stuff */
#define NESTINGSTACK_MAX_DEPTH      512
#define NESTINGSTACK_SIZE           512
#define DATASTACK_SIZE              1024
#define RETURNSTACK_SIZE            512

typedef struct forth_config 
{
    size_t dictionary_size;
    size_t wordbuf_length;
    size_t linebuf_length;
    size_t dstack_size;
    size_t rstack_size;
    size_t nstack_size;
    int    nstack_max_depth;
    int    include_file_max_depth;
} forth_config_t;

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

static FILE* current_stream;
static cell  word_length = WORDBUF_LENGTH;
static cell  line_length = LINEBUF_LENGTH;
static char* current_word = NULL;
static char* current_line = NULL;
static char* remaining_words = NULL;

// /* The primary data output function. This is the place to change if you want
// * to e.g. output data on a microcontroller via a serial interface. */
// void emit(const char* c, FILE* stream)
// {
//     fputs(c, stream);
// }

// /* C string output */
// void tell(const char *str)
// {
//     while (*str)
//         emit(*str++, current_stream);
// }

// /* toupper() clone so we don't have to pull in ctype.h */
// char up(char c)
// {
//     return (c >= 'a' && c <= 'z') ? c - 'a' + 'A' : c;
// }

static void skip_whitespace() 
{
  while(isspace(remaining_words)) remaining_words++;
}

static cell is_eol()
{
  skip_whitespace();
  return (cell)(*remaining_words == '\0');
}

static cell is_eof()
{
    return (cell)(is_eol() && feof(current_stream));
}

static char* get_next_line()
{
    if (current_stream == stdin)
    {
        printf("forth> ");
        fflush(stdout);
    }
    if (!fgets(current_line, line_length, current_stream)) return NULL;

    remaining_words = current_line;
    remaining_words[strcspn(remaining_words, "\n")] = '\0'; // todo: is this necessary? -- yes... change new line to termination character

    return remaining_words;
}

static char* get_next_word()
{
    char* new_word_buffer = current_word; // setting up a temp buffer to copy new word into current_word

    // buffer exhausted? get_next_line
    while (1)
    {
        // Skip whitespace using isspace() for all whitespace characters
        while (*remaining_words && isspace(*remaining_words))
        {
            remaining_words++;
        }

        // If we're at the end of the current buffer
        if (*remaining_words == '\0')
        {
            // printf("Buffer exhausted, getting next line...\n");
            if (!get_next_line())
            {
                printf("No more lines, returning NULL\n");
                current_word[0] = '\0';
                return NULL;
            }
            // Continue to skip whitespace on the new line
            continue;  // <-- This goes back to the START of the while loop
        }

        // Found a non-whitespace character, start reading word
        break;
    }

    // // Copy word until whitespace or end of string
    while (*remaining_words && !isspace(*remaining_words))
    {
        *new_word_buffer++ = *remaining_words++;
    }

    *new_word_buffer = '\0'; // todo: what's this doing?

    return current_word;;
}

static cell is_number(const char* token, cell result, cell base) {
    char *endptr; // todo: token is current_work, remove arg
    result = (cell)strtol(token, &endptr, (int)base);
    
    // Success if we consumed the whole string and it wasn't empty
    return (*endptr == '\0' && endptr != token);
}

static int key(reader_state_t* reader_state)
{
    if (*remaining_words == '\0')
    {
        if (!get_next_line()) return -1;
    }

    return *remaining_words++;
}

/// dictionary ///
/// word header flags ///
#define BIT(x) (1<<(x))
#define FLAG_HIDDEN     BIT(0)
#define FLAG_IMMEDIATE  BIT(1)
#define FLAG_BUILTIN    BIT(2)
#define FLAG_HASARG     BIT(3)
#define FLAG_INLINE     BIT(4)
#define FLAG_DEFERRED   BIT(5)
/// compiler state ///
#define STATE_IMMEDIATE 0
#define STATE_COMPILE   1

typedef struct word_header
{
    cell                flags;
    struct word_header* next_word;
    char                name[WORDBUF_LENGTH];
} word_header_t;

/* file-global dictionary vars */
static void*            here;
static void*            here0;
static cell             here_size = DICTIONARY_SIZE;
static word_header_t*   latest = NULL;

static word_header_t* create(const char* name, cell flags)
{
    if (!name) name = '\0'; // for unnamed words.. apparently, todo:

    word_header_t* new_word = (word_header_t*)here;
    here += sizeof(word_header_t);
    // Optional: Automatically align // You might want to make sure here is always aligned after every operation. Some Forths do this automatically after each dictionary update: .. apparently
    // here = (void*)ALIGN_ADDR((cell)here);

    strncpy(new_word->name, name, WORDBUF_LENGTH);
    new_word->flags     = flags;
    new_word->next_word = latest;

    latest = new_word;
    // todo: error case if no new word is created?
    
    return new_word;
}

static word_header_t* find(const char* name)
{
    if (!name) return NULL;

    word_header_t* word = latest;
    while (word)
    {
        if (!strncmp(word->name, name, WORDBUF_LENGTH))
            return word;

        word = word->next_word;
    }

    printf("'%s' not found...\n", name);
    return NULL;
}

static void** cfa(word_header_t* word)
{   /* void** is a ptr to a code array */
    return (void**)(word + 1);
}

static void* tick(word_header_t* word)
{   /* void** is a ptr to a code array */
    if (word->flags & FLAG_BUILTIN) return *cfa(word);
    else                            return  cfa(word);
}

static void comma(cell value)
{
    *(cell*)here = value;
    here += sizeof(cell);
}

/* bootstrap utility functions */
void* getcode(const char* name)
{
    word_header_t* word = find(name);
    return *(cfa(word)); // todo: change to tick?
}

void defcode(const char* name, void* code, cell flags)
{
    create(name, flags | FLAG_BUILTIN);
    comma((cell)code);
}

void defword(const char* name, void* code[], int wordcount, cell flags)
{
    create(name, flags);
    // todo: dont need to comma(call or docol here cause it's added in as code[0]
    // todo: should I impl this without wordcount?
    for (int i = 0; i < wordcount; i++) comma((cell)code[i]);
    comma((cell)getcode("eow")); // not sure we need this...
}

void defconst(const char* name, cell value)
{   // todo: better name for flagdef?
    void* flagdef[] = { getcode("lit"), (void*)value, getcode("exit") };
    defword(name, flagdef, 3, FLAG_INLINE);
}

void defvar(const char* name, cell value)
{
    cell address = (cell)here; // the const lit value is the address of the var's value that occurs just before the const
    comma(value);

    defconst(name, address);
}

static void todo(const char* name) { printf("word not implemented: %s\n", name); }

/// vm ///
#define NEXT() goto **ip++  // todo: why ** //
#define ARG()       (*ip++)                 // takes the next item on the ip thread as
#define INTARG()    ((cell)(*ip++))         // takes the next item on the ip thread as an int argument, casted as (cell) cause... forth
#define PUSHRS(x)   (*--rs = (void**)(x))   // grow/decrement downward first, then store
#define POPRS()     (*rs++)                 // pop, then shrink upward
#define TOP()       (*ds)
#define PEEK()      (*(ds-1)) // slarba uses TOP() for dup... why must i use this instead when I've basically modelled everything after what he's doing?
#define AT(x)       (*(ds+(x))) // todo: what's this?
#define PUSH(x)     (*--ds = (cell)(x))     // grow/decrement downward first, then store
#define POP()       (*ds++)                 // pop, then shrink upward

static void print_stack(cell* ds, cell* s0)
{
    printf("stack: [ ");
    for (cell* p = ds; p < s0; p++)
        printf("%ld ", (long)*p);
    printf("]\n");
}
// void* xt = tick(word);
// comma
// cfa
// docol (bytecode)

extern int init_forth(forth_config_t* config)
{
    if (!config) return 1;

    here0  = malloc(DICTIONARY_SIZE);
    here   = here0;
    latest = NULL;

    config->dictionary_size = DICTIONARY_SIZE;
    config->wordbuf_length  = WORDBUF_LENGTH;
    config->linebuf_length  = LINEBUF_LENGTH;

    current_stream  = stdin;
    word_length     = WORDBUF_LENGTH;
    line_length     = LINEBUF_LENGTH;
    current_word    = malloc(WORDBUF_LENGTH);
    current_line    = malloc(LINEBUF_LENGTH);
    current_word[0] = '\0';
    current_line[0] = '\0';
    remaining_words = current_line;

    config->include_file_max_depth = NESTED_INCLUDE_MAX_DEPTH;

    config->dstack_size      = DATASTACK_SIZE;
    config->rstack_size      = RETURNSTACK_SIZE;
    config->nstack_size      = NESTINGSTACK_SIZE;
    config->nstack_max_depth = NESTINGSTACK_MAX_DEPTH;

    return 0;
}

extern void free_forth()
{
    free(here0);

    free(current_word);
    free(current_line);
}

// helper macros //
#define OP(name) op_##name
#define CODE(name) &&op_##name
#define BUILTIN(name, code) OP(name): { code NEXT(); }

#define CELL_SIZE sizeof(cell)
#define CELL_MASK (CELL_SIZE - 1)
#define IS_ALIGNED(addr) (((cell) (addr) & CELL_MASK) == 0)
#define ALIGN_ADDR(addr)  ((cell)((addr) + CELL_MASK) &  ~CELL_MASK)
#define OFFSET(x)  (void*)((x) * sizeof(cell)) // todo: or (x) * sizeof...? // to calculate branch offsets

extern void start_forth(forth_config_t* config)
{
    cell     datastack[config->dstack_size];
    void**   returnstack[config->rstack_size];
    cell*    s0 = datastack   + config->dstack_size;
    void***  r0 = returnstack + config->rstack_size;
    cell*    ds = s0;
    void***  rs = r0;

    void**   ip = NULL;
    void**   nestingstack_space[config->nstack_max_depth];                 // i think this is for re-entering the interpreter after immediate execution??
    void***  nestingstack = nestingstack_space + config->nstack_max_depth; // ... as opposed to rs which is for lots of stuff // todo: rename to ns?

    // some default vars and constants
    cell base  = 10;
    cell state = 0;

    /* and finally, some quick access variables */
    register word_header_t* header;
    register char* address; // todo: shouldn't this be cell?
    register void* code;
    register void* fn;
    register cell  value;
    register cell  tmp;
    register cell  a;
    register cell  b;

    void* code_immediatebuf[] = { NULL, CODE(IRETURN) };                                  // todo: do i need to put length inside immediatebuf[2] etc
    void* word_immediatebuf[] = { CODE(CALL), NULL, CODE(IRETURN) };                      //
    void* quit[]              = { CODE(INTERPRET),  CODE(BRANCH), OFFSET(-2), CODE(EOW) }; /* the interpreter loop */

    /* ------------------------------------------------------ */
    /*     | name         | code              | flags         */
    /// WORDS NEEDED FOR INNER INTERPRETER ///
    defcode("interpret",    CODE(INTERPRET),    0);
    defcode("branch",       CODE(BRANCH),       FLAG_HASARG);
    defcode("eow",          CODE(EOW),          0);
    defcode("call",         CODE(TODO),         FLAG_HASARG);
    defcode("ireturn",      CODE(IRETURN),      0);
    defcode("lit",          CODE(LIT),          FLAG_HASARG);
    defcode("exit",         CODE(EXIT),         0);
    /// END WORDS NEEDED FOR INNER INTERPRETER ///
    //////////////////////////////////////////////
    // sys //
    defcode("bye",          CODE(BYE),          0);
    defcode("die",          CODE(DIE),          0);
    // dbg //
    defcode("ps",           CODE(PRINT_STACK),  0); 

    ip = quit;
    NEXT();

    return;

    BUILTIN(DOCOL, 
    {
        code = tick(header); // todo: rename code to xt?
        *--nestingstack = ip;

        if (header->flags & FLAG_BUILTIN)
        {
            code_immediatebuf[0] = code; // todo: or just put tick function here?
            ip = code_immediatebuf;
        } 
        else {
            word_immediatebuf[1] = code;
            ip = word_immediatebuf;
        }
    })

    BUILTIN(INTERPRET,
    {
        printf("[ interpret ]\n");
        if (!get_next_word())
        {
            if (is_eof() && current_stream != stdin) 
                current_stream = stdin; // todo: this still fires even if stream == stdin
            
            NEXT();
        }

        header = NULL;
        header = find(current_word);

        if (!header)
        {
            char* endptr = NULL;
            cell number = (cell)strtol(current_word, &endptr, (int)base);

            if (*endptr != '\0')
            {
                printf("unknown word: %s\n", current_word);
                NEXT();
            }
            else {
                if (state == STATE_COMPILE)
                {
                    comma((cell)CODE(LIT));
                    comma((cell)number);
                }
                else if (state == STATE_IMMEDIATE) { PUSH(number); }
                else { printf("error: Compiler state out of bounds. Should be either 0 or 1.\n"); return; }
            }

            print_stack(ds, s0);
            NEXT();
        }

        if (state == STATE_COMPILE && !(header->flags & FLAG_IMMEDIATE))
        { // todo: use xt 'register'? 
            if (header->flags & FLAG_BUILTIN) comma((cell)tick(header));
            else comma((cell)tick(header)); // todo: slarba for some reason uses CALL here... but CALL is already in word_immediatebuf...
        }
        else { /* state == STATE_IMMEDIATE || word->flags & FLAG_IMMEDIATE */
            goto OP(DOCOL);
        }         
    })

    BUILTIN(BRANCH,
    {
        printf("[ branch ]\n");
        tmp = INTARG();
        ip += (tmp / sizeof(void*)) - 1;
    })

    BUILTIN(CALL,
    {
        printf("[ call ]\n");
        fn = ARG();
        PUSHRS(ip);
        ip = fn;
    })

    BUILTIN(LIT, 
    {
        printf("[ lit ]\n");
        PUSH(INTARG());
    })

    BUILTIN(EOW, {})

    BUILTIN(IRETURN,
    {
        printf("[ ireturn ]\n");
        ip = *nestingstack++;
    })

    BUILTIN(EXIT,
    {
        printf("[ exit ]\n");
        ip = POPRS();
    })

    BUILTIN(DIE, { return; })

    BUILTIN(BYE, 
    {
        printf("bye bye now\n"); 
        return; // goto OP(DIE); ??
    })

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

    // #include "forth_ops_core_inner.h"
    // #include "forth_ops_core_math.h"

}

int main(int argc, char** argv)
{
    forth_config_t forth_config;
    init_forth(&forth_config);
    start_forth(&forth_config);
    free_forth();
    
    return 0;
}