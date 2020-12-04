#ifndef GUARD_GGEMS_GEOMETRIES_GGEMSSOLID_HH
#define GUARD_GGEMS_GEOMETRIES_GGEMSSOLID_HH

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
  \file GGEMSSolid.hh

  \brief GGEMS class for solid. This class store geometry about phantom or detector

  \author Julien BERT <julien.bert@univ-brest.fr>
  \author Didier BENOIT <didier.benoit@inserm.fr>
  \author LaTIM, INSERM - U1101, Brest, FRANCE
  \version 1.0
  \date Tuesday March 2, 2020
*/

#include <limits>
#include <algorithm>

#include "GGEMS/io/GGEMSTextReader.hh"
#include "GGEMS/io/GGEMSHitCollection.hh"
#include "GGEMS/tools/GGEMSRAMManager.hh"
#include "GGEMS/navigators/GGEMSNavigatorManager.hh"

class GGEMSGeometryTransformation;

/*!
  \class GGEMSSolid
  \brief GGEMS class for solid informations
*/
class GGEMS_EXPORT GGEMSSolid
{
  public:
    /*!
      \brief GGEMSSolid constructor
    */
    GGEMSSolid(void);

    /*!
      \brief GGEMSSolid destructor
    */
    ~GGEMSSolid(void);

    /*!
      \fn GGEMSSolid(GGEMSSolid const& solid) = delete
      \param solid - reference on the GGEMS solid
      \brief Avoid copy by reference
    */
    GGEMSSolid(GGEMSSolid const& solid) = delete;

    /*!
      \fn GGEMSSolid& operator=(GGEMSSolid const& solid) = delete
      \param solid - reference on the GGEMS solid
      \brief Avoid assignement by reference
    */
    GGEMSSolid& operator=(GGEMSSolid const& solid) = delete;

    /*!
      \fn GGEMSSolid(GGEMSSolid const&& solid) = delete
      \param solid - rvalue reference on the GGEMS solid
      \brief Avoid copy by rvalue reference
    */
    GGEMSSolid(GGEMSSolid const&& solid) = delete;

    /*!
      \fn GGEMSSolid& operator=(GGEMSSolid const&& solid) = delete
      \param solid - rvalue reference on the GGEMS solid
      \brief Avoid copy by rvalue reference
    */
    GGEMSSolid& operator=(GGEMSSolid const&& solid) = delete;

    /*!
      \fn void EnableTracking(void)
      \brief Enabling tracking infos during simulation
    */
    void EnableTracking(void);

    /*!
      \fn inline cl::Buffer* GetSolidData(void) const
      \brief get the informations about the solid geometry
      \return header data OpenCL pointer about solid
    */
    inline cl::Buffer* GetSolidData(void) const {return solid_data_cl_.get();};

    /*!
      \fn void ParticleSolidDistance(void)
      \brief compute distance from particle position to solid and store this distance in OpenCL particle buffer
    */
    void ParticleSolidDistance(void);

    /*!
      \fn void ProjectToSolid(void)
      \brief Projector particles to entry of solid
    */
    void ProjectToSolid(void);

    /*!
      \fn void TrackThroughSolid(std::weak_ptr<GGEMSCrossSections> cross_sections, std::weak_ptr<GGEMSMaterials> materials)
      \param cross_sections - pointer storing cross sections values
      \param materials - pointer storing materials values
      \brief Track particles through solid
    */
    virtual void TrackThroughSolid(std::weak_ptr<GGEMSCrossSections> cross_sections, std::weak_ptr<GGEMSMaterials> materials) = 0;

    /*!
      \fn void SetRotation(GGfloat3 const& rotation_xyz)
      \param rotation_xyz - rotation in X, Y and Z
      \brief set a rotation for solid
    */
    void SetRotation(GGfloat3 const& rotation_xyz);

    /*!
      \fn void SetPosition(GGfloat3 const& position_xyz)
      \param position_xyz - position in X, Y and Z
      \brief set a position for solid
    */
    void SetPosition(GGfloat3 const& position_xyz);

    /*!
      \fn void SetSolidID(std::size_t const& solid_id)
      \param solid_id - index of the solid
      \brief set the global solid index
    */
    template<typename T>
    void SetSolidID(std::size_t const& solid_id);

    /*!
      \fn void GetTransformationMatrix(void)
      \brief Get the transformation matrix for solid object
    */
    virtual void GetTransformationMatrix(void) = 0;

    /*!
      \fn void Initialize(std::weak_ptr<GGEMSMaterials> materials)
      \param materials - pointer on GGEMS materials
      \brief Initialize solid for geometric navigation
    */
    virtual void Initialize(std::weak_ptr<GGEMSMaterials> materials) = 0;

    /*!
      \fn void PrintInfos(void) const
      \brief printing infos about solid
    */
    virtual void PrintInfos(void) const = 0;

    /*
      \fn inline DurationNano GetKernelParticleSolidDistanceTimer(void) const
      \return elapsed time in particle solid distance kernel
      \brief get the elapsed time in particle solid distance kernel
    */
    inline DurationNano GetKernelParticleSolidDistanceTimer(void) const {return kernel_particle_solid_distance_timer_;}

    /*
      \fn inline DurationNano GetKernelProjectToSolidTimer(void) const
      \return elapsed time in kernel computing projection to closest solid
      \brief get the elapsed time in kernel computing projection to closest solid
    */
    inline DurationNano GetKernelProjectToSolidTimer(void) const {return kernel_project_to_solid_timer_;}

    /*
      \fn inline DurationNano GetKernelTrackThroughSolidTimer(void) const
      \return elapsed time in kernel tracking particle through closest solid
      \brief get the elapsed time in kernel tracking particle through closest solid
    */
    inline DurationNano GetKernelTrackThroughSolidTimer(void) const {return kernel_track_through_solid_timer_;}

  protected:
    /*!
      \fn void InitializeKernel(void)
      \brief Initialize kernel for particle solid distance
    */
    virtual void InitializeKernel(void) = 0;

  protected:
    std::shared_ptr<cl::Buffer> solid_data_cl_; /*!< Data about solid */
    std::shared_ptr<cl::Buffer> label_data_cl_; /*!< Pointer storing the buffer about label data, useful for voxelized solid only */

    std::weak_ptr<cl::Kernel> kernel_particle_solid_distance_cl_; /*!< OpenCL kernel computing distance between particles and solid */
    DurationNano kernel_particle_solid_distance_timer_; /*!< Timer for kernel computing particle solid distance */

    std::weak_ptr<cl::Kernel> kernel_project_to_solid_cl_; /*!< OpenCL kernel moving particles to solid */
    DurationNano kernel_project_to_solid_timer_; /*!< Timer for kernel computing projection to closest solid */

    std::weak_ptr<cl::Kernel> kernel_track_through_solid_cl_; /*!< OpenCL kernel tracking particles through a solid */
    DurationNano kernel_track_through_solid_timer_; /*!< Timer for kernel computing tracking through closest solid */

    std::string kernel_option_; /*!< Preprocessor option for kernel */

    std::unique_ptr<GGEMSGeometryTransformation> geometry_transformation_; /*!< Pointer storing the geometry transformation */

    std::string data_reg_type_; /*!< Type of registering data */
    GGEMSHitCollection hit_; /*!< Storing hit counting during navigation process */
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

template<typename T>
void GGEMSSolid::SetSolidID(std::size_t const& solid_id)
{
  // Get the OpenCL manager
  GGEMSOpenCLManager& opencl_manager = GGEMSOpenCLManager::GetInstance();

  // Get pointer on OpenCL device
  T* solid_data_device = opencl_manager.GetDeviceBuffer<T>(solid_data_cl_.get(), sizeof(T));

  solid_data_device->solid_id_ = static_cast<GGint>(solid_id);

  // Release the pointer
  opencl_manager.ReleaseDeviceBuffer(solid_data_cl_.get(), solid_data_device);
}

#endif // End of GUARD_GGEMS_GEOMETRIES_GGEMSSOLID_HH
