"""Frame that contains the startup widgets of the application.

Author: Xavier Corbillon
IMT Atlantique
"""

import logging
from Helpers import GetGlobalUserManager, GetGlobalStatistics
from tkinter import *
from tkinter.ttk import *
from tkinter import tix
from tkinter.constants import *
from functools import partial
from .DeleteTestFrame import DeleteTestFrame

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


from .ExitManager import GetExitManager


class HomeFrame(Frame):
    """Frame that contains the startup widgets."""

    def __init__(self, *args, userSelectionFrame, **kwargs):
        """Init function.

        :param userSelectionFrame: a Frame used to select an existing user
            or create a new one

        *args and **kwargs are forwared to the Frame constructor
        """
        Frame.__init__(self, *args, **kwargs)
        self.parent = args[0]
        self.logger = logging.getLogger('TestManager.GUIHelpers.HomeFrame')
        self.userSelectionFrame = userSelectionFrame
        currRow = 0
        self.startTestButton = Button(
            self,
            text='Start a new test',
            command=partial(self._pressStartNewTestButton)
            )
        self.startTestButton.grid(row=currRow, column=0)
        currRow += 1

        self.delButton = Button(
            self,
            text='Delete some tests',
            command=partial(self._pressDelButton)
            )
        self.delButton.grid(row=currRow, column=0)
        currRow += 1

        self.statsButton = Button(
            self,
            text='Computes statistics',
            command=partial(HomeFrame._pressStatsButton, self, currRow)
            )
        self.statsButton.grid(row=currRow, column=0)
        currRow += 1

        self.nbUserlabel = Label(self, text='')
        self.nbTestlabel = Label(self, text='')
        self.nbUserlabel.grid(row=currRow, column=0)
        currRow += 1
        self.nbTestlabel.grid(row=currRow, column=0)
        currRow += 1

    def grid(self, *args, **kwargs):
        """overide the Frame.grid to update the user / test stats info."""
        userManager = GetGlobalUserManager()
        nbUser = len(userManager.userDict)
        totalNbTest = 0
        for user in userManager.userDict.values():
            totalNbTest += len(user.GetExistingTestPathList())
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

    def _pressDelButton(self):
        """Display the delete test frame."""
        self.logger.info('Delete tests button pressed')
        self.grid_remove()
        DeleteTestFrame(self.parent).grid(row=0, column=0)

    def _pressStatsButton(self, row):
        """Start the computation of the statistics."""
        self.logger.info('Compute statistis button pressed')
        stats = GetGlobalStatistics()
        self.statsButton.config(state=DISABLED)
        self.delButton.config(state=DISABLED)
        self.startTestButton.config(state=DISABLED)
        self.progressBar = Progressbar(self,
                                       orient="horizontal",
                                       length=200,
                                       mode="determinate")
        self.progressBar.grid(row=row, column=1)
        exitManager = GetExitManager()
        self.exitCallbackId = exitManager.AddCallback(
            partial(self._statsComputationDone))
        stats.RunComputation(self.progressBar,
                             partial(self._statsComputationDone)
                             )

    def _statsComputationDone(self):
        """Called when the computation of the stats is done."""
        self.logger.info('Statistis computation done')
        exitManager = GetExitManager()
        exitManager.PopCallback(self.exitCallbackId)
        stats = GetGlobalStatistics()
        self.logger.debug('Join computation thread')
        stats.Join()
        self.logger.debug('Join computation thread done')
        self.statsButton.config(state=NORMAL)
        self.delButton.config(state=NORMAL)
        self.startTestButton.config(state=NORMAL)
        self.progressBar.grid_forget()
        del self.progressBar
