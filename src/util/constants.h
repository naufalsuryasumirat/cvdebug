#ifndef UTIL_CONSTANTS_H
#define UTIL_CONSTANTS_H

#include <opencv.hpp>

namespace cns {

const cv::Scalar WHT  {255},
                 BLK  {0};
const cv::Scalar WHITE{255, 255, 255},
                 BLACK{0  , 0  , 0  },
                 BLUE {255, 0  , 0  },
                 GREEN{0  , 255, 0  },
                 RED  {0  , 0  , 255};

}

#endif
