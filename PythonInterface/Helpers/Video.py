"""Contains all information needed on a video.

Author: Xavier Corbillon
IMT Atlantique
"""

import logging
import hashlib
from functools import partial


def md5sum(filePath):
    """Compute the md5 sum of a file located at filePath."""
    with open(filePath, mode='rb') as f:
        d = hashlib.md5()
        for buf in iter(partial(f.read, 128), b''):
            d.update(buf)
    return d.hexdigest()


class Video(object):
    """Class that contains all usefull information usefull on a video."""

    def __init__(self, videoPath, videoId):
        """Init function. It also compute the md5 sum of the video.

        :param videoPath: The absolute or relatif path to the video
        :param videoId: The id use to identify the video
        :type a: str
        :type b: str
        """
        self.logger = logging.getLogger('TestManager.Helpers.Video')
        self.path = videoPath
        self.id = videoId
        self.md5sum = md5sum(self.path)
        self.logger.info('New video with id {} and MD5 sum {}'.format(
            self.id,
            self.md5sum))
