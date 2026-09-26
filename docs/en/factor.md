# Factors

`Factor` describes one named indicator formula. `FactorSet` groups formulas for
calculation and comparison. These types are exported by the native core. A
factor's name and K-line period identify it; choose a stable name before using
it in research results.

```python
from hayaku.core import CLOSE, MA, Factor, FactorSet, Query

ma20 = Factor("MA20", MA(CLOSE(), 20), Query.DAY)
factors = FactorSet([ma20])
```

`Factor` also records a description, date range, security block and price
adjustment mode. `FactorSet` offers `add`, `remove`, `get_factors`, `get_values`
and `get_all_values`. Evaluation needs a configured data session and securities
with local market data; see the [quickstart](quickstart.rst).

Database persistence and licensed external adapters have their own lifecycle
and documentation outside the core research guide. The core example above
creates factor formulas in memory and does not perform a database write.
