"""Tools to store pictures in a video file.

Author: Xavier Corbillon
IMT Atlantique
"""

import subprocess as sp
import numpy as np

FFMPEG_BIN = "ffmpeg" # on Linux ans Mac OS

class VideoWrite(object):
    """Take pictures and encode them in a video."""

    def __init__(self, outputPath, width, height, fps, codec='libx264'):
        """Init the ffmpeg encoder."""
        self.width = width
        self.height = height
        self.fps = fps
        command = [ FFMPEG_BIN,
                    '-y', # (optional) overwrite output file if it exists
                    '-f', 'rawvideo',
                    '-c:v','rawvideo',
                    '-s', '{}x{}'.format(width, height), # size of one frame
                    '-pix_fmt', 'rgb24',
                    '-r', '{}'.format(fps), # frames per second
                    '-i', '-', # The imput comes from a pipe
                    '-an', # Tells FFMPEG not to expect any audio
                    '-c:v', codec,
                    outputPath ]
        self.pipe = sp.Popen( command, stdin=sp.PIPE, stderr=sp.PIPE)

    def AddPicture(self, pilImage):
        """Add a new picture to the video (a numpy array RGB picture)."""
        pilImage = pilImage.resize((self.width, self.height))
        pilImage = pilImage.convert('RGB')
        ar = np.asarray(pilImage, dtype="int8")
        self.pipe.stdin.write( ar.tostring() )

    def Close(self):
        """Close the stream and finalized the writing of the video."""
        self.pipe.stdin.close()
        if self.pipe.stderr is not None:
            self.pipe.stderr.close()
        self.pipe.wait()

    def __enter__(self):
        """Nothing to do when starting the stream."""
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        """Close the stream."""
        self.Close()
