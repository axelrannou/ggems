// GGEMS Copyright (C) 2017

/*!
 * \file linac_source.cuh
 * \brief Linac source
 * \author J. Bert <bert.jul@gmail.com>
 * \version 0.2
 * \date Thursday September 1st, 2016
 *
 * v0.2: JB - Change all structs and remove CPU exec
 *
 */

#ifndef LINAC_SOURCE_CUH
#define LINAC_SOURCE_CUH

#include "global.cuh"
#include "particles.cuh"
#include "ggems_source.cuh"
#include "prng.cuh"
#include "vector.cuh"
#include "fun.cuh"
#include "txt_reader.cuh"

struct LinacSourceData
{
    f32 *cdf_rho;
    f32 *cdf_rho_E;
    f32 *cdf_rho_E_theta;
    f32 *cdf_rho_theta_phi;

    f32 rho_max;
    f32 E_max;
    f32 theta_max;
    f32 phi_max;

    f32 s_rho;
    f32xy s_rho_E;
    f32xyz s_rho_E_theta;
    f32xyz s_rho_theta_phi;

    ui32 n_rho;
    ui32xy n_rho_E;
    ui32xyz n_rho_E_theta;
    ui32xyz n_rho_theta_phi;
};



class GGEMSource;

/*!
  \class LinacSource
  \brief This class is a source that simulate a Linac based on model.
*/
class LinacSource : public GGEMSSource
{
public:
    /*!
     * \brief LinacSource contructor
     */
    LinacSource();

    /*!
     * \brief LinacSource destructor
     */
    ~LinacSource();

    /*!
     * \fn void set_frame_axis( f32 m00, f32 m01, f32 m02, f32 m10, f32 m11, f32 m12, f32 m20, f32 m21, f32 m22 )
     * \brief Set the axis transformation of the beam compare to the global frame
     * \param m00 Element of the matrix
     * \param m01 Element of the matrix
     * \param m02 Element of the matrix
     * \param m10 Element of the matrix
     * \param m11 Element of the matrix
     * \param m12 Element of the matrix
     * \param m20 Element of the matrix
     * \param m21 Element of the matrix
     * \param m22 Element of the matrix
     */
    void set_frame_axis( f32 m00, f32 m01, f32 m02,
                         f32 m10, f32 m11, f32 m12,
                         f32 m20, f32 m21, f32 m22 );

    /*!
     * \fn void set_frame_position( f32 posx, f32 posy, f32 posz )
     * \brief Set the global origin position of the linac source (from the isocenter)
     * \param posx Position of the origin in X
     * \param posy Position of the origin in Y
     * \param posz Position of the origin in Z
     */
    void set_frame_position( f32 posx, f32 posy, f32 posz );

    /*!
     * \fn void set_rotation( f32 agx, f32 agy, f32 agz )
     * \brief Set the orientation of the beam
     * \param agx Angle along x-axis (non-coplanar angle)
     * \param agy Angle along y-axis (Carousel rotation)
     * \param agz Angle along z-axis (Gantry angle)
     */
    void set_frame_rotation( f32 agx, f32 agy, f32 agz );

    /*!
     * \brief set_model_filename( std::string filename )
     * \param filename Filename of the Linac source model
     */
    void set_model_filename( std::string filename );


public:
    f32matrix44 get_transformation_matrix();    

public: // Abstract from GGEMSSource (Mandatory funtions)

    /*!
     * \fn void get_primaries_generator( Particles particles )
     * \brief Generate particles
     * \param particles Stack of particles
     */
    void get_primaries_generator( ParticlesData *d_particles );

    /*!
     * \brief Initialize the source before running the simualtion
     * \param params Simulations parameters
     */
    void initialize( GlobalSimulationParametersData *h_params );

private: // Make LinacSource class non-copyable
    /*!
    \brief Copy constructor not implement and impossible to use for the user by security
    */
    LinacSource( LinacSource const& );

    /*!
    \brief Copy assignment not implement and impossible to use for the user by security
    */
    LinacSource& operator=( LinacSource const& );

    /*!
     * \fn void m_load_linac_model()
     * \brief Function that reads and loads a linac model into the GPU memory
     */
    void m_load_linac_model();

    /*!
     * \brief Copy LINAC model data to the GPU
     */
    void m_copy_to_gpu();

private:

    f32matrix33 m_axis_trans;             /*!< Axis transformation matrix */
    f32xyz m_angle;                       /*!< Orientation of the beamlet */
    f32xyz m_org;                         /*!< Position of Linac source frame */
    std::string m_model_filename;         /*!< filename of the Linac source model */

    f32matrix44 m_transform;              /*!< Transformation matrix */
    GlobalSimulationParametersData *mh_params;  /*!< Simulation parameters */

    LinacSourceData *h_linac_source;      /*!< Linac source model host data */
    LinacSourceData *d_linac_source;      /*!< Linac source model device data */
};

#endif


