"""Manage a specific test for a specific user.

Author: Xavier Corbillon
IMT Atlantique
"""

import logging
from .User import User
from .Video import Video
from random import shuffle
import Helpers

import time
import os
import subprocess as sub
import zmq


class Test(object):
    """A specific test of a specific video."""

    def __init__(self, user, testId, video):
        """init function.

        :type user: Helpers.User
        :type testId: int
        :type video: Helpers.Video
        """
        self.logger = logging.getLogger('TestManager.Helpers.Test')
        self.user = user
        self.testId = testId
        self.video = video
        self.logger.info(
            'Configure a new test for {} {}: testId {}, videoId {}'.format(
                self.user.firstName,
                self.user.lastName,
                self.testId,
                self.video.id
            ))
        self.testFolder = user.GetTestResultFolder(testId)
        self.configPath = os.path.join(self.testFolder,
                                       '{}.ini'.format(video.id))
        self.logFolder = os.path.join(self.testFolder,
                                      '{}'.format(video.id))

        if not os.path.exists(self.logFolder):
            os.makedirs(self.logFolder)

        self.GenerateIniFile()

    def GenerateIniFile(self):
        """Generate the ini file for the C++ software."""
        iniConfParser = Helpers.GetIniConfParser()
        fileStr = '[Config]\n'
        fileStr += 'textureConfig=Video1\n'
        fileStr += 'projectionConfig=Equirectangular\n'
        fileStr += 'logWriterConfig=LogWriter\n'
        fileStr += 'publisherLogConfig=PublisherLog\n'
        fileStr += '\n'
        fileStr += '[Video1]\n'
        fileStr += 'type=video\n'
        fileStr += 'nbFrame={}\n'.format(self.video.nbMaxFrames)
        fileStr += 'bufferSize={}\n'.format(self.video.bufferSize)
        fileStr += 'pathToVideo={}\n'.format(self.video.path)
        fileStr += '\n'
        fileStr += '[Equirectangular]\n'
        fileStr += 'type=Equirectangular\n'
        fileStr += '\n'
        fileStr += '[LogWriter]\n'
        fileStr += 'outputDirPath={}\n'.format(self.logFolder)
        fileStr += 'outputId={}\n'.format(self.video.id)
        fileStr += '\n'
        fileStr += '[PublisherLog]\n'
        fileStr += 'port={}\n'.format(
            iniConfParser.portForInterprocessCommunication
            )
        self.logger.debug('Generate ini file: {}'.format(self.configPath))
        with open(self.configPath, 'w') as o:
            o.write(fileStr)

    def Run(self, commQueue):
        """Run the test. This is a blocking call."""
        commQueue.nameQueue.put('VideoID {}:'.format(self.video.id))
        commQueue.statusQueue.put('is STARTING')
        commQueue.feedbackFpsQueue.put('No received feedback\n' +
                                       'No received feedback\n' +
                                       'No received feedback')
        commQueue.feedbackPositionQueue.put('No received feedback')

        iniConfParser = Helpers.GetIniConfParser()

        context = zmq.Context()
        socket = context.socket(zmq.SUB)
        socket.setsockopt_string(zmq.SUBSCRIBE, '')
        socket.connect('tcp://127.0.0.1:{}'.format(
            iniConfParser.portForInterprocessCommunication)
                       )

        pathToOsvrClientPlayer = iniConfParser.pathToOsvrClientPlayer

        first = True
        with sub.Popen([pathToOsvrClientPlayer,
                        '-c',
                        self.configPath]) as proc:
            while proc.poll() is None:
                if first:
                    first = False
                    socket.connect('tcp://127.0.0.1:5542')
                if commQueue.stop:
                    proc.kill()
                    time.sleep(0.1)
                    continue
                try:
                    msg = socket.recv_string(1)
                except zmq.error.Again:
                    # print('sleep')
                    time.sleep(0.1)
                else:
                    msgType, value = msg.split(': ', maxsplit=1)
                    if msgType == 'APP_STATUS':
                        commQueue.statusQueue.put(value)
                    elif msgType == 'FPS_INFO':
                        tmp = value.split(' | ')
                        value = tmp[0]
                        for t in tmp[1:]:
                            value += '\n{}'.format(t)
                        commQueue.feedbackFpsQueue.put(value)
                    elif msgType == 'POSITION_INFO':
                        commQueue.feedbackPositionQueue.put(value)
                    else:
                        self.logger.error(
                            'Unknown message type from application: {}'.format(
                                msg))

        commQueue.statusQueue.put('is DONE')

        time.sleep(0.1)

        commQueue.done = True


class TestManager(object):
    """This class manage the configuration and run of one test session."""

    def __init__(self, user, testId, trainingVideo, videoList):
        """init function.

        :type user: Helpers.User
        :type testId: int
        :type videoList: list of Helpers.Video
        """
        self.user = user
        self.testId = testId
        self.trainingVideo = trainingVideo
        self.videoList = videoList.copy()
        shuffle(self.videoList)  # randomize the video order
        if self.trainingVideo is not None:
            self.videoList.insert(0, self.trainingVideo)
        self.StoreTestInfo()

    def StoreTestInfo(self):
        """Store in the file the test information (selected video order)."""
        testFolder = self.user.GetTestResultFolder(self.testId)
        if not os.path.exists(testFolder):
            os.makedirs(testFolder)
        storeInfoPath = os.path.join(testFolder, 'testInfo.txt')
        with open(storeInfoPath, 'w') as o:
            for video in self.videoList:
                o.write('{} {}\n'.format(video.id, video.md5sum))

    def NextTest(self):
        """Return an instance of Test for the next test or None."""
        if len(self.videoList) > 0:
            nextVideo = self.videoList[0]
            self.videoList.pop(0)
            return Test(self.user, self.testId, nextVideo)
        else:
            return None
