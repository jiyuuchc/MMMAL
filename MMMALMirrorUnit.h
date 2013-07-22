/*
 * MMMALMirrorUnit.h
 *
 *  Created on: Jul 13, 2013
 *      Author: Ji-Yu
 */

#ifndef MMMALMIRRORUNIT_H_
#define MMMALMIRRORUNIT_H_

#include "DeviceBase.h"
#include "MMMALHub.h"

namespace MMMAL
{

   class MMMALMirrorUnit : public CStateDeviceBase<MMMALMirrorUnit>
   {

   public:

      static const char * const DeviceName_;
      static const char * const Description_;
      static const char * const Keyword_N_Positions_;

      MMMALMirrorUnit();
      ~MMMALMirrorUnit();

      // MMDevice API
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

#endif /* MMMALMIRRORUNIT_H_ */
