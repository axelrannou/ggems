// GGEMS Copyright (C) 2015

#ifndef PARTICLES_CUH
#define PARTICLES_CUH

#include "global.cuh"

// Stack of particles, format data is defined as SoA
struct ParticleStack{
    // property
    f64* E;
    f64* dx;
    f64* dy;
    f64* dz;
    f64* px;
    f64* py;
    f64* pz;
    f64* tof;
    // PRNG
    ui32* prng_state_1;
    ui32* prng_state_2;
    ui32* prng_state_3;
    ui32* prng_state_4;
    ui32* prng_state_5;
    // Navigation
    ui32* geometry_id; // current geometry crossed by the particle
    // simulation
    ui8* endsimu;
    ui8* level;
    ui8* pname; // particle name (photon, electron, etc)
    // stack size
    ui32 size;
}; //

// Helper to handle secondaries particles
struct SecParticle {
    f32xyz dir;
    f32 E;
    ui8 pname;
    ui8 endsimu;
};

// Helper to handle history of particles
struct OneParticleStep {
    f32xyz pos;
    f32xyz dir;
    f32 E;
};

// History class
class HistoryBuilder {
    public:
        HistoryBuilder();

        void cpu_new_particle_track(ui32 a_pname);
        void cpu_record_a_step(ParticleStack particles, ui32 id_part);

        std::vector<ui8> pname;
        std::vector<ui32> nb_steps;
        std::vector< std::vector<OneParticleStep> > history_data;

        ui8 record_flag;      // Record or not
        ui32 max_nb_particles;  // Max nb of particles keep in the history
        ui32 cur_iter;          // Current number of iterations
        ui32 stack_size;        // Size fo the particle stack

    private:
        ui32 current_particle_id;
        ui8 type_of_particles;
};


// Particles class
class ParticleBuilder {
    public:
        ParticleBuilder();
        void set_stack_size(ui32 nb);
        void set_seed(ui32 val_seed);
        void cpu_malloc_stack();
        void cpu_free_stack();
        void gpu_malloc_stack();
        void cpu_init_stack_seed();
        void copy_seed_cpu2gpu();
        void cpu_print_stack(ui32 nlim);

        ParticleStack stack, dstack; // CPU and GPU stack
        ui32 seed;

    private:


};

#endif
