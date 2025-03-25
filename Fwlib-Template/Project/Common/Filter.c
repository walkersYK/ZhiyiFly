#include "Filter.h"


struct KalmanFilter EKF[3] = {
				{0.02, 0, 0, 0, 0.001, 0.543},
        {0.02, 0, 0, 0, 0.001, 0.543},
        {0.02, 0, 0, 0, 0.001, 0.543}
			};
	
//Ò»Î¬¿¨¶ûÂüÂË²¨
void kalmanfiter(struct KalmanFilter *EKF,float input)
{
	EKF->NewP = EKF->LastP + EKF->Q;
    EKF->Kg = EKF->NewP / (EKF->NewP + EKF->R);
    EKF->Out = EKF->Out + EKF->Kg * (input - EKF->Out);
    EKF->LastP = (1-EKF->Kg) * EKF->NewP ;
}