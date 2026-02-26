/* yet another attempt */

#include <stdio.h>
#include <stdlib.h>

typedef struct forth_config 
{
    size_t dictionary_size    = 10*1024*1024;
    size_t dstack_size        = 1024;
    size_t rstack_size        = 512;
    size_t nstack_size        = 512;
    int    nstack_max_depth   = 512;
} forth_config_t;

/* The primary data output function. This is the place to change if you want
* to e.g. output data on a microcontroller via a serial interface. */
void putkey(char c)
{
    putchar(c);
}

/* The primary data input function. This is where you place the code to e.g.
* read from a serial line. */
int llkey()
{
    if (*initscript_pos) return *(initscript_pos++);
    return getchar();
}

/* Anything waiting in the keyboard buffer? */
int keyWaiting()
{
    return positionInLineBuffer < charsInLineBuffer ? -1 : 0;
}

/* Line buffered character input. We're duplicating the functionality of the
* stdio library here to make the code easier to port to other input sources */
int getkey()
{
    int c;

    if (keyWaiting())
        return lineBuffer[positionInLineBuffer++];

    charsInLineBuffer = 0;
    while ((c = llkey()) != EOF)
    {
        if (charsInLineBuffer == sizeof(lineBuffer)) break;
        lineBuffer[charsInLineBuffer++] = c;
        if (c == '\n') break;
    }

    positionInLineBuffer = 1;
    return lineBuffer[0];
}

/* C string output */
void tell(const char *str)
{
    while (*str)
        putkey(*str++);
}

/* The basic (data) stack operations */

cell pop()
{
    if (*sp == 1)
    {
        tell("? Stack underflow\n");
        errorFlag = 1;
        return 0;
    }
    return stack[--(*sp)];
}

cell tos()
{
    if (*sp == 1)
    {
        tell("? Stack underflow\n");
        errorFlag = 1;
        return 0;
    }
    return stack[(*sp)-1];
}

void push(cell data)
{
    if (*sp >= STACK_SIZE)
    {
        tell("? Stack overflow\n");
        errorFlag = 1;
        return;
    }
    stack[(*sp)++] = data;
}

dcell dpop()
{
    cell tmp[2];
    tmp[1] = pop();
    tmp[0] = pop();
    return *((dcell*)tmp);
}

void dpush(dcell data)
{
    cell tmp[2];
    *((dcell*)tmp) = data;
    push(tmp[0]);
    push(tmp[1]);
}

/* The basic return stack operations */

cell rpop()
{
    if (*rsp == 1)
    {
        tell("? RStack underflow\n");
        errorFlag = 1;
        return 0;
    }
    return rstack[--(*rsp)];
}

void rpush(cell data)
{
    if (*rsp >= RSTACK_SIZE)
    {
        tell("? RStack overflow\n");
        errorFlag = 1;
        return;
    }
    rstack[(*rsp)++] = data;
}

// /* Secure memory access */

// cell readMem(cell address)
// {
//     if (address > MEM_SIZE)
//     {
//         tell("Internal error in readMem: Invalid addres\n");
//         errorFlag = 1;
//         return 0;
//     }
//     return *((cell*)(memory + address));
// }

// void writeMem(cell address, cell value)
// {
//     if (address > MEM_SIZE)
//     {
//         tell("Internal error in writeMem: Invalid address\n");
//         errorFlag = 1;
//         return;
//     }
//     *((cell*)(memory + address)) = value;
// }

/* Reading a word into the input line buffer */
byte readWord()
{
    char *line = (char*)memory;
    byte len = 0;
    int c;

    while ((c = getkey()) != EOF)
    {
        if (c == ' ') continue;
        if (c == '\n') continue;
        if (c != '\\') break;

        while ((c = getkey()) != EOF)
            if (c == '\n')
                break;
    }

    while (c != ' ' && c != '\n' && c != EOF)
    {
        if (len >= (INPUT_LINE_SIZE - 1))
            break;
        line[++len] = c;
        c = getkey();
    }
    line[0] = len;
    return len;
}

/* toupper() clone so we don't have to pull in ctype.h */
char up(char c)
{
    return (c >= 'a' && c <= 'z') ? c - 'a' + 'A' : c;
}

/* Dictionary lookup */
cell findWord(cell address, cell len)
{
    cell ret = *latest;
    char *name = (char*)&memory[address];
    cell i;
    int found;

    for (ret = *latest; ret; ret = readMem(ret))
    {
        if ((memory[ret + CELL_SIZE] & MASK_NAMELENGTH) != len) continue;
        if (memory[ret + CELL_SIZE] & FLAG_HIDDEN) continue;

        found = 1;
        for (i = 0; i < len; i++)
        {
            if (up(memory[ret + i + 1 + CELL_SIZE]) != up(name[i]))
            {
                found = 0;
                break;
            }
        }
        if (found) break;
    }
    return ret;
}

// tick
// comma

#define OP(name) op_##name
#define BUILTIN(name, code) OP(name) { code NEXT(); }




void do_forth()
{
    cell     datastack[1024];
    void**   returnstack[512];
    cell*    ds = datastack + 1024;
    void***  rs = returnstack + 512;
    cell*    s0 = ds;
    void***  r0 = rs;
    void**   nestingstack_space[512];
    void***  nestingstack = nestingstack_space + 512;
    void**   ip = NULL;
}

int main(int argc, char** argv)
{
    OP(DOCOL):{}
    
    OP(INTERPRET):{}
    
    OP(LIT):{}
    
    OP(EXIT):{}
    
    OP(EOW):{}
    
    return 0;
}
