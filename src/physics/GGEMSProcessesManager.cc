/*!
  \file GGEMSProcessesManager.cc

  \brief GGEMS class managing the processes in GGEMS simulation

  \author Julien BERT <julien.bert@univ-brest.fr>
  \author Didier BENOIT <didier.benoit@inserm.fr>
  \author LaTIM, INSERM - U1101, Brest, FRANCE
  \version 1.0
  \date Monday March 9, 2020
*/

#include "GGEMS/physics/GGEMSProcessesManager.hh"
#include "GGEMS/tools/GGEMSPrint.hh"
#include "GGEMS/tools/GGEMSSystemOfUnits.hh"
#include "GGEMS/global/GGEMSConstants.hh"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

GGEMSProcessesManager::GGEMSProcessesManager(void)
: cross_section_table_number_of_bins_(GGEMSLimit::CROSS_SECTION_TABLE_NUMBER_BINS),
  cross_section_table_min_energy_(GGEMSLimit::CROSS_SECTION_TABLE_ENERGY_MIN),
  cross_section_table_max_energy_(GGEMSLimit::CROSS_SECTION_TABLE_ENERGY_MAX)
{
  GGcout("GGEMSProcessesManager", "GGEMSProcessesManager", 3) << "Allocation of GGEMSProcessesManager..." << GGendl;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

GGEMSProcessesManager::~GGEMSProcessesManager(void)
{
  GGcout("GGEMSProcessesManager", "~GGEMSProcessesManager", 3) << "Deallocation of GGEMSProcessesManager..." << GGendl;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSProcessesManager::SetCrossSectionTableNumberOfBins(GGushort const& number_of_bins)
{
  cross_section_table_number_of_bins_ = number_of_bins;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSProcessesManager::SetCrossSectionTableMinimumEnergy(GGdouble const& energy, char const* unit)
{
  cross_section_table_min_energy_ = GGEMSUnits::BestEnergyUnit(energy, unit);

  // Checking the min value
  if (cross_section_table_min_energy_ < GGEMSLimit::CROSS_SECTION_TABLE_ENERGY_MIN) {
    std::ostringstream oss(std::ostringstream::out);
    oss << "The minimum of energy in the cross section table is 990 eV, yours is " << cross_section_table_min_energy_/GGEMSUnits::eV << " eV!!!";
    GGEMSMisc::ThrowException("GGEMSProcessesManager", "SetCrossSectionTableMinimumEnergy", oss.str());
  }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSProcessesManager::SetCrossSectionTableMaximumEnergy(GGdouble const& energy, char const* unit)
{
  cross_section_table_max_energy_ = GGEMSUnits::BestEnergyUnit(energy, unit);

  // Checking the max value
  if (cross_section_table_max_energy_ > GGEMSLimit::CROSS_SECTION_TABLE_ENERGY_MAX) {
    std::ostringstream oss(std::ostringstream::out);
    oss << "The maximum of energy in the cross section table is 250 MeV, yours is " << cross_section_table_max_energy_/GGEMSUnits::MeV << " MeV!!!";
    GGEMSMisc::ThrowException("GGEMSProcessesManager", "SetCrossSectionTableMaximumEnergy", oss.str());
  }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSProcessesManager::PrintInfos(void) const
{
  GGcout("GGEMSProcessesManager", "PrintInfos", 0) << "Printing infos about processes" << GGendl;
  GGcout("GGEMSProcessesManager", "PrintInfos", 0) << "Number of bins for the cross section table: " << cross_section_table_number_of_bins_ << GGendl;
  GGcout("GGEMSProcessesManager", "PrintInfos", 0) << "Range in energy of cross section table: [" << cross_section_table_min_energy_/GGEMSUnits::keV << ", " << cross_section_table_max_energy_/GGEMSUnits::keV << "] keV" << GGendl;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSProcessesManager::PrintAvailableProcesses(void) const
{
  GGcout("GGEMSProcessesManager", "PrintAvailableProcesses", 0) << "Printing infos about available processes" << GGendl;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

GGEMSProcessesManager* get_instance_processes_manager(void)
{
  return &GGEMSProcessesManager::GetInstance();
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void set_cross_section_table_number_of_bins_processes_manager(GGEMSProcessesManager* processes_manager, GGushort const number_of_bins)
{
  processes_manager->SetCrossSectionTableNumberOfBins(number_of_bins);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void set_cross_section_table_minimum_energy_processes_manager(GGEMSProcessesManager* processes_manager, GGdouble const energy, char const* unit)
{
  processes_manager->SetCrossSectionTableMinimumEnergy(energy, unit);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void set_cross_section_table_maximum_energy_processes_manager(GGEMSProcessesManager* processes_manager, GGdouble const energy, char const* unit)
{
  processes_manager->SetCrossSectionTableMaximumEnergy(energy, unit);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void print_infos_processes_manager(GGEMSProcessesManager* processes_manager)
{
  processes_manager->PrintInfos();
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void print_available_processes_manager(GGEMSProcessesManager* processes_manager)
{
  processes_manager->PrintAvailableProcesses();
}