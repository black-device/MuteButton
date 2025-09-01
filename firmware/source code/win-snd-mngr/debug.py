#!/usr/bin/env python3

from datetime import datetime

__callbackList = []



def dbgPrint(txt: str):
    dt = datetime.now().strftime('%y%m%d %H:%M:%S.%f')[:-3]
    line = f'{dt} {txt}'
    print(line)
    for c in __callbackList: c(line)


def dbgAddCallback(callback):
    __callbackList.append(callback)


def dbgRemoveCallback(callback):
    __callbackList.remove(callback)