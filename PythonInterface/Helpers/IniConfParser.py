"""Parse the ini configuration file to generate usefull information.

Author: Xavier Corbillon
IMT Atlantique
"""

import logging
import configparser
from .Video import Video
from .VideoManager import VideoManager

global_initConfParser = None


def GetIniConfParser(*args, **kwargs):
    """Return the global IniConfParser or create it."""
    global global_initConfParser
    if global_initConfParser is None:
        global_initConfParser = IniConfParser(*args, **kwargs)
    return global_initConfParser


class IniConfParser(object):
    """Parse a ini file and store usefull object inside."""

    def __init__(self, pathToIniFile, ch=None, fh=None):
        """init function.

        :param pathToIniFile: a string containing the path to the ini file
        :param ch: console handler for logging module or None
        :param fh: file handler for logging module or None
        """
        # read the configuration file
        self.config = configparser.ConfigParser()
        self.config.read(pathToIniFile)

        # the main section is the [AppConfig] section
        self.resultFolder = self.config['AppConfig']['resultFolder']
        self.pathToOsvrClientPlayer = \
            self.config['AppConfig']['pathToOsvrClientPlayer']
        self.portForInterprocessCommunication = \
            self.config['AppConfig']['portForInterprocessCommunication']
        if ch is not None:
            consolLogLevel = self.config['AppConfig']['consoleLogLevel']
            ch.setLevel(logging.DEBUG if 'DEBUG' == consolLogLevel
                        else logging.INFO if 'INFO' == consolLogLevel
                        else logging.WARNING if 'WARNING' == consolLogLevel
                        else logging.ERROR
                        )
        if fh is not None:
            fileLogLevel = self.config['AppConfig']['fileLogLevel']
            fh.setLevel(logging.DEBUG if 'DEBUG' == fileLogLevel
                        else logging.INFO if 'INFO' == fileLogLevel
                        else logging.WARNING if 'WARNING' == fileLogLevel
                        else logging.ERROR if 'ERROR' == fileLogLevel
                        else logging.INFO
                        )

        self.videoManager = VideoManager()
        trainingVideoConfig = self.config['AppConfig']['trainingVideo'].strip()
        if len(trainingVideoConfig) > 0:
            videoId = self.config[trainingVideoConfig]['id']
            videoPath = self.config[trainingVideoConfig]['path']
            nbMaxFrames = self.config[trainingVideoConfig]['nbMaxFrames']
            bufferSize = self.config[trainingVideoConfig]['bufferSize']
            self.videoManager.SetTrainingContent(
                Video(videoPath=videoPath,
                      videoId=videoId,
                      nbMaxFrames=nbMaxFrames,
                      bufferSize=bufferSize
                      )
            )
        for videoConfig in \
                self.config['AppConfig']['videoConfigList'].split(','):
            videoConfig = videoConfig.strip()
            videoId = self.config[videoConfig]['id']
            videoPath = self.config[videoConfig]['path']
            nbMaxFrames = self.config[videoConfig]['nbMaxFrames']
            bufferSize = self.config[videoConfig]['bufferSize']
            self.videoManager.AddVideo(
                Video(videoPath=videoPath,
                      videoId=videoId,
                      nbMaxFrames=nbMaxFrames,
                      bufferSize=bufferSize
                      )
            )
