/*
 * MMMALAutoFocus.h
 *
 *  Created on: Jul 12, 2013
 *      Author: Ji-Yu
 */

#ifndef MMMALAUTOFOCUS_H_
#define MMMALAUTOFOCUS_H_

#include "MMDevice.h"
#include "MMDeviceConstants.h"
#include "DeviceBase.h"
#include "MMMALHub.h"

namespace MMMAL 
{
   class MMMALAutofocus : public CAutoFocusBase<MMMALAutofocus>
   {
   public:
      static const char * const DeviceName_;
      static const char * const Description_;
      static const char * const Keyword_AFSearchRange_;
      static const char * const Keyword_AFOffset_;
      static const char * const Keyword_AFStatus_;
      static const char * const Keyword_ContinuousFocus_;

      MMMALAutofocus();
      ~MMMALAutofocus();

      //API
      bool Busy();
      void GetName(char *pszName) const;

      int Initialize();
      int Shutdown();
      // AutoFocus API
      virtual int SetContinuousFocusing(bool state);
      virtual int GetContinuousFocusing(bool& state);
      virtual bool IsContinuousFocusLocked();
      virtual int FullFocus();
      virtual int IncrementalFocus();
      virtual int GetLastFocusScore(double& score);
      virtual int GetCurrentFocusScore(double& score);
      virtual int AutoSetParameters();
      virtual int GetOffset(double &offset);
      virtual int SetOffset(double offset);

      virtual int AFStatusChanged();
      virtual int AFOffsetChanged();

   protected:
      int OnContinuousFocus(MM::PropertyBase* pProp, MM::ActionType eAct);
      int OnAutoFocusOffset(MM::PropertyBase* pProp, MM::ActionType eAct);
      int OnAutoFocusSearchRange(MM::PropertyBase* pProp, MM::ActionType eAct);
      int OnAutoFocusStatus(MM::PropertyBase* pProp, MM::ActionType eAct);

   private:
      bool initialized_;
      MMMALHub* hub_;
      bool continuousFocusing_;

      void GetStatusString(char * buf);
   };
}
#endif /* MMMALAUTOFOCUS_H_ */
