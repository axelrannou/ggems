/*!
  \file GGEMSTools.cc

  \brief Namespaces for different useful fonctions

  \author Julien BERT <julien.bert@univ-brest.fr>
  \author Didier BENOIT <didier.benoit@inserm.fr>
  \author LaTIM, Brest, FRANCE
  \version 1.0
  \date Wednesday October 10, 2018
*/

#include <sstream>
#include <cerrno>
#include <cstring>
#include <cmath>
#include <iostream>

#include "GGEMS/tools/GGEMSTools.hh"
#include "GGEMS/tools/GGEMSPrint.hh"

#include "GGEMS/global/GGEMSManager.hh"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSFileStream::CheckInputStream(std::ifstream const& input_stream,
  std::string const& filename)
{
  if (!input_stream) {
    std::ostringstream oss(std::ostringstream::out);
    #ifdef _WIN32
    char buffer_error[ 256 ];
    oss << "Problem reading filename '" << filename << "': "
        << strerror_s(buffer_error, 256, errno);
    #else
    oss << "Problem reading filename '" << filename << "': "
        << strerror(errno);
    #endif
    throw std::runtime_error(oss.str());
  }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/// @cond
template GGbool GGEMSMisc::IsEqual<GGdouble>(GGdouble const&, GGdouble const&);
template GGbool GGEMSMisc::IsEqual<float>(GGfloat const&, GGfloat const&);
/// @endcond

template <typename T>
GGbool GGEMSMisc::IsEqual(T const& a, T const& b)
{
  return std::nextafter(a, std::numeric_limits<T>::lowest()) <= b
    && std::nextafter(a, std::numeric_limits<T>::max()) >= b;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSMisc::ThrowException(std::string const& class_name,
  std::string const& method_name, std::string const& message)
{
  std::ostringstream oss(std::ostringstream::out);
  oss << message;
  GGcerr(class_name, method_name, 0) << oss.str() << GGendl;
  throw std::runtime_error("");
}
