"""Contains the UserManager class.

Author: Xavier Corbillon
IMT Atlantique
"""

from .User import User
import logging
import os
import math

global_user_manager = None


def GetGlobalUserManager(*args, **kwargs):
    """Get the unique user manager or create it."""
    global global_user_manager
    if global_user_manager is None:
        global_user_manager = UserManager(*args, **kwargs)
    return global_user_manager


class UserManager(object):
    """class that manager all the known users in the system.

    This class is used to parse the existing user file and to add a new user
    """

    def __init__(self, pathToExistingUserFile, rootResultFolder):
        """init function that parse the existing user file.

        :param pathToExistingUserFile: Path to the existing user file.
            This file contains one line for each user. It contains the user
            first name, last name and the user id separated with ';'.
        :type pathToExistingUserFile: str
        :param rootResultFolder: The path to the root result folder
        :type rootResultFolder: str
        """
        self.logger = logging.getLogger('TestManager.Helpers.UserManager')
        self.pathToExistingUserFile = pathToExistingUserFile
        self.rootResultFolder = rootResultFolder

        self.userDict = dict()
        self.maxUid = -1
        if os.path.exists(self.pathToExistingUserFile):
            with open(self.pathToExistingUserFile, 'r') as i:
                for line in i:
                    info = line.split(';')
                    uid = int(info[2])
                    self.maxUid = max(uid, self.maxUid)
                    firstName = info[0]
                    lastName = info[1]
                    self.logger.info(
                        'Add existing user: [{}] {} {}'.format(uid,
                                                               firstName,
                                                               lastName)
                        )
                    self.userDict[uid] = User(firstName,
                                              lastName,
                                              uid,
                                              rootResultFolder
                                              )

    def GetExistingUserList(self):
        """Return a dict of uid,string: first and last name."""
        ans = dict()
        for uid, user in self.userDict.items():
            ans[uid] = '{} {}'.format(user.firstName, user.lastName)
        return ans

    def GetUserByUid(self, uid):
        """Return the user with this uid or None."""
        return self.userDict[uid] if uid in self.userDict else None

    def AddNewUser(self, firstName, lastName):
        """Add a new user to the user dict and return the uid."""
        self.maxUid += 1
        self.logger.info(
            'Create a new user with uid {}: {} {}'.format(self.maxUid,
                                                          firstName,
                                                          lastName)
            )
        self.userDict[self.maxUid] = User(firstName, lastName, self.maxUid,
                                          self.rootResultFolder)
        self.StoreUserList()
        return self.maxUid

    def StoreUserList(self):
        """Store the user dict to the pathToExistingUserFile file."""
        with open(self.pathToExistingUserFile, 'w') as o:
            self.logger.debug('Update existing user file')
            for user in self.userDict.values():
                o.write('{};{};{}\n'.format(user.firstName,
                                            user.lastName,
                                            user.uid)
                        )

    def StoreUserStats(self, pathId):
        '''Compute stats on the users.

        :param pathId: the path to store the results without extension
        '''
        pathGlobalStats = '{}_globalStats.txt'.format(pathId)
        pathAgeStats = '{}_AgeStats.txt'.format(pathId)

        filteredUser = dict()  # remove users without tests
        for userId in self.userDict:
            user =  self.userDict[userId]
            if user.GetNextTestId() > 0:
                filteredUser[userId] = user

        womanByAge = dict()
        manByAge = dict()
        ageList = list()
        ageStep = 10
        firstTimeByAge = dict()
        for user in filteredUser.values():
            ageList.append(user.age)
            age = math.floor(user.age/ageStep)
            if age not in firstTimeByAge:
                firstTimeByAge[age] = 0
            firstTimeByAge[age] += 1 if user.nbHourHMD == 0 else 0
            if user.sex == 'woman':
                if age not in womanByAge:
                    womanByAge[age] = 0
                womanByAge[age] += 1
            else:
                if age not in manByAge:
                    manByAge[age] = 0
                manByAge[age] += 1

        with open(pathGlobalStats, 'w') as o:
            o.write('\\begin{tabular}{|c|c|c|c|c|c|}'
                    '\n\hline\n'
                    '\\textbf{Number of user}&\\textbf{Minimum age}&'
                    '\\textbf{Average age}&\\textbf{Maximum age}&'
                    '\\textbf{Ratio of woman}&\\textbf{Ratio of first times}'
                    '\\\\\n\hline\n')
            o.write('{}&{}&{:2.2f}&{}&{}\\%&{}\\%\\\\\n\hline\n'.format(
                len(filteredUser),
                min(ageList),
                sum(ageList)/len(ageList) if len(ageList) > 0 else -1,
                max(ageList),
                int(100*sum(womanByAge.values())/len(filteredUser))
                    if len(filteredUser) > 0 else -1,
                int(100*sum(firstTimeByAge.values()) / len(filteredUser))
                    if len(filteredUser) > 0 else -1
            ))
            o.write('\end{tabular}\n')

        with open(pathAgeStats, 'w') as o:
            o.write('ageMin ageMax nbUser nbWoman nbMan nbFirstTime\n')
            for age in sorted(firstTimeByAge.keys()):
                if age not in manByAge:
                    manByAge[age] = 0
                if age not in womanByAge:
                    womanByAge[age] = 0
                o.write('{} {} {} {} {} {}\n'.format(
                    age * ageStep,
                    (age+1) * ageStep,
                    manByAge[age] + womanByAge[age],
                    womanByAge[age],
                    manByAge[age],
                    firstTimeByAge[age]
                ))
