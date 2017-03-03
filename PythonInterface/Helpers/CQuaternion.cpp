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

  auto ToSpherical(void) const
  {
    auto theta = std::atan2(m_y, m_x);
    auto phi = std::acos(m_z/Norm());
    return boost::python::make_tuple(theta, phi);
  }

  static Vector FromSpherical(SCALAR theta, SCALAR phi)
  {
    auto sinP = std::sin(phi);
    return Vector(sinP*std::cos(theta), sinP*std::sin(theta), std::cos(phi));
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
      Quaternion q = *this;
      q.Normalize();
      return (q*v*q.Conj()).GetV();
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
  std::vector<SCALAR> ssl;
  for (size_t i = 0; i < boost::python::len(segSizeList); ++i)
  {
    SCALAR segSize = boost::python::extract<SCALAR>(segSizeList[i]);
    ssl.push_back(segSize);
    ans[segSize] = std::map<SCALAR, SCALAR>();
    maxSegSize = std::max(maxSegSize, segSize);
  }
  boost::python::list keys = filteredQuaternions.keys();
  std::vector<SCALAR> keysList;
  std::vector<SCALAR> keysToCheck;
  std::map<SCALAR, Quaternion> quaternionMap;
  SCALAR maxTimestamp = 0;
  for (size_t i = 0; i < boost::python::len(keys); ++i)
  {
    SCALAR t = boost::python::extract<SCALAR>(keys[i]);
    keysList.push_back(t);
    Quaternion q = boost::python::extract<Quaternion>(filteredQuaternions[keys[i]]);
    q.Normalize();
    quaternionMap[t] = q;
    maxTimestamp = std::max(maxTimestamp, t);
  }
  for (size_t i = 0; i < keysList.size(); ++i)
  {
    SCALAR& t2 = keysList[i];
    Quaternion& q2 = quaternionMap[t2];
    while(!keysToCheck.empty() && t2-keysToCheck[0] > maxSegSize)
    {
      keysToCheck.erase(keysToCheck.begin());
    }
    for (auto t1: keysToCheck)
    {
      for (size_t i = 0; i < ssl.size(); ++i)
      {
        SCALAR& segSize = ssl[i];
        if (maxTimestamp - t2 >= segSize)
        {
          ans[segSize][t2] = 0;
          if (t2-t1 < segSize)
          {
            Quaternion& q1 = quaternionMap[t1];
            auto orthoDist = Quaternion::OrthodromicDistance(q1, q2);
            ans[segSize][t1] = std::max(ans[segSize][t1], orthoDist);
          }
        }
      }
    }
    keysToCheck.push_back(t2);
  }
  boost::python::dict outputDict;
  for (size_t i = 0; i < ssl.size(); ++i)
  {
    SCALAR segSize = ssl[i];
    boost::python::list outList;
    for (auto k: ans[segSize])
    {
      outList.append(k.second);
    }
    outputDict[segSize] = outList;
  }
  return outputDict;
}

constexpr SCALAR PI = 3.141592653589793238L;

boost::python::list ComputeVision(boost::python::dict& filteredQuaternions,
    size_t width, size_t height, SCALAR horizontalFoVAngle, SCALAR verticalFoVAngle)
{
  boost::python::list ans;
  for (size_t i = 0; i < width; ++i)
  {
    boost::python::list l;
    l.append(0);
    l *= height;
    ans.append(l);
  }
  auto y = std::sqrt(1-std::cos(horizontalFoVAngle));
  auto z = std::sqrt(1-std::cos(verticalFoVAngle));
  auto a = Vector(1, y, z);
  auto b = Vector(1, y, -z);
  auto c = Vector(1, -y, -z);
  auto d = Vector(1, -y, z);
  // compute inward normal to the delimitation plan
  auto n_ab = a ^ b;
  n_ab = n_ab/n_ab.Norm();
  auto n_bc = b ^ c;
  n_bc = n_bc/n_bc.Norm();
  auto n_cd = c ^ d;
  n_cd = n_cd/n_cd.Norm();
  auto n_da = d ^ a;
  n_da = n_da/n_da.Norm();
  boost::python::list keys = filteredQuaternions.keys();
  size_t len = boost::python::len(keys);
  for (size_t k = 0; k < len; ++k)
  {
    Quaternion q = boost::python::extract<Quaternion>(filteredQuaternions[keys[k]]);
    std::vector<std::tuple<size_t, size_t>>  hits;
    for (size_t i = 0; i < width; ++i)
    {
      for (size_t j = 0; j < height; ++j)
      {
        auto theta = ((2.0*PI*i)/width);
        auto phi = PI-(PI*j)/height;
        //p is the direction vector of this pixel
        auto p = Vector::FromSpherical(theta, phi);
        auto p_headFrame = q.Conj().Rotation(p);

        // auto n_ab_rot = q.Rotation(n_ab);
        // auto n_bc_rot = q.Rotation(n_bc);
        // auto n_cd_rot = q.Rotation(n_cd);
        // auto n_da_rot = q.Rotation(n_da);
        // test if p is inside the viewport
        if (p_headFrame * n_ab > 0 &&
                p_headFrame * n_bc > 0 &&
                p_headFrame * n_cd > 0 &&
                p_headFrame * n_da > 0)
        // if (p * n_ab_rot > 0 &&
        //     p * n_bc_rot > 0 &&
        //     p * n_cd_rot > 0 &&
        //     p * n_da_rot > 0)
        {
          hits.push_back(std::make_tuple(i, j));
        }
      }
    }
    size_t hitsCount = hits.size();
    for (auto tuple: hits)
    {
        ans[std::get<0>(tuple)][std::get<1>(tuple)] += 1.0/(len*hitsCount);
    }
  }
  return ans;
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
        .def("ToSpherical", &Vector::ToSpherical)
        .def("FromSpherical", &Vector::FromSpherical)
        .staticmethod("FromSpherical")
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
    def("ComputeVision", ComputeVision);

    implicitly_convertible<SCALAR, Quaternion>();
    implicitly_convertible<Vector, Quaternion>();
}
