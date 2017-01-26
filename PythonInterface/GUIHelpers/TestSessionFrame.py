"""A frame to manage a running test session.

Author: Xavier Corbillon
IMT Atlantique
"""

from Helpers import TestManager, User, Video
from GUIHelpers import GetHomeFrame, GetRootFrame
from tkinter import *
from functools import partial
import logging

import threading
import queue


class CommunicationQueues(object):
    """Class used for the working thread to communicat with tkinter."""

    def __init__(self):
        """init function."""
        self.nameQueue = queue.Queue()
        self.statusQueue = queue.Queue()
        self.feedbackQueue = queue.Queue()
        self.done = False

    @staticmethod
    def __ProcessQueue(q, label):
        try:
            string = q.get_nowait()
        except queue.Empty:
            pass
        else:
            label.config(text=string)

    def LoopUpdateTkinterLabels(self,
                                nameLabel,
                                statusLabel,
                                feedbackLabel,
                                doneCallback
                                ):
        """Called from the tkinter thread to update the labels until done."""
        self.__ProcessQueue(self.nameQueue, nameLabel)
        self.__ProcessQueue(self.statusQueue, statusLabel)
        self.__ProcessQueue(self.feedbackQueue, feedbackLabel)
        if not self.done:
            GetRootFrame().after(500,
                                 CommunicationQueues.LoopUpdateTkinterLabels,
                                 self,
                                 nameLabel,
                                 statusLabel,
                                 feedbackLabel,
                                 doneCallback
                                 )
        else:
            doneCallback()


class TestSessionFrame(Frame):
    """Frame that manage a running test session."""

    def __init__(self, *args, selectedUser, testId, videoList, **kwargs):
        """init function."""
        Frame.__init__(self, *args, **kwargs)
        self.logger = logging.getLogger(
            'TestManager.GUIHelpers.TestSessionFrame'
            )
        self.testManager = TestManager(selectedUser, testId, videoList)
        self.currentTest = None

        self.frameTitle = \
            Label(self,
                  text='Test #{} for {} {}'.format(
                                       testId,
                                       selectedUser.firstName,
                                       selectedUser.lastName
                                       )
                  )
        self.frameTitle.grid(row=0, column=0)

        self.startButton = Button(
            self,
            text='Start test',
            command=partial(self.PressedStartButton)
            )
        self.startButton.grid(row=1, column=0)

    def PressedStartButton(self):
        """Callback for the self.startTrainingButton."""
        self.startButton.config(state=DISABLED)
        self.logger.info('Start test pressed')

        self.testName = \
            Label(self,
                  text='1'
                  )
        self.testName.grid(row=3, column=0)

        self.testStatus = \
            Label(self,
                  text='2'
                  )
        self.testStatus.grid(row=3, column=1)

        self.testFeedback = \
            Label(self,
                  text='3'
                  )
        self.testFeedback.grid(row=4, column=0)

        self.__RunNextText()

    def __RunNextText(self):
        self.currentTest = self.testManager.NextTest()
        if self.currentTest is not None:
            self.logger.info('Next test for {} {} using video {}'.format(
                self.currentTest.user.firstName,
                self.currentTest.user.lastName,
                self.currentTest.video.id
            ))
            commQueue = CommunicationQueues()
            self.workingThread = threading.Thread(
                target=partial(self.currentTest.Run, commQueue)
                )
            self.workingThread.start()
            commQueue.LoopUpdateTkinterLabels(self.testName,
                                              self.testStatus,
                                              self.testFeedback,
                                              partial(self.__RunNextText)
                                              )
        else:
            self.testName.grid_forget()
            self.testStatus.grid_forget()
            self.testFeedback.grid_forget()

            self.quitButton = Button(self,
                                     text='Test Done: Quit',
                                     command=partial(self.PressedQuitButton)
                                     )
            self.quitButton.grid(row=1, column=0)

    def PressedQuitButton(self):
        """Callback for the self.quitButton."""
        self.grid_forget()
        GetHomeFrame().grid(row=0, column=0)
        self.destroy()
