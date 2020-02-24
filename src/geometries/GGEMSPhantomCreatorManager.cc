/*!
  \file GGEMSPhantomCreatorManager.cc

  \brief Singleton class generating voxelized phantom from analytical volume

  \author Julien BERT <julien.bert@univ-brest.fr>
  \author Didier BENOIT <didier.benoit@inserm.fr>
  \author LaTIM, INSERM - U1101, Brest, FRANCE
  \version 1.0
  \date Thursday January 9, 2020
*/

#include <algorithm>

#include "GGEMS/geometries/GGEMSPhantomCreatorManager.hh"
#include "GGEMS/tools/GGEMSPrint.hh"
#include "GGEMS/tools/GGEMSTools.hh"
#include "GGEMS/io/GGEMSMHDImage.hh"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

GGEMSPhantomCreatorManager::GGEMSPhantomCreatorManager(void)
: element_sizes_(GGdouble3{{0.0, 0.0, 0.0}}),
  phantom_dimensions_(GGuint3{{0, 0, 0}}),
  number_elements_(0),
  offsets_(GGdouble3{{0.0, 0.0, 0.0}}),
  isocenter_position_(GGdouble3{{0.0, 0.0, 0.0}}),
  output_basename_(""),
  format_(""),
  voxelized_phantom_(nullptr),
  opencl_manager_(GGEMSOpenCLManager::GetInstance())
{
  GGcout("GGEMSPhantomCreatorManager", "GGEMSPhantomCreatorManager", 3) << "Allocation of Phantom Creator Manager singleton..." << GGendl;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

GGEMSPhantomCreatorManager::~GGEMSPhantomCreatorManager(void)
{
  GGcout("GGEMSPhantomCreatorManager", "~GGEMSPhantomCreatorManager", 3) << "Deallocation of Phantom Creator Manager singleton..." << GGendl;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSPhantomCreatorManager::SetElementSizes(GGdouble const& voxel_width, GGdouble const& voxel_height, GGdouble const& voxel_depth)
{
  element_sizes_.s[0] = voxel_width;
  element_sizes_.s[1] = voxel_height;
  element_sizes_.s[2] = voxel_depth;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSPhantomCreatorManager::SetPhantomDimensions(GGuint const& phantom_width, GGuint const& phantom_height, GGuint const& phantom_depth)
{
  phantom_dimensions_.s[0] = phantom_width;
  phantom_dimensions_.s[1] = phantom_height;
  phantom_dimensions_.s[2] = phantom_depth;
  number_elements_ = phantom_width * phantom_height * phantom_depth;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSPhantomCreatorManager::SetIsocenterPositions(GGdouble const& iso_pos_x, GGdouble const& iso_pos_y, GGdouble const& iso_pos_z)
{
  isocenter_position_.s[0] = iso_pos_x;
  isocenter_position_.s[1] = iso_pos_y;
  isocenter_position_.s[2] = iso_pos_z;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSPhantomCreatorManager::SetOutputBasename(char const* output_basename, char const* format)
{
  // Output basename
  std::string output_basename_str(output_basename);
  output_basename_ = output_basename_str;

  // Output format
  std::string format_str(format);
  format_ = format_str;

  // Transform the string to lower character
  std::transform(format_.begin(), format_.end(), format_.begin(), ::tolower);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSPhantomCreatorManager::CheckParameters(void) const
{
  GGcout("GGEMSPhantomCreatorManager", "CheckParameters", 3) << "Checking parameters for phantom creator manager..." << GGendl;

  // Checking phantom dimensions
  if (phantom_dimensions_.s[0] == 0 && phantom_dimensions_.s[1] == 0 && phantom_dimensions_.s[2] == 0) {
    GGEMSMisc::ThrowException("GGEMSPhantomCreatorManager", "CheckParameters", "Phantom dimensions have to be > 0!!!");
  }

  // Checking size of voxels
  if (GGEMSMisc::IsEqual(element_sizes_.s[0], 0.0) && GGEMSMisc::IsEqual(element_sizes_.s[1], 0.0) && GGEMSMisc::IsEqual(element_sizes_.s[2], 0.0)) {
    GGEMSMisc::ThrowException("GGEMSPhantomCreatorManager", "CheckParameters", "Phantom voxel sizes have to be > 0.0!!!");
    }

  // Checking output name
  if (output_basename_.empty()) {
    GGEMSMisc::ThrowException("GGEMSPhantomCreatorManager", "CheckParameters", "A basename has to be done to phantom manager!!!");
  }

  // Checking the format
  if (format_ != "mhd") {
    GGEMSMisc::ThrowException("GGEMSPhantomCreatorManager", "CheckParameters", "Output format can be only: mhd");
  }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSPhantomCreatorManager::Initialize(void)
{
  GGcout("GGEMSPhantomCreatorManager", "Initialize", 3) << "Initializing phantom creator manager..." << GGendl;

  // Check mandatory parameters
  CheckParameters();

  // Allocation of memory on OpenCL device
  voxelized_phantom_ = opencl_manager_.Allocate(nullptr, number_elements_ * sizeof(GGfloat), CL_MEM_READ_WRITE);

  // Initialize the buffer to zero
  GGfloat* voxelized_phantom = opencl_manager_.GetDeviceBuffer<GGfloat>(voxelized_phantom_, number_elements_ * sizeof(GGfloat));

  for (GGulong i = 0; i < number_elements_; ++i) voxelized_phantom[i] = 0.0;

  // Release the pointers
  opencl_manager_.ReleaseDeviceBuffer(voxelized_phantom_, voxelized_phantom);

  // Computing the phantom offsets
  offsets_.s[0] = phantom_dimensions_.s[0]*element_sizes_.s[0];
  offsets_.s[1] = phantom_dimensions_.s[1]*element_sizes_.s[1];
  offsets_.s[2] = phantom_dimensions_.s[2]*element_sizes_.s[2];

  offsets_.s[0] /= 2.0;
  offsets_.s[1] /= 2.0;
  offsets_.s[2] /= 2.0;

  // Apply volume position
  offsets_.s[0] += isocenter_position_.s[0];
  offsets_.s[1] += isocenter_position_.s[1];
  offsets_.s[2] += isocenter_position_.s[2];
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSPhantomCreatorManager::Write(void) const
{
  // Write output in the correct format
  if (format_ == "mhd") WriteMHDImage();
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSPhantomCreatorManager::WriteMHDImage(void) const
{
  GGcout("GGEMSPhantomCreatorManager", "WriteMHD", 3)
    << "Writing MHD output file..." << GGendl;

  // Write MHD file
  GGEMSMHDImage mhdImage;
  mhdImage.SetBaseName(output_basename_);
  mhdImage.SetDimensions(phantom_dimensions_);
  mhdImage.SetElementSizes(element_sizes_);
  mhdImage.SetOffsets(offsets_);
  mhdImage.Write(voxelized_phantom_);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

GGEMSPhantomCreatorManager* get_instance_phantom_creator_manager(void)
{
  return &GGEMSPhantomCreatorManager::GetInstance();
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void set_phantom_dimension_phantom_creator_manager(GGEMSPhantomCreatorManager* phantom_creator_manager, GGuint const phantom_width, GGuint const phantom_height, GGuint const phantom_depth)
{
  phantom_creator_manager->SetPhantomDimensions(phantom_width, phantom_height, phantom_depth);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void set_element_sizes_phantom_creator_manager(GGEMSPhantomCreatorManager* phantom_creator_manager, GGdouble const voxel_width, GGdouble const voxel_height, GGdouble const voxel_depth)
{
  phantom_creator_manager->SetElementSizes(voxel_width, voxel_height, voxel_depth);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void set_isocenter_positions(GGEMSPhantomCreatorManager* phantom_creator_manager, GGdouble const iso_pos_x, GGdouble const iso_pos_y, GGdouble const iso_pos_z)
{
  phantom_creator_manager->SetIsocenterPositions(iso_pos_x, iso_pos_y, iso_pos_z);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void set_output_basename_phantom_creator_manager(GGEMSPhantomCreatorManager* phantom_creator_manager, char const* output_basename, char const* format)
{
  phantom_creator_manager->SetOutputBasename(output_basename, format);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void initialize_phantom_creator_manager(GGEMSPhantomCreatorManager* phantom_creator_manager)
{
  phantom_creator_manager->Initialize();
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void write_phantom_creator_manager(GGEMSPhantomCreatorManager* phantom_creator_manager)
{
  phantom_creator_manager->Write();
}
