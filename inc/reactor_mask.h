/**************************************
Author:             Lior Bondorevsky
Creation date:      10/11/2014
Last modofied date: 12/11/2014
Description:        Reactor Mask
**************************************/
#ifndef __REACTOR_MASK__
#define __REACTOR_MASK__

namespace adv_cpp
{

enum Mask
{
  READ        = 1,
  WRITE       = 2,
  OUT_OF_BAND = 4 
};  

} // namespace adv_cpp

#endif // ifndef __REACTOR_MASK__
