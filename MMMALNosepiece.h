/*
 * MMMALNosepiece.h
 *
 *  Created on: Jul 13, 2013
 *      Author: Ji-Yu
 */

#ifndef MMMALNOSEPIECE_H_
#define MMMALNOSEPIECE_H_

#include "DeviceBase.h"
#include "MMMALHub.h"

namespace MMMAL
{

   class MMMALNosepiece : public CStateDeviceBase<MMMALNosepiece>
   {
   public:
      static const char * const DeviceName_;
      static const char * const Description_;

      MMMALNosepiece();
      ~MMMALNosepiece();

      // API
      int Initialize();
      int Shutdown();
      void GetName(char *pszName) const;
      bool Busy();
      unsigned long GetNumberOfPositions() const;

      // action interface
      int OnState(MM::PropertyBase* pProp, MM::ActionType eAct);

   private:
      bool initialized_;
      MMMALHub* hub_;

   };

}

#endif /* MMMALNOSEPIECE_H_ */
