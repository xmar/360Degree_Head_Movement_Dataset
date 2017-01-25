"""Contains all information needed on a user.

Author: Xavier Corbillon
IMT Atlantique
"""

import os


class User(object):
    """Class that contains all usefull information on a user."""

    def __init__(self, userFirstName, userLastName, userId):
        """init function.

        :type userFirstName: str
        :type userLastName: str
        :type userId: int
        """
        self.firstName = userFirstName
        self.lastName = userLastName
        self.uid = userId

    def GetUserResultFolder(self, rootResultFolder):
        """Return the path to the user result folder.

        :param rootResultFolder: The path to the root of result folder
        :type rootResultFolder: str
        :rtype: str
        """
        return os.path.join(rootResultFolder, 'uid'+str(self.uid))
