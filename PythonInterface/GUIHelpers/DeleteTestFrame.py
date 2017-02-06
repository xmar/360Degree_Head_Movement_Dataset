"""Contains the Frame used to delete and backup existing results.

Author: Xavier Corbillon
IMT Atlantique
"""

from tkinter import *
from Helpers import GetGlobalUserManager
import os
import logging
from functools import partial
import GUIHelpers.HomeFrame
import GUIHelpers.VerticalScrolledFrame as VSF
from shutil import make_archive, rmtree
import time
import datetime


class DeleteTestFrame(VSF.VerticalScrolledFrame):
    """This frame manage the deletion and backup of existing results."""

    def __init__(self, *args, **kwargs):
        """Init function."""
        super().__init__(*args, **kwargs)
        self.logger = \
            logging.getLogger('TestManager.GUIHelpers.DeleteTestFrame')
        self.userManager = GetGlobalUserManager()
        self.InitWidgets()

    def InitWidgets(self):
        """Init the widgets of the frame."""
        currentRow = 0
        self.userNamesLabels = dict()
        self.userTestLabels = dict()
        self.userTestDelButtons = dict()
        userDict = self.userManager.userDict
        for userId in userDict:
            user = userDict[userId]
            userNames = '[{}] {} {}:'.format(userId,
                                             user.firstName,
                                             user.lastName)
            self.userNamesLabels[userId] = Label(self.interior, text=userNames)
            self.userNamesLabels[userId].grid(row=currentRow, column=0)

            self.userTestLabels[userId] = dict()
            self.userTestDelButtons[userId] = dict()
            for testPath in user.GetExistingTestPathList():
                testName = os.path.split(testPath)[1]
                self.userTestLabels[userId][testPath] = Label(self.interior,
                                                              text=testName)
                self.userTestLabels[userId][testPath].grid(row=currentRow,
                                                           column=1)
                self.userTestDelButtons[userId][testPath] = \
                    Button(self.interior,
                           text='Del',
                           command=partial(self.PressedDelButton,
                                           userId,
                                           testPath)
                           )
                self.userTestDelButtons[userId][testPath].grid(row=currentRow,
                                                               column=2)
                currentRow += 1

            currentRow += 1

    def PressedDelButton(self, uid, testPath):
        """Callback of the delete button."""
        backupPath = '{}_{}'.format(testPath,
                                    datetime.datetime.fromtimestamp(
                                        time.time()).strftime(
                                            '%Y_%m_%d_%H_%M_%S'))
        self.logger.info('Delete button pressed for uid {}: delete test '
                         '{} and backup to file {}'.format(uid,
                                                           testPath,
                                                           backupPath))
        make_archive(backupPath, 'gztar', testPath)
        rmtree(testPath)
        self.grid_forget()
        self.destroy()
        GUIHelpers.HomeFrame.GetHomeFrame().grid(row=0, column=0)
