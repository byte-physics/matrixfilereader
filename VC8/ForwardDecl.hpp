/*
  Forward Declarations of all XOP classes
*/
#pragma once
#include <boost/smart_ptr.hpp>
#include <boost/function/function_fwd.hpp>

class GlobalData;
class ExtremaData;
class Bricklet;
typedef boost::shared_ptr<Bricklet> BrickletPtr;
class DLLHandler;
class Wave;

// Forward Declare Vernissage classes also
namespace Vernissage
{
  class Session;
} // namespace Vernissage

namespace Bstrlib
{
  struct CBString;
}

typedef std::pair<Bstrlib::CBString, Bstrlib::CBString> StringPair;
typedef std::vector<StringPair> StringPairVector;
typedef StringPairVector::iterator StringPairVectorIt;
typedef StringPairVector::const_iterator StringPairVectorCIt;
