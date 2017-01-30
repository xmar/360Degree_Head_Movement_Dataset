"""Tools to manage exit from the application."""

from .HomeFrame import GetRootFrame
from functools import partial
from tkinter import *
import logging
import traceback

global_exit_manager = None


def RepportExceptionAndExit(self, *args):
    """Called when an exception is raised in tkinter callbacks."""
    err = traceback.format_exception(*args)
    logger = logging.getLogger(
        'TestManager.GUIHelpers.ExitManager'
        )
    strAns = ''
    for line in err:
        strAns += line
    logger.critical(strAns)
    root = GetRootFrame()
    if global_exit_manager is not None:
        global_exit_manager.ExitCallback()
    else:
        root.destroy()


def GetExitManager():
    """Return the unique exit manager or create it."""
    global global_exit_manager
    if global_exit_manager is None:
        global_exit_manager = ExitManager()
        root = GetRootFrame()
        root.protocol("WM_DELETE_WINDOW", partial(ExitManager.ExitCallback,
                                                  global_exit_manager))
        Tk.report_callback_exception = \
            RepportExceptionAndExit
    return global_exit_manager


class ExitManager(object):
    """Class to manange the exit callback of the app."""

    def __init__(self):
        """init function."""
        self.callbackDict = dict()
        self.nextId = 0
        self.callbackIdStack = list()
        self.root = GetRootFrame()
        self.logger = logging.getLogger(
            'TestManager.GUIHelpers.ExitManager'
            )

    def ExitCallback(self):
        """Function call when exiting the apps."""
        self.logger.info('Exiting the app')
        while len(self.callbackIdStack) > 0:
            self.logger.debug('Calling exit callback')
            callbackId = self.callbackIdStack.pop()
            self.callbackDict[callbackId]()
            del self.callbackDict[callbackId]
        self.root.destroy()

    def AddCallback(self, callback):
        """Add an exit callback to the top of the stack."""
        self.logger.debug('Add an exit callback')
        callbackId = self.nextId
        self.nextId += 1
        self.callbackIdStack.append(callbackId)
        self.callbackDict[callbackId] = callback

    def PopCallback(self, callbackId):
        """Remove one exit callback."""
        self.logger.debug('Pop an exit callback')
        if callbackId in self.callbackIdStack:
            self.callbackIdStack.remove(callbackId)
            del self.callbackDict[callbackId]
