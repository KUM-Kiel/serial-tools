# serial-tools

## Usage

### rs232cat

Opens a connection to a tty and forwards stdin to the tty and the tty to stdout.

### tunslip

Opens a connection to a tty and starts a SLIP connection.

Packets from the tty are forwarded to a tun device and vice versa.

### trillium

Works like tunslip with the `--trillium` option.

Sets the baud rate automatically and prints informations about a connected seismometer.

If the Trillium is connected correctly, there shold be a message with the model name and URL of the seismometer upon startup.

If you type `NMX` into the terminal, the message should be printed again.

For more info type

```text
$ ./trillium --help
```

## Downloading the Binaries

You can get the latest binaries for 64-bit Linux [in the releases section](https://github.com/KUM-Kiel/serial-tools/releases/latest).

## Building Yourself

Building the tools should be fairly easy.
The tool is, however, Linux specific.

If you are under Ubuntu, you can get the necessary tools by typing

```text
$ sudo apt-get update
$ sudo apt-get install build-essential
```

For building just type

```text
$ make
```
