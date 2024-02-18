#include "screenSaverFactory.h"
#include "blinds/blinds.h"
#include "bubbles/bubbles.h"
#include "lattice/lattice.h"
//#include "stripe/stripe.h"

CScreenSaver* createScreenSaver(CScreenSaver::TYPE_T type) {

    switch (type) {
        case CScreenSaver::TYPE_BLINDS: { /* 百叶窗 */
            return new CBlinds();
        }
        break;
        case CScreenSaver::TYPE_BUBBLE: { /* 气泡 */
            return new CBubbles();
        }
        break;
        case CScreenSaver::TYPE_LATTICE: { /* 格子 */
            return new CLattice();
        }
        break;
        /*case CScreenSaver::TYPE_STRIPE: { / * 条纹 * /
            return new CStripe();
        }
        break;*/
        default: {
            return nullptr;
        }
        break;
    }

    return nullptr;
}
