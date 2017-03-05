"""Functions/Classes to compute statistics on test logs.

Author: Xavier Corbillon
IMT Atlantique
"""

# import Helpers.Quaternion as Q
import Helpers.CQuaternion as Q
import Helpers.FFmpeg as FFmpeg
import math
import numpy as np
import matplotlib.pyplot as plt
import os
import sys
import threading
import configparser
from functools import partial
import io
import PIL
from pathos.multiprocessing import ProcessingPool
import dill
import sys


PATH_TO_STATISTIC_RESULTS = 'results/statistics'

ORIGINAL_POSITION = Q.Vector(-1, 0, 0)

def StoreAngularVelocity(processedResultList, filePath, isAggr):
    """Store angular velocity cdf to file."""
    angVel = list()
    if isAggr:
        for processedResult in processedResultList:
            for p in processedResult.angularVelocityDict.values():
                angVel.append(p)
    else:
        for p in processedResultList.values():
            angVel.append(p)
    angVelNorm = list()
    verticalAngVel = list()
    horizontalAngVel = list()
    yawAngVel = list()
    pitchAngVel = list()
    rollAngVel = list()
    for q, w in angVel:
        angVelNorm.append(w.Norm())
        horizontalAngVel.append(abs(w.z))
        verticalAngVel.append(
            (Q.Vector(w.x, w.y, 0)).Norm())
        q.Normalize()
        yawAngVel.append(abs(q.Rotation(Q.Vector(0, 0, 1)) * w))
        pitchAngVel.append(abs(q.Rotation(Q.Vector(0, 1, 0)) * w))
        rollAngVel.append(abs(q.Rotation(Q.Vector(1, 0, 0)) * w))
    angVelNorm = sorted(angVelNorm)
    verticalAngVel = sorted(verticalAngVel)
    horizontalAngVel = sorted(horizontalAngVel)
    yawAngVel = sorted(yawAngVel)
    pitchAngVel = sorted(pitchAngVel)
    rollAngVel = sorted(rollAngVel)
    with open(filePath, 'w') as o:
        o.write('cdf angVel angVelDeg verticalAngVel ' +
                'verticalAngVelDeg horizontalAngVel horizontalAngVelDeg' +
                'yawAngVel yawAngVelDeg pitchAngVel pitchAngVelDeg ' +
                'rollAngVel rollAngVelDeg\n')
        for r in range(0, 101):
            values = np.percentile(angVelNorm, r) if len(angVelNorm) > 0 else -1
            o.write('{} {} {} '.format(r, values, values*180/math.pi))
            values = np.percentile(verticalAngVel, r) if len(verticalAngVel) > 0 else -1
            o.write('{} {} '.format(values, values*180/math.pi))
            values = np.percentile(horizontalAngVel, r) if len(horizontalAngVel) > 0 else -1
            o.write('{} {} '.format(values, values*180/math.pi))
            values = np.percentile(yawAngVel, r) if len(yawAngVel) > 0 else -1
            o.write('{} {} '.format(values, values*180/math.pi))
            values = np.percentile(pitchAngVel, r) if len(pitchAngVel) > 0 else -1
            o.write('{} {} '.format(values, values*180/math.pi))
            values = np.percentile(rollAngVel, r) if len(rollAngVel) > 0 else -1
            o.write('{} {}\n'.format(values, values*180/math.pi))


def StoreAngularVelocityPerSegment(processedResultList, segmentSize, filePath,
                                   useRealTimestamp=True):
    """Compute the min, max, median angular velocity on all segments.

    :param segmentSize: the segment size in second
    """
    results = dict() # segmentId: AngVelValues
    class AngVelValues(object):
        def __init__(self):
            self.angVelNorm = list()
            self.verticalAngVel = list()
            self.horizontalAngVel = list()
            self.yawAngVel = list()
            self.pitchAngVel = list()
            self.rollAngVel = list()

        def GetMinMedMaxString(self, angVelTypeList):
            s = ''
            listOfOutputList = []
            for angVelName in angVelTypeList:
                listOfOutputList.append(getattr(self, angVelName))
            for l in listOfOutputList:
                minA = np.percentile(l, 10) if len(l) > 0 else -1
                p25A = np.percentile(l, 25) if len(l) > 0 else -1
                medA = np.percentile(l, 50) if len(l) > 0 else -1
                p75A = np.percentile(l, 75) if len(l) > 0 else -1
                maxA = np.percentile(l, 90) if len(l) > 0 else -1
                s += '{}{} {} {} {} {}'.format(' ' if len(s) > 0 else '',
                                         minA, p25A,  medA, p75A, maxA)
            return s
    for processedResult in processedResultList:
        angVel = processedResult.angularVelocityDict
        for t in angVel:
            t_used = t
            if not useRealTimestamp:
                t_used -= processedResult.startOffsetInSecond + processedResult.skiptime
            segmentId = math.floor(t_used/segmentSize)
            if segmentId not in results:
                results[segmentId] = AngVelValues()
            (q, w) = angVel[t]
            r = results[segmentId]
            r.angVelNorm.append(w.Norm())
            r.horizontalAngVel.append(abs(w.z))
            r.verticalAngVel.append(
                (Q.Vector(w.x, w.y, 0)).Norm())
            r.yawAngVel.append(abs(q.Rotation(Q.Vector(0, 0, 1)) * w))
            r.pitchAngVel.append(abs(q.Rotation(Q.Vector(0, 1, 0)) * w))
            r.rollAngVel.append(abs(q.Rotation(Q.Vector(1, 0, 0)) * w))
    for segId in results:
        r = results[segId]
        r.angVelNorm = sorted(r.angVelNorm)
        r.verticalAngVel = sorted(r.verticalAngVel)
        r.horizontalAngVel = sorted(r.horizontalAngVel)
        r.yawAngVel = sorted(r.yawAngVel)
        r.pitchAngVel = sorted(r.pitchAngVel)
        r.rollAngVel = sorted(r.rollAngVel)
    with open(filePath, 'w') as o:
        angVelTypeList = ['angVelNorm', 'verticalAngVel', 'horizontalAngVel',
                          'yawAngVel', 'pitchAngVel', 'rollAngVel']
        colName = 'segId'
        for angVelName in angVelTypeList:
            colName += ' min{0} 25{0} med{0} 75{0} max{0}'.format(
                angVelName[0].upper() + angVelName[1:])
        colName += '\n'
        o.write(colName)
        firstSegId = min(results.keys())
        for segId in results:
            r = results[segId]
            o.write('{} {}\n'.format(segId - firstSegId,
                                     r.GetMinMedMaxString(angVelTypeList)))


class AggregatedResults(object):
    """Contains aggregated results (i.e. results of results)."""

    def __init__(self):
        """Generate an aggregate with nothing.

        To fill it use the __add__ function.
        """
        self.aggPositionMatrix = None
        self.aggVisionMatrix = None
        self.processedResultList = list()
        self.minStartTime = sys.maxsize
        self.maxEndTime = 0
        self.maxOrthodromicDistance = None
        self.step = None

    def __add__(self, processedResult):
        """Add results to the aggregator."""
        if isinstance(processedResult, ResultContainer):
            processedResult = processedResult.GetProcessedResult()
        self.processedResultList.append(processedResult)
        self.minStartTime = min(self.minStartTime,
                                min(processedResult.quaternions.keys())  # +
                                # processedResult.startOffsetInSecond +
                                # processedResult.skiptime
                                )
        self.maxEndTime = max(self.minStartTime,
                              max(processedResult.quaternions.keys())  # +
                              # processedResult.startOffsetInSecond +
                              # processedResult.skiptime
                              )
        if self.maxOrthodromicDistance is None:
            self.maxOrthodromicDistance = \
                processedResult.maxOrthodromicDistance.copy()
        else:
            for segSize in processedResult.maxOrthodromicDistance:
                if segSize not in self.maxOrthodromicDistance:
                    print('ERR: cannot aggregate if do not '
                          'have the same segSize')
                    exit(1)
                else:
                    self.maxOrthodromicDistance[segSize] += \
                        processedResult.maxOrthodromicDistance[segSize]
        if self.aggPositionMatrix is None:
            self.aggPositionMatrix = processedResult.positionMatrix.copy()
        else:
            self.aggPositionMatrix += processedResult.positionMatrix
        if self.aggVisionMatrix is None:
            self.aggVisionMatrix = processedResult.visionMatrix.copy()
        else:
            self.aggVisionMatrix += processedResult.visionMatrix
        if self.step is None:
            self.step = processedResult.step
        if self.step != processedResult.step:
            print('ERR: cannot aggregate if do not have the same step')
            exit(1)
        return self

    def Normalize(self):
        """Normalize the values."""
        self.aggPositionMatrix /= self.aggPositionMatrix.sum()

    def StoreOrthodromicDistance(self, filePath):
        """Store the orthodrimic distance CDFs to a file."""
        with open(filePath, 'w') as o:
            o.write('cdf')
            for segSize in self.maxOrthodromicDistance:
                o.write(' {}s'.format(segSize))
                self.maxOrthodromicDistance[segSize].sort()
            o.write('\n')
            for r in range(0, 101):
                o.write('{}'.format(r))
                for segSize in self.maxOrthodromicDistance:
                    val = np.percentile(self.maxOrthodromicDistance[segSize],
                                        r) \
                        if len(self.maxOrthodromicDistance[segSize]) > 0 \
                        else -1
                    o.write(' {}'.format(val))
                o.write('\n')

    def StoreAngularVelocity(self, filePath):
        """Store angular velocity cdf to file."""
        StoreAngularVelocity(self.processedResultList, filePath, True)

    def StoreAngularVelocityPerSegment(self, segmentSize, filePath,
                                       useRealTimestamp=True):
        """Store angular velocity per segment to file.

        :param segmentSize: segment size in second
        :param useRealTimestamp: if set to True will use real timestamp,
        otherwise timestamp relatif to the beginning of the test
        """
        StoreAngularVelocityPerSegment(self.processedResultList,
                                       segmentSize, filePath, useRealTimestamp)

    def StorePositions(self, filePath, vmax=None):
        """Store the position matrix image in a file."""
        if vmax is not None:
            plt.matshow(self.aggPositionMatrix, vmin=0, vmax=vmax, cmap='hot'
                        )
        else:
            plt.matshow(self.aggPositionMatrix, cmap='hot')
        plt.savefig('{}.pdf'.format(filePath), bbox_inches='tight')
        plt.close()
        plt.matshow(self.aggPositionMatrix, cmap='hot')
        plt.savefig('{}_2.pdf'.format(filePath), bbox_inches='tight')
        plt.close()
        with open('{}_pos.txt'.format(filePath), 'w') as o:
            o.write('i j value\n')
            height, width = self.aggPositionMatrix.shape
            for i in range(0, width):
                for j in range(0, height):
                    o.write('{} {} {}\n'.format(
                        i, j, self.aggPositionMatrix[j, i]
                    ))

    def StoreVision(self, filePath):
        """Store the position matrix image in a file."""
        plt.matshow(self.aggVisionMatrix, cmap='hot')
        plt.savefig('{}.pdf'.format(filePath), bbox_inches='tight')
        plt.close()
        with open('{}_vision.txt'.format(filePath), 'w') as o:
            o.write('i j value\n')
            height, width = self.aggVisionMatrix.shape
            for i in range(0, width):
                for j in range(0, height):
                    o.write('{} {} {}\n'.format(
                        i, j, self.aggVisionMatrix[j, i]
                    ))

    def WriteVideo(self, outputPath, fps, segmentSize, width, height):
        """Generate a video of the average position in time."""
        with FFmpeg.VideoWrite(outputPath,
                               width=width,
                               height=height,
                               fps=fps) as vo:
            posMatList = list()
            vmax = 0
            for timestamp in np.arange(self.minStartTime,
                                       self.maxEndTime,
                                       1/fps):
                startTime = timestamp
                endTime = timestamp + segmentSize
                posMat = np.zeros(self.aggPositionMatrix.shape)
                posMatList.append((startTime, endTime, posMat))

            for result in self.processedResultList:
                for t in result.filteredQuaternions.keys():
                    for (startTime, endTime, posMat) in posMatList:
                        t_real = t # + result.startOffsetInSecond + \
                                 # result.skiptime
                        if t_real >= startTime and t_real <= endTime:
                            h, w = posMat.shape
                            q = result.filteredQuaternions[t]
                            v = q.Rotation(ORIGINAL_POSITION)
                            theta, phi = v.ToPolar()
                            i = int(w*(theta + math.pi)/(2*math.pi))
                            j = int(h*phi/math.pi)
                            posMat[j, i] += 1
            for (startTime, endTime, posMat) in posMatList:
                if endTime <= self.maxEndTime:
                    sumPos = posMat.sum()
                    if sumPos > 0:
                        posMat /= sumPos
                    vmax = max(vmax, posMat.max())

            for (startTime, endTime, posMat) in posMatList:
                plt.matshow(posMat, cmap='hot', vmax=vmax, vmin=0)
                buffer_ = io.BytesIO()
                plt.axis('off')
                plt.title('From {:6.2f} s to {:6.2f} s'.format(startTime,
                                                               endTime))
                plt.colorbar()
                plt.savefig(buffer_, format = "png",
                            bbox_inches='tight') # ,
                            # pad_inches = 0)
                buffer_.seek(0)
                image = PIL.Image.open(buffer_)
                image.load()
                buffer_.close()
                plt.close()
                vo.AddPicture(image)
                plt.close()

    def WriteVideoVision(self, outputPath, fps, segmentSize, widthVideo,
                         heightVideo, widthEqui, heightEqui,
                         horizontalFoVAngle, verticalFoVAngle):
        """Generate a video of the average position in time."""
        with FFmpeg.VideoWrite(outputPath,
                               width=widthVideo,
                               height=heightVideo,
                               fps=fps) as vo:
            posMatList = list()
            vmax = 0
            for timestamp in np.arange(self.minStartTime,
                                       self.maxEndTime,#-segmentSize,
                                       1/fps):
                startTime = timestamp
                endTime = timestamp + segmentSize
                posMat = np.zeros((heightEqui, widthEqui))
                posMatList.append((startTime, endTime, posMat))

            for result in self.processedResultList:
                for (startTime, endTime, posMat) in posMatList:
                    h, w = posMat.shape
                    d = dict((t, q) for (t, q) in result.filteredQuaternions.items()
                             if t >= startTime and t < endTime)
                    ans = Q.ComputeVision(d, w, h,
                                          horizontalFoVAngle, verticalFoVAngle)
                    for i in range(len(ans)):
                        for j in range(len(ans[i])):
                            posMat[j, i] += ans[i][j]
            for (startTime, endTime, posMat) in posMatList:
                sumPos = posMat.sum()
                if sumPos > 0:
                    posMat /= sumPos
                vmax = max(vmax, posMat.max())

            for (startTime, endTime, posMat) in posMatList:
                plt.matshow(posMat, cmap='hot', vmax=vmax, vmin=0)
                buffer_ = io.BytesIO()
                plt.axis('off')
                plt.title('From {:6.2f} s to {:6.2f} s'.format(startTime,
                                                               endTime))
                plt.colorbar()
                plt.savefig(buffer_, format = "png",
                            bbox_inches='tight',
                            pad_inches = 0)
                buffer_.seek(0)
                image = PIL.Image.open(buffer_)
                image.load()
                buffer_.close()
                plt.close()
                vo.AddPicture(image)
                plt.close()

class ProcessedResult(object):
    """Contains the quaternions and timestamp information of a result."""

    def __init__(self, resultPath, skiptime=10, step=0.03):
        """Get the results from the result file.

        :param skiptime: time in second to skip
        :param step: step in second for the filtering
        """
        self.step = step
        self.skiptime = skiptime
        self.quaternions = dict()
        self.filteredQuaternions = dict()
        self.frameIds = dict()
        self.angularVelocityDict = dict()
        self.maxOrthodromicDistance = dict()  # key: seg size for moving window
        self.positionMatrix = np.zeros((1, 1))
        self.visionMatrix = np.zeros((1, 1))
        firstTimestamp = None
        isSkiping = True
        pathToOsvrClientIni = '{}.ini'.format(os.path.dirname(resultPath))
        self.__GetStartOffset(pathToOsvrClientIni)

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
                    timestamp += self.startOffsetInSecond + self.skiptime
                    q = Q.Quaternion(w=float(values[2]),
                                      v=Q.Vector(x=float(values[3]),
                                                 y=float(values[4]),
                                                 z=float(values[5])
                                                 )
                                     )
                    # print('x\'=',q.Rotation(Q.Vector(1, 0, 0)),'y\'=',q.Rotation(Q.Vector(0, 1, 0)),'z\'=',q.Rotation(Q.Vector(0, 0, 1)))
                    q.Normalize()
                    frameId = int(values[1])
                    self.frameIds[timestamp] = frameId
                    self.quaternions[timestamp] = q
        # print(resultPath, max(self.quaternions.keys())
        #                   if len(self.quaternions.keys()) > 0 else -1,
        #                   self.startOffsetInSecond)
        self.__filterQuaternion()

    def __radd__(self, other):
        """To be able to generate an AggregatedResults from sum()."""
        if other is 0:
            return AggregatedResults() + self
        else:
            print('Error')
            exit(3)

    def __GetStartOffset(self, pathToOsvrClientIni):
        """Get the start offset in seconds from the config file of the test."""
        configParser = configparser.ConfigParser()
        configParser.read(pathToOsvrClientIni)
        videoConfigSection = configParser['Config']['textureConfig']
        self.startOffsetInSecond = \
            float(configParser[videoConfigSection]['startOffsetInSecond'])

    def ComputeAngularVelocity(self):
        """Compute the angular velocity."""
        qList = list()
        tList = list()
        for t in self.filteredQuaternions:
            q = self.filteredQuaternions[t]
            qList.append(q)
            tList.append(t)
            if len(qList) == 2:
                q1 = qList[0]
                q2 = qList[1]
                # q3 = qList[2]

                # velocity = (Q.Quaternion.Distance(q1, q2) +
                #             Q.Quaternion.Distance(q2, q3))/2
                # velocity /= (tList[2]-tList[0])
                velocity = Q.Quaternion.AverageAngularVelocity(q1,
                                                               q2,
                                                               tList[1]-tList[0])
                self.angularVelocityDict[tList[0] + (tList[1]-tList[0])/2] = \
                    (q2, velocity)

                qList = qList[1:]
                tList = tList[1:]
        self.__filterVelocity()

    def ComputeMaxOrthodromicDistances(self, segSizeList):
        """Compute the max ortho distance on moving windows.

        :param segSizeList: the list of segment size to use
        """
        # tmpDict = dict()  # key: timestamp, values: dict key: seg size,
        # # values maxOrthoDist
        # for t in sorted(self.filteredQuaternions.keys()):
        #     tmpDict[t] = dict()
        #     for segSize in segSizeList:
        #         tmpDict[t][segSize] = 0
        #     for t2 in sorted(tmpDict.keys()):
        #         for segSize in segSizeList:
        #             if t - t2 <= segSize:
        #                 tmpDict[t2][segSize] = \
        #                     max(tmpDict[t2][segSize],
        #                         Q.Quaternion.OrthodromicDistance(
        #                             self.filteredQuaternions[t],
        #                             self.filteredQuaternions[t2]))
        # for segSize in segSizeList:
        #     self.maxOrthodromicDistance[segSize] = list()
        # for t in tmpDict:
        #     for segSize in tmpDict[t]:
        #         self.maxOrthodromicDistance[segSize].append(
        #             tmpDict[t][segSize])
        self.maxOrthodromicDistance = \
            Q.ComputeMaxOrthodromicDistances(self.filteredQuaternions,
                                             segSizeList)

    def ComputePositions(self, width=50, height=50):
        """Compute the position matrix.

        :param width: the width of the equirectangular picture generated
        :param height: the height of the equirectangular picture generated
        """
        self.positionMatrix = np.zeros((height, width))
        for t in self.filteredQuaternions:
            q = self.filteredQuaternions[t]
            v = q.Rotation(ORIGINAL_POSITION)
            theta, phi = v.ToSpherical()
            i = int(width*(theta + math.pi)/(2*math.pi))
            j = int(height*phi/math.pi)
            self.positionMatrix[j, i] += 1
        if len(self.filteredQuaternions) > 0:
            self.positionMatrix /= self.positionMatrix.sum()

    def ComputeVision(self, width=50, height=50, horizontalFoVAngle=110,
                      verticalFoVAngle=90):
        """Compute the vision matrix.

        The vision matrix is the matrix that contains the probability of vision
        of each pixel in an equirectangular projection.

        :param width: the width of the equirectangular picture generated
        :param height: the height of the equirectangular picture generated
        """
        self.visionMatrix = np.zeros((height, width))
        # #  compute viewport delimination vector in the original orientation
        # y = math.sqrt(1-math.cos(horizontalFoVAngle))
        # z = math.sqrt(1-math.cos(verticalFoVAngle))
        # a = Q.Vector(1, y, z)
        # b = Q.Vector(1, y, -z)
        # c = Q.Vector(1, -y, -z)
        # d = Q.Vector(1, -y, z)
        # # compute inward normal to the delimitation plan
        # n_ab = a ^ b
        # n_ab = n_ab/n_ab.Norm()
        # n_bc = b ^ c
        # n_bc = n_bc/n_bc.Norm()
        # n_cd = c ^ d
        # n_cd = n_cd/n_cd.Norm()
        # n_da = d ^ a
        # n_da = n_da/n_da.Norm()
        # for t in self.filteredQuaternions:
        #     q = self.filteredQuaternions[t]
        #     for i in range(0, width):
        #         for j in range(0, height):
        #             theta = (2*math.pi*i - math.pi)/width
        #             phi = math.pi*j/height
        #             # p is the direction vector of this pixel
        #             p = Q.Vector.FromSpherical(theta, phi)
        #             p_headFrame = q.Conj().Rotation(p)
        #             # test if p is inside the viewport
        #             if p_headFrame * n_ab > 0 and \
        #                     p_headFrame * n_bc > 0 and \
        #                     p_headFrame * n_cd > 0 and \
        #                     p_headFrame * n_da > 0:
        #                 self.visionMatrix[j, i] += 1
        ans = Q.ComputeVision(self.filteredQuaternions, width, height,
                              horizontalFoVAngle, verticalFoVAngle)
        for i in range(len(ans)):
            for j in range(len(ans[i])):
                self.visionMatrix[j, i] = ans[i][j]
        if len(self.filteredQuaternions) > 0:
            self.visionMatrix /= self.visionMatrix.sum()


    def __filterVelocity(self):
        """Fill the angularVelocityWindow tuple."""
        pass
        # windowedVelocity = dict()
        # maxTimestamp = max(self.angularVelocityDict.keys())
        # for t in self.angularVelocityDict:
        #     index = int(t/step)
        #     if index not in windowedVelocity:
        #         windowedVelocity[index] = list()
        #     windowedVelocity[index].append(self.angularVelocityDict[t])
        # self.angularVelocityWindow = (step, list())
        # angVel = self.angularVelocityWindow[1]
        # for index in windowedVelocity:
        #     angVel.append(
        #         sum(windowedVelocity[index])/len(windowedVelocity[index]))

    def __filterQuaternion(self):
        """Filter the quaternions."""
        step = self.step
        # self.filteredQuaternions = self.quaternions
        if len(self.quaternions.keys()) == 0:
            self.filteredQuaternions = self.quaternions
            return
        minTimestamp = self.startOffsetInSecond + self.skiptime
        maxTimestamp = max(self.quaternions.keys())
        # timestampList = sorted(self.quaternions.keys())
        # for t_mid in np.arange(0, maxTimestamp, step/2):
        #     t1 = None
        #     t2 = None
        #     for t in timestampList:
        #         if t < t_mid-step:
        #             timestampList.pop(0)
        #         elif t <= t_mid and t >= t_mid-step:
        #             t1 = t
        #             timestampList.pop(0)
        #         elif t > t_mid and t <= t_mid+step:
        #             t2 = t
        #             # timestampList.pop(0)
        #             break
        #         elif t > t_mid+step/2:
        #             break
        #     q_mid = None
        #     if t1 is None and t2 is not None:
        #         q_mid = self.quaternions[t2]
        #     elif t2 is None and t1 is not None:
        #         q_mid = self.quaternions[t1]
        #     elif t1 is not None and t2 is not None:
        #         k = (t_mid - t1)/(t2 - t1)
        #         q_mid = Q.Quaternion.SLERP(self.quaternions[t1],
        #                                    self.quaternions[t2],
        #                                    k)
        #     if q_mid is not None:
        #         self.filteredQuaternions[t_mid] = q_mid
        timestampList1 = sorted(self.quaternions.keys())
        timestampList2 = timestampList1.copy()
        t1 = timestampList1[0]
        t2 = timestampList2[0]
        for t_mid in np.arange(minTimestamp, maxTimestamp, step/2):
            while len(timestampList1) > 0 and timestampList1[0] <= t_mid:
                t1 = timestampList1.pop(0)
            while len(timestampList2) > 0 and timestampList2[0] < t_mid:
                timestampList2.pop(0)
                t2 = timestampList2[0]
            if t1 != t2:
                k = (t_mid - t1)/(t2 - t1)
                q_mid = Q.Quaternion.SLERP(self.quaternions[t1],
                                           self.quaternions[t2],
                                           k)
                # q_mid = self.quaternions[t1]
            else:
                q_mid = self.quaternions[t1]
            q_mid.Normalize()
            self.filteredQuaternions[t_mid] = q_mid

    def StoreAngularVelocity(self, filePath):
        """Store the position matrix image in a file."""
        StoreAngularVelocity(self.angularVelocityDict, filePath, False)

    def StorePositions(self, filePath, vmax=None):
        """Store the position matrix image in a file."""
        if vmax is not None:
            plt.matshow(self.positionMatrix, vmin=0, vmax=vmax, cmap='hot'
                        )
        else:
            plt.matshow(self.positionMatrix, cmap='hot')
        plt.savefig('{}.pdf'.format(filePath), bbox_inches='tight')
        plt.close()
        plt.matshow(self.positionMatrix, cmap='hot')
        plt.savefig('{}_2.pdf'.format(filePath), bbox_inches='tight')
        plt.close()
        with open('{}_pos.txt'.format(filePath), 'w') as o:
            o.write('i j value\n')
            height, width = self.positionMatrix.shape
            for i in range(0, width):
                for j in range(0, height):
                    o.write('{} {} {}\n'.format(
                        i, j, self.positionMatrix[j, i]
                    ))


class ResultContainer(object):
    """This class contains information about a result but not the result."""

    def __init__(self, resultPath, resultId, user, videoId):
        """Init the class.

        :param resultPath: the path to the raw result file.
        """
        self.resultPath = resultPath
        self.resultId = resultId
        self.user = user
        self.videoId = videoId
        self.pathToIndividualStatistic = os.path.join(PATH_TO_STATISTIC_RESULTS,
                                                      'individual',
                                                      self.resultId)
        self.step = None
        self.processedResult = None
        self.resultContainerDumpPath = \
            '{}.dump'.format(self.pathToIndividualStatistic)
        self.resultProcessedDumpPath = \
            '{}_processed.dump'.format(self.pathToIndividualStatistic)
        self.isNew = False

    def GetProcessedResult(self, step=None):
        """Get the stored processedResult or compute it."""
        if step is None:
            step = self.step
        if self.step == step:
            if self.processedResult is None:
                pr = Load(self.resultProcessedDumpPath)
                if pr is not None:
                    self.processedResult = pr
                else:
                    self.step = 0
        elif self.step != step:
            self.step = step
            self.processedResult = None
            Store(self, self.resultContainerDumpPath)
            self.processedResult = ProcessedResult(self.resultPath,
                                                   skiptime=10,
                                                   step=step)
            self.processedResult.ComputeAngularVelocity()
            self.processedResult.ComputeMaxOrthodromicDistances([1,
                                                                 2,
                                                                 3,
                                                                 5,
                                                                 10])
            self.processedResult.ComputePositions(width=100, height=100)
            self.processedResult.ComputeVision(width=100, height=50,
                                               horizontalFoVAngle=110,
                                               verticalFoVAngle=90)
            Store(self.processedResult, self.resultProcessedDumpPath)
        return self.processedResult

    def __radd__(self, other):
        """Right hand addition."""
        return other + self.GetProcessedResult()


    @staticmethod
    def LoadResultContainer(resultPath, resultId, user, videoId):
        """Load the existing result container or create it."""
        pathToIndividualStatistic = os.path.join(PATH_TO_STATISTIC_RESULTS,
                                                 'individual',
                                                 resultId)
        resultContainerDumpPath = \
            '{}.dump'.format(pathToIndividualStatistic)
        rc = Load(resultContainerDumpPath)
        if rc is not None:
            rc.isNew = False
        else:
            rc = ResultContainer(resultPath, resultId, user, videoId)
            rc.isNew = True
        return rc


class AggregateContainer(object):
    """Contains informations about an aggregate."""

    def __init__(self, step, size):
        """Init the container."""
        self.step = step
        self.size = size
        self.isNew = False

    @staticmethod
    def Load(dumpPath, step, size):
        """Load or create the AggregateContainer."""
        ac = Load(dumpPath)
        if ac is not None and ac.step == step and ac.size == size:
            ac.isNew = False
        else:
            ac = AggregateContainer(step, size)
            ac.isNew = True
        return ac

global_statistics = None


def Load(pathToFile):
    """Load the object of exist or return None."""
    if not os.path.exists(pathToFile):
        return None
    else:
        try:
            with open(pathToFile, 'rb') as f:
                return dill.load(f)
        except:
            return None


def Store(obj, pathToFile):
    """Store the object."""
    with open(pathToFile, 'wb') as f:
        dill.dump(obj, f)


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

    def PrintProgress(self):
        """Print a progress bar."""
        ratio = 100*self.progressBar['value'] / self.progressBar['maximum']
        s = '\033[1K\r['
        n = math.floor(ratio)
        s += '=' * n
        if n < 100:
            s += '>' + '.'*(100-n-1)
        s += '] {:6.2f} %'.format(ratio)
        print(s, end='')
        sys.stdout.flush()

    def RunComputation(self, withVideo=False):
        """Do the work to compute the statistics."""
        self.progressBar = dict()
        self.doneCallback = None
        # self.resultsByIdInfo = dict()
        self.resultsContainers = dict()
        self.resultsById = dict()
        self.resultsByUser = dict()
        self.resultsByVideo = dict()
        self.resultsByAge = dict()
        self.resultsBySex = {'man':list(), 'woman': list()}
        for userId in self.userManager.userDict:
            user = self.userManager.userDict[userId]
            user.ParseFormAnswers()
            self.resultsByUser[userId] = list()
            testPathList = user.GetExistingTestPathList()
            for testPath in testPathList:
                testId = os.path.basename(testPath)
                for root, dirs, files in os.walk(testPath):
                    for videoId in dirs:
                        if 'training' not in videoId: # and 'Rollercoaster' in videoId:
                            if videoId not in self.resultsByVideo:
                                self.resultsByVideo[videoId] = list()

                            resultId = '{}_{}_{}'.format(userId,
                                                         testId,
                                                         videoId)
                            resultPath = \
                                os.path.join(root,
                                             videoId,
                                             '{}_0.txt'.format(videoId))
                            # self.resultsByIdInfo[resultId] = (resultPath,
                            #                                   userId,
                            #                                   videoId,
                            #                                   user.sex,
                            #                                   user.age)

                            self.resultsContainers[resultId] = \
                                ResultContainer.LoadResultContainer(resultPath,
                                                                    resultId,
                                                                    user,
                                                                    videoId)
                        # resultsById[resultId] = ProcessedResult(resultPath)
                        # resultsByUser[userId].append(resultsById[resultId])
                        # resultsByVideo[videoId].append(resultsById[resultId])
        self.ageStep = 10
        for age in range(0, 100, self.ageStep):
            self.resultsByAge[age] = list()
        self.progressBar['value'] = 0
        self.progressBar['maximum'] = len(self.resultsContainers) + \
                                      len(self.resultsByVideo) + \
                                      len(self.resultsByUser) + \
                                      len(self.resultsByAge) + 1
        #self.workingThread = threading.Thread(
            # target=partial(Statistics._ComputationWorkThread, self, withVideo)
            # )
        self._ComputationWorkThread(withVideo)
        self.done = False
        #self.workingThread.start()

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

    def _ComputationWorkThread(self, withVideo):
        """Thread main function that do the actual computation."""
        if not os.path.exists(PATH_TO_STATISTIC_RESULTS + '/individual'):
            os.makedirs(PATH_TO_STATISTIC_RESULTS + '/individual')
        if not os.path.exists(PATH_TO_STATISTIC_RESULTS + '/users'):
            os.makedirs(PATH_TO_STATISTIC_RESULTS + '/users')
        if not os.path.exists(PATH_TO_STATISTIC_RESULTS + '/videos'):
            os.makedirs(PATH_TO_STATISTIC_RESULTS + '/videos')
        if not os.path.exists(PATH_TO_STATISTIC_RESULTS + '/byAge'):
            os.makedirs(PATH_TO_STATISTIC_RESULTS + '/byAge')
        if not os.path.exists(PATH_TO_STATISTIC_RESULTS + '/total'):
            os.makedirs(PATH_TO_STATISTIC_RESULTS + '/total')
        vmax = 0
        step = 0.03

        self.userManager.StoreUserStats(PATH_TO_STATISTIC_RESULTS +
                                        '/total/users')

        pool = ProcessingPool()
        for resultId in self.resultsContainers:
            rc = self.resultsContainers[resultId]
            userId = rc.user.uid
            videoId = rc.videoId
            rc.user.ParseFormAnswers()
            sex = rc.user.sex
            age = rc.user.age

            self.resultsByUser[userId].append(
                rc)
            self.resultsByVideo[videoId].append(
                rc)
            self.resultsByAge[age - age % self.ageStep].append(
                rc)
            self.resultsBySex[sex].append(
                rc)
            # vmax = \
            #     max(vmax,
            #         self.resultsById[resultId].positionMatrix.max()
            #         )

        print('\r\033[2KProcess individual results')
        self.PrintProgress()
        def WorkerResults(step, rc):
            if rc.isNew:
                processedResult = rc.GetProcessedResult(step)
                processedResult.StorePositions(
                    PATH_TO_STATISTIC_RESULTS+'/individual/{}'.format(rc.resultId),
                    vmax=None
                )
                processedResult.StoreAngularVelocity(
                    PATH_TO_STATISTIC_RESULTS+'/individual/{}.txt'.format(rc.resultId)
                )
            return rc
        async_result = [
            pool.apipe(
                WorkerResults,
                step, self.resultsContainers[resultId]
                ) for resultId in self.resultsContainers
            ]
        for r in async_result:
            rc = r.get()
            self.resultsContainers[rc.resultId] = rc
            self.progressBar['value'] += 1
            self.PrintProgress()
        aggrUserResults = dict()
        aggrVideoResults = dict()
        aggrAgeResults = dict()
        print('\r\033[2KProcess results by user')
        self.PrintProgress()
        def WorkerUsers(resultsByUser, userId, step):
            aggSize = len(resultsByUser)
            if aggSize > 0:
                dumpPath = \
                    PATH_TO_STATISTIC_RESULTS+'/users/uid-{}.dump'.format(userId)
                ac = AggregateContainer.Load(dumpPath, step, aggSize)
                if ac.isNew:
                    aggResult = sum(resultsByUser)
                    aggResult.StorePositions(
                        PATH_TO_STATISTIC_RESULTS+'/users/uid-{}'.format(userId),
                        vmax=None
                    )
                    aggResult.StoreVision(
                        PATH_TO_STATISTIC_RESULTS+'/users/uid-{}_vision'.format(userId)
                    )
                    aggResult.StoreAngularVelocity(
                        PATH_TO_STATISTIC_RESULTS+'/users/uid-{}.txt'.format(userId)
                    )
                    aggResult.StoreOrthodromicDistance(
                        PATH_TO_STATISTIC_RESULTS +
                        '/users/uid-{}_orthoDist.txt'.format(userId)
                    )
                    # vmax = max(vmax,
                    #            aggResult.aggPositionMatrix.max())
                    Store(ac, dumpPath)
            return None
        async_result = [
            pool.apipe(
                WorkerUsers,
                self.resultsByUser[userId], userId, step
                ) for userId in self.resultsByUser
            ]
        for r in async_result:
            r.get()
            self.progressBar['value'] += 1
            self.PrintProgress()

        print('\r\033[2KProcess results by age')
        self.PrintProgress()
        def WorkerAge(resultsByAge, ageStep, age, step):
            aggSize = len(resultsByAge)
            if aggSize > 0:
                dumpPath = PATH_TO_STATISTIC_RESULTS+'/byAge/{}_{}.dump'.format(
                    age, age + ageStep
                    )
                ac = AggregateContainer.Load(dumpPath, step, aggSize)
                if ac.isNew:
                    aggResult = sum(resultsByAge)
                    aggResult.StorePositions(
                        PATH_TO_STATISTIC_RESULTS+'/byAge/{}_{}'.format(
                            age, age + ageStep),
                        vmax=None
                    )
                    aggResult.StoreVision(
                        PATH_TO_STATISTIC_RESULTS+'/byAge/{}_{}_vision'.format(
                            age, age + ageStep)
                    )
                    aggResult.StoreAngularVelocity(
                        PATH_TO_STATISTIC_RESULTS+'/byAge/{}_{}.txt'.format(
                            age, age + ageStep)
                    )
                    aggResult.StoreOrthodromicDistance(
                        PATH_TO_STATISTIC_RESULTS +
                        '/byAge/{}_{}_orthoDist.txt'.format(age, age + ageStep)
                    )
                    # vmax = max(vmax,
                    #            aggResult.aggPositionMatrix.max())
                    Store(ac, dumpPath)
            return None

        async_result = [
            pool.apipe(
                WorkerAge,
                self.resultsByAge[age], self.ageStep, age, step
                ) for age in self.resultsByAge
            ]
        for r in async_result:
            r.get()
            self.progressBar['value'] += 1
            self.PrintProgress()

        print('\r\033[2KProcess results by video')
        self.PrintProgress()
        def WorkerVideo(resultsByVideo, videoId, step, withVideo):
            aggSize = len(resultsByVideo)
            if aggSize > 0:
                dumpPath = PATH_TO_STATISTIC_RESULTS+'/videos/{}.dump'.format(videoId)
                ac = AggregateContainer.Load(dumpPath, step, aggSize)
                if ac.isNew:
                    aggResult = sum(resultsByVideo)
                    aggResult.StorePositions(
                        PATH_TO_STATISTIC_RESULTS+'/videos/{}'.format(videoId),
                        vmax=None
                    )
                    aggResult.StoreVision(
                        PATH_TO_STATISTIC_RESULTS+'/videos/{}_vision'.format(videoId)
                    )
                    aggResult.StoreAngularVelocity(
                        PATH_TO_STATISTIC_RESULTS+'/videos/{}.txt'.format(videoId)
                    )
                    aggResult.StoreOrthodromicDistance(
                        PATH_TO_STATISTIC_RESULTS +
                        '/videos/{}_orthoDist.txt'.format(videoId)
                    )
                    for segmentSize in [1, 2, 3]:
                        aggResult.StoreAngularVelocityPerSegment(
                            segmentSize=segmentSize,
                            filePath=PATH_TO_STATISTIC_RESULTS+'/videos/' +
                            '{}_angVelPerSegment_{}s.txt'.format(videoId,
                                                                 segmentSize)
                        )
                    if withVideo:
                        # aggResult.WriteVideo(
                        aggResult.WriteVideoVision(
                            PATH_TO_STATISTIC_RESULTS+'/videos/{}.mkv'.format(videoId),
                            fps=5,
                            segmentSize=1/5,
                            widthVideo=960,
                            heightVideo=480,
                            widthEqui=100,
                            heightEqui=50,
                            horizontalFoVAngle=110,
                            verticalFoVAngle=90
                        )
                    # vmax = max(vmax,
                    #            aggResult.aggPositionMatrix.max())
                    Store(ac, dumpPath)
            return None

        async_result = [
            pool.apipe(
                WorkerVideo,
                self.resultsByVideo[videoId], videoId, step, withVideo
                ) for videoId in self.resultsByVideo
            ]
        for r in async_result:
            r.get()
            self.progressBar['value'] += 1
            self.PrintProgress()

        print('\r\033[2KProcess results total')
        self.PrintProgress()
        aggSize = len(self.resultsContainers)
        dumpPath = PATH_TO_STATISTIC_RESULTS+'/total/{}.dump'.format('total')
        ac = AggregateContainer.Load(dumpPath, step, aggSize)
        if ac.isNew:
            aggTotal = sum(self.resultsContainers.values())
            aggTotal.StorePositions(
                PATH_TO_STATISTIC_RESULTS+'/total/{}'.format('total'),
                vmax=None
            )
            aggTotal.StoreVision(
                PATH_TO_STATISTIC_RESULTS+'/total/{}_vision'.format('total')
            )
            aggTotal.StoreAngularVelocity(
                PATH_TO_STATISTIC_RESULTS+'/total/{}.txt'.format('total')
            )
            aggTotal.StoreOrthodromicDistance(
                PATH_TO_STATISTIC_RESULTS+'/total/{}.txt'.format('orthoDist')
            )
            for segmentSize in [1, 2, 3]:
                aggTotal.StoreAngularVelocityPerSegment(
                    segmentSize=segmentSize,
                    filePath=PATH_TO_STATISTIC_RESULTS+'/total/' +
                    '{}_angVelPerSegment_{}s.txt'.format('total',
                                                         segmentSize),
                    useRealTimestamp=False
                )
            Store(ac, dumpPath)
        self.progressBar['value'] += 1
        self.PrintProgress()

        # def worker(videoId, processedResult):
        #     processedResult.WriteVideo(
        #         'results/statistics/videos/{}.mkv'.format(videoId),
        #         fps=5,
        #         segmentSize=1/5,
        #         width=480,
        #         height=480
        #     )
        #     return None
        # async_result = [
        #     pool.apipe(
        #         worker,
        #         videoId, aggrVideoResults[videoId]
        #         ) for videoId in aggrVideoResults
        #     ]
        # for r in async_result:
        #     r.get()
        #     self.progressBar['value'] += 1
        # pool.close()
        # pool.join()
        # del self.workingThread
        self.done = True
        self.progressBar = None
        self.workingThread = None
        if self.doneCallback is not None:
            self.doneCallback()
