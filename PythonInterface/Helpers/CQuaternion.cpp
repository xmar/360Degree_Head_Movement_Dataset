#include <boost/python.hpp>
#include <iostream>
#include <map>
#include <vector>

typedef  double SCALAR;

class Vector
{
public:
  Vector(void): m_x(0), m_y(0), m_z(0) {}
  Vector(SCALAR x, SCALAR y, SCALAR z): m_x(x), m_y(y), m_z(z) {}

  SCALAR DotProduct(const Vector& v) const {return m_x*v.m_x + m_y*v.m_y + m_z*v.m_z;}
  SCALAR Norm(void) const {return std::sqrt(DotProduct(*this));}

  Vector operator+(const Vector& v) const {return Vector(m_x+v.m_x, m_y+v.m_y, m_z+v.m_z);}
  Vector operator-(void) const {return Vector(-m_x, -m_y, -m_z);}
  Vector operator-(const Vector& v) const {return Vector(m_x-v.m_x, m_y-v.m_y, m_z-v.m_z);}
  Vector operator*(const SCALAR& s) const {return Vector(s*m_x, s*m_y, s*m_z);}
  Vector operator/(const SCALAR& s) const {return Vector(m_x/s, m_y/s, m_z/s);}
  //dot product
  SCALAR operator*(const Vector& v) const {return DotProduct(v);}
  //Vector product
  Vector operator^(const Vector& v) const {return Vector(m_y*v.m_z-m_z*v.m_y,
                    m_z*v.m_x-m_x*v.m_z,
                    m_x*v.m_y-m_y*v.m_x);}

  Vector VectorProduct(const Vector& v) const {return (*this)^v; }

  std::ostream& operator<<(std::ostream& o) const
  {
    o << "(" << m_x << ", "  << m_y << ", " <<  m_z << ")";
    return o;
  }

  auto ToPolar(void) const
  {
    auto theta = std::atan2(m_y, m_x);
    auto phi = std::acos(m_z/Norm());
    return boost::python::make_tuple(theta, phi);
  }

  auto GetX(void) const {return m_x;}
  auto GetY(void) const {return m_y;}
  auto GetZ(void) const {return m_z;}
private:
  SCALAR m_x;
  SCALAR m_y;
  SCALAR m_z;
};
std::ostream& operator<<(std::ostream& o, const Vector& v) {return v.operator<<(o);}
Vector operator*(const SCALAR& s, const Vector& v) {return v * s;}

struct not_unit_quaternion_exception : std::exception
{
  char const* what() const throw() { return "Rotation require unit quaternion"; }
};

void translate(not_unit_quaternion_exception const& e)
{
    // Use the Python 'C' API to set up an exception object
    PyErr_SetString(PyExc_RuntimeError, e.what());
}

struct vector_pickle_suite : boost::python::pickle_suite
{
  static
  boost::python::tuple
  getinitargs(Vector const& v)
  {
      return boost::python::make_tuple(v.GetX(), v.GetY(), v.GetZ());
  }
};

class Quaternion
{
public:
  Quaternion(void): m_w(0), m_v(), m_isNormalized(false) {}
  Quaternion(const SCALAR& w, const Vector& v): m_w(w), m_v(v), m_isNormalized(false) {}
  Quaternion(const SCALAR& w): m_w(w), m_v(), m_isNormalized(false) {}
  Quaternion(const Vector& v): m_w(0), m_v(v), m_isNormalized(false) {}

  SCALAR DotProduct(const Quaternion& q) const {return m_w*q.m_w + m_v.DotProduct(q.m_v);}
  SCALAR Norm(void) const {return std::sqrt(DotProduct(*this));}

  Quaternion operator+(const Quaternion& q) const
  {
    return Quaternion(m_w+q.m_w, m_v+q.m_v);
  }
  Quaternion operator-(const Quaternion& q) const
  {
    return Quaternion(m_w-q.m_w, m_v-q.m_v);
  }
  Quaternion operator-(void) const
  {
    return Quaternion(-m_w, -m_v);
  }
  Quaternion operator*(const Quaternion& q) const
  {
    return Quaternion((m_w*q.m_w) - (m_v*q.m_v),
                      (m_w*q.m_v) + (q.m_w*m_v) + (m_v ^ q.m_v));
  }
  Quaternion operator*(SCALAR s) const
  {
    return Quaternion(m_w*s, m_v*s);
  }
  Quaternion operator/(const SCALAR& s) const
  {
    return Quaternion(m_w/s, m_v/s);
  }

  std::ostream& operator<<(std::ostream& o) const
  {
    o << m_w << " + "  << m_v.GetX() << " i + " << m_v.GetY() << " j + " << m_v.GetZ() << " k ";
    return o;
  }

  Quaternion& operator=(const Quaternion& q)
  {
    m_w = q.m_w;
    m_v = q.m_v;
    return *this;
  }

  void Normalize(void)
  {
    if (!m_isNormalized)
    {
      *this = *this / Norm();
      m_isNormalized = true;
    }
  }

  auto GetW(void) const {return m_w;}
  auto GetV(void) const {return m_v;}

  bool IsPur(void) const {return m_w == 0;}
  Quaternion Conj(void) const {return Quaternion(m_w, -m_v);}
  Quaternion Inv(void) const {return m_isNormalized ? Conj() : Conj()/std::pow(Norm(),2);}
  Vector Rotation(const Vector& v) const
  {
    if (m_isNormalized)
    {
      return ((*this)*v*(this->Conj())).GetV();
    }
    else
    {
      throw not_unit_quaternion_exception();
    }
  }

  static Quaternion Exp(const Quaternion& q)
  {
    auto expW = std::exp(q.m_w);
    auto normV = q.m_v.Norm();
    return Quaternion(std::cos(normV)*expW,
                      normV != 0 ? std::sin(normV) * (q.m_v / normV) : q.m_v
                    );
  }
  static Quaternion Log(const Quaternion& q)
  {
    auto expW = std::log(q.Norm());
    auto normV = q.m_v.Norm();
    auto normQ = q.Norm();
    return Quaternion(std::log(normQ),
                      normV != 0 && normQ != 0 ? std::acos(q.m_w/normQ)*(q.m_v/normV) : q.m_v
                    );
  }
  static SCALAR Distance(const Quaternion& q1, const Quaternion& q2)
  {
    return (q2-q1).Norm();
  }

  static SCALAR OrthodromicDistance(const Quaternion& q1, const Quaternion& q2)
  {
    auto origine = Vector(1, 0, 0);
    Quaternion p1 = q1.Rotation(origine);
    Quaternion p2 = q2.Rotation(origine);
    auto p = p1 * p2;
    // p1 and p2 are pur so -p.m_w is the dot product and p.m_v is the vector product of p1 and p2
    return std::atan2(p.m_v.Norm(), -p.m_w);
  }

  static Quaternion pow(const Quaternion& q, const SCALAR& k)
  {
    return Quaternion::Exp(Quaternion::Log(q) * k);
  }

  static Quaternion SLERP(const Quaternion& q1, const Quaternion& q2, const SCALAR& k)
  {
    if (q1.DotProduct(q2) < 0)
    {
      return q1 * Quaternion::pow(q1.Inv() * (-q2), k);
    }
    else
    {
      return q1 * Quaternion::pow(q1.Inv() * q2, k);
    }
  }

  static  Quaternion QuaternionFromAngleAxis(const SCALAR& theta, const Vector& u)
  {
    return Quaternion(std::cos(theta/2), std::sin(theta/2)*(u/u.Norm()));
  }

  static Vector AverageAngularVelocity(Quaternion q1, Quaternion q2, const SCALAR& deltaT)
  {
    if (q1.DotProduct(q2) < 0)
    {
      q2 = -q2;
    }
    if (!q1.IsPur())
    {
      if (!q1.m_isNormalized)
      {
        q1.Normalize();
      }
      q1 = q1.Rotation(Vector(1, 0, 0));
    }
    if (!q2.IsPur())
    {
      if (!q2.m_isNormalized)
      {
        q2.Normalize();
      }
      q2 = q2.Rotation(Vector(1, 0, 0));
    }
    auto deltaQ = q2 - q1;
    auto W = (deltaQ * (2.0 / deltaT))*q1.Inv();
    return W.m_v;
  }

private:
  SCALAR m_w;
  Vector m_v;
  bool m_isNormalized;
};

struct quaternion_pickle_suite : boost::python::pickle_suite
{
  static
  boost::python::tuple
  getinitargs(Quaternion const& q)
  {
      return boost::python::make_tuple(q.GetW(), q.GetV());
  }
};

std::ostream& operator<<(std::ostream& o, const Quaternion& q) {return q.operator<<(o);}
Quaternion operator+(const SCALAR& s, const Quaternion& q) {return Quaternion(s) + q;}
Quaternion operator-(const SCALAR& s, const Quaternion& q) {return Quaternion(s) - q;}
Quaternion operator+(const Vector& v, const Quaternion& q) {return Quaternion(v) + q;}
Quaternion operator-(const Vector& v, const Quaternion& q) {return Quaternion(v) - q;}
Quaternion operator+(const Vector& v, const SCALAR& s) {return Quaternion(v) + Quaternion(s);}
Quaternion operator-(const Vector& v, const SCALAR& s) {return Quaternion(v) - Quaternion(s);}
Quaternion operator*(const SCALAR& s, const Quaternion& q) {return q * s;}
Quaternion operator*(const Vector& v, const Quaternion& q) {return Quaternion(v) * q;}
Quaternion operator*(const Quaternion& q, const Vector& v) {return q * Quaternion(v);}

Quaternion pow(const Quaternion& q, const SCALAR& k)
{
  return Quaternion::pow(q, k);
}

boost::python::dict ComputeMaxOrthodromicDistances(boost::python::dict& filteredQuaternions,
        boost::python::list& segSizeList)
{
  std::map<SCALAR, std::map<SCALAR, SCALAR>> ans;
  SCALAR maxSegSize = 0;
  for (size_t i = 0; i < boost::python::len(segSizeList); ++i)
  {
    SCALAR segSize = boost::python::extract<SCALAR>(segSizeList[i]);
    ans[segSize] = std::map<SCALAR, SCALAR>();
    maxSegSize = std::max(maxSegSize, segSize);
  }
  boost::python::list keys = filteredQuaternions.keys();
  std::vector<SCALAR> keysToCheck;
  SCALAR maxTimestamp = 0;
  for (size_t i = 0; i < boost::python::len(keys); ++i)
  {
    SCALAR t = boost::python::extract<SCALAR>(keys[i]);
    maxTimestamp = std::max(maxTimestamp, t);
  }
  for (size_t i = 0; i < boost::python::len(keys); ++i)
  {
    SCALAR t2 = boost::python::extract<SCALAR>(keys[i]);
    Quaternion q2 = boost::python::extract<Quaternion>(filteredQuaternions[keys[i]]);
    while(!keysToCheck.empty() && t2-keysToCheck[0] > maxSegSize)
    {
      keysToCheck.erase(keysToCheck.begin());
    }
    for (auto t1: keysToCheck)
    {
      for (size_t i = 0; i < boost::python::len(segSizeList); ++i)
      {
        SCALAR segSize = boost::python::extract<SCALAR>(segSizeList[i]);
        if (maxTimestamp - t2 >= segSize)
        {
          ans[segSize][t2] = 0;
          if (t2-t1 < segSize)
          {
            Quaternion q1 = boost::python::extract<Quaternion>(filteredQuaternions[t1]);
            auto orthoDist = Quaternion::OrthodromicDistance(q1, q2);
            ans[segSize][t1] = std::max(ans[segSize][t1], orthoDist);
          }
        }
      }
    }
    keysToCheck.push_back(t2);
  }
  boost::python::dict outputDict;
  for (size_t i = 0; i < boost::python::len(segSizeList); ++i)
  {
    SCALAR segSize = boost::python::extract<SCALAR>(segSizeList[i]);
    boost::python::list outList;
    for (auto k: ans[segSize])
    {
      outList.append(k.second);
    }
    outputDict[segSize] = outList;
  }
  return outputDict;
}

BOOST_PYTHON_MODULE(CQuaternion)
{
  using namespace boost::python;

  register_exception_translator<not_unit_quaternion_exception>(&translate);

  class_<Vector>("Vector")
        .def(init<SCALAR, SCALAR, SCALAR>(args("x", "y", "z")))
        .def(self_ns::str(self_ns::self))
        .def(self_ns::self + self_ns::self)
        .def(self_ns::self - self_ns::self)
        .def(-self_ns::self)
        .def(self_ns::self * self_ns::self)
        .def(SCALAR() * self_ns::self)
        .def(self_ns::self * SCALAR())
        .def(self_ns::self / SCALAR())
        .def(self_ns::self ^ self_ns::self)
        .def("DotProduct", &Vector::DotProduct)
        .def("VectorProduct", &Vector::VectorProduct)
        .def("Norm", &Vector::Norm)
        .def("ToPolar", &Vector::ToPolar)
        .add_property("x", &Vector::GetX)
        .add_property("y", &Vector::GetY)
        .add_property("z", &Vector::GetZ)
        .def_pickle(vector_pickle_suite())
    ;

  class_<Quaternion>("Quaternion")
        .def(init<SCALAR, Vector>(args("w", "v")))
        .def(init<SCALAR>(args("w")))
        .def(init<Vector>(args("v")))
        .def(self_ns::str(self_ns::self))
        .def(self_ns::self + self_ns::self)
        .def(Vector() + self_ns::self)
        .def(SCALAR() + self_ns::self)
        .def(self_ns::self - self_ns::self)
        .def(Vector() - self_ns::self)
        .def(SCALAR() - self_ns::self)
        .def(-self_ns::self)
        .def(self_ns::self * self_ns::self)
        .def(SCALAR() * self_ns::self)
        .def(Vector() * self_ns::self)
        .def(self_ns::self / SCALAR())
        .def("DotProduct", &Quaternion::DotProduct)
        .def("Dot", &Quaternion::DotProduct)
        .def("Norm", &Quaternion::Norm)
        .def("Normalize", &Quaternion::Normalize)
        .def("IsPur", &Quaternion::IsPur)
        .def("Conj", &Quaternion::Conj)
        .def("Inv", &Quaternion::Inv)
        .def("Rotation", &Quaternion::Rotation)
        .def("Exp", &Quaternion::Exp)
        .staticmethod("Exp")
        .def("Log", &Quaternion::Log)
        .staticmethod("Log")
        .def("Distance", &Quaternion::Distance)
        .staticmethod("Distance")
        .def("OrthodromicDistance", &Quaternion::OrthodromicDistance)
        .staticmethod("OrthodromicDistance")
        .def("SLERP", &Quaternion::SLERP)
        .staticmethod("SLERP")
        .def("QuaternionFromAngleAxis", &Quaternion::QuaternionFromAngleAxis)
        .staticmethod("QuaternionFromAngleAxis")
        .def("AverageAngularVelocity", &Quaternion::AverageAngularVelocity)
        .staticmethod("AverageAngularVelocity")
        .def(pow(self_ns::self, SCALAR()))
        .add_property("w", &Quaternion::GetW)
        .add_property("v", &Quaternion::GetV)
        .def_pickle(quaternion_pickle_suite())
    ;

    def("ComputeMaxOrthodromicDistances", ComputeMaxOrthodromicDistances);

    implicitly_convertible<SCALAR, Quaternion>();
    implicitly_convertible<Vector, Quaternion>();
}
