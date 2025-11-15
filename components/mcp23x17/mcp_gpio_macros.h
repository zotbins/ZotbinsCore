/* Quick access to GPIO pin numbers for PORTA/PORTB */

// mcp23x17 implements versions of these functions that take a number (0,1,1...15) and convert it to the bitmask for you but these are more explicit and match the datasheet
#define MCP_PORTA_GPIO0 0
#define MCP_PORTA_GPIO1 1
#define MCP_PORTA_GPIO2 2
#define MCP_PORTA_GPIO3 3
#define MCP_PORTA_GPIO4 4
#define MCP_PORTA_GPIO5 5
#define MCP_PORTA_GPIO6 6
#define MCP_PORTA_GPIO7 7
#define MCP_PORTB_GPIO0 8
#define MCP_PORTB_GPIO1 9
#define MCP_PORTB_GPIO2 10
#define MCP_PORTB_GPIO3 11
#define MCP_PORTB_GPIO4 12
#define MCP_PORTB_GPIO5 13
#define MCP_PORTB_GPIO6 14
#define MCP_PORTB_GPIO7 15

// You can switch polarity of the gpio mode registers but this is default
#define MCP_OUTPUT 0
#define MCP_INPUT 1

#define MCP_HIGH 1
#define MCP_LOW 0