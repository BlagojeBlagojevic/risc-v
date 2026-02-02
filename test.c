#define UART_STORE 0x10000000
#define UART_READ  0x10000001
//tHIS IS FLAT BINARY!!!
void putchar(char c);
void puts_simple();  
int main() {
    puts_simple();
    
    // Write 'a' to UART as requested
    volatile char *ptr = (volatile char *)UART_STORE;
    volatile char *read = (volatile char *)UART_READ;
    
    *ptr = 'a';
    
    __asm__("nop");
    
    char dummy = *read;
    return 0;
}

void putchar(char c) {
    volatile char *ptr = (volatile char *)UART_STORE;
    *ptr = c;
}

void puts_simple() {
    putchar('H');
    putchar('e');
    putchar('l');
    putchar('l');
    putchar('o');
    putchar(' ');
    putchar('W');
    putchar('o');
    putchar('r');
    putchar('l');
    putchar('d');
    putchar('!');
    putchar('\n');
}

