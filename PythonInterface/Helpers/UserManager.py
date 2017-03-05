"""Contains the UserManager class.

Author: Xavier Corbillon
IMT Atlantique
"""

from .User import User
import logging
import os
import math
import uuid
import numpy as np

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
        # self.maxUid = -1
        if os.path.exists(self.pathToExistingUserFile):
            with open(self.pathToExistingUserFile, 'r') as i:
                for line in i:
                    info = line.rstrip().split(';')
                    uid = info[2]
                    # self.maxUid = max(uid, self.maxUid)
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
        if os.path.exists(rootResultFolder):
            for (dirpath, dirnames, filenames) in os.walk(rootResultFolder):
                if dirpath == rootResultFolder:
                    for d in dirnames:
                        d = d.rstrip()
                        if len(d) > 4 and d[0:4] == 'uid-':
                            uid = d[4:]
                            if len(uid) > 0 and uid not in self.userDict:
                                self.logger.info(
                                    'Add anonymous existing user:'
                                    ' [{}] {} {}'.format(uid,
                                                         'uid',
                                                         uid)
                                    )
                                self.userDict[uid] = User('uid',
                                                          uid,
                                                          uid,
                                                          rootResultFolder)


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
        newUid = uuid.uuid4()
        while newUid in self.userDict:
            newUid = uuid.uuid4()
        self.logger.info(
            'Create a new user with uid {}: {} {}'.format(newUid,
                                                          firstName,
                                                          lastName)
            )
        self.userDict[newUid] = User(firstName, lastName, newUid,
                                          self.rootResultFolder)
        self.StoreUserList()
        return newUid

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
        pathGlobalStats = '{}_globalStats.tex'.format(pathId)
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
        nbHourUsedIfNotFirstTime = list()
        for user in filteredUser.values():
            ageList.append(user.age)
            age = math.floor(user.age/ageStep)
            if age not in firstTimeByAge:
                firstTimeByAge[age] = 0
            if user.nbHourHMD == 0:
                firstTimeByAge[age] += 1
            else:
                nbHourUsedIfNotFirstTime.append(user.nbHourHMD)
            if user.sex == 'woman':
                if age not in womanByAge:
                    womanByAge[age] = 0
                womanByAge[age] += 1
            else:
                if age not in manByAge:
                    manByAge[age] = 0
                manByAge[age] += 1

        if len(nbHourUsedIfNotFirstTime) > 0:
            print('Average nb hours used if not first time = ',
                  sum(nbHourUsedIfNotFirstTime)/len(nbHourUsedIfNotFirstTime)
                  )
            print('Median nb hours used if not first time = ',
                  np.percentile(nbHourUsedIfNotFirstTime, 50))

        with open(pathGlobalStats, 'w') as o:
            o.write('\\newcommand\\nbUser{{{}}}\n'.format(len(filteredUser)))
            o.write('\\newcommand\\minAge{{{}}}\n'.format(min(ageList)))
            o.write('\\newcommand\\meanAge{{{:2.2f}}}\n'.format(
                sum(ageList)/len(ageList) if len(ageList) > 0 else -1,))
            o.write('\\newcommand\\meanAgeFloor{{{:2.0f}}}\n'.format(
                sum(ageList)/len(ageList) if len(ageList) > 0 else -1,))
            o.write('\\newcommand\\maxAge{{{}}}\n'.format(max(ageList)))
            o.write('\\newcommand\\ratioWoman{{{}}}\n'.format(
                int(100*sum(womanByAge.values())/len(filteredUser))
                if len(filteredUser) > 0 else -1))
            o.write('\\newcommand\\ratioFirstTime{{{}}}\n'.format(
                int(100*sum(firstTimeByAge.values()) / len(filteredUser))
                    if len(filteredUser) > 0 else -1))
            o.write('\\newcommand\\meanNbHourUsed{{{}}}\n'.format(
                np.percentile(nbHourUsedIfNotFirstTime, 50)))

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
