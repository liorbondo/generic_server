/**********************************************************
Author: 			 			Lior Bondorevsky
Creation date: 			23/11/2014
Last modofied date: 23/11/2014
Description: 				Singleton API & Implementation
NOTES:              NON Multi Threading safe version!!
**********************************************************/
#ifndef __SINGLETON_H__
#define __SINGLETON_H__

#include <cstdlib> // atexit

namespace adv_cpp
{

template <typename T>
class Singleton
{
public:
  static T& Instance();

private:
  Singleton();
 ~Singleton();
  static void Destroy();
  
  static T*    m_data; 
};

template <typename T>
T* Singleton<T>::m_data = NULL;

template <typename T>
T& Singleton<T>::Instance()
{
  // non MT safe version 
  if(!m_data)
  {
    m_data = new T;
    atexit(Destroy);   
  }

  return *m_data;
}

template<typename T>
void Singleton<T>::Destroy()
{
	delete m_data;
}

} // namespace adv_cpp


#endif // #ifndef __SINGLETON_H__
