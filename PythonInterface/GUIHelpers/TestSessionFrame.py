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

    def __init__(self, nameLabel, statusLabel, feedbackFpsLabel,
                 feedbackPositionLabel):
        """init function."""
        self.nameQueue = queue.Queue()
        self.statusQueue = queue.Queue()
        self.feedbackFpsQueue = queue.Queue()
        self.feedbackPositionQueue = queue.Queue()
        self.nameLabel = nameLabel
        self.statusLabel = statusLabel
        self.feedbackFpsLabel = feedbackFpsLabel
        self.feedbackPositionLabel = feedbackPositionLabel
        self.done = False
        self.__allEmpty = True

    def __ProcessQueue(self, q, label):
        try:
            string = q.get_nowait()
        except queue.Empty:
            pass
        else:
            self.__allEmpty = False
            label.config(text=string)
            q.task_done()

    def LoopUpdateTkinterLabels(self, doneCallback):
        """Called from the tkinter thread to update the labels until done."""
        self.__allEmpty = True
        self.__ProcessQueue(self.nameQueue, self.nameLabel)
        self.__ProcessQueue(self.statusQueue, self.statusLabel)
        self.__ProcessQueue(self.feedbackFpsQueue, self.feedbackFpsLabel)
        self.__ProcessQueue(self.feedbackPositionQueue,
                            self.feedbackPositionLabel)
        if not self.done:
            if self.__allEmpty:
                GetRootFrame().after(
                    500,
                    CommunicationQueues.LoopUpdateTkinterLabels,
                    self,
                    doneCallback
                    )
            else:
                self.LoopUpdateTkinterLabels(doneCallback)
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
                  text=''
                  )
        self.testName.grid(row=3, column=0)

        self.testStatus = \
            Label(self,
                  text=''
                  )
        self.testStatus.grid(row=3, column=1)

        self.testFeedbackFps = \
            Label(self,
                  text=''
                  )
        self.testFeedbackFps.grid(row=4, column=0)

        self.testFeedbackPosition = \
            Label(self,
                  text=''
                  )
        self.testFeedbackPosition.grid(row=5, column=0)

        self.__RunNextText()

    def __RunNextText(self):
        self.currentTest = self.testManager.NextTest()
        if self.currentTest is not None:
            self.logger.info('Next test for {} {} using video {}'.format(
                self.currentTest.user.firstName,
                self.currentTest.user.lastName,
                self.currentTest.video.id
            ))
            commQueue = CommunicationQueues(self.testName, self.testStatus,
                                            self.testFeedbackFps,
                                            self.testFeedbackPosition)
            self.workingThread = threading.Thread(
                target=partial(self.currentTest.Run, commQueue)
                )
            self.workingThread.start()
            commQueue.LoopUpdateTkinterLabels(partial(self.__RunNextText))
        else:
            self.testName.grid_forget()
            self.testStatus.grid_forget()
            self.testFeedbackFps.grid_forget()
            self.testFeedbackPosition.grid_forget()

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
