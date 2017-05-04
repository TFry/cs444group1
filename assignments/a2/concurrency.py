import threading
import random
import time

# 5 Locks to represent the 5 forks
forks = [threading.Lock(), threading.Lock(), threading.Lock(), threading.Lock(), threading.Lock()]
threads = []


class Philosopher (threading.Thread):
    def __init__(self, thread_id, name, left_fork, right_fork):
        # Initialize Thread
        threading.Thread.__init__(self)
        # Record instance variables
        self.thread_id = thread_id
        self.name = name
        self.left_fork = left_fork
        self.right_fork = right_fork
        # Variable used in while loops to tell if process is still running
        self.dining = True

    def run(self):
        while self.dining:
            # Sleeps from 1 to 20 seconds
            self.think()
            print(self.name + " is hungry.\n")
            # Tries to acquire forks (locks)
            self.get_forks()
            # Eats from 2 to 9 seconds
            self.eat()
            # Releases forks (locks)
            print("RELEASE FORKS.\n")
            self.put_forks()
            print(self.name + " is now done eating. Going to go think now.\n")
    
    def think(self):
        time.sleep(random.randrange(1,20))

    def get_forks(self):
        while self.dining:
            self.left_fork.acquire(True)
            # Passing False here makes it not block incase the lock isn't available
            procured_lock = self.right_fork.acquire(False)
            if procured_lock:
                break
            else:
                # Release the first fork, sleep, and try again
                self.left_fork.acquire(False)
                self.left_fork.release()

    def eat(self):
            print(self.name + " is eating his meal.\n")
            time.sleep(random.uniform(2,9))

    def put_forks(self):
        self.left_fork.acquire(False)
        self.left_fork.release()
        self.right_fork.acquire(False)
        self.right_fork.release()



def main():
    # 5 Names for the 5 threads
    names = ["Plato", "Descartes", "Aristotle", "Socrates", "Hobbes"]
    # Iterate 5 times and instantiate the Philospher (thread) class
    # (0,1) (1,2) (2,3) (3,4) (4,0) <- Fork pairs
    #  x % 5, (x+1) % 5 <-  This is how we get the number pairs above
    for x in range(0, 5):
        thread = Philosopher(x, names[x], forks[x % 5], forks[(x+1) % 5])
        threads.append(thread)
        threads[x].start()
    # Set runtime here
    time.sleep(30)
    # After runtime is depleted, set their dining variable to False so the threads stop running
    for t in threads:
        t.dining = False
    print "Exiting Main Thread.\n"

main()

