Reaction Timer Game – MSP430FR6989

· Description

This project consists of the design and implementation of a reaction game using the MSP430FR6989 microcontroller.

The system allows two players to compete by pressing a button as close as possible to a target time set by the referee. An LCD screen displays numbers and messages, and UART communication is used for user interaction via a terminal. The design is based on a finite state machine (FSM) that manages the game states in real time.

· Objectives

Implement an interactive game on a real microcontroller
Design and control an FSM for game logic
Handle hardware interfaces: buttons, LEDs, and LCD display
Integrate UART communication for user interaction
Demonstrate modular and clear embedded code design

· Features

 Two-player competitive game
 Countdown and internal stopwatch using timers
 Numbers and messages displayed on LCD
 Feedback signals using LEDs
 UART communication with PC terminal for input and results

· System Architecture

The system is divided into several modules:
Top/Main Module: Initializes hardware and manages the main game loop
FSM Controller: Handles game states: WAITING, COUNTDOWN, PLAYING, RESOLUTION
LCD Controller: Initializes the LCD and displays numbers and characters
Input Controller: Handles button presses and detection
Timer Module: Manages the stopwatch and countdown
UART Module: Sends and receives data through a PC terminal

· FSM Overview

Typical states:

WAITING: Waits for the referee to enter the target time
COUNTDOWN: Performs countdown before the game starts
PLAYING: Detects players’ button presses and measures time
RESOLUTION: Calculates errors, determines winner, and shows results

· Hardware Used

Microcontroller: MSP430 (e.g., MSP430FR6989 or compatible with LCD and UART)
Inputs: Player push buttons
Outputs: LEDs and segmented LCD
Communication: UART for PC terminal interaction

· Project Structure

reaction_timer_msp430/
├── src/
│   └── main.c                 # Main code including FSM, LCD, UART, and timer handling
├── include/
│   └── lcd_numbers.h          # Constants and maps for numbers and letters on the LCD
├── docs/
│   └── README.md
├── Makefile                    # For compiling and loading on MSP430
└── simulation/                 # Optional: tests and code simulations

· How to Run

Open the project in Code Composer Studio (CCS) or a compatible MSP430 IDE
Add main.c and any header files
Compile the project
Load the program onto the MSP430 board
Open a serial terminal at 9600 bps to enter the target time and receive results
Follow on-screen instructions and compete with another player

· Key Concepts

Finite State Machines (FSM)
Real-time hardware interaction
Timer and interrupt handling
Segmented LCD control
UART communication in microcontrollers

· Future Improvements

Enhanced interface: Add sounds or more LEDs for visual feedback
Result history: Save high scores using FRAM
Practice mode: Individual training with random times
Code optimization: Further modularization and dynamic LCD management

· Author

Name: Alejandro Benito
Degree: Computer Engineering
University: Universidad Complutense de Madrid

Description: Academic project in embedded systems, demonstrating hardware control, game logic, and real-time communication using C and the MSP430 microcontroller.
