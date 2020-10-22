#ifndef GUARD_GGEMS_MATHS_GGEMSMATRIXOPERATIONS_HH
#define GUARD_GGEMS_MATHS_GGEMSMATRIXOPERATIONS_HH

// ************************************************************************
// * This file is part of GGEMS.                                          *
// *                                                                      *
// * GGEMS is free software: you can redistribute it and/or modify        *
// * it under the terms of the GNU General Public License as published by *
// * the Free Software Foundation, either version 3 of the License, or    *
// * (at your option) any later version.                                  *
// *                                                                      *
// * GGEMS is distributed in the hope that it will be useful,             *
// * but WITHOUT ANY WARRANTY; without even the implied warranty of       *
// * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        *
// * GNU General Public License for more details.                         *
// *                                                                      *
// * You should have received a copy of the GNU General Public License    *
// * along with GGEMS.  If not, see <https://www.gnu.org/licenses/>.      *
// *                                                                      *
// ************************************************************************

/*!
  \file GGEMSMatrixOperations.hh

  \brief Definitions of functions using matrix

  \author Julien BERT <julien.bert@univ-brest.fr>
  \author Didier BENOIT <didier.benoit@inserm.fr>
  \author LaTIM, INSERM - U1101, Brest, FRANCE
  \version 1.0
  \date Monday December 16, 2019
*/

#include "GGEMS/maths/GGEMSMatrixTypes.hh"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/*!
  \fn inline GGfloat33 MakeFloat33(GGGfloat3 const m0, GGfloat3 const m1, GGfloat3 const m2)
  \param m0 - Row 0 in the matrix 3x3 for local axis
  \param m1 - Row 1 in the matrix 3x3 for local axis
  \param m2 - Row 2 in the matrix 3x3 for local axis
  \return a 3x3 float matrix
  \brief Make a GGfloat33 with custom values
*/
inline GGfloat33 MakeFloat33(GGfloat3 const m0, GGfloat3 const m1, GGfloat3 const m2)
{
  GGfloat33 tmp;
  tmp.m0_ = m0;
  tmp.m1_ = m1;
  tmp.m2_ = m2;
  return tmp;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/*!
  \fn inline GGfloat33 MakeFloat33Zeros(void)
  \return a 3x3 float matrix of 0
  \brief Make a GGfloat33 with zeros for value
*/
inline GGfloat33 MakeFloat33Zeros(void)
{
  GGfloat33 tmp;
  GGfloat3 zero = {0.0f, 0.0f, 0.0f};
  tmp.m0_ = zero;
  tmp.m1_ = zero;
  tmp.m2_ = zero;
  return tmp;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/*!
  \fn inline GGfloat44 MakeFloat44(GGfloat4 const m0, GGfloat4 const m1, GGfloat4 const m2, GGfloat4 const m3)
  \param m0 - Row 0 in the matrix 4x4 for local axis
  \param m1 - Row 1 in the matrix 4x4 for local axis
  \param m2 - Row 2 in the matrix 4x4 for local axis
  \param m3 - Row 3 in the matrix 4x4 for local axis
  \return a 4x4 float matrix
  \brief Make a GGfloat44 with custom values
*/
inline GGfloat44 MakeFloat44(GGfloat4 const m0, GGfloat4 const m1, GGfloat4 const m2, GGfloat4 const m3)
{
  GGfloat44 tmp;
  tmp.m0_ = m0;
  tmp.m1_ = m1;
  tmp.m2_ = m2;
  tmp.m3_ = m3;
  return tmp;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/*!
  \fn inline GGfloat44 MakeFloat44Zeros(void)
  \return a 4x4 float matrix
  \brief Make a GGfloat44 with zeros for value
*/
inline GGfloat44 MakeFloat44Zeros(void)
{
  GGfloat44 tmp;
  GGfloat4 zero = {0.0f, 0.0f, 0.0f, 0.0f};
  tmp.m0_ = zero;
  tmp.m1_ = zero;
  tmp.m2_ = zero; 
  tmp.m3_ = zero;
  return tmp;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/*!
  \fn inline GGfloat3 RotateUnitZ(GGfloat3 vector, GGfloat3 const new_uz)
  \param vector - vector to change
  \param new_uz - new direction
  \return a vector of 3x1 float
  \brief rotateUz, function from CLHEP
*/
#ifdef __OPENCL_C_VERSION__
inline GGfloat3 RotateUnitZ(GGfloat3 vector, GGfloat3 const new_uz)
{
  GGfloat const u1 = new_uz.x;
  GGfloat const u2 = new_uz.y;
  GGfloat const u3 = new_uz.z;

  GGfloat up = u1*u1 + u2*u2;
  if (up > 0) {
    up = sqrt(up);
    GGfloat px = vector.x,  py = vector.y, pz = vector.z;
    vector.x = (u1*u3*px - u2*py) /up + u1*pz;
    vector.y = (u2*u3*px + u1*py) /up + u2*pz;
    vector.z =    -up*px +             u3*pz;
  }
  else if (u3 < 0.) {
    vector.x = -vector.x;    // phi=0  theta=gpu_pi
    vector.z = -vector.z;
  }

  return MakeFloat3(vector.x, vector.y, vector.z);
}
#endif

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/*!
  \fn inline GGfloat3 GGfloat3Sub(GGfloat3 const u, GGfloat3 const v)
  \param u - 3D vector
  \param v - 3D vector
  \return the 3D vector u - v
  \brief Substract the vector v to vector u
*/
inline GGfloat3 GGfloat3Sub(GGfloat3 const u, GGfloat3 const v)
{
  GGfloat3 vector = {
    #ifdef __OPENCL_C_VERSION__
    u.x-v.x, u.y-v.y, u.z-v.z
    #else
    {u.s[0]-v.s[0], u.s[1]-v.s[1], u.s[2]-v.s[2]}
    #endif
  };
  return vector;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/*!
  \fn inline GGfloat3 GGfloat3Add(GGfloat3 const u, GGfloat3 const v)
  \param u - 3D vector
  \param v - 3D vector
  \return the 3D vector u + v
  \brief Add the vector v to vector u
*/
inline GGfloat3 GGfloat3Add(GGfloat3 const u, GGfloat3 const v)
{
  GGfloat3 vector = {
    #ifdef __OPENCL_C_VERSION__
    u.x+v.x, u.y+v.y, u.z+v.z
    #else
    {u.s[0]+v.s[0], u.s[1]+v.s[1], u.s[2]+v.s[2]}
    #endif
  };
  return vector;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/*!
  \fn inline GGfloat3 GGfloat3Scale(GGfloat3 const u, GGfloat const s)
  \param u - 3D vector
  \param s - a scalar
  \return a 3D vector u * s
  \brief Scale vector u with scalar s
*/
inline GGfloat3 GGfloat3Scale(GGfloat3 const u, GGfloat const s)
{
  GGfloat3 vector = {
    #ifdef __OPENCL_C_VERSION__
    u.x*s, u.y*s, u.z*s
    #else
    {u.s[0]*s, u.s[1]*s, u.s[2]*s}
    #endif
  };
  return vector;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#ifdef __OPENCL_C_VERSION__
/*!
  \fn inline GGfloat3 GGfloat44MultGGfloat3(__global GGfloat44 const* matrix, GGfloat3 const point)
  \param matrix - A matrix (4x4)
  \param point - Point in 3D (x, y, z)
  \return a vector 3x1
  \brief Compute the multiplication of matrix 4x4 and a point 3x1
*/
inline GGfloat3 GGfloat44MultGGfloat3(__global GGfloat44 const* matrix, GGfloat3 const point)
{
  GGfloat4 point4D = {point.x, point.y, point.z, 1.0f};

  GGfloat3 vector = {
    dot(matrix->m0_, point4D),
    dot(matrix->m1_, point4D),
    dot(matrix->m2_, point4D)
  };

  return vector;
}
#endif

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#ifdef __OPENCL_C_VERSION__
/*!
 \fn inline GGfloat3 LocalToGlobalPosition(__global GGfloat44 const* matrix, GGfloat3 const point)
 \param matrix - A matrix (4x4)
 \param point - Point in 3D (x, y, z)
 \return The point expresses in the global frame
 \brief Transform a 3D point from local to global frame
*/
inline GGfloat3 LocalToGlobalPosition(__global GGfloat44 const* matrix, GGfloat3 const point)
{
  return GGfloat44MultGGfloat3(matrix, point);
}
#endif

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#ifndef __OPENCL_C_VERSION__
/*!
  \fn inline GGfloat44 GGfloat44MultGGfloat44(GGfloat44 const A, GGfloat44 const B)
  \param A - first matrix
  \param B - second matrix
  \brief Perform the matrix (4x4) multiplication
  \return New matrix AxB
*/
inline GGfloat44 GGfloat44MultGGfloat44(GGfloat44 const A, GGfloat44 const B)
{
  GGfloat44 tmp = MakeFloat44Zeros();

  // Row 1
  tmp.m0_.s[0] = A.m0_.s[0]*B.m0_.s[0] + A.m0_.s[1]*B.m1_.s[0] + A.m0_.s[2]*B.m2_.s[0] + A.m0_.s[3]*B.m3_.s[0];
  tmp.m0_.s[1] = A.m0_.s[0]*B.m0_.s[1] + A.m0_.s[1]*B.m1_.s[1] + A.m0_.s[2]*B.m2_.s[1] + A.m0_.s[3]*B.m3_.s[1];
  tmp.m0_.s[2] = A.m0_.s[0]*B.m0_.s[2] + A.m0_.s[1]*B.m1_.s[2] + A.m0_.s[2]*B.m2_.s[2] + A.m0_.s[3]*B.m3_.s[2];
  tmp.m0_.s[3] = A.m0_.s[0]*B.m0_.s[3] + A.m0_.s[1]*B.m1_.s[3] + A.m0_.s[2]*B.m2_.s[3] + A.m0_.s[3]*B.m3_.s[3];

  // Row 2
  tmp.m1_.s[0] = A.m1_.s[0]*B.m0_.s[0] + A.m1_.s[1]*B.m1_.s[0] + A.m1_.s[2]*B.m2_.s[0] + A.m1_.s[3]*B.m3_.s[0];
  tmp.m1_.s[1] = A.m1_.s[0]*B.m0_.s[1] + A.m1_.s[1]*B.m1_.s[1] + A.m1_.s[2]*B.m2_.s[1] + A.m1_.s[3]*B.m3_.s[1];
  tmp.m1_.s[2] = A.m1_.s[0]*B.m0_.s[2] + A.m1_.s[1]*B.m1_.s[2] + A.m1_.s[2]*B.m2_.s[2] + A.m1_.s[3]*B.m3_.s[2];
  tmp.m1_.s[3] = A.m1_.s[0]*B.m0_.s[3] + A.m1_.s[1]*B.m1_.s[3] + A.m1_.s[2]*B.m2_.s[3] + A.m1_.s[3]*B.m3_.s[3];

  // Row 3
  tmp.m2_.s[0] = A.m2_.s[0]*B.m0_.s[0] + A.m2_.s[1]*B.m1_.s[0] + A.m2_.s[2]*B.m2_.s[0] + A.m2_.s[3]*B.m3_.s[0];
  tmp.m2_.s[1] = A.m2_.s[0]*B.m0_.s[1] + A.m2_.s[1]*B.m1_.s[1] + A.m2_.s[2]*B.m2_.s[1] + A.m2_.s[3]*B.m3_.s[1];
  tmp.m2_.s[2] = A.m2_.s[0]*B.m0_.s[2] + A.m2_.s[1]*B.m1_.s[2] + A.m2_.s[2]*B.m2_.s[2] + A.m2_.s[3]*B.m3_.s[2];
  tmp.m2_.s[3] = A.m2_.s[0]*B.m0_.s[3] + A.m2_.s[1]*B.m1_.s[3] + A.m2_.s[2]*B.m2_.s[3] + A.m2_.s[3]*B.m3_.s[3];

  // Row 4
  tmp.m3_.s[0] = A.m3_.s[0]*B.m0_.s[0] + A.m3_.s[1]*B.m1_.s[0] + A.m3_.s[2]*B.m2_.s[0] + A.m3_.s[3]*B.m3_.s[0];
  tmp.m3_.s[1] = A.m3_.s[0]*B.m0_.s[1] + A.m3_.s[1]*B.m1_.s[1] + A.m3_.s[2]*B.m2_.s[1] + A.m3_.s[3]*B.m3_.s[1];
  tmp.m3_.s[2] = A.m3_.s[0]*B.m0_.s[2] + A.m3_.s[1]*B.m1_.s[2] + A.m3_.s[2]*B.m2_.s[2] + A.m3_.s[3]*B.m3_.s[2];
  tmp.m3_.s[3] = A.m3_.s[0]*B.m0_.s[3] + A.m3_.s[1]*B.m1_.s[3] + A.m3_.s[2]*B.m2_.s[3] + A.m3_.s[3]*B.m3_.s[3];

  return tmp;
}
#endif

#endif // End of GUARD_GGEMS_MATHS_MATRIX_FUNCTIONS_HH
