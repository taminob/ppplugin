import time


def initialize(something):
    print("initialize: ", something)


def loop(something):
    # this only works because of the I/O and sleep;
    # a loop without any "non-python" operations may block all threads
    # wanting to execute python plugins;
    # if python 3.12 or newer is used, multiple threads can execute
    # python code at the same time
    while True:
        print("loop: ", something)
        time.sleep(1)


print("loading")
