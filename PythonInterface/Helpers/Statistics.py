"""Functions/Classes to compute statistics on test logs.

Author: Xavier Corbillon
IMT Atlantique
"""

import Helpers.Quaternion as Q
import math
import numpy as np
import matplotlib.pyplot as plt
import os
import threading
from functools import partial


class AggregatedResults(object):
    """Contains aggregated results (i.e. results of results)."""

    def __init__(self):
        """Generate an aggregate with nothing.

        To fill it use the __add__ function.
        """
        self.aggPositionMatrix = None
        self.aggAngularVelocity = None

    def __add__(self, processedResult):
        """Add results to the aggregator."""
        if self.aggPositionMatrix is None:
            self.aggPositionMatrix = processedResult.positionMatrix.copy()
        else:
            self.aggPositionMatrix += processedResult.positionMatrix
        if self.aggAngularVelocity is None:
            self.aggAngularVelocity = \
                (processedResult.angularVelocityWindow[0],
                 processedResult.angularVelocityWindow[1].copy())
        elif self.aggAngularVelocity[0] == \
                processedResult.angularVelocityWindow[0]:
            self.aggAngularVelocity[1].extend(
                processedResult.angularVelocityWindow[1]
                )
        else:
            print('ERR: cannot aggregate if do not have the same step')
            exit(1)
        return self

    def Normalize(self):
        """Normalize the values."""
        self.aggPositionMatrix /= self.aggPositionMatrix.sum()

    def StoreAngularVelocity(self, filePath):
        """Store angular velocity cdf to file."""
        with open(filePath, 'w') as o:
            o.write('cdf angVel angVelDeg\n')
            angVel = self.aggAngularVelocity[1]
            for r in range(0, 101):
                values = np.percentile(angVel, r)
                o.write('{} {} {}\n'.format(r, values, values*180/math.pi))

    def StorePositions(self, filePath, vmax):
        """Store the position matrix image in a file."""
        plt.matshow(self.aggPositionMatrix, vmin=0, vmax=vmax)
        plt.savefig(filePath, bbox_inches='tight')
        plt.close()


class ProcessedResult(object):
    """Contains the quaternions and timestamp information of a result."""

    def __init__(self, resultPath, skiptime=10, step=0.03):
        """Get the results from the result file.

        :param skiptime: time in second to skip
        :param step: step in second for the filtering
        """
        self.step = step
        self.quaternions = dict()
        self.filteredQuaternions = dict()
        self.frameIds = dict()
        self.angularVelocityDict = dict()
        self.angularVelocityWindow = (0, list())  # first = step,
        #                                          seconds = values
        self.positionMatrix = np.zeros((1, 1))
        firstTimestamp = None
        isSkiping = True
        with open(resultPath, 'r') as i:
            for line in i:
                values = line.split(' ')
                timestamp = float(values[0])
                if firstTimestamp is None:
                    firstTimestamp = timestamp
                timestamp -= firstTimestamp
                if isSkiping:
                    if timestamp > skiptime:
                        firstTimestamp = timestamp
                        isSkiping = False
                else:
                    q = Q.Quaternion(w=float(values[2]),
                                     v=Q.Vector(x=float(values[3]),
                                                y=float(values[4]),
                                                z=float(values[5])
                                                )
                                     )
                    frameId = int(values[1])
                    self.frameIds[timestamp] = frameId
                    self.quaternions[timestamp] = q
        self.__filterQuaternion()

    def __radd__(self, other):
        """To be able to generate an AggregatedResults from sum()."""
        if other is 0:
            return AggregatedResults() + self
        else:
            print('Error')
            exit(3)

    def ComputeAngularVelocity(self):
        """Compute the angular velocity."""
        qList = list()
        tList = list()
        for t in self.filteredQuaternions:
            q = self.filteredQuaternions[t]
            qList.append(q)
            tList.append(t)
            if len(qList) == 3:
                q1 = qList[0]
                q2 = qList[1]
                q3 = qList[2]

                velocity = (Q.Quaternion.Distance(q1, q2) +
                            Q.Quaternion.Distance(q2, q3))/2
                velocity /= (tList[2]-tList[0])
                self.angularVelocityDict[tList[1]] = velocity

                qList = qList[1:]
                tList = tList[1:]
        self.__filterVelocity()

    def ComputePositions(self, width=50, height=50):
        """Compute the position matrix."""
        self.positionMatrix = np.zeros((width, height))
        for t in self.filteredQuaternions:
            q = self.filteredQuaternions[t]
            v = q.Rotation(Q.Vector(1, 0, 0)).v
            theta, phi = v.ToPolar()
            i = int(width*(theta + math.pi)/(2*math.pi))
            j = int(height*phi/math.pi)
            self.positionMatrix[j, i] += 1
        if len(self.filteredQuaternions) > 0:
            self.positionMatrix /= self.positionMatrix.sum()

    def __filterVelocity(self):
        """Fill the angularVelocityWindow tuple."""
        step = self.step
        windowedVelocity = dict()
        maxTimestamp = max(self.angularVelocityDict.keys())
        for t in self.angularVelocityDict:
            index = int(t/step)
            if index not in windowedVelocity:
                windowedVelocity[index] = list()
            windowedVelocity[index].append(self.angularVelocityDict[t])
        self.angularVelocityWindow = (step, list())
        angVel = self.angularVelocityWindow[1]
        for index in windowedVelocity:
            angVel.append(
                sum(windowedVelocity[index])/len(windowedVelocity[index]))

    def __filterQuaternion(self):
        """Filter the quaternions."""
        step = self.step
        self.filteredQuaternions = self.quaternions

    def StoreAngularVelocity(self, filePath):
        """Store the position matrix image in a file."""
        angVel = self.angularVelocityWindow[1]
        with open(filePath, 'w') as o:
            o.write('cdf angVel angVelDeg\n')
            for r in range(0, 101):
                values = np.percentile(angVel, r)
                o.write('{} {} {}\n'.format(r, values, values*180/math.pi))

    def StorePositions(self, filePath, vmax):
        """Store the position matrix image in a file."""
        plt.matshow(self.positionMatrix, vmin=0, vmax=vmax)
        plt.savefig(filePath, bbox_inches='tight')
        plt.close()


global_statistics = None


def GetGlobalStatistics(*args, **kwargs):
    """Return the unique global statistics object."""
    global global_statistics
    if global_statistics is None:
        global_statistics = Statistics(*args, **kwargs)
    return global_statistics


class Statistics(object):
    """Class that compute statistics information about the different tests."""

    def __init__(self, userManager):
        """Init the statistics with the userManager object."""
        self.userManager = userManager
        self.workingThread = None
        self.progressBar = None
        self.done = True

    def RunComputation(self, progressBar, doneCallback):
        """Do the work to compute the statistics."""
        self.progressBar = progressBar
        self.doneCallback = doneCallback
        self.resultsByIdInfo = dict()
        self.resultsById = dict()
        self.resultsByUser = dict()
        self.resultsByVideo = dict()
        for userId in self.userManager.userDict:
            user = self.userManager.userDict[userId]
            self.resultsByUser[userId] = list()
            testPathList = user.GetExistingTestPathList()
            for testPath in testPathList:
                for root, dirs, files in os.walk(testPath):
                    for videoId in dirs:
                        if 'training' not in videoId:
                            if videoId not in self.resultsByVideo:
                                self.resultsByVideo[videoId] = list()
                            resultId = '{}_{}'.format(userId, videoId)
                            resultPath = \
                                os.path.join(root,
                                             videoId,
                                             '{}_0.txt'.format(videoId))
                            self.resultsByIdInfo[resultId] = (resultPath,
                                                              userId,
                                                              videoId)
                        # resultsById[resultId] = ProcessedResult(resultPath)
                        # resultsByUser[userId].append(resultsById[resultId])
                        # resultsByVideo[videoId].append(resultsById[resultId])
        self.progressBar['value'] = 0
        self.progressBar['maximum'] = 2 * (len(self.resultsByIdInfo) +
                                           len(self.resultsByVideo) +
                                           len(self.resultsByUser))
        self.workingThread = threading.Thread(
            target=partial(Statistics._ComputationWorkThread, self)
            )
        self.done = False
        self.workingThread.start()

    def Join(self):
        """Join the working thread."""
        if self.workingThread is not None:
            self.done = True
            self.doneCallback = None
            # import time
            # time.sleep(2)
            del self.workingThread
            # self.workingThread.join()
            self.workingThread = None

    def _ComputationWorkThread(self):
        """Thread main function that do the actual computation."""
        if not os.path.exists('results/statistics/individual'):
            os.makedirs('results/statistics/individual')
        if not os.path.exists('results/statistics/users'):
            os.makedirs('results/statistics/users')
        if not os.path.exists('results/statistics/videos'):
            os.makedirs('results/statistics/videos')
        vmax = 0
        for resultId in self.resultsByIdInfo:
            if not self.done:
                resultPath, userId, videoId = self.resultsByIdInfo[resultId]
                processedResult = ProcessedResult(resultPath)
                if len(processedResult.filteredQuaternions) > 10:
                    self.resultsById[resultId] = processedResult
                    vmax = max(vmax,
                               self.resultsById[resultId].positionMatrix.max())
                    self.resultsByUser[userId].append(
                        self.resultsById[resultId])
                    self.resultsByVideo[videoId].append(
                        self.resultsById[resultId])
                    self.resultsById[resultId].ComputeAngularVelocity()
                    self.resultsById[resultId].ComputePositions()
                self.progressBar['value'] += 1
            else:
                print('thread done')
                return
        aggrUserResults = dict()
        aggrVideoResults = dict()
        for userId in self.resultsByUser:
            if len(self.resultsByUser[userId]) > 0:
                aggrUserResults[userId] = sum(self.resultsByUser[userId])
                aggrUserResults[userId].Normalize()
                vmax = max(vmax,
                           aggrUserResults[userId].aggPositionMatrix.max())
                self.progressBar['value'] += 1
        for videoId in self.resultsByVideo:
            if len(self.resultsByVideo[videoId]) > 0:
                aggrVideoResults[videoId] = sum(self.resultsByVideo[videoId])
                aggrVideoResults[videoId].Normalize()
                vmax = max(vmax,
                           aggrVideoResults[videoId].aggPositionMatrix.max())
                self.progressBar['value'] += 1
        print('vmax = ', vmax)
        for userId in aggrUserResults:
            aggrUserResults[userId].StorePositions(
                'results/statistics/users/uid{}.pdf'.format(userId), vmax
            )
            aggrUserResults[userId].StoreAngularVelocity(
                'results/statistics/users/uid{}.txt'.format(userId)
            )
            self.progressBar['value'] += 1
        for videoId in aggrVideoResults:
            aggrVideoResults[videoId].StorePositions(
                'results/statistics/videos/{}.pdf'.format(videoId), vmax
            )
            aggrVideoResults[videoId].StoreAngularVelocity(
                'results/statistics/videos/{}.txt'.format(videoId)
            )
            self.progressBar['value'] += 1
        for resultId in self.resultsById:
            self.resultsById[resultId].StorePositions(
                'results/statistics/individual/{}.pdf'.format(resultId), vmax
            )
            self.resultsById[resultId].StoreAngularVelocity(
                'results/statistics/individual/{}.txt'.format(resultId)
            )
            self.progressBar['value'] += 1
        # del self.workingThread
        self.done = True
        self.progressBar = None
        self.workingThread = None
        if self.doneCallback is not None:
            self.doneCallback()
