/*
 * MMMALLamp.h
 *
 *  Created on: Jul 19, 2013
 *      Author: Ji-Yu
 */

#ifndef MMMALLAMP_H_
#define MMMALLAMP_H_

#include "DeviceBase.h"
#include "MMMALHub.h"

namespace MMMAL
{

   class MMMALLamp : public CGenericBase<MMMALLamp>
   {
   public:

      static const char * const DeviceName_;
      static const char * const Description_;
      static const char * const Keyword_Voltage_;

      MMMALLamp();
      ~MMMALLamp();

      // MMDevice API
      int Initialize();
      int Shutdown();
      void GetName(char *pszName) const;
      bool Busy();

      int OnVoltage(MM::PropertyBase* pProp, MM::ActionType eAct);

      int VoltageChanged();

   private:
      bool initialized_;
      MMMALHub* hub_;
   };

}

#endif /* MMMALLAMP_H_ */
