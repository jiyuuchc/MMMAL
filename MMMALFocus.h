/*
 * MMMALFocus.h
 *
 *  Created on: Jul 13, 2013
 *      Author: Ji-Yu
 */

#ifndef MMMALFOCUS_H_
#define MMMALFOCUS_H_

#include "DeviceBase.h"
#include "MMMALHub.h"

namespace MMMAL
{
   class MMMALFocus : public CStageBase <MMMALFocus>
   {
   public:

      static const char * const DeviceName_;
      static const char * const Description_;
      static const char * const Keyword_Position_;
      static const char * const Keyword_NearLimit_;
      static const char * const Keyword_FarLimit_;
	  static const char * const Keyword_Sensitivity_;

      MMMALFocus();
      ~MMMALFocus();

      //API
      bool Busy();
      void GetName(char *pszName) const;

      int Initialize();
      int Shutdown();

      virtual int SetOrigin();
      virtual double GetStepSize();
      int SetPositionUm(double pos);
      int GetPositionUm(double& pos);
      int SetPositionSteps(long steps);
      int GetPositionSteps(long& steps);
      virtual int GetLimits(double& lower, double& upper);

      // action interface
      int OnPosition(MM::PropertyBase* pProp, MM::ActionType eAct);
      int OnNearLimit(MM::PropertyBase* pProp, MM::ActionType eAct);
      int OnFarLimit(MM::PropertyBase* pProp, MM::ActionType eAct);
      int OnSensitivity(MM::PropertyBase* pProp, MM::ActionType eAct);

      int IsStageSequenceable(bool& seq) const;
      bool IsContinuousFocusDrive() const;

      int PositionChanged();

   private:
      bool initialized_;
      MMMALHub* hub_;
      long origin_;
      const double stepSize_;
   };
}

#endif /* MMMALFOCUS_H_ */
