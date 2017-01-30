"""Frame that contains the startup widgets of the application.

Author: Xavier Corbillon
IMT Atlantique
"""

import logging
from Helpers import GetGlobalUserManager
from tkinter import *
from tkinter.ttk import *
from tkinter import tix
from tkinter.constants import *
from functools import partial

global_home_frame = None
global_root_frame = None


def GetHomeFrame(*args, **kwargs):
    """Get the global home frame or create it."""
    global global_home_frame
    if global_home_frame is None:
        global_home_frame = HomeFrame(*args, **kwargs)
    return global_home_frame


def GetRootFrame():
    """Get the global root frame or create it."""
    global global_root_frame
    if global_root_frame is None:
        global_root_frame = tix.Tk()
        global_root_frame.tk.eval('package require Tix')
    return global_root_frame


class HomeFrame(Frame):
    """Frame that contains the startup widgets."""

    def __init__(self, *args, userSelectionFrame, **kwargs):
        """Init function.

        :param userSelectionFrame: a Frame used to select an existing user
            or create a new one

        *args and **kwargs are forwared to the Frame constructor
        """
        Frame.__init__(self, *args, **kwargs)
        self.logger = logging.getLogger('TestManager.GUIHelpers.HomeFrame')
        self.userSelectionFrame = userSelectionFrame
        self.startTestButton = Button(
            self,
            text='Start a new test',
            command=partial(self._pressStartNewTestButton)
            )
        self.startTestButton.grid(row=0, column=0)

        self.nbUserlabel = Label(self, text='')
        self.nbTestlabel = Label(self, text='')
        self.nbUserlabel.grid(row=1, column=0)
        self.nbTestlabel.grid(row=2, column=0)

    def grid(self, *args, **kwargs):
        """overide the Frame.grid to update the user / test stats info."""
        userManager = GetGlobalUserManager()
        nbUser = len(userManager.userDict)
        totalNbTest = 0
        for user in userManager.userDict.values():
            totalNbTest += user.GetNumberExistingTest()
        self.nbUserlabel.config(text='Total number of user: {}'.format(nbUser))
        self.nbTestlabel.config(text='Total number of test: '
                                '{}'.format(totalNbTest))
        super(Frame, self).grid(*args, **kwargs)

    def _pressStartNewTestButton(self):
        """Call when the start a new test button is pressed.

        Hide this frame and display the userSelectionFrame
        """
        self.logger.info('Start a new test button pressed')
        self.grid_remove()
        self.userSelectionFrame.grid(row=0, column=0)
