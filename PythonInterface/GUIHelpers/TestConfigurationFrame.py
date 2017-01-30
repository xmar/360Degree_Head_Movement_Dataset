"""Frame to configure the new test once a user is selected.

Author: Xavier Corbillon
IMT Atlantique
"""

import logging
from GUIHelpers import GetExitManager
from tkinter import *
from tkinter.ttk import *
from functools import partial
from .TestSessionFrame import TestSessionFrame
import shutil
import os


class TestConfigurationFrame(Frame):
    """Frame used to configure to generate a new test configuration."""

    def __init__(self, *args, videoManager, selectedUser, **kwargs):
        """init function.

        :type videoManager: Helpers.VideoManager
        :type selectedUser: Helpers.User
        """
        Frame.__init__(self, *args, **kwargs)
        self.parent = args[0]
        self.selectedUser = selectedUser
        self.videoManager = videoManager
        self.logger = \
            logging.getLogger('TestManager.GUIHelpers.TestConfigurationFrame')
        self.selectedUserLabel = \
            Label(self,
                  text='Test configuration for {} {}'.format(
                                       selectedUser.firstName,
                                       selectedUser.lastName
                                       )
                  )
        self.selectedUserLabel.grid(row=0, column=0)

        self.videosList = list()
        for videoId in videoManager.GetVideoDict():
            self.videosList.append(videoId)
        self.videoListBox = Listbox(self,
                                    selectmode="multiple"
                                    )
        for i in range(len(self.videosList)):
            self.videoListBox.insert(i, self.videosList[i])
        self.videoListBox.select_set(0, END)  # start with all selected
        self.videoListBox.grid(row=3, column=0)

        self.startTestButton = Button(
            self,
            text='Start the test',
            command=partial(self.PressedTestButton)
            )
        self.startTestButton.grid(row=4, column=0)

        exitManager = GetExitManager()
        self.exitCallbackId = \
            exitManager.AddCallback(partial(self.ExitCallback))

    def ExitCallback(self):
        """if called we exited before really starting any test."""
        self.logger.debug('Called ExitCallback from '
                          'the TestConfigurationFrame')
        testNb = self.selectedUser.GetNumberExistingTest()-1
        testFolder = self.selectedUser.GetTestResultFolder(testNb)
        if os.path.exists(testFolder):
            self.logger.debug('Remove test #{} folder for user {} {}'.format(
                testNb,
                self.selectedUser.firstName,
                self.selectedUser.lastName
            ))
            shutil.rmtree(testFolder)

    def PressedTestButton(self):
        """Callback for when the startTestButton is pressed."""
        selectedVideoList = list()
        videoNameList = ''
        testNb = self.selectedUser.GetNumberExistingTest()
        for index in self.videoListBox.curselection():
            selectedVideoList.append(
                self.videoManager.GetVideoDict()[self.videosList[index]]
                )
            videoNameList += self.videosList[index] + ', '
        self.logger.info('Start the test #{} for {} {} with videos {}'.format(
                             testNb,
                             self.selectedUser.firstName,
                             self.selectedUser.lastName,
                             videoNameList[:-2] if len(videoNameList) > 0
                             else '',
                             )
                         )
        testSessionFrame = \
            TestSessionFrame(self.parent,
                             selectedUser=self.selectedUser,
                             testId=testNb,
                             trainingVideo=self.videoManager.trainingContent,
                             videoList=selectedVideoList
                             )
        self.grid_forget()
        testSessionFrame.grid(row=0, column=0)

        exitManager = GetExitManager()
        exitManager.PopCallback(self.exitCallbackId)
        self.destroy()
