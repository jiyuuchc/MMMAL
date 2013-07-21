/*
 * MMMALLightPath.h
 *
 *  Created on: Jul 13, 2013
 *      Author: Ji-Yu
 */

#ifndef MMMALLIGHTPATH_H_
#define MMMALLIGHTPATH_H_

#include "DeviceBase.h"
#include "MMMALHub.h"

namespace MMMAL
{

   class MMMALLightPath : public CStateDeviceBase<MMMALLightPath>
   {
   public:
      static const char * const DeviceName_;
      static const char * const Description_;

      MMMALLightPath();
      ~MMMALLightPath();

      // API
      int Initialize();
      int Shutdown();
      void GetName(char *pszName) const;
      bool Busy();
      unsigned long GetNumberOfPositions() const;

      // action interface
      int OnState(MM::PropertyBase* pProp, MM::ActionType eAct);

      int StateChanged();

   private:
      bool initialized_;
      MMMALHub* hub_;
   };

}

#endif /* MMMALLIGHTPATH_H_ */
