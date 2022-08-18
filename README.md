# OS_secondproject_senderReciever
The second project is similar to producer-consumer problem. we have a limited buffer and consumers
and producers. The producers generate random strings and write them into empty slots, if there aren’t
any, they will be blocked until an empty slot exists. The consumers should read these slots and write
them on a shared memory between consumers. There is a consumers handler which reads this shared
memory and show it as an output. Consumers will be blocked if all slots are empty. Multi-thread,
semaphore, conditional variables is used in this project. The programming language is C++ and as far
as it was a university project no QT version of semaphores or other features is used.
