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
  \file GGEMSSTLReader.cc

  \brief I/O class handling STL mesh file

  \author Julien BERT <julien.bert@univ-brest.fr>
  \author Didier BENOIT <didier.benoit@inserm.fr>
  \author LaTIM, INSERM - U1101, Brest, FRANCE
  \version 1.0
  \date Thrusday July 7, 2022
*/

#include <fstream>
#include <limits>

#include "GGEMS/tools/GGEMSPrint.hh"
#include "GGEMS/tools/GGEMSTools.hh"
#include "GGEMS/io/GGEMSSTLReader.hh"
#include "GGEMS/global/GGEMSOpenCLManager.hh"
#include "GGEMS/maths/GGEMSMathAlgorithms.hh"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

GGEMSSTLReader::GGEMSSTLReader(void)
{
  GGcout("GGEMSSTLReader", "GGEMSSTLReader", 3) << "GGEMSSTLReader creating..." << GGendl;

  GGcout("GGEMSSTLReader", "GGEMSSTLReader", 3) << "GGEMSSTLReader created!!!" << GGendl;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

GGEMSSTLReader::~GGEMSSTLReader(void)
{
  GGcout("GGEMSSTLReader", "~GGEMSSTLReader", 3) << "GGEMSSTLReader erasing!!!" << GGendl;

  if (triangles_) {
    delete[] triangles_;
    triangles_ = nullptr;
  }

  GGcout("GGEMSSTLReader", "~GGEMSSTLReader", 3) << "GGEMSSTLReader erased!!!" << GGendl;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSSTLReader::Read(std::string const& meshed_phantom_filename)
{
  GGcout("GGEMSSTLReader", "Read", 2) << "Reading STL Image and loading mesh triangles..." << GGendl;

  //GGEMSOpenCLManager& opencl_manager = GGEMSOpenCLManager::GetInstance();
  //size_t number_activated_devices = opencl_manager.GetNumberOfActivatedDevice();

  // Opening STL file
  std::ifstream stl_stream(meshed_phantom_filename, std::ios::in | std::ios::binary);
  GGEMSFileStream::CheckInputStream(stl_stream, meshed_phantom_filename);

  stl_stream.read(reinterpret_cast<char*>(header_), sizeof(GGuchar) * 80);
  stl_stream.read(reinterpret_cast<char*>(&number_of_triangles_), sizeof(GGuint) * 1);

  // Min and max points
  GGEMSPoint3 lo;
  lo.x_ = FLT_MAX; lo.y_ = FLT_MAX; lo.z_ = FLT_MAX;

  GGEMSPoint3 hi;
  hi.x_ = FLT_MIN; hi.y_ = FLT_MIN; hi.z_ = FLT_MIN;

  GGfloat data[12]; // Vertices for triangle from STL file
  GGushort octet_attribut; // Useless parameter from STL file

  triangles_ = new GGEMSMeshTriangle[number_of_triangles_];

  for (unsigned int i = 0; i < number_of_triangles_; ++i) {
    stl_stream.read(reinterpret_cast<char*>(data), sizeof(float) * 12);
    stl_stream.read(reinterpret_cast<char*>(&octet_attribut), sizeof(unsigned short) * 1);

    GGEMSPoint3 p1, p2, p3;
    p1.x_= data[3]; p1.y_ = data[4]; p1.z_ = data[5];
    p2.x_= data[6]; p2.y_ = data[7]; p2.z_ = data[8];
    p3.x_= data[9]; p3.y_ = data[10]; p3.z_ = data[11];

    triangles_[i] = GGEMSMeshTriangle(p1, p2, p3);

    for (int j = 0; j < 3; ++j) { // Loop over points
      if (triangles_[i].pts_[j].x_ < lo.x_) lo.x_ = triangles_[i].pts_[j].x_;
      if (triangles_[i].pts_[j].y_ < lo.y_) lo.y_ = triangles_[i].pts_[j].y_;
      if (triangles_[i].pts_[j].z_ < lo.z_) lo.z_ = triangles_[i].pts_[j].z_;
      if (triangles_[i].pts_[j].x_ > hi.x_) hi.x_ = triangles_[i].pts_[j].x_;
      if (triangles_[i].pts_[j].y_ > hi.y_) hi.y_ = triangles_[i].pts_[j].y_;
      if (triangles_[i].pts_[j].z_ > hi.z_) hi.z_ = triangles_[i].pts_[j].z_;
    }
  }

  // Expand it by 10% so that all points are well interior
  GGfloat expanding_x = (hi.x_ - lo.x_) * 0.1f;
  GGfloat expanding_y = (hi.y_ - lo.y_) * 0.1f;
  GGfloat expanding_z = (hi.z_ - lo.z_) * 0.1f;

  lo.x_ -= expanding_x;
  hi.x_ += expanding_x;
  lo.y_ -= expanding_y;
  hi.y_ += expanding_y;
  lo.z_ -= expanding_z;
  hi.z_ += expanding_z;

  if (lo.x_ < 0.0f) lo.x_ = std::floor(lo.x_);
  else lo.x_ = std::ceil(lo.x_);

  if (lo.y_ < 0.0f) lo.y_ = std::floor(lo.y_);
  else lo.y_ = std::ceil(lo.y_);

  if (lo.z_ < 0.0f) lo.z_ = std::floor(lo.z_);
  else lo.z_ = std::ceil(lo.z_);

  if (hi.x_ < 0.0f) hi.x_ = std::floor(hi.x_);
  else hi.x_ = std::ceil(hi.x_);

  if (hi.y_ < 0.0f) hi.y_ = std::floor(hi.y_);
  else hi.y_ = std::ceil(hi.y_);

  if (hi.z_ < 0.0f) hi.z_ = std::floor(hi.z_);
  else hi.z_ = std::ceil(hi.z_);

  // Computing the center of octree box
  center_.x_ = (hi.x_+lo.x_)*0.5f;
  center_.y_ = (hi.y_+lo.y_)*0.5f;
  center_.z_ = (hi.z_+lo.z_)*0.5f;

  // Computing half width for each axes
  half_width_[0] = (hi.x_-lo.x_)*0.5f;
  half_width_[1] = (hi.y_-lo.y_)*0.5f;
  half_width_[2] = (hi.z_-lo.z_)*0.5f;

  stl_stream.close();
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

GGEMSSTLReader::GGEMSMeshTriangle::GGEMSMeshTriangle(GGEMSPoint3 const& p0, GGEMSPoint3 const& p1, GGEMSPoint3 const& p2)
{
  pts_[0] = p0;
  pts_[1] = p1;
  pts_[2] = p2;

  GGEMSSphere3 s;
  SphereFromDistantPoints(s, pts_);
  for (int i = 0; i < 3; ++i) {
    SphereOfSphereAndPoint(s, pts_[i]);
  }

  bounding_sphere_.center_ = s.center_;
  bounding_sphere_.radius_ = s.radius_;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSSTLReader::GGEMSMeshTriangle::MostSeparatedPointsOnAABB(GGEMSPoint3 pts[3], GGint& min, GGint& max)
{
  // First find most extreme points along principal axes
  int minx{0}, maxx{0};
  int miny{0}, maxy{0};
  int minz{0}, maxz{0};

  for (int i = 1; i < 3; ++i) {
    if (pts[i].x_ < pts[minx].x_) minx = i;
    if (pts[i].x_ > pts[maxx].x_) maxx = i;
    if (pts[i].y_ < pts[miny].y_) miny = i;
    if (pts[i].y_ > pts[maxy].y_) maxy = i;
    if (pts[i].z_ < pts[minz].z_) minz = i;
    if (pts[i].z_ > pts[maxz].z_) maxz = i;
  }

  // Compute the squared distances for the three pairs of points
  GGEMSPoint3 px, py, pz;

  px.x_ = pts[maxx].x_ - pts[minx].x_;
  px.y_ = pts[maxx].y_ - pts[minx].y_;
  px.z_ = pts[maxx].z_ - pts[minx].z_;

  py.x_ = pts[maxy].x_ - pts[miny].x_;
  py.y_ = pts[maxy].y_ - pts[miny].y_;
  py.z_ = pts[maxy].z_ - pts[miny].z_;

  pz.x_ = pts[maxz].x_ - pts[minz].x_;
  pz.y_ = pts[maxz].y_ - pts[minz].y_;
  pz.z_ = pts[maxz].z_ - pts[minz].z_;

  float dist2x = Dot(px, px);
  float dist2y = Dot(py, py);
  float dist2z = Dot(pz, pz);

  // Pick the pair (min,max) of points most distant
  min = minx;
  max = maxx;
  if (dist2y > dist2x && dist2y > dist2z) {
    min = miny;
    max = maxy;
  }
  else if (dist2z > dist2y && dist2z > dist2x) {
    min = minz;
    max = maxz;
  }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSSTLReader::GGEMSMeshTriangle::SphereFromDistantPoints(GGEMSSphere3& s, GGEMSPoint3 pts[3])
{
  // Find the most separated point pair defining the encompassing AABB
  int min{0};
  int max{0};

  MostSeparatedPointsOnAABB(pts, min, max);

  GGEMSPoint3 center;
  center.x_ = (pts[min].x_ + pts[max].x_) * 0.5f;
  center.y_ = (pts[min].y_ + pts[max].y_) * 0.5f;
  center.z_ = (pts[min].z_ + pts[max].z_) * 0.5f;

  // Set up sphere to just encompass these two points
  s.center_ = center;

  GGEMSPoint3 distance;
  distance.x_ = pts[max].x_ - center.x_;
  distance.y_ = pts[max].y_ - center.y_;
  distance.z_ = pts[max].z_ - center.z_;

  s.radius_ = Dot(distance, distance);
  s.radius_ = sqrtf(s.radius_);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSSTLReader::GGEMSMeshTriangle::SphereOfSphereAndPoint(GGEMSSphere3& s, GGEMSPoint3& p)
{
  // Compute squared distance between point and sphere center
  GGEMSPoint3 p_scenter;
  p_scenter.x_ = p.x_ - s.center_.x_;
  p_scenter.y_ = p.y_ - s.center_.y_;
  p_scenter.z_ = p.z_ - s.center_.z_;

  float dist2 = Dot(p_scenter, p_scenter);

  if (dist2 > s.radius_ * s.radius_) {
    float dist = sqrtf(dist2);
    float new_radius = (s.radius_ + dist) * 0.5;
    float k = (new_radius - s.radius_) / dist;
    s.radius_ = new_radius;
    s.center_.x_ += p_scenter.x_ * k;
    s.center_.y_ += p_scenter.y_ * k;
    s.center_.z_ += p_scenter.z_ * k;
  }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSSTLReader::LoadTriangles(GGEMSTriangle3* triangles)
{
  for (GGuint i = 0; i < number_of_triangles_; ++i) {
    triangles[i].pts_[0].x_ = triangles_[i].pts_[0].x_;
    triangles[i].pts_[0].y_ = triangles_[i].pts_[0].y_;
    triangles[i].pts_[0].z_ = triangles_[i].pts_[0].z_;

    triangles[i].pts_[1].x_ = triangles_[i].pts_[1].x_;
    triangles[i].pts_[1].y_ = triangles_[i].pts_[1].y_;
    triangles[i].pts_[1].z_ = triangles_[i].pts_[1].z_;

    triangles[i].pts_[2].x_ = triangles_[i].pts_[2].x_;
    triangles[i].pts_[2].y_ = triangles_[i].pts_[2].y_;
    triangles[i].pts_[2].z_ = triangles_[i].pts_[2].z_;

    triangles[i].bounding_sphere_.center_.x_ = triangles_[i].bounding_sphere_.center_.x_;
    triangles[i].bounding_sphere_.center_.y_ = triangles_[i].bounding_sphere_.center_.y_;
    triangles[i].bounding_sphere_.center_.z_ = triangles_[i].bounding_sphere_.center_.z_;

    triangles[i].bounding_sphere_.radius_ = triangles_[i].bounding_sphere_.radius_;
  }
}
