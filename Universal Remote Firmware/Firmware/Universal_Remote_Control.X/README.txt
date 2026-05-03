This Project Contains the Code for the Universal Remote Control as of Fall 2024.  
This Version of the Code Sends The Status of the Controllers Buttons and Joysticks Over UART @ 115.2K Baud.

The Data Packets Sent Contian 6 Bytes of Data in the Following Format:
| HandShake | Robot Address | Command | Data Byte 1 | Data Byte 2 | Data Byte 3 |
|        $          |  8 Bit Address  |   Control   |      Data       |      Data        |        Data      |

There are data packets sent:
Command : J
Data 1 = Joystick 1 Up and Down
Data 2 = Joystick 1 Left and Right
Data 3 = Joystick 2 Up and Down

Command : j
Data 1 = Joystick 2 Left and Right
Data 2 = Joystick 3 Up and Down
Data 3 = Joystick 3 Left and Right

Command : B
See Project report for Button Data Bit Maps
