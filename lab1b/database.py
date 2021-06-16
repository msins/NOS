import sys
import json
from multiprocessing import Process, Pipe
from time import sleep
from random import randint
from enum import Enum
import heapq


class MessageType(Enum):
    ZAHTJEV = 1
    ODGOVOR = 2
    IZLAZAK = 3


class Message:
    def __init__(self, msg_type, pid, clock):
        self.type = msg_type
        self.pid = pid
        self.clock = clock

    def __eq__(self, other):
        return self.pid == other.pid \
               and self.clock == other.clock

    def __str__(self):
        return f'({self.type}, {self.pid}, {self.clock})'

    def __lt__(self, other):
        if self.clock == other.clock:
            return self.pid < other.pid
        return self.clock < other.clock


class Database:
    def __init__(self, filename, pids):
        self.filename = filename

        entries = ",".join([f'"{pid}":{json.dumps({"clock": -1, "count": 0})}' for pid in pids])
        print(entries)

        with open(self.filename, "w") as file:
            json.dump(json.loads(f'{{{entries}}}'), file)

    def add(self, pid, clock):
        with open(self.filename, "r") as file:
            logs = json.load(file)

        logs[str(pid)]["clock"] = clock
        logs[str(pid)]["count"] += 1

        with open(self.filename, "w") as file:
            json.dump(logs, file)

    def print(self):
        with open(self.filename, "r") as file:
            logs = json.load(file)
        print("=====DB=====")
        for pid, log in logs.items():
            print(f'[{pid}]:clock={log["clock"]},count={log["count"]}')
        print("============")


def notify(transmitters, message):
    for transmitter in transmitters.values():
        transmitter.send(message)


def process(n, pid, receiver, transmitters, db):
    """
    Starts a process.

    :param n: num of processes
    :param pid: pid of current process
    :param receiver: pipe receiver for this process
    :param transmitters: transmitters of other processes
    :param db: database
    :return: None
    """
    localClock = randint(0, 50)
    localQueue = []
    heapq.heapify(localQueue)

    # Number of times each process has to print db state
    dbInteractions = 5

    # Number of messages each process needs to receive
    receivedCount = n - 1

    while dbInteractions > 0:
        # Request access to critical section
        if len(localQueue) == 0:
            startMessage = Message(MessageType.ZAHTJEV, pid, localClock)
            notify(transmitters, startMessage)
            heapq.heappush(localQueue, startMessage)

        if receiver.poll(100):
            receivedMessage = receiver.recv()

            # Someone left critical section, remove from local queue
            if receivedMessage.type == MessageType.IZLAZAK:
                localClock = max(localClock, receivedMessage.clock) + 1
                # print(f'[{pid}] Removing from queue: {receivedMessage}')
                localQueue.remove(receivedMessage)
                heapq.heapify(localQueue)
            # Someone wants to go in critical section
            elif receivedMessage.type == MessageType.ZAHTJEV:
                localClock = max(localClock, receivedMessage.clock) + 1
                transmitMessage = Message(MessageType.ODGOVOR, receivedMessage.pid, localClock)
                transmitters[receivedMessage.pid].send(transmitMessage)
                heapq.heappush(localQueue, receivedMessage)
                # print(f'[{pid}] send reply {transmitMessage}')
            # Someone acknowledged to this process that it can go into c.s.
            elif receivedMessage.type == MessageType.ODGOVOR:
                localClock = max(localClock, receivedMessage.clock) + 1
                # print(f'[{pid}] received from {receivedMessage}')
                receivedCount -= 1
            else:
                continue

        if receivedCount > 0 or localQueue[0].pid != pid:
            continue
        receivedCount = n - 1

        print(f'[{pid}] entering critical section.')
        db.add(pid, localClock)
        db.print()
        sleep(randint(100, 2000) / 1000)
        print(f'[{pid}] leaving critical section.')

        responseMessage = heapq.heappop(localQueue)
        responseMessage.type = MessageType.IZLAZAK
        # print(f'[{pid}] notifying i\'m leaving: {responseMessage}')
        notify(transmitters, responseMessage)

        dbInteractions -= 1

    receiver.close()
    for t in transmitters.values():
        t.close()


if __name__ == '__main__':
    n = int(sys.argv[1])
    processes = []
    pids = []
    pipes = {}

    # create pipes (receiver, transmitter) and map them to pid of a process
    for pid in range(n):
        pids.append(pid)
        pipes[pid] = Pipe()

    db = Database("./db.json", pids)

    for pid in range(n):
        # (receiver, _) object from pipe
        receiver = pipes[pid][0]
        transmitters = {}
        # add all (_, transmitter) objects except for the current pipe
        for i, pipe in enumerate(pipes):
            if i != pid:
                transmitters[i] = pipes[i][1]
        proc = Process(target=process, args=(n, pid, receiver, transmitters, db))
        processes.append(proc)
        proc.start()

    for proc in processes:
        proc.join()
