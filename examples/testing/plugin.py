import time


def initialize(something):
    print("initialize: ", something)


def loop(something):
    while True:
        print("loop: ", something)
        time.sleep(1)


print("loading")
