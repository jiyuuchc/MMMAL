/*
 * MMMALShutter.h
 *
 *  Created on: Jul 13, 2013
 *      Author: Ji-Yu
 */

#ifndef MMMALSHUTTER_H_
#define MMMALSHUTTER_H_

#include <windows.h>
#include "MalMicroscope.h"
#include "DeviceBase.h"
#include "MMMalHub.h"

namespace MMMAL
{

   class MMMALShutter : public CShutterBase<MMMALShutter>
   {
   public:

      static const char * const DeviceNameDIA_;
      static const char * const DeviceNameEPI_;

      MMMALShutter(const char * deviceName);
      ~MMMALShutter();

      //API
      int Initialize() ;
      int Shutdown();
      void GetName(char *pszName) const;
      bool Busy();

      int SetOpen(bool open = true);
      int GetOpen(bool& open);
      int Fire(double interval);

      int StatusChanged();

   private:
      bool initialized_;
      MMMALHub* hub_;

      MAL_IOTARGET channel_;
   };

}

#endif /* MMMALSHUTTER_H_ */
