#include <boost/python.hpp>
#include <iostream>

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

  std::ostream& operator<<(std::ostream& o) const
  {
    o << "(" << m_x << ", "  << m_y << ", " <<  m_z << ")";
    return o;
  }
  const auto& GetX(void) const {return m_x;}
  const auto& GetY(void) const {return m_y;}
  const auto& GetZ(void) const {return m_z;}
private:
  SCALAR m_x;
  SCALAR m_y;
  SCALAR m_z;
};
std::ostream& operator<<(std::ostream& o, const Vector& v) {return v.operator<<(o);}
Vector operator*(const SCALAR& s, const Vector& v) {return v * s;}

class Quaternion
{
public:
  Quaternion(void): m_w(0), m_v() {}
  Quaternion(const SCALAR& w, const Vector& v): m_w(w), m_v(v) {}
  Quaternion(const SCALAR& w): m_w(w), m_v() {}
  Quaternion(const Vector& v): m_w(0), m_v(v) {}

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
    return Quaternion(m_w*q.m_w + m_v*q.m_v,
                      m_w*q.m_v + q.m_w*m_v + m_v ^ q.m_v);
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
private:
  SCALAR m_w;
  Vector m_v;
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

BOOST_PYTHON_MODULE(CQuaternion)
{
  using namespace boost::python;
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
        .def("Norm", &Vector::Norm)
    ;

  class_<Quaternion>("Quaternion")
        .def(init<SCALAR, Vector>(args("w", "v")))
        .def(init<SCALAR>(args("w")))
        .def(init<Vector>(args("v")))
        .def(self_ns::str(self_ns::self))
        .def(self_ns::self + self_ns::self)
        .def(self_ns::self + Vector())
        .def(Vector() + self_ns::self)
        .def(self_ns::self + SCALAR())
        .def(SCALAR() + self_ns::self)
        .def(self_ns::self - self_ns::self)
        .def(SCALAR() - self_ns::self)
        .def(self_ns::self - SCALAR())
        .def(-self_ns::self)
        .def(self_ns::self * self_ns::self)
        .def(SCALAR() * self_ns::self)
        .def(self_ns::self * SCALAR())
        .def(self_ns::self / SCALAR())
        .def("DotProduct", &Quaternion::DotProduct)
        .def("Norm", &Quaternion::Norm)
    ;
}
