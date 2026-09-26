#include "Bindings.h"

namespace py = pybind11;

void export_Indicator_ta_lib(py::module_& m);
void export_global_main(py::module_& m);

void bindExtensions(py::module_& m) {
  export_Indicator_ta_lib(m);
  export_global_main(m);
}
