"""Some tools to manipulate quaternions.

Author: Xavier Corbillon
IMT Atlantique
"""

import math


class Vector(object):
    """This class represent a vector from R3."""

    def __init__(self, x=0, y=0, z=0):
        """Init function."""
        self.x = x
        self.y = y
        self.z = z

    def __mul__(self, s_v):
        """Scalar muliplication or ScalarProduct if s is a vector."""
        if isinstance(s_v, Vector):
            return Vector.ScalarProduct(s_v, self)
        elif isinstance(s_v, Quaternion):
            return Quaternion(v=self)*s_v
        else:
            s = s_v
            return Vector(x=s*self.x, y=s*self.y, z=s*self.z)

    __rmul__ = __mul__

    def __truediv__(self, s):
        """Divide by a scalar."""
        return Vector(x=self.x/s, y=self.y/s, z=self.z/s)

    def __xor__(self, v):
        """Vectorial product (self is the left-hand vector)."""
        return Vector.VectorProduct(self, v)

    def __add__(self, v):
        """Addition of two vectors."""
        return Vector(x=self.x + v.x,
                      y=self.y + v.y,
                      z=self.z + v.z)

    def __radd__(self, other):
        """Reverse add."""
        if other is 0:
            return self
        else:
            self.__add__(other)  # add is commutatif

    def __sub__(self, v):
        """Substraction of two vectors."""
        return Vector(x=self.x - v.x,
                      y=self.y - v.y,
                      z=self.z - v.z)

    def __str__(self):
        """To string function (x, y, z)."""
        return '({}, {}, {})'.format(self.x, self.y, self.z)

    def Norm(self):
        """Return the norm of the vector."""
        return math.sqrt(Vector.ScalarProduct(self, self))

    def __neg__(self):
        """Return -v."""
        return Vector(x=-self.x,
                      y=-self.y,
                      z=-self.z)

    def ToPolar(self):
        """Return theta and phi from the polar coordinate of the vector.

        theta = azimuth
        phi = inclination
        """
        theta = math.atan2(self.y, self.x)
        phi = math.acos(self.z/self.Norm())
        return (theta, phi)

    @staticmethod
    def ScalarProduct(v1, v2):
        """Scalar product of two vector."""
        return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z

    @staticmethod
    def VectorProduct(v1, v2):
        """Vector product of two vector."""
        return Vector(x=v1.y*v2.z-v1.z*v2.y,
                      y=v1.z*v2.x-v1.x*v2.z,
                      z=v1.x*v2.y-v1.y*v2.x)


class Quaternion(object):
    """This class represent a quaternion."""

    def __init__(self, w=0, v=Vector(x=0, y=0, z=0)):
        """Init function with default values [0, 0, 0, 0]."""
        self.w = w
        self.v = v
        self._isNormalized = False

    def Dot(self, other):
        """Dot product of two quaternions."""
        return self.w * other.w + Vector.ScalarProduct(self.v, other.v)

    def Norm(self):
        """Return the norm of the quaternion."""
        if self.IsNormalized():
            return 1
        else:
            return math.sqrt(self.w*self.w + self.v*self.v)

    def Normalize(self):
        """Normalize this vector."""
        if not self.IsNormalized():
            tmp = self / self.Norm()
            self.w = tmp.w
            self.v = tmp.v
            self._isNormalized = True
        return self

    def __mul__(self, other):
        """Multiplication operator.

        other is a Quaternion, an integer, a float or a long.
        """
        if not isinstance(other, Quaternion):
            if isinstance(other, Vector):
                other = Quaternion(v=other)
            else:
                other = Quaternion(w=other)
        ans = Quaternion()
        ans.w = (self.w * other.w) - (self.v * other.v)
        ans.v = (self.w * other.v) + (other.w * self.v) + (self.v ^ other.v)
        return ans

    __rmul__ = __mul__

    def __truediv__(self, s):
        """Divide by a scalar."""
        return Quaternion(w=self.w/s,
                          v=self.v/s)

    def __add__(self, other):
        """Addition of two quaternions."""
        if isinstance(other, Vector):
            other = Quaternion(v=other)
        if not isinstance(other, Quaternion):
            other = Quaternion(w=other)
        return Quaternion(w=self.w + other.w,
                          v=self.v + other.v)

    def __radd__(self, other):
        """Reverse add."""
        if other is 0:
            return self
        else:
            self.__add__(other)  # add is commutatif

    def __sub__(self, other):
        """Substraction of two quaternions."""
        if not isinstance(other, Quaternion):
            other = Quaternion(w=other)
        return Quaternion(w=self.w - other.w,
                          v=self.v - other.v)

    __rsub__ = __sub__

    def __neg__(self):
        """Return -v."""
        return Quaternion(w=-self.w, v=-self.v)

    def __str__(self):
        """Return a string 'w + x * i + y * j + k * z'."""
        return '{} + {} * i + {} * j + {} * k'.format(self.w,
                                                      self.v.x,
                                                      self.v.y,
                                                      self.v.z)

    def IsPur(self):
        """Return true if the quaternion is pur (i.e. w == 0)."""
        return self.w == 0

    def IsNormalized(self):
        """Return true if the quaternion is normalized."""
        return self._isNormalized

    def Conj(self):
        """Return the conjugate of self."""
        return Quaternion(w=self.w, v=-self.v)

    def Inv(self):
        """Return the inverse of this quaternion."""
        if self.IsNormalized():
            return self.Conj()
        else:
            return self.Conj()/self.Norm()**2

    def Rotation(self, v):
        """Return a vector v' result of the rotation the vector v by self."""
        self.Normalize()
        return (self*v*self.Conj())

    def __pow__(self, k):
        """Define the power of a quaternion with k a real."""
        return Quaternion.Exp(k * Quaternion.Log(self))

    @staticmethod
    def QuaternionFromAngleAxis(theta, u):
        """Generate a quaternion from an angle theta and a direction."""
        w = math.cos(theta/2)
        v = math.sin(theta/2)*u/u.Norm()
        return Quaternion(w=w, v=v)

    @staticmethod
    def Exp(q):
        """Exponential function for a quaternion."""
        expW = math.exp(q.w)
        w = math.cos(q.v.Norm())*expW
        # if q.v.Norm() == 0 then q is a real number
        v = math.sin(q.v.Norm())*q.v/q.v.Norm() if q.v.Norm() != 0 else q.v
        return Quaternion(w=w, v=v)

    @staticmethod
    def Log(q):
        """Logarithm of a quaternion."""
        w = math.log(q.Norm())
        # if q.v.Norm() == 0 then q is a real number
        v = math.acos(q.w/q.Norm())*q.v/q.v.Norm() if q.v.Norm() != 0 else q.v
        return Quaternion(w=w, v=v)

    @staticmethod
    def Distance(q1, q2):
        """Distance between two quaternions."""
        return (q2-q1).Norm()

    @staticmethod
    def SLERP(q1, q2, k):
        """Compute the slerp interpolation of q1, q2 with a weight k."""
        if q1.Dot(q2) < 0:
            q2 = -q2
        return q1 * (q1.Inv() * q2)**k


def AverageAngularVelocity(q1, q2, deltaT):
    """Compute the average angular velocity.

    Average angular velocity to move from q1 to q2 during deltaT.
    q1 is the old value
    q2 is the new value
    """
    if q1.Dot(q2) < 0:
        q2 = -q2
    if not q1.IsPur():
        q1 = q1.Rotation(Vector(1, 0, 0))
    if not q2.IsPur():
        q2 = q2.Rotation(Vector(1, 0, 0))
    # deltaQ = q2 - q1
    deltaQ = q2*q1.Inv()
    # W = ((deltaQ*2)/deltaT)*q2.Inv()
    W = (2 * Quaternion.Exp(Quaternion.Log(deltaQ)) / deltaT)*q1.Inv()
    return W
