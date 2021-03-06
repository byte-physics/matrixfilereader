/*
  Forward Declarations of all XOP classes
*/
#pragma once
#include <boost/smart_ptr.hpp>

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
