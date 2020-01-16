/*!
  \file GGEMSXRaySource.cc

  \brief This class define a XRay source in GGEMS useful for CT/CBCT simulation

  \author Julien BERT <julien.bert@univ-brest.fr>
  \author Didier BENOIT <didier.benoit@inserm.fr>
  \author LaTIM, INSERM - U1101, Brest, FRANCE
  \version 1.0
  \date Tuesday October 22, 2019
*/

/*#include <sstream>

#include "GGEMS/sources/GGEMSXRaySource.hh"
#include "GGEMS/tools/GGEMSPrint.hh"
#include "GGEMS/tools/GGEMSTools.hh"
#include "GGEMS/global/GGEMSConstants.hh"
#include "GGEMS/maths/GGEMSGeometryTransformation.hh"
#include "GGEMS/processes/GGEMSParticles.hh"
#include "GGEMS/randoms/GGEMSPseudoRandomGenerator.hh"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

GGEMSXRaySource::GGEMSXRaySource(void)
: GGEMSSourceManager(),
  beam_aperture_(std::numeric_limits<GGfloat>::min()),
  is_monoenergy_mode_(false),
  monoenergy_(-1.0f),
  energy_spectrum_filename_(""),
  number_of_energy_bins_(0),
  p_energy_spectrum_(nullptr),
  p_cdf_(nullptr)
{
  GGcout("GGEMSXRaySource", "GGEMSXRaySource", 3)
    << "Allocation of GGEMSXRaySource..." << GGendl;

  // Initialization of parameters
  focal_spot_size_ = MakeFloat3(
    std::numeric_limits<GGfloat>::min(),
    std::numeric_limits<GGfloat>::min(),
    std::numeric_limits<GGfloat>::min()
  );

  // Initialization of local axis
  p_geometry_transformation_->SetAxisTransformation(
    0.0f, 0.0f, -1.0f,
    0.0f, 1.0f, 0.0f,
    1.0f, 0.0f, 0.0f
  );
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

GGEMSXRaySource::~GGEMSXRaySource(void)
{
  // Freeing the device buffers
  if (p_energy_spectrum_) {
    opencl_manager_.Deallocate(p_energy_spectrum_,
      number_of_energy_bins_ * sizeof(GGdouble));
    p_energy_spectrum_ = nullptr;
  }

  if (p_cdf_) {
    opencl_manager_.Deallocate(p_cdf_,
      number_of_energy_bins_ * sizeof(GGdouble));
    p_cdf_ = nullptr;
  }

  GGcout("GGEMSXRaySource", "~GGEMSXRaySource", 3)
    << "Deallocation of GGEMSXRaySource..." << GGendl;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSXRaySource::InitializeKernel(void)
{
  GGcout("GGEMSXRaySource", "InitializeKernel", 3)
    << "Initializing kernel..." << GGendl;

  // Getting the path to kernel
  std::string const kOpenCLKernelPath = OPENCL_KERNEL_PATH;
  std::string const kFilename = kOpenCLKernelPath
    + "/GetPrimariesGGEMSXRaySource.cl";

  // Compiling the kernel
  p_kernel_get_primaries_ = opencl_manager_.CompileKernel(kFilename,
    "get_primaries_ggems_xray_source");
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSXRaySource::GetPrimaries(GGulong const& number_of_particles)
{
  GGcout("GGEMSXRaySource", "GetPrimaries", 3) << "Generating "
    << number_of_particles << " new particles..." << GGendl;

  // Get command queue and event
  cl::CommandQueue* p_queue = opencl_manager_.GetCommandQueue();
  cl::Event* p_event = opencl_manager_.GetEvent();

  // Get the OpenCL buffers
  cl::Buffer* p_particles = p_particle_->GetPrimaryParticles();
  cl::Buffer* p_randoms = p_pseudo_random_generator_->GetPseudoRandomNumbers();
  cl::Buffer* p_matrix_transformation =
    p_geometry_transformation_->GetTransformationMatrix();

  // Set parameters for kernel
  p_kernel_get_primaries_->setArg(0, *p_particles);
  p_kernel_get_primaries_->setArg(1, *p_randoms);
  p_kernel_get_primaries_->setArg(2, particle_type_);
  p_kernel_get_primaries_->setArg(3, *p_energy_spectrum_);
  p_kernel_get_primaries_->setArg(4, *p_cdf_);
  p_kernel_get_primaries_->setArg(5, number_of_energy_bins_);
  p_kernel_get_primaries_->setArg(6, beam_aperture_);
  p_kernel_get_primaries_->setArg(7, focal_spot_size_);
  p_kernel_get_primaries_->setArg(8, *p_matrix_transformation);

  // Define the number of work-item to launch
  cl::NDRange global(number_of_particles);
  cl::NDRange offset(0);

  // Launching kernel
  cl_int kernel_status = p_queue->enqueueNDRangeKernel(*p_kernel_get_primaries_,
    offset, global, cl::NullRange, nullptr, p_event);
  opencl_manager_.CheckOpenCLError(kernel_status, "GGEMSXRaySource",
    "GetPrimaries");
  p_queue->finish(); // Wait until the kernel status is finish

  // Displaying time in kernel
  opencl_manager_.DisplayElapsedTimeInKernel("GetPrimaries");
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSXRaySource::PrintInfos(void) const
{
  GGcout("GGEMSXRaySource", "PrintInfos", 0) << GGendl;
  GGcout("GGEMSXRaySource", "PrintInfos", 0) << "GGEMSXRaySource Infos:"
    << GGendl;
  GGcout("GGEMSXRaySource", "PrintInfos", 0) << "-----------------" << GGendl;
  GGcout("GGEMSXRaySource", "PrintInfos", 0) << "*Particle type: ";
  if (particle_type_ == GGEMSParticleName::PHOTON) {
    std::cout << "Photon" << std::endl;
  }
  if (particle_type_ == GGEMSParticleName::ELECTRON) {
    std::cout << "Electron" << std::endl;
  }
  GGcout("GGEMSXRaySource", "PrintInfos", 0) << "*Energy mode: ";
  if (is_monoenergy_mode_) std::cout << "Monoenergy" << std::endl;
  else std::cout << "Polyenergy" << std::endl;
  GGcout("GGEMSXRaySource", "PrintInfos", 0) << "*Position: " << "("
    << p_geometry_transformation_->GetPosition().s[0] << ", "
    << p_geometry_transformation_->GetPosition().s[1] << ", "
    << p_geometry_transformation_->GetPosition().s[2] << " ) m3" << GGendl;
  GGcout("GGEMSXRaySource", "PrintInfos", 0) << "*Rotation: " << "("
    << p_geometry_transformation_->GetRotation().s[0] << ", "
    << p_geometry_transformation_->GetRotation().s[1] << ", "
    << p_geometry_transformation_->GetRotation().s[2] << ") degree"
    << GGendl;
  GGcout("GGEMSXRaySource", "PrintInfos", 0) << "*Beam aperture: "
    << beam_aperture_ << " degrees" << GGendl;
  GGcout("GGEMSXRaySource", "PrintInfos", 0) << "*Focal spot size: " << "("
    << focal_spot_size_.s[0] << ", "
    << focal_spot_size_.s[1] << ", "
    << focal_spot_size_.s[2] << ") mm3" << GGendl;
  GGcout("GGEMSXRaySource", "PrintInfos", 0) << "*Local axis: " << GGendl;
  GGcout("GGEMSXRaySource", "PrintInfos", 0) << "[" << GGendl;
  GGcout("GGEMSXRaySource", "PrintInfos", 0) << "    "
    << p_geometry_transformation_->GetLocalAxis().m00_ << " "
    << p_geometry_transformation_->GetLocalAxis().m01_ << " "
    << p_geometry_transformation_->GetLocalAxis().m02_ << GGendl;
  GGcout("GGEMSXRaySource", "PrintInfos", 0) << "    "
    << p_geometry_transformation_->GetLocalAxis().m10_ << " "
    << p_geometry_transformation_->GetLocalAxis().m11_ << " "
    << p_geometry_transformation_->GetLocalAxis().m12_ << GGendl;
  GGcout("GGEMSXRaySource", "PrintInfos", 0) << "    "
    << p_geometry_transformation_->GetLocalAxis().m20_ << " "
    << p_geometry_transformation_->GetLocalAxis().m21_ << " "
    << p_geometry_transformation_->GetLocalAxis().m22_ << GGendl;
  GGcout("GGEMSXRaySource", "PrintInfos", 0) << "]" << GGendl;
  GGcout("GGEMSXRaySource", "PrintInfos", 0) << GGendl;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSXRaySource::SetMonoenergy(GGfloat const& monoenergy)
{
  monoenergy_ = monoenergy;
  is_monoenergy_mode_ = true;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSXRaySource::SetPolyenergy(char const* energy_spectrum_filename)
{
  energy_spectrum_filename_ = energy_spectrum_filename;
  is_monoenergy_mode_ = false;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSXRaySource::CheckParameters(void) const
{
  GGcout("GGEMSXRaySource", "CheckParameters", 3)
    << "Checking the mandatory parameters..." << GGendl;

  // Checking the parameters of Source Manager
  GGEMSSourceManager::CheckParameters();

  // Checking the beam aperture
  if (GGEMSMisc::IsEqual(beam_aperture_, std::numeric_limits<GGfloat>::min())) {
    std::ostringstream oss(std::ostringstream::out);
    oss << "You have to set a beam aperture for the source!!!";
    GGEMSMisc::ThrowException("GGEMSXRaySource", "CheckParameters", oss.str());
  }
  else if (beam_aperture_ < 0.0f) {
    std::ostringstream oss(std::ostringstream::out);
    oss << "The beam aperture must be >= 0!!!";
    GGEMSMisc::ThrowException("GGEMSXRaySource", "CheckParameters", oss.str());
  }

  // Checking the focal spot size
  if (GGEMSMisc::IsEqual(focal_spot_size_.s[0],
    std::numeric_limits<GGfloat>::min()) ||
      GGEMSMisc::IsEqual(focal_spot_size_.s[1],
    std::numeric_limits<GGfloat>::min()) ||
      GGEMSMisc::IsEqual(focal_spot_size_.s[2],
    std::numeric_limits<GGfloat>::min())) {
    std::ostringstream oss(std::ostringstream::out);
    oss << "You have to set a focal spot size!!!";
    GGEMSMisc::ThrowException("GGEMSXRaySource", "CheckParameters", oss.str());
  }

  // Focal spot size must be a positive value
  if (focal_spot_size_.s[0] < 0.0f ||
      focal_spot_size_.s[1] < 0.0f ||
      focal_spot_size_.s[2] < 0.0f) {
    std::ostringstream oss(std::ostringstream::out);
    oss << "The focal spot size is a posivite value!!!";
    GGEMSMisc::ThrowException("GGEMSXRaySource", "CheckParameters", oss.str());
  }

  // Checking the energy
  if (is_monoenergy_mode_) {
    if (GGEMSMisc::IsEqual(monoenergy_, -1.0f)) {
      std::ostringstream oss(std::ostringstream::out);
      oss << "You have to set an energy in monoenergetic mode!!!";
      GGEMSMisc::ThrowException("GGEMSXRaySource", "CheckParameters",
        oss.str());
    }

    if (monoenergy_ < 0.0f) {
      std::ostringstream oss(std::ostringstream::out);
      oss << "The energy must be a positive value!!!";
      GGEMSMisc::ThrowException("GGEMSXRaySource", "CheckParameters",
        oss.str());
    }
  }

  if (!is_monoenergy_mode_) {
    if (energy_spectrum_filename_.empty()) {
      std::ostringstream oss(std::ostringstream::out);
      oss << "You have to provide a energy spectrum file in polyenergy mode!!!";
      GGEMSMisc::ThrowException("GGEMSXRaySource", "CheckParameters",
        oss.str());
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSXRaySource::FillEnergy(void)
{
  GGcout("GGEMSXRaySource", "FillEnergy", 3) << "Filling energy..." << GGendl;

  // Monoenergy mode
  if (is_monoenergy_mode_) {
    number_of_energy_bins_ = 2;

    // Allocation of memory on OpenCL device
    // Energy
    p_energy_spectrum_ = opencl_manager_.Allocate(nullptr, 2 * sizeof(GGdouble),
      CL_MEM_READ_WRITE);

    // Cumulative distribution function
    p_cdf_ = opencl_manager_.Allocate(nullptr, 2 * sizeof(GGdouble),
      CL_MEM_READ_WRITE);

    // Get the energy pointer on OpenCL device
    cl_double* p_energy_spectrum = opencl_manager_.GetDeviceBuffer<GGdouble>(
      p_energy_spectrum_, 2 * sizeof(GGdouble));

    // Get the cdf pointer on OpenCL device
    cl_double* p_cdf = opencl_manager_.GetDeviceBuffer<GGdouble>(p_cdf_,
      2 * sizeof(GGdouble));

    p_energy_spectrum[0] = static_cast<GGdouble>(monoenergy_);
    p_energy_spectrum[1] = static_cast<GGdouble>(monoenergy_);

    p_cdf[0] = 1.0;
    p_cdf[1] = 1.0;

    // Release the pointers
    opencl_manager_.ReleaseDeviceBuffer(p_energy_spectrum_, p_energy_spectrum);
    opencl_manager_.ReleaseDeviceBuffer(p_cdf_, p_cdf);
  }
  else { // Polyenergy mode 
    // Read a first time the spectrum file counting the number of lines
    std::ifstream spectrum_stream(energy_spectrum_filename_, std::ios::in);
    GGEMSFileStream::CheckInputStream(spectrum_stream,
      energy_spectrum_filename_);

    // Compute number of energy bins
    std::string line;
    while (std::getline(spectrum_stream, line)) ++number_of_energy_bins_;

    // Returning to beginning of the file to read it again
    spectrum_stream.clear();
    spectrum_stream.seekg(0, std::ios::beg);

    // Allocation of memory on OpenCL device
    // Energy
    p_energy_spectrum_ = opencl_manager_.Allocate(nullptr,
      number_of_energy_bins_ * sizeof(GGdouble), CL_MEM_READ_WRITE);

    // Cumulative distribution function
    p_cdf_ = opencl_manager_.Allocate(nullptr,
      number_of_energy_bins_ * sizeof(GGdouble), CL_MEM_READ_WRITE);

    // Get the energy pointer on OpenCL device
    GGdouble* p_energy_spectrum = opencl_manager_.GetDeviceBuffer<GGdouble>(
      p_energy_spectrum_, number_of_energy_bins_ * sizeof(GGdouble));

    // Get the cdf pointer on OpenCL device
    GGdouble* p_cdf = opencl_manager_.GetDeviceBuffer<GGdouble>(p_cdf_,
      number_of_energy_bins_ * sizeof(GGdouble));

    // Read the input spectrum and computing the sum for the cdf
    GGint line_index = 0;
    GGdouble sum_cdf = 0.0;
    while (std::getline(spectrum_stream, line)) {
      std::istringstream iss(line);
      iss >> p_energy_spectrum[line_index] >> p_cdf[line_index];
      sum_cdf += p_cdf[line_index];
      ++line_index;
    }

    // Compute CDF and normalized it
    p_cdf[0] /= sum_cdf;
    for (GGuint i = 1; i < number_of_energy_bins_; ++i) {
      p_cdf[i] = p_cdf[i]/sum_cdf + p_cdf[i-1];
    }

    // By security, final value of cdf must be 1 !!!
    p_cdf[number_of_energy_bins_-1] = 1.0;

    // Release the pointers
    opencl_manager_.ReleaseDeviceBuffer(p_energy_spectrum_, p_energy_spectrum);
    opencl_manager_.ReleaseDeviceBuffer(p_cdf_, p_cdf);

    // Closing file
    spectrum_stream.close();
  }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSXRaySource::Initialize(void)
{
  GGcout("GGEMSXRaySource", "Initialize", 3)
    << "Initializing the GGEMS X-Ray source..." << GGendl;

  // Check the mandatory parameters
  CheckParameters();

  // Initializing the kernel for OpenCL
  InitializeKernel();

  // Filling the energy
  FillEnergy();
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSXRaySource::SetBeamAperture(GGfloat const& beam_aperture)
{
  beam_aperture_ = beam_aperture;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSXRaySource::SetFocalSpotSize(GGfloat const& width,
  GGfloat const& height, GGfloat const& depth)
{
  focal_spot_size_ = MakeFloat3(width, height, depth);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

GGEMSXRaySource* create_ggems_xray_source(void)
{
  return GGEMSXRaySource::GetInstance();
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void initialize_ggems_xray_source(GGEMSXRaySource* p_source_manager)
{
  p_source_manager->Initialize();
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void set_position_ggems_xray_source(GGEMSXRaySource* p_source_manager,
  GGfloat const pos_x, GGfloat const pos_y, GGfloat const pos_z)
{
  p_source_manager->SetPosition(pos_x, pos_y, pos_z);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void print_infos_ggems_xray_source(GGEMSXRaySource* p_source_manager)
{
  p_source_manager->PrintInfos();
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void set_source_particle_type_ggems_xray_source(
  GGEMSXRaySource* p_source_manager, char const* particle_name)
{
  p_source_manager->SetSourceParticleType(particle_name);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void set_beam_aperture_ggems_xray_source(GGEMSXRaySource* p_source_manager,
  GGfloat const beam_aperture)
{
  p_source_manager->SetBeamAperture(beam_aperture);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void set_focal_spot_size_ggems_xray_source(GGEMSXRaySource* p_source_manager,
  GGfloat const width, GGfloat const height, GGfloat const depth)
{
  p_source_manager->SetFocalSpotSize(width, height, depth);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void set_local_axis_ggems_xray_source(GGEMSXRaySource* p_source_manager,
  GGfloat const m00, GGfloat const m01, GGfloat const m02,
  GGfloat const m10, GGfloat const m11, GGfloat const m12,
  GGfloat const m20, GGfloat const m21, GGfloat const m22)
{
  p_source_manager->SetLocalAxis(
    m00, m01, m02,
    m10, m11, m12,
    m20, m21, m22
  );
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void set_rotation_ggems_xray_source(GGEMSXRaySource* p_source_manager,
  GGfloat const rx, GGfloat const ry, GGfloat const rz)
{
  p_source_manager->SetRotation(rx, ry, rz);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void update_rotation_ggems_xray_source(GGEMSXRaySource* p_source_manager,
  GGfloat const rx, GGfloat const ry, GGfloat const rz)
{
  p_source_manager->UpdateRotation(rx, ry, rz);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void set_monoenergy_ggems_xray_source(GGEMSXRaySource* p_source_manager,
  GGfloat const monoenergy)
{
  p_source_manager->SetMonoenergy(monoenergy);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void set_polyenergy_ggems_xray_source(GGEMSXRaySource* p_source_manager,
  char const* energy_spectrum)
{
  p_source_manager->SetPolyenergy(energy_spectrum);
}*/
