# serial_manager
 Puente ligero UART↔ZeroMQ para SBCs Linux: lee tramas seriales delimitadas y las publica por IPC, con canal de escritura.

A small, single-purpose C daemon that decouples a serial (UART) device from the rest of an application using ZeroMQ. It reads delimiter-terminated frames from the serial port and publishes them on a PUB socket, while accepting outbound data on a PULL socket to write back to the port. Designed for embedded Linux gateways (e.g. Raspberry Pi) where several processes need access to a single serial line without contention.

Protocol-agnostic: it does not interpret frame contents, only frames them by a configurable delimiter.
