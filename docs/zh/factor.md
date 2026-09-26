# 因子

`Factor` 描述一个具名指标公式，`FactorSet` 将多个公式组合起来计算和比较。
这两个类型由原生 core 导出。因子名与 K 线周期共同标识一个因子；研究结果中应
使用稳定的名称。

```python
from hayaku.core import CLOSE, MA, Factor, FactorSet, Query

ma20 = Factor("MA20", MA(CLOSE(), 20), Query.DAY)
factors = FactorSet([ma20])
```

`Factor` 还记录说明、起始日期、证券板块和复权方式。`FactorSet` 提供
`add`、`remove`、`get_factors`、`get_values` 和 `get_all_values`。
计算前需要配置数据 Session 及本地行情数据，参见[快速入门](quickstart.rst)。

数据库持久化与需授权的外部适配器有独立的生命周期和文档，不属于 core
研究入门。上面的示例只在内存中建立因子公式，不执行数据库写入。
