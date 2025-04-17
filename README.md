# SmartCapacitorTester

**SmartCapacitorTester** is a digital capacitance meter based on the Arduino Pro Mini. It features automatic range detection using a cascade of resistors and displays results on an ST7567 128x32 SPI screen, including the nearest commercial capacitor value for reference.

## Features

- Automatic capacitance measurement from ~1pF to 1000µF
- Smart range selection using 4 resistors in cascade (1k, 10k, 100k, 1M)
- Displays accurate measured value (in pF, nF or µF)
- Suggests the closest commercial standard capacitor value
- Alerts when the component is likely an inductor, resistor, or short circuit

## Recommended Hardware

- Arduino Pro Mini 5V
- ST7567 128x32 SPI display
- 4 precision resistors: 1kΩ, 10kΩ, 100kΩ, 1MΩ
- Measurement pin: A4
- Resistors connected in cascade: A0 → A1 → A2 → A3 → A4

## Usage

Connect the capacitor to be tested between A4 and GND.  
The circuit will automatically select the most appropriate resistor to measure the charge time and calculate the capacitance.  
Values are shown both on the display and via the Serial Monitor.

## License

This project is licensed under the [MIT License](LICENSE).

## Author

Developed by [https://github.com/JulioBianor], with support from OpenAI ChatGPT for structure, documentation, and embedded development best practices.
