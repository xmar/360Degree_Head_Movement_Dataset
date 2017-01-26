"""Manager all video.

Author: Xavier Corbillon
IMT Atlantique
"""

import Helpers.Video


class VideoManager(object):
    """Class used to manage all video."""

    def __init__(self):
        """Init function."""
        self.trainingContent = None
        self.videoDict = dict()

    def SetTrainingContent(self, trainingVideo):
        """Set the training video.

        :type trainingVideo: Helpers.Video
        """
        self.trainingContent = trainingVideo

    def AddVideo(self, video):
        """Add a new video.

        :type video: Helpers.Video
        """
        self.videoDict[video.id] = video

    def GetTrainingContent(self):
        """Get the training video.

        :rtype: None or a Helpers.Video
        """
        return self.trainingContent

    def GetVideoDict(self):
        """Get the video dictionnary.

        :rtype: dictionnary of (string, Helpers.Video)
        """
        return self.videoDict
