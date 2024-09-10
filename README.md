# ChainReaction

# Atomic Energy Simulation

This C program simulates atomic energy generation, splitting atoms, and managing energy levels over time. It features multi-threading using `pthread` and synchronization mechanisms like semaphores and mutexes to manage concurrent tasks.

## Key Features
- **Atom Generation**: Generates a predefined number of atoms with random atomic numbers.
- **Energy Management**: Tracks energy generation, energy demand, and waste produced during the simulation.
- **Atomic Splitting**: Atoms are split into smaller parts, with energy released or consumed based on atomic interactions.
- **Multi-threaded Operations**: Utilizes `pthread` for concurrent execution of atom processing, activation, feeding new atoms, and generating reports.
- **Thread Synchronization**: Employs semaphores and mutexes for managing thread execution and resource access.
- **Real-time Reporting**: Periodically prints energy generation, demand, and waste status over the simulation's duration.


