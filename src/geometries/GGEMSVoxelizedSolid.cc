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
  \file GGEMSVoxelizedSolid.cc

  \brief GGEMS class for voxelized solid

  \author Julien BERT <julien.bert@univ-brest.fr>
  \author Didier BENOIT <didier.benoit@inserm.fr>
  \author LaTIM, INSERM - U1101, Brest, FRANCE
  \version 1.0
  \date Wednesday June 10, 2020
*/

#include "GGEMS/geometries/GGEMSVoxelizedSolid.hh"
#include "GGEMS/io/GGEMSMHDImage.hh"
#include "GGEMS/maths/GGEMSGeometryTransformation.hh"
#include "GGEMS/geometries/GGEMSVoxelizedSolidStack.hh"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

GGEMSVoxelizedSolid::GGEMSVoxelizedSolid(std::string const& volume_header_filename, std::string const& range_filename)
: GGEMSSolid(),
  volume_header_filename_(volume_header_filename),
  range_filename_(range_filename)
{
  GGcout("GGEMSVoxelizedSolid", "GGEMSVoxelizedSolid", 3) << "Allocation of GGEMSVoxelizedSolid..." << GGendl;

  // Get the OpenCL manager
  GGEMSOpenCLManager& opencl_manager = GGEMSOpenCLManager::GetInstance();

  // Get the RAM manager
  GGEMSRAMManager& ram_manager = GGEMSRAMManager::GetInstance();

  // Allocation of memory on OpenCL device for header data
  solid_data_cl_ = opencl_manager.Allocate(nullptr, sizeof(GGEMSVoxelizedSolidData), CL_MEM_READ_WRITE);
  ram_manager.AddGeometryRAMMemory(sizeof(GGEMSVoxelizedSolidData));
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

GGEMSVoxelizedSolid::~GGEMSVoxelizedSolid(void)
{
  GGcout("GGEMSVoxelizedSolid", "~GGEMSVoxelizedSolid", 3) << "Deallocation of GGEMSVoxelizedSolid..." << GGendl;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSVoxelizedSolid::InitializeKernel(void)
{
  GGcout("GGEMSVoxelizedSolid", "InitializeKernel", 3) << "Initializing kernel for voxelized solid..." << GGendl;

  // Getting OpenCL manager
  GGEMSOpenCLManager& opencl_manager = GGEMSOpenCLManager::GetInstance();

  // Getting the path to kernel
  std::string const kOpenCLKernelPath = OPENCL_KERNEL_PATH;
  std::string const kFilename1 = kOpenCLKernelPath + "/ParticleSolidDistanceGGEMSVoxelizedSolid.cl";
  std::string const kFilename2 = kOpenCLKernelPath + "/ProjectToVoxelizedSolid.cl";
  std::string const kFilename3 = kOpenCLKernelPath + "/TrackThroughVoxelizedSolid.cl";

  // Compiling the kernels
  kernel_distance_cl_ = opencl_manager.CompileKernel(kFilename1, "particle_solid_distance_ggems_voxelized_solid", nullptr, const_cast<char*>(tracking_kernel_option_.c_str()));
  //kernel_project_to_cl_ = opencl_manager.CompileKernel(kFilename2, "project_to_voxelized_solid");
  //kernel_track_through_cl_ = opencl_manager.CompileKernel(kFilename3, "track_through_voxelized_solid", nullptr, const_cast<char*>(tracking_kernel_option_.c_str()));
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSVoxelizedSolid::Initialize(std::weak_ptr<GGEMSMaterials> materials)
{
  GGcout("GGEMSVoxelizedSolid", "Initialize", 3) << "Initializing voxelized solid..." << GGendl;

  // Initializing kernels and loading image
  InitializeKernel();
  LoadVolumeImage(materials);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSVoxelizedSolid::SetPosition(GGfloat3 const& position_xyz)
{
  GGcout("GGEMSVoxelizedSolid", "SetPosition", 3) << "Setting position of voxelized solid..." << GGendl;

  // Set position in geometric transformation
  geometry_transformation_->SetTranslation(position_xyz);

  // Get the OpenCL manager
  GGEMSOpenCLManager& opencl_manager = GGEMSOpenCLManager::GetInstance();

  // Get pointer on OpenCL device
  GGEMSVoxelizedSolidData* solid_data_device = opencl_manager.GetDeviceBuffer<GGEMSVoxelizedSolidData>(solid_data_cl_.get(), sizeof(GGEMSVoxelizedSolidData));

  for (GGint i = 0; i < 3; ++i ) {
    solid_data_device->obb_geometry_.border_min_xyz_[i] = (-solid_data_device->number_of_voxels_xyz_[i]*solid_data_device->voxel_sizes_xyz_[i]*0.5f)+position_xyz.s[i];
    solid_data_device->obb_geometry_.border_max_xyz_[i] = (solid_data_device->number_of_voxels_xyz_[i]*solid_data_device->voxel_sizes_xyz_[i]*0.5f)+position_xyz.s[i];
  }

  // Release the pointer
  opencl_manager.ReleaseDeviceBuffer(solid_data_cl_.get(), solid_data_device);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSVoxelizedSolid::UpdateTransformationMatrix(void)
{
  geometry_transformation_->UpdateTransformationMatrix();

  // Get the OpenCL manager
  GGEMSOpenCLManager& opencl_manager = GGEMSOpenCLManager::GetInstance();

  // Copy information to OBB
  GGEMSVoxelizedSolidData* solid_data_device = opencl_manager.GetDeviceBuffer<GGEMSVoxelizedSolidData>(solid_data_cl_.get(), sizeof(GGEMSVoxelizedSolidData));
  GGfloat44* transformation_matrix_device = opencl_manager.GetDeviceBuffer<GGfloat44>(geometry_transformation_->GetTransformationMatrix(), sizeof(GGfloat44));

  // solid_data_device->obb_geometry_.matrix_transformation_.m0_ = transformation_matrix_device->m0_;
  // solid_data_device->obb_geometry_.matrix_transformation_.m1_ = transformation_matrix_device->m1_;
  // solid_data_device->obb_geometry_.matrix_transformation_.m2_ = transformation_matrix_device->m2_;
  // solid_data_device->obb_geometry_.matrix_transformation_.m3_ = transformation_matrix_device->m3_;

  // Release the pointer
  opencl_manager.ReleaseDeviceBuffer(solid_data_cl_.get(), solid_data_device);
  opencl_manager.ReleaseDeviceBuffer(geometry_transformation_->GetTransformationMatrix(), transformation_matrix_device);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSVoxelizedSolid::PrintInfos(void) const
{
  // Get the OpenCL manager
  GGEMSOpenCLManager& opencl_manager = GGEMSOpenCLManager::GetInstance();

  // Get pointer on OpenCL device
  GGEMSVoxelizedSolidData* solid_data_device = opencl_manager.GetDeviceBuffer<GGEMSVoxelizedSolidData>(solid_data_cl_.get(), sizeof(GGEMSVoxelizedSolidData));

  GGcout("GGEMSVoxelizedSolid", "PrintInfos", 0) << GGendl;
  GGcout("GGEMSVoxelizedSolid", "PrintInfos", 0) << "GGEMSVoxelizedSolid Infos:" << GGendl;
  GGcout("GGEMSVoxelizedSolid", "PrintInfos", 0) << "--------------------------" << GGendl;
  GGcout("GGEMSVoxelizedSolid", "PrintInfos", 0) << "* Dimension: " << solid_data_device->number_of_voxels_xyz_[0] << " " << solid_data_device->number_of_voxels_xyz_[1] << " " << solid_data_device->number_of_voxels_xyz_[2] << GGendl;
  GGcout("GGEMSVoxelizedSolid", "PrintInfos", 0) << "* Number of voxels: " << solid_data_device->number_of_voxels_ << GGendl;
  GGcout("GGEMSVoxelizedSolid", "PrintInfos", 0) << "* Size of voxels: (" << solid_data_device->voxel_sizes_xyz_[0] << "x" << solid_data_device->voxel_sizes_xyz_[1] << "x" << solid_data_device->voxel_sizes_xyz_[2] << ") mm3" << GGendl;
  GGcout("GGEMSVoxelizedSolid", "PrintInfos", 0) << "* Position: (" << geometry_transformation_->GetPosition().s0 << "x" <<geometry_transformation_->GetPosition().s1 << "x" << geometry_transformation_->GetPosition().s2 << ") mm3" << GGendl;
  GGcout("GGEMSVoxelizedSolid", "PrintInfos", 0) << "* Oriented bounding box (OBB):" << GGendl;
  GGcout("GGEMSVoxelizedSolid", "PrintInfos", 0) << "    - X: " << solid_data_device->obb_geometry_.border_min_xyz_[0] << " <-> " << solid_data_device->obb_geometry_.border_max_xyz_[0] << GGendl;
  GGcout("GGEMSVoxelizedSolid", "PrintInfos", 0) << "    - Y: " << solid_data_device->obb_geometry_.border_min_xyz_[1] << " <-> " << solid_data_device->obb_geometry_.border_max_xyz_[1] << GGendl;
  GGcout("GGEMSVoxelizedSolid", "PrintInfos", 0) << "    - Z: " << solid_data_device->obb_geometry_.border_min_xyz_[2] << " <-> " << solid_data_device->obb_geometry_.border_max_xyz_[2] << GGendl;
  GGcout("GGEMSVoxelizedSolid", "PrintInfos", 0) << "    - Transformation matrix:" << GGendl;
  // GGcout("GGEMSVoxelizedSolid", "PrintInfos", 0) << "    [" << GGendl;
  // GGcout("GGEMSVoxelizedSolid", "PrintInfos", 0) << "        " << solid_data_device->obb_geometry_.matrix_transformation_.m0_.s0 << " " << solid_data_device->obb_geometry_.matrix_transformation_.m0_.s1 << " " << solid_data_device->obb_geometry_.matrix_transformation_.m0_.s2 << " " << solid_data_device->obb_geometry_.matrix_transformation_.m0_.s3 << GGendl;
  // GGcout("GGEMSVoxelizedSolid", "PrintInfos", 0) << "        " << solid_data_device->obb_geometry_.matrix_transformation_.m1_.s0 << " " << solid_data_device->obb_geometry_.matrix_transformation_.m1_.s1 << " " << solid_data_device->obb_geometry_.matrix_transformation_.m1_.s2 << " " << solid_data_device->obb_geometry_.matrix_transformation_.m1_.s3 << GGendl;
  // GGcout("GGEMSVoxelizedSolid", "PrintInfos", 0) << "        " << solid_data_device->obb_geometry_.matrix_transformation_.m2_.s0 << " " << solid_data_device->obb_geometry_.matrix_transformation_.m2_.s1 << " " << solid_data_device->obb_geometry_.matrix_transformation_.m2_.s2 << " " << solid_data_device->obb_geometry_.matrix_transformation_.m2_.s3 << GGendl;
  // GGcout("GGEMSVoxelizedSolid", "PrintInfos", 0) << "        " << solid_data_device->obb_geometry_.matrix_transformation_.m3_.s0 << " " << solid_data_device->obb_geometry_.matrix_transformation_.m3_.s1 << " " << solid_data_device->obb_geometry_.matrix_transformation_.m3_.s2 << " " << solid_data_device->obb_geometry_.matrix_transformation_.m3_.s3 << GGendl;
  // GGcout("GGEMSVoxelizedSolid", "PrintInfos", 0) << "    ]" << GGendl;
  GGcout("GGEMSVoxelizedSolid", "PrintInfos", 0) << "* Solid index: " << solid_data_device->solid_id_ << GGendl;
  GGcout("GGEMSVoxelizedSolid", "PrintInfos", 0) << GGendl;

  // Release the pointer
  opencl_manager.ReleaseDeviceBuffer(solid_data_cl_.get(), solid_data_device);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSVoxelizedSolid::LoadVolumeImage(std::weak_ptr<GGEMSMaterials> materials)
{
  GGcout("GGEMSVoxelizedSolid", "LoadVolumeImage", 3) << "Loading volume image from mhd file..." << GGendl;

  // Read MHD input file
  GGEMSMHDImage mhd_input_phantom;
  mhd_input_phantom.Read(volume_header_filename_, solid_data_cl_);

  // Get the name of raw file from mhd reader
  std::string const kRawFilename = mhd_input_phantom.GetRawMDHfilename();

  // Get the type
  std::string const kDataType = mhd_input_phantom.GetDataMHDType();

  // Convert raw data to material id data
  if (!kDataType.compare("MET_CHAR")) {
    ConvertImageToLabel<char>(kRawFilename, range_filename_, materials);
  }
  else if (!kDataType.compare("MET_UCHAR")) {
    ConvertImageToLabel<unsigned char>(kRawFilename, range_filename_, materials);
  }
  else if (!kDataType.compare("MET_SHORT")) {
    ConvertImageToLabel<GGshort>(kRawFilename, range_filename_, materials);
  }
  else if (!kDataType.compare("MET_USHORT")) {
    ConvertImageToLabel<GGushort>(kRawFilename, range_filename_, materials);
  }
  else if (!kDataType.compare("MET_INT")) {
    ConvertImageToLabel<GGint>(kRawFilename, range_filename_, materials);
  }
  else if (!kDataType.compare("MET_UINT")) {
    ConvertImageToLabel<GGuint>(kRawFilename, range_filename_, materials);
  }
  else if (!kDataType.compare("MET_FLOAT")) {
    ConvertImageToLabel<GGfloat>(kRawFilename, range_filename_, materials);
  }
}
