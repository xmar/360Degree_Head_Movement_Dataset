"""Contains all information needed on a video.

Author: Xavier Corbillon
IMT Atlantique
"""

import logging
import hashlib
from functools import partial
import os


def md5sum(filePath):
    """Compute the md5 sum of a file located at filePath."""
    dirPath, basename = os.path.split(filePath)
    fileName, extension = os.path.splitext(basename)
    md5StoreFile = os.path.join(dirPath, '.{}.md5'.format(fileName))
    if os.path.isfile(md5StoreFile):
        with open(md5StoreFile, 'r') as i:
            for line in i:
                # the first line contains the MD5sum + '\n'
                return line[:-1]
    else:
        with open(filePath, mode='rb') as f:
            d = hashlib.md5()
            for buf in iter(partial(f.read, 128), b''):
                d.update(buf)
            output = d.hexdigest()
            with open(md5StoreFile, 'w') as o:
                o.write('{}\n'.format(output))
        return output


class Video(object):
    """Class that contains all usefull information usefull on a video."""

    def __init__(self, videoPath, videoId, nbMaxFrames, bufferSize,
                 startOffsetInSecond):
        """Init function. It also compute the md5 sum of the video.

        :param videoPath: The absolute or relatif path to the video
        :param videoId: The id use to identify the video
        :type videoPath: str
        :type videoId: str
        :type nbMaxFrames: int
        :type bufferSize: int
        """
        self.logger = logging.getLogger('TestManager.Helpers.Video')
        self.path = videoPath
        self.id = videoId
        self.nbMaxFrames = nbMaxFrames
        self.bufferSize = bufferSize
        self.startOffsetInSecond = startOffsetInSecond
        self.md5sum = md5sum(self.path)
        self.logger.info('New video with id {} and MD5 sum {}'.format(
            self.id,
            self.md5sum))
