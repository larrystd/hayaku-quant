#pragma once

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-05-27
 *      Author: fasiondog
 */

#include "OperatorSelector.h"

namespace hayaku {

class OperatorAddSelector : public OperatorSelector {
  OPERATOR_SELECTOR_IMP(OperatorAddSelector, "SE_Add")
  OPERATOR_SELECTOR_SERIALIZATION
};

}  // namespace hayaku

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-05-27
 *      Author: fasiondog
 */

#include "OperatorValueSelector.h"

namespace hayaku {

class OperatorAddValueSelector : public OperatorValueSelector {
  OPERATOR_VALUE_SELECTOR_IMP(OperatorAddValueSelector, "SE_AddValue")
  OPERATOR_VALUE_SELECTOR_SERIALIZATION
};

}  // namespace hayaku

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-05-27
 *      Author: fasiondog
 */

namespace hayaku {

class OperatorDivSelector : public OperatorSelector {
  OPERATOR_SELECTOR_IMP(OperatorDivSelector, "SE_Div")
  OPERATOR_SELECTOR_SERIALIZATION
};

}  // namespace hayaku

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-05-27
 *      Author: fasiondog
 */

namespace hayaku {

class OperatorDivValueSelector : public OperatorValueSelector {
  OPERATOR_VALUE_SELECTOR_IMP(OperatorDivValueSelector, "SE_DivValue")
  OPERATOR_VALUE_SELECTOR_SERIALIZATION
};

}  // namespace hayaku

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-05-27
 *      Author: fasiondog
 */

namespace hayaku {

class OperatorInvertDivValueSelector : public OperatorValueSelector {
  OPERATOR_VALUE_SELECTOR_IMP(OperatorInvertDivValueSelector, "SE_DivValue")
  OPERATOR_VALUE_SELECTOR_SERIALIZATION
};

}  // namespace hayaku

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-05-27
 *      Author: fasiondog
 */

namespace hayaku {

class OperatorInvertSubValueSelector : public OperatorValueSelector {
  OPERATOR_VALUE_SELECTOR_IMP(OperatorInvertSubValueSelector, "SE_SubValue")
  OPERATOR_VALUE_SELECTOR_SERIALIZATION
};

}  // namespace hayaku

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-05-27
 *      Author: fasiondog
 */

namespace hayaku {

class OperatorMulSelector : public OperatorSelector {
  OPERATOR_SELECTOR_IMP(OperatorMulSelector, "SE_Multi")
  OPERATOR_SELECTOR_SERIALIZATION
};

}  // namespace hayaku

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-05-27
 *      Author: fasiondog
 */

namespace hayaku {

class OperatorMulValueSelector : public OperatorValueSelector {
  OPERATOR_VALUE_SELECTOR_IMP(OperatorMulValueSelector, "SE_MultiValue")
  OPERATOR_VALUE_SELECTOR_SERIALIZATION
};

}  // namespace hayaku

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-05-27
 *      Author: fasiondog
 */

namespace hayaku {

class OperatorSubSelector : public OperatorSelector {
  OPERATOR_SELECTOR_IMP(OperatorSubSelector, "SE_Sub")
  OPERATOR_SELECTOR_SERIALIZATION
};

}  // namespace hayaku

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-05-27
 *      Author: fasiondog
 */

namespace hayaku {

class OperatorSubValueSelector : public OperatorValueSelector {
  OPERATOR_VALUE_SELECTOR_IMP(OperatorSubValueSelector, "SE_SubValue")
  OPERATOR_VALUE_SELECTOR_SERIALIZATION
};

}  // namespace hayaku
